#include <cli.h>
#include <art.h>
#include <filesys/ofs.h>
#include <script.h>
#include <drivers/speaker.h>


using namespace os;
using namespace os::drivers;
using namespace os::common;
using namespace os::filesystem;

//kernel.cc functions used for everything
void putchar(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void printfTUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void printf(char*);
void printOsaka(uint8_t);


uint16_t strlen(char* args);
uint32_t str2int(char* args);
char* int2str(uint32_t num);
char* argparse(char*, uint8_t);
uint8_t argcount(char*);


void makeBeep(uint32_t freq);
void explodeMain();

uint32_t prng();

void WmDisk(uint32_t, char*);
void RmDisk(uint32_t, uint16_t);
uint32_t FileList();

void sleep(uint32_t);

void memWrite(uint32_t, uint32_t);
uint32_t memRead(uint32_t);


uint32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum);





uint16_t hash(char* cmd) {

	uint32_t val = fnv1a(cmd);
	return (val >> 16) ^ (val & 0xffff);
}



void say(char* args, CommandLine* cli) {

	printf(args);
	printf("\n");
}



void andagi(char* args, CommandLine* cli) {

	printf("sata andagi\n");
}



void test(char* args, CommandLine* cli) {

	printf(int2str(argcount(args)));
	printf("\n");
}



void delay(char* args, CommandLine* cli) {

	uint32_t repeat = numOrVar(args, cli, 0);
	sleep(repeat);
}

void wmem(char* args, CommandLine* cli) {
		
	uint32_t mem = numOrVar(args, cli, 0);
	uint32_t val = numOrVar(args, cli, 1);
	
	memWrite(mem, val);
	
	printf("Wrote ");
	printf(int2str(val));	
	printf(" to memory location ");
	printf(int2str(mem));	
	printf(".\n");
}

void rmem(char* args, CommandLine* cli) {

	uint32_t mem = numOrVar(args, cli, 0);
	uint32_t val = memRead(mem);
	
	printf("Reading from ");
	printf(int2str(mem));	
	printf(": ");
	printf(int2str(val));
	printf(".\n");

	cli->returnVal = val;
}

	


void explode(char* args, CommandLine* cli) {

	explodeMain();
}


void wdisk(char* args, CommandLine* cli) {

	uint32_t sector = numOrVar(args, cli, 0);

	//shift string
	for (int i = 0; args[i] != '\0'; i++) {
		args[i] = args[i+5];
	}
	
	WmDisk(sector, args);
}


void rdisk(char* args, CommandLine* cli) {

	uint32_t sector = numOrVar(args, cli, 0);
	uint32_t size = numOrVar(args, cli, 1);

	RmDisk(sector, size);
}


void files(char* args, CommandLine* cli) {

	uint32_t fileNum = FileList();
	char* num = int2str(fileNum);
	
	printf("\n");
	printf(num);
	printf(" files have been allocated.\n");
	
	cli->returnVal = fileNum;
}


void size(char* args, CommandLine* cli) {
		
	uint32_t size = GetFileSize(args);

	if (size) {
	
		printf("'");
		printf(args);
		printf("' is ");
		printf(int2str(size));
		printf(" bytes large.\n");
	} else {
	
		printf("How can you measure the size of something that doesn't exist?\n");
	}

	cli->returnVal = size;
}



void deleteFile(char* args, CommandLine* cli) {

	if (DeleteFile(args)) {
	
		printf(args);
		printf(" was deleted.\n");
	} else {
		printf(args);
		printf(" isn't a file dumbass.\n");
	}
}







//********************************************************AyumuScript*****************************************************************

uint32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum) {
	
	char* name = argparse(args, argNum);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] != 0xffffffff) {
	
		return cli->varTable[hashVar];

	} else if (name[0] == '$' && name[1] == 'R' && name[2] == '\0') {

		return cli->returnVal;
	} else { 
		return str2int(name);
	}
}



void varInt(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);

	if ((uint8_t)(name[0] - '0') < 10) {
	
		printf("int error: name cannot begin with number.\n");
		return;
	}

	if (name[0] == '$') {
	
		printf("int error: name cannot begin with '$'.\n");
		return;
	}


	uint32_t value = numOrVar(args, cli, 1);
	uint16_t hashVar = hash(name) % 1024;
	
	cli->varTable[hashVar] = value;	
}



//comments
void comment(char* args, CommandLine* cli) {
}



//conditionals

bool trueOrFalse(char* op, uint32_t arg1, uint32_t arg2) {

	bool result = false;	

	switch (op[0]) {
	
		case '=':	
			result = (arg1 == arg2);
			break;
		case '|':
			result = (arg1 || arg2);
			break;
		case '&':
			result = (arg1 && arg2);
			break;
		
		case '>':
			if (op[1] == '=') {

				result = (arg1 >= arg2);
			} else { 
				result = (arg1 > arg2);
			}
			break;
		case '<':
			if (op[1] == '=') {

				result = (arg1 <= arg2);
			} else { 
				result = (arg1 < arg2);
			}
			break;

		default:
			printf("condition error: use correct syntx (e.g. 'if x < y')\n");
			break;
	}
	return result;
}


// if and loop statements

void If(char* args, CommandLine* cli) {

	uint32_t arg1 = numOrVar(args, cli, 0);
	uint32_t arg2 = numOrVar(args, cli, 2);

	char* op = argparse(args, 1);
	cli->conditionIf = trueOrFalse(op, arg1, arg2);
}

void Fi(char* args, CommandLine* cli) {

	cli->conditionIf = true;
}

void loop(char* args, CommandLine* cli) {

	uint32_t arg1 = numOrVar(args, cli, 0);
	uint32_t arg2 = numOrVar(args, cli, 2);

	char* op = argparse(args, 1);
	cli->conditionLoop = trueOrFalse(op, arg1, arg2);
}

void pool(char* args, CommandLine* cli) {

	cli->conditionLoop = true;
}


// input





// math functions for cli

uint32_t math(char* args, CommandLine* cli, uint8_t op) {

	uint32_t result = 0;
	uint32_t num = 0;

	for (int i = 0; i < (argcount(args)-1); i++) {

		num = numOrVar(args, cli, i+1);

	
		if (!i) {
		
			result += num;
		} else {

			switch (op) {
		
			case 0:
				result += num;
				break;
			case 1:
				result -= num;
				break;
			case 2:
				result *= num;
				break;
			case 3:
				result /= num;
				break;
			case 4:
				result %= num;
				break;
			case 5:
				result &= num;
				break;
			case 6:
				result |= num;
				break;
			case 7:
				result ^= num;
				break;
			default:
				break;
			}
		}
	}
	return result;
}


void add(char* args, CommandLine* cli) {

	uint32_t sum = math(args, cli, 0);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] += sum;
	cli->returnVal = sum;
}


void sub(char* args, CommandLine* cli) {

	uint32_t dif = math(args, cli, 1);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] -= dif;
	cli->returnVal = dif;
}


void mul(char* args, CommandLine* cli) {

	uint32_t pro = math(args, cli, 2);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] *= pro;
	cli->returnVal = pro;
}


void div(char* args, CommandLine* cli) {

	uint32_t quo = math(args, cli, 3);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] /= quo;
	cli->returnVal = quo;
}


void mod(char* args, CommandLine* cli) {

	uint32_t quo = math(args, cli, 4);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] %= quo;
	cli->returnVal = quo;
}


void bit_and(char* args, CommandLine* cli) {

	uint32_t bit = math(args, cli, 5);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] &= bit;
	cli->returnVal = bit;
}


void bit_or(char* args, CommandLine* cli) {

	uint32_t bit = math(args, cli, 6);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] |= bit;
	cli->returnVal = bit;
}


void bit_xor(char* args, CommandLine* cli) {

	uint32_t bit = math(args, cli, 7);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] ^= bit;
	cli->returnVal = bit;
}


//tui

void tuichar(char* args, CommandLine* cli) {
	
	uint8_t charVal = numOrVar(args, cli, 0);
	
	uint8_t fore = numOrVar(args, cli, 1);
	uint8_t back = numOrVar(args, cli, 2);
	
	uint8_t x = numOrVar(args, cli, 3);
	uint8_t y = numOrVar(args, cli, 4);

	putchar(charVal, fore, back, x, y);
}


void tui(char* args, CommandLine* cli) {
	
	uint8_t fore = numOrVar(args, cli, 0);
	uint8_t back = numOrVar(args, cli, 1);

	uint8_t x1 = numOrVar(args, cli, 2);
	uint8_t y1 = numOrVar(args, cli, 3);
	
	uint8_t x2 = numOrVar(args, cli, 4);
	uint8_t y2 = numOrVar(args, cli, 5);
	
	bool shadow = (bool)numOrVar(args, cli, 6);

	printfTUI(fore, back, x1, y1, x2, y2, shadow);
}


//execute script

void ex(char* args, CommandLine* cli) {

	if (FileIf(fnv1a(args))) {
	
		AyumuScriptCli(args, cli);
	} else {
		printf("Script file wasn't found.\n");
	}
}


void exi(char* args, CommandLine* cli) {


	if (FileIf(fnv1a(args))) {
	
		cli->scriptFile = args;
		cli->cliMode = 1;
	} else {
		cli->cliMode = 0;
		printf("Script file wasn't found.\n");
	}
}

//********************************************************END*****************************************************************









void version(char* args, CommandLine* cli) {

	printf("osakaOS v1.0\n");
}





void help(char* args, CommandLine* cli) {
	
	char* ch = " \n";
	ch[0] = 2;

	printf("\nWelcome to osakaOS!\n");
	printf("This is the command line interface.\n");
	printf("Please take it easy ");
	printf(ch);

	printf("If you feel lost, try the following commands: \n\n");

	printf("    'say (string)', 'osaka (int)', 'beep (int)',\n");
	printf("    'files', 'delete (file)', 'ex (file)',\n");
	printf("    'int (string) (int)', '+ (string) (int)',\n");
	printf("    'rdisk (int) (int)', 'wdisk (int) (string)'\n");
	printf("    'rmem (int)', 'wmem (int) (int)'\n\n");
	
	printf("Or use these keyboard shortcuts: \n\n");
	
	printf("    ctrl+e - file edit mode (ctrl-w to write file)\n");
	printf("    ctrl+p - piano mode (press keys to play notes)\n");
	printf("    ctrl+s - snake mode (wasd to play snake game)\n");
	printf("    ctrl+c - exit mode (return back to cli)\n");
}



void beep(char* args, CommandLine* cli) {

	uint32_t freq = numOrVar(args, cli, 0);
	
	if (freq < 1) {
	
		printf("No valid frequency was passed.\n");
		return;
	}

	makeBeep(freq);
}


void random(char* args, CommandLine* cli) {

	uint16_t prngNum = prng();	
	prngNum %= cli->cmdIndex;


	uint16_t hashIndex = cli->cmdList[prngNum];
	(cli->cmdTable[hashIndex])(args, cli);
}


void rng(char* args, CommandLine* cli) {

	uint32_t prngNum = prng();
	uint32_t bits = str2int(argparse(args, 0));

	switch (bits) {
	
		case 8:
			prngNum >> 8;
			break;
		case 32:
			(prngNum << 16) | 0xffff;
			break;
		default:
			break;
	}	
	
	printf(int2str(prngNum));
	printf("\n");
	
	cli->returnVal = prngNum;
}


void PANIC(char* args, CommandLine* cli) {

	Speaker speaker;

	while (1) {
	
		for (int i = 0; i < 4; i++) {
		
			printf("___   ___   _______   ___     _____         ___     ___   _______    _    \n");
			printf("| |   | |   | ____|   | |     |    \\        |  \\   /  |   | ____|   | |   \n");
			printf("| |___| |   | |____   | |     | ___/        |   \\ /   |   | |____   | |   \n");
			printf("| _____ |   | ____|   | |     | |           | |  v  | |   | ____|   |_|   \n");
			printf("| |   | |   | |____   | |___  | |           | |\\   /| |   | |____    _    \n");
			printf("|_|   |_|   |_____|   |____|  |_|           |_| \\ / |_|   |_____|   |_|   \n");
		}
		speaker.PlaySound(1100);
		sleep(700);
		

		printf("\v");
		speaker.NoSound();
		sleep(700);
	}
}	


void sata(char* args, CommandLine* cli) {

	uint32_t loop = numOrVar(args, cli, 0);
	str2int(args);
	uint8_t length = strlen(args);
	

	for (uint32_t i = 0; i < loop; i++) {
	
		cli->command(args, length);
	}

	//if sata 0 or no arg given go on forever and ever
	while (!loop) {
	
		cli->command(args, length);
	}
}


void clear(char* args, CommandLine* cli) {
	
	printf("\v");
}


void osaka(char* args, CommandLine* cli) {

	uint32_t value = numOrVar(args, cli, 0);
	printOsaka(value);
}


void dad(char* args, CommandLine* cli) {

	Funny cat;

	printf("\v");
	cat.cat();
	
	sleep(3000);
	
	printf("\v");
	cat.god();
}








//not working yet
void CommandLine::script(bool pressed, char key, bool ctrl) {

	static uint32_t fileSize = GetFileSize(this->scriptFile);
	static uint8_t file[1920];
	static uint8_t LBA = 0;

	static uint32_t nestedLoop[16];

	static uint32_t indexF = 0;
	static uint32_t indexLoopF = 0;
	
	ReadLBA(this->scriptFile, file, LBA);

	//get input key
	this->varTable[hash("$")] = (uint8_t)key;	
	this->varTable[hash("$CTRL")] = ctrl;	
	this->varTable[hash("$PRESS")] = pressed;	


	//interpret line
	AyumuScriptInput(this, file, fileSize, indexF, nestedLoop, indexLoopF);


	//exit script
	if (indexF >= fileSize) {

		LBA = 0;
		indexF = 0;
		indexLoopF = 0;
		this->conditionIf = true;
		this->conditionLoop = true;
		
		for (int i = 0; i < 1920; i++) {
		
			file[i] = 0x00;
			if (i < 16) { nestedLoop[i] = 0x00; }
		}
		
		this->cliMode = 0;
	}
}




void CommandLine::getTM(GlobalDescriptorTable* gdt, TaskManager* tm) {

	this->cli_gdt = gdt;
	this->cli_tm = tm;
}



void CommandLine::hash_add(char* cmd, void func(char*, CommandLine* cli)) {

	uint16_t hashIndex = hash(cmd);


	while (this->cmdTable[hashIndex] != nullptr) {
		
		hashIndex++;
	}


	this->cmdTable[hashIndex] = func;

	this->cmdList[this->cmdIndex] = hashIndex;
	this->cmdIndex++;
}


void CommandLine::hash_cli_init() {

	for (int i = 0; i < 65536; i++) {
	
		this->cmdTable[i] = nullptr;
		
		if (i < 1024) {
		
			this->varTable[i] = 0xffffffff;
		}
	}


	this->varTable[hash("$")] = 0x00;	
	this->varTable[hash("$CTRL")] = 0x00;	
	this->varTable[hash("$PRESS")] = 0x00;	


	this->conditionIf = true;
	this->conditionLoop = true;


	//compute and add functions 
	//to hash table on boot
	this->hash_add("say", say);
	this->hash_add("help", help);
	this->hash_add("clear", clear);
	this->hash_add("delay", delay);
	this->hash_add("random", random);
	this->hash_add("rng", rng);
	this->hash_add("osaka", osaka);
	this->hash_add("dad", dad);
	this->hash_add("beep", beep);
	this->hash_add("PANIC", PANIC);
	this->hash_add("explode", explode);
	this->hash_add("wmem", wmem);
	this->hash_add("rmem", rmem);
	this->hash_add("wdisk", wdisk);
	this->hash_add("rdisk", rdisk);
	this->hash_add("files", files);
	this->hash_add("size", size);
	this->hash_add("delete", deleteFile);
	this->hash_add("test", test);
	this->hash_add("andagi", andagi);
	this->hash_add("version", version);
		


	//special commands
	this->hash_add("ex", ex);
	this->hash_add("exi", exi);
	
	this->hash_add("int", varInt);
	
	this->hash_add("//", comment);

	this->hash_add("if", If);
	this->hash_add("fi", Fi);
	
	this->hash_add("loop", loop);
	this->hash_add("pool", pool);
	
	this->hash_add("+", add);
	this->hash_add("-", sub);
	this->hash_add("*", mul);
	this->hash_add("/", div);
	this->hash_add("%", mod);
	this->hash_add("&", bit_and);
	this->hash_add("|", bit_or);
	this->hash_add("^", bit_xor);
	
	this->hash_add("tuichar", tuichar);
	this->hash_add("tui", tui);
	
	this->hash_add("sata", sata);
	//this->hash_add("task", task);
	
}



char* CommandLine::command(char* cmd, uint8_t length) {

	char* command = cmd;
	char arguments[length];

		
	bool args = false;
	uint8_t argLength = 0;
	uint8_t cmdLength = length;
	

	for (uint8_t i = 0; i < length; i++) {
		
		if (command[i] == ' ' && argLength == 0) {
			
			argLength = length - i;
			args = (argLength > 0);
			
			cmdLength = i;
		}

		if (args) {

			arguments[i - (cmdLength + 1)] = cmd[i];
		}
	}
	


	command[cmdLength] = '\0';
	arguments[argLength-1] = '\0';


	uint16_t result = hash(command);

	
	if (this->conditionIf == false && result != hash("fi")) {	
		return "i dont care anymore its 4am lol";
	}
	
	if (this->conditionLoop == false && result != hash("pool")) {	
		return "i dont care anymore its 4am lol";
	}



	if (this->cmdTable[result] != nullptr)  {	

		(*cmdTable[result])(arguments, this); //execute function from array
		arguments[0] = '\0';
	
	} else {			

		//variable
		if (this->varTable[result % 1024] != 0xffffffff) {
			
			printf(int2str(this->varTable[result % 1024]));
			printf("\n");
		
		//unknown command		
		} else {

			if (length < 0xff) {

				printf("'");
				printf(command);
				printf("' is an unknown command or variable.\n");		
				makeBeep(60);

			} else {
	
				printf("No command needs to be this long.\n");		
				makeBeep(1);
			}
		}
	}

	return cmd;
}

