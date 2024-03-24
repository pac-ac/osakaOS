#include <cli.h>
#include <script.h>


using namespace os;
using namespace os::gui;
using namespace os::drivers;
using namespace os::common;
using namespace os::filesystem;



//kernel.cc functions used for everything
void TUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void putcharTUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
uint16_t setTextColor(bool set, uint16_t color = 0x07);
void printf(char*);
void printOsaka(uint8_t, bool);

uint16_t strlen(char* args);
uint32_t str2int(char* args);
char* int2str(uint32_t num);
char* argparse(char*, uint8_t);
uint8_t argcount(char*);

void makeBeep(uint32_t freq);
void explodeMain();

uint32_t prng();

void reboot();
void sleep(uint32_t);

void memWrite(uint32_t, uint32_t);
uint32_t memRead(uint32_t);

uint32_t FileList(CommandLine* cli);

uint32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum);

void lulalu();






uint16_t hash(char* cmd) {

	uint32_t val = fnv1a(cmd) + 1;
	return (val >> 16) ^ (val & 0xffff);
}


void say(char* args, CommandLine* cli) {

	cli->PrintCommand(args);
	cli->PrintCommand("\n");
}

void print(char* args, CommandLine* cli) {

	uint32_t charNum = numOrVar(args, cli, 0);
	
	if (charNum != 0) {
	
		char* foo = " ";
		foo[0] = (int8_t)(charNum);
		
		cli->PrintCommand(foo);
	} else {
		cli->PrintCommand(args);
	}
}


void textcolor(char* args, CommandLine* cli) {
		
	uint16_t newColor = (uint16_t)(str2int(args));
	if (!newColor) { return; }

	if (cli->desktop == false) {

		setTextColor(true, newColor);
	} else {
	
		cli->appWindow->textColor = newColor;
	}
}


void muteNonErrors(char* args, CommandLine* cli) {

	cli->mute ^= 1;
}


void rebootCMD(char* args, CommandLine* cli) {

	reboot();
}

void andagi(char* args, CommandLine* cli) {

	cli->PrintCommand("sata andagi\n");
}








void newtask(char* args, CommandLine* cli) {

	/*
	Task taskA(cli->gdt, task1);
	cli->tm->AddTask(&taskA);
	
	Task taskB(cli->gdt, task2);
	cli->tm->AddTask(&taskB);
	*/

	//UNHANDLED INTERRUPT 0D WHEN TASK REACHES END OF FUNCTION

	//works if put in while loop
}


void song(char* args, CommandLine* cli) {

	if (cli->tm->numTasks == 0) {
	
		//Task task(cli->gdt, lulalu, "Soramimi Cake");
		Task* task = 0;
		task = (Task*)MemoryManager::activeMemoryManager->malloc(sizeof(Task));
		new (task) Task(cli->gdt, lulalu, "Soramimi Cake");
		
		
		cli->tm->AddTask(task);
	} else {
	
		cli->tm->DeleteTask(cli->tm->currentTask);
	}

}




void tasks(char* args, CommandLine* cli) {

	if (cli->mute == false) {
	
		for (int i = 0; i < cli->tm->numTasks; i++) {
		
			cli->PrintCommand("PID: ");
			cli->PrintCommand(int2str(i));
			cli->PrintCommand("    ");
			cli->PrintCommand(cli->tm->tasks[i]->taskname);
			cli->PrintCommand("\n");
		}

	}

	cli->returnVal = cli->tm->numTasks;
}


void delay(char* args, CommandLine* cli) {

	uint32_t repeat = numOrVar(args, cli, 0);
	sleep(repeat);
}

void wmem(char* args, CommandLine* cli) {
		
	uint32_t mem = numOrVar(args, cli, 0);
	uint32_t val = numOrVar(args, cli, 1);
	
	memWrite(mem, val);

	if (cli->mute == false) {	
		
		cli->PrintCommand("Wrote ");
		cli->PrintCommand(int2str(val));	
		cli->PrintCommand(" to memory location ");
		cli->PrintCommand(int2str(mem));	
		cli->PrintCommand(".\n");
	}
}

void rmem(char* args, CommandLine* cli) {

	uint32_t mem = numOrVar(args, cli, 0);
	uint32_t val = memRead(mem);
	
	cli->returnVal = val;
	
	if (cli->mute == false) {	
	
		cli->PrintCommand("Reading from ");
		cli->PrintCommand(int2str(mem));	
		cli->PrintCommand(": ");
		cli->PrintCommand(int2str(val));
		cli->PrintCommand(".\n");
	}
}

	


void explode(char* args, CommandLine* cli) {

	explodeMain();
}


void offsetptr(char* args, CommandLine* cli) {

	uint32_t val = str2int(args);
	cli->offsetVal = val;
	
	if (cli->mute == false) {
	
		cli->PrintCommand("Command offset updated to ");
		cli->PrintCommand(int2str(val));
		cli->PrintCommand(".\n");
	}
}


void wdisk(char* args, CommandLine* cli) {

	uint32_t sector = numOrVar(args, cli, 0);
	
	if (sector < 64 && cli->mute == false) {
	
		cli->PrintCommand("Hopefully you're not overwriting anything important.\n");
	}


	//shift string
	char* cmp = argparse(args, 0);
	uint8_t offset = 0;

	for (offset; cmp[offset] != '\0'; offset++) {} offset++;
	for (int i = 0; args[i] != '\0'; i++) {
	
		args[i] = args[i+offset];
	}
	
	//write data
	//cli->ata0m->Write28(sector, (uint8_t*)args, strlen(args), cli->offsetVal);
	cli->ata0m->Write28(sector, (uint8_t*)args, strlen(args), 0);
	cli->ata0m->Flush();

	if (cli->mute == false) {
	
		cli->PrintCommand("Wrote: ");
		cli->PrintCommand(args);
		cli->PrintCommand(" to disk sector ");
		cli->PrintCommand(int2str(sector));
		cli->PrintCommand(".\n");
	}
}


void rdisk(char* args, CommandLine* cli) {

	uint32_t sector = numOrVar(args, cli, 0);
	uint32_t size = numOrVar(args, cli, 1);
	
	//read data
	uint8_t data[512];
	cli->ata0m->Read28(sector, data, size, 0);
	//cli->ata0m->Read28(sector, data, size, cli->offsetVal);

	//print and empty data
	cli->PrintCommand((char*)data);
	cli->PrintCommand("\n");
	
	for (int i = 0; i < 512; i++) data[i] = 0;
}


void files(char* args, CommandLine* cli) {

	char name[33];
	for (int i = 0; i < 33; i++) { name[i] = 0x00; }
	uint32_t location = 0;
	uint32_t fileNum = GetFileCount();
	cli->returnVal = fileNum;

	//go through each file entry and print
	for (int i = 0; i < fileNum; i++) {
	
		location = GetFileName(i, name);
		
		if (cli->mute == false) {
		
			//bug with printing file names wrong (kinda fixed???)
			cli->PrintCommand(int2str(location));
			cli->PrintCommand("    ");
			cli->PrintCommand(name);
			cli->PrintCommand("\n");
		}
	}



	//print file count	
	char* strNum = int2str(fileNum);
	
	if (cli->mute == false) {
	
		cli->PrintCommand("\n");
		cli->PrintCommand(strNum);
		cli->PrintCommand(" files have been allocated.\n");
	}
}


void size(char* args, CommandLine* cli) {
		
	uint32_t size = GetFileSize(args);
	cli->returnVal = size;

	
	if (cli->mute == false) {
	
		if (size) {
	
			cli->PrintCommand("'");
			cli->PrintCommand(args);
			cli->PrintCommand("' is ");
			cli->PrintCommand(int2str(size));
			cli->PrintCommand(" bytes large.\n");
		} else {
			cli->PrintCommand("How can you measure the size of something that doesn't exist?\n");
		}
	}

}



void deleteFile(char* args, CommandLine* cli) {

	bool deleted = DeleteFile(args);


	if (cli->mute == false) {
	
		if (deleted) {
		
			cli->PrintCommand("'");
			cli->PrintCommand(args);
			cli->PrintCommand("' was deleted.\n");
		} else {
			cli->PrintCommand(args);
			cli->PrintCommand(" isn't a file dumbass.\n");
		}
	}
}







//********************************************************AyumuScript*****************************************************************

uint32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum) {
	
	char* name = argparse(args, argNum);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] != 0xffffffff) {
	
		return cli->varTable[hashVar];

	} else if (name[0] == '$' && name[2] == '\0') {

		//given arguments to script
		if (name[1] <= '9' && name[1] >= '0') {
		
			return cli->argTable[name[1]-'0'];
		}

		//value returned from most recent command
		if (name[1] == 'R') {
		
			return cli->returnVal;
		}

		return 0;

	} else if (name[0] == '@' && name[2] == '\0') {

		return (uint8_t)(name[1]);

	} else { 
		return str2int(name);
	}
}



void varInt(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);

	if ((uint8_t)(name[0] - '0') < 10) {
	
		cli->PrintCommand("int error: name cannot begin with number.\n");
		return;
	}

	if (name[0] == '$') {
	
		cli->PrintCommand("int error: name cannot begin with '$'.\n");
		return;
	}

	if (name[0] == '@') {
	
		cli->PrintCommand("int error: name cannot begin with '@'.\n");
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

bool trueOrFalse(char* op, uint32_t arg1, uint32_t arg2, CommandLine* cli) {

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
			cli->PrintCommand("condition error: use correct syntax (e.g. 'if x < y')\n");
			result = true;
			break;
	}
	return result;
}


// if and loop statements

void If(char* args, CommandLine* cli) {

	uint32_t arg1 = numOrVar(args, cli, 0);
	uint32_t arg2 = numOrVar(args, cli, 2);

	char* op = argparse(args, 1);
	cli->conditionIf = trueOrFalse(op, arg1, arg2, cli);
}

void Fi(char* args, CommandLine* cli) {

	cli->conditionIf = true;
}

void loop(char* args, CommandLine* cli) {

	uint32_t arg1 = numOrVar(args, cli, 0);
	uint32_t arg2 = numOrVar(args, cli, 2);

	char* op = argparse(args, 1);
	cli->conditionLoop = trueOrFalse(op, arg1, arg2, cli);
}

void pool(char* args, CommandLine* cli) {

	cli->conditionLoop = true;
}


// input





// math functions for cli

uint32_t mathCMD(char* args, CommandLine* cli, uint8_t op) {

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




//this is pretty ugly but every command must be its own function

void add(char* args, CommandLine* cli) {

	uint32_t sum = mathCMD(args, cli, 0);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] += sum;
	cli->returnVal = sum;
}


void sub(char* args, CommandLine* cli) {

	uint32_t dif = mathCMD(args, cli, 1);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] -= dif;
	cli->returnVal = dif;
}


void mul(char* args, CommandLine* cli) {

	uint32_t pro = mathCMD(args, cli, 2);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] *= pro;
	cli->returnVal = pro;
}


void div(char* args, CommandLine* cli) {

	uint32_t quo = mathCMD(args, cli, 3);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	
	if (quo) {
		cli->varTable[hashVar] /= quo;
		cli->returnVal = quo;
	} else {
		cli->returnVal = 0;
	}
}


void mod(char* args, CommandLine* cli) {

	uint32_t quo = mathCMD(args, cli, 4);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	
	if (quo) {
		cli->varTable[hashVar] %= quo;
		cli->returnVal = quo;
	} else {
		cli->returnVal = 0;
	}
}


void bit_and(char* args, CommandLine* cli) {

	uint32_t bit = mathCMD(args, cli, 5);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] &= bit;
	cli->returnVal = bit;
}


void bit_or(char* args, CommandLine* cli) {

	uint32_t bit = mathCMD(args, cli, 6);

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	cli->varTable[hashVar] |= bit;
	cli->returnVal = bit;
}


void bit_xor(char* args, CommandLine* cli) {

	uint32_t bit = mathCMD(args, cli, 7);

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

	putcharTUI(charVal, fore, back, x, y);
}


void tui(char* args, CommandLine* cli) {
	
	uint8_t fore = numOrVar(args, cli, 0);
	uint8_t back = numOrVar(args, cli, 1);

	uint8_t x1 = numOrVar(args, cli, 2);
	uint8_t y1 = numOrVar(args, cli, 3);
	
	uint8_t x2 = numOrVar(args, cli, 4);
	uint8_t y2 = numOrVar(args, cli, 5);
	
	bool shadow = (bool)numOrVar(args, cli, 6);

	TUI(fore, back, x1, y1, x2, y2, shadow);
}


//gui
void putpixel(char* args, CommandLine* cli) {
	
	uint8_t x = numOrVar(args, cli, 0);
	uint8_t y = numOrVar(args, cli, 1);
	uint8_t color = numOrVar(args, cli, 2);

	//cli->appWindow->DrawPixel(x, y, color);
}



//execute script

void ex(char* args, CommandLine* cli) {

	char* file = argparse(args, 0);
	uint32_t fileSector = fnv1a(file);

	if (FileIf(fileSector)) {

		//parse script arguments
		for (int i = 0; i < 10; i++) {
		
			cli->argTable[i] = str2int(argparse(args, i+1));
		}

		AyumuScriptCli(args, cli);
		//AyumuScriptCli(file, cli);

		//clear script arguments
		for (int i = 0; i < 10; i++) { cli->argTable[i] = 0; }
		
		return;
	}
		
	if (cli->mute == false) {
	
		cli->PrintCommand("Script file wasn't found.\n");
	}
}


void exi(char* args, CommandLine* cli) {

	char* file = argparse(args, 0);

	if (FileIf(fnv1a(file))) {
	
		cli->scriptFile = args;
		cli->cliMode = 1;
		return;
	}
	cli->cliMode = 0;
	

	if (cli->mute == false) {
		
		cli->PrintCommand("Script file wasn't found.\n");
	}
}

/*
void alias(char* args, CommandLine* cli) {

	cli->hash_add(args);
}
*/

//********************************************************END*****************************************************************









void version(char* args, CommandLine* cli) {

	cli->PrintCommand("osakaOS v1.1\n");
}





void help(char* args, CommandLine* cli) {
	
	char* ch = " \n";
	ch[0] = 2;

	cli->PrintCommand("\nWelcome to osakaOS!\n");
	cli->PrintCommand("This is the command line interface.\n");
	cli->PrintCommand("Please take it easy ");
	cli->PrintCommand(ch);

	cli->PrintCommand("If you feel lost, try the following commands: \n\n");
	cli->PrintCommand("    'say (string)', 'osaka (int)', 'beep (int)',\n");
	cli->PrintCommand("    'files', 'delete (file)', 'ex (file)',\n");
	cli->PrintCommand("    'int (string) (int)', '+ (string) (int)',\n");
	cli->PrintCommand("    'rdisk (int) (int)', 'wdisk (int) (string)'\n");
	cli->PrintCommand("    'rmem (int)', 'wmem (int) (int)'\n\n");
	
	cli->PrintCommand("Or use these keyboard shortcuts: \n\n");
	
	cli->PrintCommand("    ctrl+e - file edit mode (ctrl-w to write file)\n");
	cli->PrintCommand("    ctrl+p - piano mode (press keys to play notes)\n");
	cli->PrintCommand("    ctrl+s - snake mode (wasd to play snake game)\n");
	cli->PrintCommand("    ctrl+c - exit mode (return back to cli)\n");
}



void beep(char* args, CommandLine* cli) {

	uint32_t freq = numOrVar(args, cli, 0);
	
	if (freq < 1) {
	
		cli->PrintCommand("No valid frequency was passed.\n");
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
	
	cli->PrintCommand(int2str(prngNum));
	cli->PrintCommand("\n");
	
	cli->returnVal = prngNum;
}


void PANIC(char* args, CommandLine* cli) {

	Speaker speaker;

	if (cli->desktop == false) {
		
		while (1) {
			for (int i = 0; i < 4; i++) {
		
				cli->PrintCommand("___   ___   _______   ___     _____         ___     ___   _______    _    \n");
				cli->PrintCommand("| |   | |   | ____|   | |     |    \\        |  \\   /  |   | ____|   | |   \n");
				cli->PrintCommand("| |___| |   | |____   | |     | ___/        |   \\ /   |   | |____   | |   \n");
				cli->PrintCommand("| _____ |   | ____|   | |     | |           | |  v  | |   | ____|   |_|   \n");
				cli->PrintCommand("| |   | |   | |____   | |___  | |           | |\\   /| |   | |____    _    \n");
				cli->PrintCommand("|_|   |_|   |_____|   |____|  |_|           |_| \\ / |_|   |_____|   |_|   \n");
			}
			speaker.PlaySound(1100);
			sleep(700);
		
			cli->PrintCommand("\v");
			speaker.NoSound();
			sleep(700);
		}
	} else {
		cli->appWindow->Print("This command is only available in text mode.\n");
	}
}	


void sata(char* args, CommandLine* cli) {

	uint32_t loop = numOrVar(args, cli, 0);
	
	
	//shift string
	char* cmp = argparse(args, 0);
	uint8_t offset = 0;

	for (offset; cmp[offset] != '\0'; offset++) {} offset++;
	for (int i = 0; args[i] != '\0'; i++) {
	
		args[i] = args[i+offset];
	}
	
	uint8_t length = strlen(args);

	
	//execute commands
	for (uint32_t i = 0; i < loop; i++) {
	
		cli->command(args, length);
	}

	//if sata 0 or no arg given go on forever and ever
	while (!loop) {
	
		cli->command(args, length);
	}
}


void clear(char* args, CommandLine* cli) {
	
	cli->PrintCommand("\v");
}


void osaka(char* args, CommandLine* cli) {

	if (cli->desktop == false) {

		uint32_t value = numOrVar(args, cli, 0);
		printOsaka(value, 0);
	} else {
		cli->appWindow->Print("This command is only available in text mode.\n");
	}
}


void dad(char* args, CommandLine* cli) {

	if (cli->desktop == false) {

		Funny cat;

		printf("\v");
		cat.cat();
	
		sleep(3000);
	
		printf("\v");
		cat.god();
	} else {
		cli->appWindow->Print("This command is only available in text mode.\n");
	}
}





CommandLine::CommandLine(GlobalDescriptorTable* gdt, 
			TaskManager* tm, 
			AdvancedTechnologyAttachment* ata0m) {

	this->gdt = gdt;
	this->tm = tm;
	this->ata0m = ata0m;
}

CommandLine::~CommandLine() {
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
		
			if (i < 10) {
			
				this->argTable[i] = 0;
			}
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
	this->hash_add("print", print);
	this->hash_add("textcolor", textcolor);
	this->hash_add("help", help);
	this->hash_add("clear", clear);
	this->hash_add("tasks", tasks);
	this->hash_add("reboot", rebootCMD);
	this->hash_add("mute", muteNonErrors);
	this->hash_add("delay", delay);
	this->hash_add("random", random);
	this->hash_add("rng", rng);
	this->hash_add("osaka", osaka);
	this->hash_add("dad", dad);
	this->hash_add("beep", beep);
	this->hash_add("song", song);
	this->hash_add("PANIC", PANIC);
	this->hash_add("explode", explode);
	this->hash_add("wmem", wmem);
	this->hash_add("rmem", rmem);
	this->hash_add("wdisk", wdisk);
	this->hash_add("rdisk", rdisk);
	this->hash_add("files", files);
	this->hash_add("size", size);
	this->hash_add("delete", deleteFile);
	this->hash_add("andagi", andagi);
	this->hash_add("version", version);
	this->hash_add("newtask", newtask);


	//special commands for scripting
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

	this->hash_add("offset", offsetptr);
	
	this->hash_add("tuichar", tuichar);
	this->hash_add("tui", tui);
	
	this->hash_add("sata", sata);


	//init script on boot
	if (FileIf(fnv1a("wakeup"))) {
	
		this->command("ex wakeup", strlen("ex wakeup"));
	}
}


//cli for desktop
void CommandLine::PrintCommand(char* str) {

	if (desktop) {
		this->appWindow->Print(str);
	} else {
		printf(str);
	}
}


void CommandLine::OnKeyDown(char ch, Widget* widget) {
}

void CommandLine::OnKeyUp(char ch, Widget* widget) {
}

void CommandLine::OnMouseDown(int32_t x, int32_t y, uint8_t button, Widget* widget) {
}


void CommandLine::ComputeAppState(GraphicsContext* gc, Widget* widget) {
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
			
			if (mute == false) {
			
				PrintCommand(int2str(this->varTable[result % 1024]));
				PrintCommand("\n");
			}
		//unknown command		
		} else {
			if (length < 0xff) {

				PrintCommand("'");
				PrintCommand(command);
				PrintCommand("' is an unknown command or variable.\n");		
				makeBeep(60);

			} else {
				PrintCommand("No command needs to be this long.\n");		
				makeBeep(1);
			}
		}
	}
	return cmd;
}

