#include <script.h>
#include <gui/desktop.h>

using namespace os;
using namespace os::gui;
using namespace os::math;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;


void printf(char*);
uint8_t* memset(uint8_t*, int, size_t);
uint16_t hash(char* str);


Desktop* LoadDesktopForTask(bool set, Desktop* desktop = 0);
TaskManager* LoadTaskManager(bool set, TaskManager* tm = 0);
//void sleep(uint32_t)

os::CommandLine* LoadScriptForTask(bool set, os::CommandLine* cli = 0);


Script::Script(CommandLine* cli) {

	this->parentCli = cli;
	this->appType = APP_TYPE_SCRIPT;
}

Script::~Script() {}

void Script::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {
}
void Script::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {
}


//allow command line to allocate new window again
void Script::Close() { 
	
	this->parentCli->userWindow = nullptr; 
	this->parentCli->targetWindow = false;
}


void Script::SaveOutput(char* fileName, CompositeWidget* widget) {
}
void Script::ReadInput(char* fileName, CompositeWidget* widget) {
}


void Script::OnKeyDown(char ch, CompositeWidget* widget) {

	this->parentCli->Key = ch;
}

void Script::OnKeyUp(char ch, CompositeWidget* widget) {
}


void Script::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {

	this->parentCli->MouseX = x;
	this->parentCli->MouseY = y;

	//widget->Dragging = true;
	switch (button) {
	
		case 1:
			break;
		default:
			break;
	}
}


void Script::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
}

void Script::OnMouseMove(int32_t oldx, int32_t oldy, 
		    int32_t newx, int32_t newy, CompositeWidget* widget) {
}



//function for script multitasking
//for each terminal
void UserScript() {

	//TaskManager* tm = LoadTaskManager(false);
	CommandLine* cli = LoadScriptForTask(false);
	TaskManager* tm = cli->tm;
	cli->PrintCommand("\n");
	AyumuScriptCli(cli->scriptName, cli);

	cli->userTask->kill = true;

	//infinite loop here to stall the task and 
	//give the scheduler enough time to remove it
	while (1) {}
}


//intepreter that parses through file for 
//commands and executes them as they come
void AyumuScriptCli(char* name, os::CommandLine* cli, uint8_t* scriptBuf, uint32_t scriptSize) {
	
	cli->scriptRunning = true;

	//issue with buff here
	uint32_t size = scriptSize;
	uint8_t* file = scriptBuf;


	//get file data
	//load from disk if buffer not provided
	if (scriptBuf == nullptr) {
	
		size = cli->filesystem->GetFileSize(argparse(name, 0));
		file = (uint8_t*)cli->mm->malloc(sizeof(uint8_t)*size);
		memset(file, 0x00, size);
	
		for (int i = 0; i < (size/OFS_BLOCK_SIZE); i++) {
	
			uint8_t data[OFS_BLOCK_SIZE];
			cli->filesystem->ReadLBA(argparse(name, 0), data, i);

			for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
		
				file[j+(i*OFS_BLOCK_SIZE)] = data[j];
			}
		}
	}

	//file index
	uint32_t indexF = 0;
	
	//line index
	uint16_t indexL = 0;

	//store individual command
	char line[256];
	bool start = false;
	bool startLoop = false;
	
	//number of loops allowed (16)
	uint16_t nestedLoop[16];
	bool nestedLoopCondition[16];
	uint32_t fileIndexLoop = 0;
	
	for (uint8_t i = 0; i < 16; i++) {

		nestedLoop[i] = 0; 
		nestedLoopCondition[i] = false;
	}
	//loop indexes
	uint8_t indexLoopF = 0;



	while (indexF < size) {

		//terminate script
		if (cli->scriptKillSwitch) { 
			
			cli->mm->free(file);
			CleanUpScript(cli);
			return;	
		}

		//parse and interpret
		switch (file[indexF]) {

			case '\n':
			case '\v':
			case 0x00:
				if (!start) { break; }

				line[indexL] = '\0';
				indexL = 0;
				
				//loop logic
				if (strcmp("loop ", line)) {

					//add for every new loop command
					if (indexLoopF < 16) {

						nestedLoop[indexLoopF] = indexF - ((strlen(line) + 1));
						indexLoopF++;
					}
					startLoop = true;

				} else if (strcmp("pool", line) && indexLoopF > 0) {

					//outer pool checks inner loop condition instead of outer
					if (nestedLoopCondition[indexLoopF-1] == true) {

						//go back to start of loop if
						//condition is still true
						indexF -= (indexF - nestedLoop[indexLoopF-1]);
						indexF = nestedLoop[indexLoopF-1];
					} else {
						//remove loop index and
						//proceed with program
						nestedLoop[indexLoopF-1] = 0;
						nestedLoopCondition[indexLoopF-1] = false;
					}
					indexLoopF -= (1 * (indexLoopF > 0));
				
				//init function definition
				} else if (strcmp("function ", line)) {
				
					cli->setStartIndex = indexF;
				
				} else if (strcmp("end", line)) {
				
					if (cli->callStack->lastNode != nullptr && cli->callStack->numOfNodes > 0) {
						
						//return to index according to call stack
						indexF = ((CallStackUnit*)cli->callStack->lastNode->value)->returnIndex;
						cli->callStack->Pop();
					}
				} else {
				}

				//executing a program with functions for the 3rd time in the same 
				//terminal loops and function is not recognized as variable???

				
				//execute as command or
				//function has been called
				if (cli->command(line, strlen(line)) == nullptr) {
				
					//push function call on stack 
					//to execute and return later
					FunctionType* func = getVarFunction(line, cli, 0);
					CallStackUnit* call = createCall(func, cli, indexF);
					cli->callStack->Push(call);
		
					indexF = func->funcStartIndex;
				}

				
				if (startLoop && indexLoopF > 0) {
				
					nestedLoopCondition[indexLoopF-1] = cli->conditionLoop;
					startLoop = false;
				}
				start = false;
				break;
			default:
				line[indexL] = file[indexF];
				indexL++;
				start = true;
				break;
		}
		indexF++;
	}
	cli->mm->free(file);
	CleanUpScript(cli);
}

void CleanUpScript(os::CommandLine* cli) {
	
	//reset conditions and script arguments
	cli->setStartIndex = 0;
	cli->setReturnIndex = 0;
	cli->conditionIf = true;
	cli->conditionLoop = true;
	for (int i = 0; i < 256; i++) { cli->scriptInput[i] = '\0'; }
	
	for (int i = 0; i < ARG_TABLE_SIZE; i++) { 
		
		if (cli->argTable[i] != nullptr) {
		
			cli->mm->free(cli->argTable[i]);
		}
		cli->argTable[i] = nullptr;
	}

	//reset variables
	for (int i = 0; i < VAR_TABLE_SIZE; i++) {
	
		Type* var = cli->varTable[i];

		if (var != nullptr) {
		
			switch (var->type) {
			
				case TYPE_STRING:
					//cli->mm->free(var->typeStr);
					break;
				case TYPE_FUNCTION:
					if (var->typeFunc->buf != nullptr) {
						
						//cli->mm->free(var->typeFunc->buf);
					}
					if (var->typeFunc->args != nullptr) {
					
						var->typeFunc->args->DestroyList();
						//cli->mm->free(var->typeFunc->args);
					}
					//cli->mm->free(var->typeFunc);
					break;
			}
			//cli->mm->free(var);
			//cli->varTable[i] = nullptr;
		}
	}

	cli->callStack->DestroyList();
	
	cli->scriptKillSwitch = false;
	cli->scriptRunning = false;
}


void AyumuScriptFindDegree(uint8_t* file, uint32_t size, List* degreeVals) {

	uint32_t indexF = 0;
	uint16_t indexL = 0;
	char line[256];
	bool start = false;

	uint8_t currentIfCount = 0;
	uint8_t currentLoopCount = 0;
	uint8_t currentFunctionCount = 0;
	
	uint64_t currentDegree = 0;

	while (indexF < size) {

		switch (file[indexF]) {
			
			case '\n':
			case '\v':
			case 0x00:
				if (!start) { break; }
				
				line[indexL] = '\0';
				indexL = 0;
				
				
				//max of 21 nested functions/loops/ifs for degree
				for (int i = 0; i < currentFunctionCount; i++) { currentDegree ^= (0b100 << ((i*4)+1)); }
				for (int i = 0; i < currentLoopCount; i++) { 	 currentDegree ^= (0b10 << ((i*4)+1)); }
				for (int i = 0; i < currentIfCount; i++) { 	 currentDegree ^= (0b1 << ((i*4)+1)); }

				//add degree val
				degreeVals->AddInt(currentDegree);
				
				if (strcmp("function ", line)) { 				currentFunctionCount++;
				} else if (strcmp("end", line) && currentFunctionCount > 0) {	currentFunctionCount--;
				} else if (strcmp("loop ", line)) {				currentLoopCount++;
				} else if (strcmp("pool", line) && currentLoopCount > 0) {	currentLoopCount--;
				} else if (strcmp("if ", line)) {				currentIfCount++;
				} else if (strcmp("fi", line) && currentIfCount > 0) { 		currentIfCount--;
				} else {
					if (findCharInStr('$', line)) {
					
						currentDegree ^= 1; 
					}
				}

				start = false;
				break;
			default:
				line[indexL] = file[indexF];
				indexL++;
				start = true;
				break;
		}
		indexF++;
	}
}



//execute script
void ex(char* args, CommandLine* cli) {

	char* file = argparse(args, 0);
	uint32_t fileSector = cli->filesystem->GetFileSector(file);

	if (cli->filesystem->FileIf(fileSector)) {

		//parse script arguments
		for (int i = 0; i < ARG_TABLE_SIZE; i++) {
		
			//cli->argTable[i] = str2int(argparse(args, i+1));
			cli->argTable[i] = evalVar(args, cli, i+1);
		}

		AyumuScriptCli(args, cli);
		return;
	}
	cli->PrintCommand("Script file wasn't found.\n");
}



void ext(char* args, CommandLine* cli) {
	
	char* file = argparse(args, 0);
	uint32_t priority = numOrVar(args, cli, 1) % 256;
	uint32_t fileSector = cli->filesystem->GetFileSector(file);
	int i = 0;
	
	if (cli->filesystem->FileIf(fileSector)) {

		//parse script arguments
		for (i = 0; i < ARG_TABLE_SIZE; i++) {
		
			//cli->argTable[i] = str2int(argparse(args, i+1));
			cli->argTable[i] = evalVar(args, cli, i+1);
		}	
		
		//load and create task
		//passing this given cli
		//while script creates its own
		//cli to use for commands
		LoadScriptForTask(true, cli);
	
		//prepare file name to share with
		//a bunch of other bullshit	
		for (i = 0; args[i] != '\0' && args[i] != ' '; i++) {
		
			cli->scriptName[i] = args[i];
		}
		cli->scriptName[i] = '\0';
	
		//add task finally
		cli->tm->taskPriority[cli->tm->numTasks-1] = priority;
		cli->DeleteTaskForScript();
		cli->CreateTaskForScript(file);
		return;
	}
	cli->PrintCommand("Script file wasn't found.\n");
}


void inputStr(char* args, CommandLine* cli) {

	cli->getCurrentInput = true;

	//pass this off to OnKeyDown of CommandLine
	while (cli->getCurrentInput == true) {
	}
}


void setReturn(char* args, CommandLine* cli) {

	//int32_t returnVal = numOrVar();
}


void function(char* args, CommandLine* cli) {

	//stop executing during 
	//function declaration
	//
	//the "end" command is the 
	//same as the "fi" command
	//which just allows normal
	//command execution
	if (cli->scriptRunning == false) {
		
		cli->PrintCommand("Functions can only be defined in scripts.\n");
		return;
	}
	
	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;
	Type* oldVar = cli->varTable[hashVar];

	if (oldVar != nullptr) {

		if (oldVar->type == TYPE_FUNCTION) {
		
			oldVar->typeFunc->args->DestroyList();
			cli->mm->free(oldVar->typeFunc->args);
			cli->mm->free(oldVar->typeFunc);
			cli->mm->free(oldVar);
		} else {
			hashVar++;
		}
	}

	uint32_t functionIndex = cli->setStartIndex;
	uint8_t numOfArgs = argcount(args)-1;

	//create function
	FunctionType* func = (FunctionType*)cli->mm->malloc(sizeof(FunctionType));
	new (func) FunctionType(cli->scriptName, functionIndex, numOfArgs);
	func->args = (List*)cli->mm->malloc(sizeof(List));
	new (func->args) List(cli->mm);


	//create variable containing function
  	Type* newVar = (Type*)cli->mm->malloc(sizeof(Type));
  	new (newVar) Type(TYPE_FUNCTION, (void*)func);
	
	//add as variable
	cli->varTable[hashVar] = newVar;
	cli->conditionIf = false;
}


void setFunctionArgs(char* args, CommandLine* cli, FunctionType* func) {
	
	uint8_t argNum = argcount(args)+1;	

	for (int i = 0; i < argNum; i++) {

		Type* var = evalVar(args, cli, i);	
		func->args->Push(var);
	}
}


CallStackUnit* createCall(FunctionType* func, CommandLine* cli, uint32_t returnIndex) {

	CallStackUnit* call = (CallStackUnit*)cli->mm->malloc(sizeof(CallStackUnit));
	call->returnIndex = returnIndex;
	call->startIndex = func->funcStartIndex;
	call->args = func->args;

	return call;
}



int32_t evalExpr(char* args, CommandLine* cli) {

	//set up inner parentheses string

	uint8_t parIndex = 0;
	uint8_t parCount = 0;
	bool outerMostParFound = false;
	bool parDone = false;

	char recursiveStr[strlen(args)];
	recursiveStr[0] = '\0';
	uint8_t newReStrIndex = 0;

	for (int i = 0; args[i] != '\0' && parDone == false; i++) {

		if (outerMostParFound) {
		
			recursiveStr[newReStrIndex] = args[i];
			newReStrIndex++;
		}

		switch (args[i]) {
		
			case '(':
				if (!outerMostParFound) { parIndex = i; }
				outerMostParFound = true;
				parCount++;
				break;
			case ')':
				parCount -= (1 * outerMostParFound);
				
				if (parCount == 0 && newReStrIndex > 0) {
					
					recursiveStr[--newReStrIndex] = '\0';
					parDone = true;
				}
				break;
		}
	}

	/*	
	printf("string is ");
	printf(recursiveStr);
	printf(" from ");
	printf(args);
	printf("\n");
	*/

	//if parentheses expression 
	//needs to be evaluated first
	if (recursiveStr[0] != '\0') {
		
		char* evalStr = recursiveStr;

		//we do this string manipulation first so known 
		//parenthesis are replaced before we check for more
		do {	
			newReStrIndex = strlen(evalStr);
			int32_t evalParExpr = evalExpr(evalStr, cli);
			char* resultStr = int2str(evalParExpr);

			//shift string chars to make room for str form of result
			if (newReStrIndex < strlen(resultStr)) {
		
				uint16_t maxResultStrLength = strlen(resultStr);
				uint16_t oldStrLen = strlen(args);

				for (int i = oldStrLen; i > parIndex+newReStrIndex; i--) {
			
					args[i+maxResultStrLength] = args[i];
					args[i] = ' ';
				}
				args[oldStrLen+maxResultStrLength+1] = '\0';
			}

			//put new str in there to eval
			for (int i = 0; i < newReStrIndex+2; i++) {
		
				args[parIndex+i] = ' ';

				if (i < strlen(resultStr)) {
				
					args[parIndex+i] = resultStr[i];
				}
			}
		
			evalStr = args;

			//this checks for other parenthesis in the same level
		} while (findCharInStr('(', args));
	}




	/*
	//assume space seperation if you have to
	char* operatorStr = "+-/*%<>=&|^";
	char str[128];
	memset((uint8_t*)str, '\0', 128);
	int prevOperatorCharIndex = 0;

	for (int i = pIndex+1; i < pLength-1; i++) {

		//seperate operands and operators with spaces 
		//for argparse and argcount functions
		if (findCharInStr(args[i], operatorStr) && i > 0) {
		
			//extra spaces are ok
			bool addLeftSpace = (args[i-1] != ' ');
			bool addRightSpace = (args[i+1] != ' ');
			
			if (addLeftSpace) { str[i-(pIndex+1)] = ' '; }
			
			//get operator char
			str[i-(pIndex+1)+addLeftSpace] = args[i];

			//get possible 2nd operator char
			if (findCharInStr(args[i+1], operatorStr)) {
			
				str[i-(pIndex+1)+addLeftSpace+1] = args[i+1];
				args[i+1] = ' ';
			
				if (addRightSpace) { str[i-(pIndex+1)+addLeftSpace+2] = ' '; }
			} else {
				if (addRightSpace) { str[i-(pIndex+1)+addLeftSpace+1] = ' '; }
			}
		} else {
			str[i-(pIndex+1)] = args[i];
		}
	}
	*/

	char* str = args;
	
	/*
	printf("space sep done\n");
	printf("string is ");
	printf(str);
	printf("\n");
	*/

	//sort ops here
	int32_t operandStack[16];
	int32_t operatorStack[16];
	int operandIndex = 0;
	int operatorIndex = 0;


	for (int i = 0; i < argcount(str); i++) {
	
		//operands
		if (i % 2 == 0) {
			//evaluate each operand
			operandStack[operandIndex] = numOrVar(str, cli, i);
			operandIndex++;
		//operators
		} else {
			uint8_t operatorVal = 0;

			switch (argparse(str, i)[0]) {
		
				case '+':operatorVal = OPERATOR_ADD;break;
				case '-':operatorVal = OPERATOR_SUB;break;
				case '*':operatorVal = OPERATOR_MUL;break;
				case '/':operatorVal = OPERATOR_DIV;break;
				case '%':operatorVal = OPERATOR_MOD;break;

				case '=':operatorVal = OPERATOR_EQUAL;break;
				case '&':operatorVal = OPERATOR_AND;break;
				case '|':operatorVal = OPERATOR_OR;break;
			
				case '<':
					 operatorVal = OPERATOR_LESS;
				 
				 	if (argparse(str, i)[1] == '=') {
				 
						operatorVal = OPERATOR_LESS_EQUAL; 
				 	}
				case '>':
					 operatorVal = OPERATOR_MORE;
				 
				 	if (argparse(str, i)[1] == '=') {
				 
						operatorVal = OPERATOR_MORE_EQUAL; 
				 	}
				 	break;
			
				default:operatorVal = 0;break;
			}
			operatorStack[operatorIndex] = operatorVal;
			operatorIndex++;
		}
	}

	//sort for order of operations
	//printf("ops sorted\n");

	//check if single value and early return
	if (operandIndex == 1 && operatorIndex == 0) {
	
		return operandStack[0];
	}


	/*
	for (int i = 0; i < operatorIndex; i++) { printf(int2str(operatorStack[i])); }
	printf("\n");
	for (int i = 0; i < operandIndex; i++) { printf(int2str(operandStack[i])); }
	printf("\n");
	*/

	//compute as tree
	int32_t result = 0;
	int32_t op1 = operandStack[operandIndex-1];
	int32_t op2 = operandStack[operandIndex-2];
	operandIndex -= 2;
	
	
	while (operatorIndex >= 0 && operandIndex >= 0) {

	
		switch (operatorStack[operatorIndex-1]) {

			case OPERATOR_ADD: result = op1 + op2; break;
			case OPERATOR_SUB: result = op2 - op1; break;
			case OPERATOR_MUL: result = op1 * op2; break;
			case OPERATOR_DIV: 
					   if (op1) { result = op2 / op1; }
					   //if (op2) { result = op1 / op2; }
					   else { result = 0; }
					   break;
			case OPERATOR_MOD:
					   if (op1) { result = op2 % op1; }
					   //if (op2) { result = op1 % op2; }
					   else { result = 0; }
					   break;
			//boolean expr
			case OPERATOR_EQUAL: 	 result = (1 * (op1 == op2)); break;
			case OPERATOR_AND:   	 result = (1 * (op1 && op2)); break;
			case OPERATOR_OR:    	 result = (1 * (op1 || op2)); break;
			case OPERATOR_LESS:  	 result = (1 * (op1 < op2));  break;
			case OPERATOR_LESS_EQUAL:result = (1 * (op1 <= op2)); break;
			case OPERATOR_MORE:  	 result = (1 * (op1 > op2));  break;
			case OPERATOR_MORE_EQUAL:result = (1 * (op1 >= op2)); break;
			
			default:result = 0;break;
		}

		//return result of expression
		if (operatorIndex == 0) { return result; }

		operatorIndex--;

		op1 = operandStack[operandIndex-1];
		op2 = result;
		
		operandIndex--;
	}
	return result;
}


uint8_t determineType(char* arg, CommandLine* cli) {

	if (arg[0] == '\"' && arg[strlen(arg)-1] == '\"') {

		return LITERAL_STRING;

	} else if (findCharInStr('.', arg)) {
	
		return LITERAL_FLOAT;
	
	} else if (arg[0] >= '0' && arg[0] <= '9') {
		
		return LITERAL_INT;
	} else {
		return TYPE_NULL;
	}
}


Type* evalVar(char* args, CommandLine* cli, uint8_t argNum) {
				
	char* varInitStr = (char*)cli->mm->malloc(sizeof(char)*strlen(args));
	strcpy(varInitStr, args);
	uint16_t varLength = strlen(varInitStr);
	
	for (int i = 0; i < argNum; i++) { argshift(varInitStr, varLength); }
	varLength = strlen(varInitStr);
	
	Type* var = cli->varTable[hash(varInitStr) % VAR_TABLE_SIZE];
	
	//type and val of the variable
	uint8_t type = 0;
	void* val = nullptr;
	
	if (var == nullptr) {

		//check if string literal
		if (varInitStr[0] == '\"' && varInitStr[varLength-1] == '\"') {
		
			type = TYPE_STRING;
			char* initStr = (char*)cli->mm->malloc(sizeof(char)*(varLength-1));
		
			for (int i = 0; i < varLength-2; i++) {
		
				initStr[i] = varInitStr[i+1];
			}
			initStr[varLength-2] = '\0';
			val = (void*)strOrVar(initStr, cli, -1);
	
		//check if float
		} else if (findCharInStr('.', varInitStr)) {

			type = TYPE_FLOAT;
			float valFloat = floatOrVar(args, cli, argNum);
			val = (void*)(&valFloat);
	
		//else just treat as int
		} else {
			type = TYPE_INT;
			int32_t valInt = evalExpr(varInitStr, cli);
			val = (void*)(&valInt);
		}

	//copy var from given
	} else {
		var->type = type;
	
		switch (type) {
		
			case TYPE_INT: 	var->typeInt    = *((int32_t*)val);	break;
			case TYPE_FLOAT:var->typeFloat  = *((float*)val);	break;
			case TYPE_STRING:var->typeStr 	= ((char*)val);		break;
			case TYPE_FUNCTION:var->typeFunc = ((FunctionType*)val);break;
		}	
		return var; 
	}
	cli->mm->free(varInitStr);
	
	//create and assign variable	
	Type* newVar = (Type*)cli->mm->malloc(sizeof(Type));
	new (newVar) Type(type, val);

	return newVar;
}


int32_t getVarNum(char* args, CommandLine* cli, uint8_t argNum) {

	return cli->varTable[hash(argparse(args, argNum)) % VAR_TABLE_SIZE]->typeInt;
}

FunctionType* getVarFunction(char* args, CommandLine* cli, uint8_t argNum) {
	
	return cli->varTable[hash(argparse(args, argNum)) % VAR_TABLE_SIZE]->typeFunc;
}



int32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum) {
	
	char* name = argparse(args, argNum);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;
	
	if (cli->varTable[hashVar] != nullptr) { 
			
		return cli->varTable[hashVar]->typeInt;
		
		/*
		//execute function for some return value
		if (cli->varTable[hashVar]->type == TYPE_FUNCTION) {
		
			//cli->command(name, strlen(name));
		} else {
			return cli->varTable[hashVar]->typeInt;
		}
		*/
	
	//input stream for int
	} else if (strcmp("$input", name)) {

		return str2int(cli->scriptInput);

	} else if (name[0] == '$' && name[2] == '\0') {

		//given arguments to script
		if (name[1] <= '9' && name[1] >= '0') { return cli->argTable[name[1]-'0']->typeInt; }

		//value returned from most recent command
		if (name[1] == 'R') { return cli->returnVar->typeInt; }
	
	//function arguments
	} else if (name[0] == '$' && name[1] == 'F' && name[2] >= '0' && name[3] == '\0' && cli->scriptRunning) {

		Type* val = (Type*)(((CallStackUnit*)cli->callStack->lastNode->value)->args->Read((uint8_t)(name[2]-'0')));
		return val->typeInt;
	//chars
	} else if (name[0] == '@' && name[2] == '\0') { return (uint8_t)(name[1]);
	
	//mouse/keyboard input	
	} else if (strcmp("$KEY_CHAR", name)) { return (uint8_t)(cli->Key);
	} else if (strcmp("$KEY_PRESS", name)) { return cli->userWindow->keypress; 
	} else if (strcmp("$LEFT_CLICK", name)) { return cli->userWindow->mouseclick; 
	} else if (strcmp("$MOUSE_X", name)) { return cli->MouseX;
	} else if (strcmp("$MOUSE_Y", name)) { return cli->MouseY;
	
	//str2int
	} else { return str2int(name); }
}

float floatOrVar(char* args, CommandLine* cli, uint8_t argNum) {
	
	char* name = argparse(args, argNum);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;
	
	if (cli->varTable[hashVar] != nullptr) {
	
		return cli->varTable[hashVar]->typeFloat;
	
	} else if (name[0] == '$' && name[1] == 'F' && name[2] >= '0' && cli->scriptRunning) {
		
		Type* val = (Type*)(((CallStackUnit*)cli->callStack->lastNode->value)->args->Read((uint8_t)(name[2]-'0')));
		return val->typeFloat;
	
	} else { return str2float(name); }
}

char* strOrVar(char* args, CommandLine* cli, int8_t argNum) {

	char* name = args;
	
	if (argNum >= 0) {
	
		name = argparse(args, argNum);
	}
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;
	
	if (cli->varTable[hashVar] != nullptr) {
	
		return cli->varTable[hashVar]->typeStr;
	
	} else if (strcmp("$input", name)) {
	
		return cli->scriptInput;
	
	} else if (name[0] == '$' && name[1] == 'F' && name[2] >= '0' && cli->scriptRunning) {
		
		Type* val = (Type*)(((CallStackUnit*)cli->callStack->lastNode->value)->args->Read((uint8_t)(name[2]-'0')));
		return val->typeStr;
	} else {
		return name;
	}
}


//compute math expressions
void evalMath(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;

	//if (false) {
	if (cli->varTable[hashVar] != nullptr) {
	
		for (int i = 0; i < arg1len(args); i++) { args[i] = ' '; }
		cli->varTable[hashVar]->typeInt = evalExpr(args, cli);
		cli->varTable[hashVar]->type = TYPE_INT;
	} else {
		cli->PrintCommand(int2str(evalExpr(args, cli)));
		cli->PrintCommand("\n");
	}
	
}


//create variable
void var(char* args, CommandLine* cli) {

	if (argcount(args) < 2) { return; }

	char* name = argparse(args, 0);

	if (((uint8_t)(name[0] - '0') < 10) || (name[0] == '$') || (name[0] == '@')) {
		
		char* foo = " "; 
		foo[0] = name[0];
	
		cli->PrintCommand("error: name cannot begin with '");
		cli->PrintCommand(foo);
		cli->PrintCommand("'.\n");
		return;
	}

	Type* type = evalVar(args, cli, 1);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;
	cli->varTable[hashVar] = type;
}


//destroy raw int from table
void dvar(char* args, CommandLine* cli) {

	int32_t index = getVarNum(args, cli, 0);

	if (index >= VAR_TABLE_SIZE) {
	
		cli->PrintCommand("int error: index is greater than size of table (VAR_TABLE_SIZE).\n");
		return;
	}
	
	if (cli->varTable[index] != nullptr) {
	
		if (cli->varTable[index]->typeStr != nullptr) {
		
			cli->mm->free(cli->varTable[index]->typeStr);
		}
		cli->mm->free(cli->varTable[index]);
	}	
	cli->varTable[index] = nullptr;
}


//create list
void varList(char* args, CommandLine* cli) {
	
	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;

	if (((uint8_t)(name[0] - '0') < 10) || (name[0] == '$') || (name[0] == '@')) {
		
		char* foo = " "; 
		foo[0] = name[0];
	
		cli->PrintCommand("error: name cannot begin with '");
		cli->PrintCommand(foo);
		cli->PrintCommand("'.\n");
		return;
	}

	cli->varTable[hashVar]->typeInt = cli->lists->numOfNodes;
	
	List* list = (List*)(cli->mm->malloc(sizeof(List)));
	new (list) List(cli->mm);
	cli->lists->Push(list);
}

//print each element in list
void printList(char* args, CommandLine* cli) {

	uint32_t varNum = getVarNum(args, cli, 0);

	if (varNum >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}

	List* list = (List*)(cli->lists->Read(varNum));	
	Node* node = list->entryNode;

	for (int i = 0; i < list->numOfNodes; i++) {

		Type* var = (Type*)(node->value);

		switch (var->type) {
		
			case TYPE_INT:
				cli->PrintCommand(int2str(var->typeInt));
				break;
			case TYPE_STRING:
				cli->PrintCommand(var->typeStr);
				break;
			case TYPE_FLOAT:
				cli->PrintCommand(float2str(var->typeFloat));
				break;
			case TYPE_FUNCTION:
				cli->PrintCommand("FUNC @");
				cli->PrintCommand(int2str(var->typeFunc->funcStartIndex));
				break;
		}
		cli->PrintCommand(",");

		node = node->next;
	}
	cli->PrintCommand("\n");
}


//add to list
void insertList(char* args, CommandLine* cli) {

	uint32_t varNum = getVarNum(args, cli, 0);
	
	if (varNum >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(varNum));	

	Type* var = evalVar(args, cli, 1);
	uint32_t index = numOrVar(args, cli, 2);

	if (argcount(args) < 3) { list->Push(var);
	} else { list->Insert(var, index); }
}


//remove from list
void removeList(char* args, CommandLine* cli) {

	uint32_t varNum = getVarNum(args, cli, 0);
	
	if (varNum >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}

	List* list = (List*)(cli->lists->Read(varNum));
	uint32_t index = numOrVar(args, cli, 1);

	if (argcount(args) < 2) { list->Pop(); }
	else { list->Remove(index); }
}


//read from list index and save into variable
void readList(char* args, CommandLine* cli) {

	uint32_t varNum = getVarNum(args, cli, 0);
	
	if (varNum >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(varNum));

	uint32_t index = numOrVar(args, cli, 1);
	uint32_t* value = (uint32_t*)(list->Read(index));

	//save to variable
	char* varName = argparse(args, 2);
	uint16_t readHashVar = hash(varName) % VAR_TABLE_SIZE;
	
	if (argcount(args) > 2) { cli->varTable[readHashVar]->typeInt = *value; }
	cli->returnVar->typeInt = *value;
}

//write to preexisting node
void writeList(char* args, CommandLine* cli) {

	uint32_t varNum = getVarNum(args, cli, 0);
	
	if (varNum >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(varNum));
	
	uint32_t index = numOrVar(args, cli, 1);
	uint32_t value = numOrVar(args, cli, 2);
	
	uint32_t* newNum = (uint32_t*)(cli->mm->malloc(sizeof(uint32_t)));
	new (newNum) uint32_t;
	*newNum = value;
	
	list->Write(newNum, index);
}



//destroy list
void destroyList(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % VAR_TABLE_SIZE;
	
	if (cli->varTable[hashVar]->typeInt >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}

	//if you remove list that was not most recently
	//allocated, lists index in hash table will be fucked	
	uint32_t index = cli->varTable[hashVar]->typeInt;	
	List* list = (List*)(cli->lists->Read(index));
	list->DestroyList();
	cli->lists->Remove(index);
	
	cli->mm->free(cli->varTable[hashVar]);
	cli->varTable[hashVar] = nullptr;
}


//comments
void comment(char* args, CommandLine* cli) {
}


//conditionals
/*
bool trueOrFalse(char* op, int32_t arg1, int32_t arg2, CommandLine* cli) {

	bool result = false;

	switch (op[0]) {
	
		case '=': result = (arg1 == arg2); break;
		case '!': result = (arg1 != arg2); break;
		case '|': result = (arg1 || arg2); break;
		case '&': result = (arg1 && arg2); break;
		case '>':
			if (op[1] == '=') { result = (arg1 >= arg2);
			} else { 	    result = (arg1 > arg2); }
			break;
		case '<':
			if (op[1] == '=') { result = (arg1 <= arg2);
			} else {	    result = (arg1 < arg2); }
			break;
		default:
			cli->PrintCommand("condition error: use correct syntax (e.g. 'if x < y')\n");
			result = true;
			break;
	}
	return result;
}
*/

// if and loop statements

void If(char* args, CommandLine* cli) {

	cli->conditionIf = (evalExpr(args, cli) > 0);

	/*
	int32_t arg1 = numOrVar(args, cli, 0);
	
	//not enough args, just eval first arg
	if (argcount(args) < 3) {
	
		cli->conditionIf = (arg1 > 0);
		return;
	}
	
	int32_t arg2 = numOrVar(args, cli, 2);
	char* op = argparse(args, 1);

	cli->conditionIf = trueOrFalse(op, arg1, arg2, cli);
	*/
}
void Else(char* args, CommandLine* cli) { cli->conditionIf ^= 1; }
void Fi(char* args, CommandLine* cli) { cli->conditionIf = true; }



void loop(char* args, CommandLine* cli) {

	cli->conditionLoop = (evalExpr(args, cli) > 0);
	/*
	int32_t arg1 = numOrVar(args, cli, 0);
	
	//not enough args, just eval first arg
	if (argcount(args) < 3) {
	
		cli->conditionLoop = (arg1 > 0);
		return;
	}
	
	int32_t arg2 = numOrVar(args, cli, 2);
	char* op = argparse(args, 1);
	cli->conditionLoop = trueOrFalse(op, arg1, arg2, cli);
	*/
}
void pool(char* args, CommandLine* cli) { cli->conditionLoop = true; }
