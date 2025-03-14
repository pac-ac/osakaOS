#include <cli.h>
#include <script.h>


using namespace os;
using namespace os::gui;
using namespace os::math;
using namespace os::drivers;
using namespace os::common;
using namespace os::filesystem;
using namespace os::hardwarecommunication;


//the entire command line of osakaOS is stored here
void (*CommandLine::cmdTable[65536])(char* str, CommandLine* cli);
bool CommandLine::WakeupInit = false;


//kernel.cc functions used for everything
uint16_t hash(char* str);
	
void TUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void putcharTUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
uint16_t setTextColor(bool set, uint16_t color = 0x07);
void printf(char*);
void printOsaka(uint8_t, bool);

uint16_t strlen(char* args);
bool strcmp(char* str1, char* str2);
uint32_t str2int(char* args);
char* int2str(uint32_t num);
char* argparse(char*, uint8_t);
uint8_t argcount(char*);

float str2float(char* str);


void makeBeep(uint32_t freq);

uint32_t prng();

void reboot();
//void sleep(uint32_t);

void memWrite(uint32_t, uint32_t);
uint32_t memRead(uint32_t);
uint32_t cmosDetectMemory();

uint32_t FileList(CommandLine* cli);

uint32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum);		

CommandLine* LoadScriptForTask(bool set, CommandLine* cli = 0);








uint16_t hash(char* str) {

	uint32_t val = 0x811c9dc5;
	
	for (int i = 0; str[i] != '\0'; i++) {
	
		val ^= str[i];
		val *= 0x01000193;
	}
	val++;
	
	return (val >> 16) ^ (val & 0xffff);
}


void say(char* args, CommandLine* cli) {

	cli->PrintCommand(args);
	cli->PrintCommand("\n");
}

void print(char* args, CommandLine* cli) {

	uint32_t charNum = numOrVar(args, cli, 0);
	char* foo = " ";
	foo[0] = (int8_t)(charNum);
	
	if (charNum < cli->lists->numOfNodes) {
	
		List* list = (List*)(cli->lists->Read(charNum));

		for (int i = 0; i < list->numOfNodes; i++) {
		
			foo[0] = *(char*)(list->Read(i));
			cli->PrintCommand(foo);
		}

	} else if (charNum != 0) { cli->PrintCommand(foo); 
	} else { 		   cli->PrintCommand(args); }
}


void textcolor(char* args, CommandLine* cli) {
		
	uint16_t newColor = (uint16_t)(str2int(args));
	if (!newColor) { return; }
	setTextColor(true, newColor);
}


void muteNonErrors(char* args, CommandLine* cli) { cli->mute ^= 1; }
void rebootCMD(char* args, CommandLine* cli) { reboot(); }


//multitasking commands
void tasks(char* args, CommandLine* cli) {

	for (int i = 0; i < cli->tm->numTasks; i++) {
		
		cli->PrintCommand("PID: ");
		cli->PrintCommand(int2str(i));
		cli->PrintCommand("    ");
		cli->PrintCommand(cli->tm->tasks[i]->taskname);
		cli->PrintCommand("    PRI: ");
		cli->PrintCommand(int2str(cli->tm->taskPriority[i]));
		cli->PrintCommand("\n");
	}
	cli->returnVal = cli->tm->numTasks;
}

void kill(char* args, CommandLine* cli) {

	uint32_t taskNum = numOrVar(args, cli, 0) % 256;

	//check if tasks exists
	if (cli->tm->tasks[taskNum] == nullptr) {
	
		cli->PrintCommand("Task does not exist.\n");
		return;
	}

	//kill task
	uint32_t priority = cli->tm->taskPriority[taskNum] % 256;
	//stall lol
	for (int i = 0; i < 0xffff; i++) {}
	
	//delete task
	cli->tm->DeleteTask(taskNum);

	//free task from kernel heap
	cli->DeleteTaskForScript(taskNum);

	//let user know
	cli->PrintCommand("Task ");
	cli->PrintCommand(int2str(taskNum));
	cli->PrintCommand(" killed.\n");
}

void schedule(char* args, CommandLine* cli) {

	uint32_t pid = numOrVar(args, cli, 0);
	uint32_t newPriority = numOrVar(args, cli, 1) % 256;
	cli->tm->taskPriority[pid] = newPriority;
}


//driver commands
void delay(char* args, CommandLine* cli) {

	uint32_t repeat = numOrVar(args, cli, 0);
	cli->cmos->pit->sleep(repeat);
}

void userSleep(char* args, CommandLine* cli) {

	uint32_t repeat = numOrVar(args, cli, 0);
	uint32_t oldTime = cli->cmos->GetUpdate();

	while ((cli->cmos->GetUpdate() - oldTime) < repeat) {}
}



void wmem(char* args, CommandLine* cli) {
		
	uint32_t mem = numOrVar(args, cli, 0);
	uint32_t val = numOrVar(args, cli, 1);
	
	memWrite(mem, val);

	cli->PrintCommand("Wrote ");
	cli->PrintCommand(int2str(val));	
	cli->PrintCommand(" to memory location ");
	cli->PrintCommand(int2str(mem));	
	cli->PrintCommand(".\n");
}

void rmem(char* args, CommandLine* cli) {

	uint32_t mem = numOrVar(args, cli, 0);
	uint32_t val = memRead(mem);
	
	cli->returnVal = val;
	
	cli->PrintCommand("Reading from ");
	cli->PrintCommand(int2str(mem));	
	cli->PrintCommand(": ");
	cli->PrintCommand(int2str(val));
	cli->PrintCommand(".\n");
}


void heap(char* args, CommandLine* cli) {

	cli->PrintCommand("Allocated ");
	cli->PrintCommand(int2str(cli->mm->size));
	cli->PrintCommand(" bytes on heap.\n");
}


//asm
void startInterrupts(char* args, CommandLine* cli) { asm volatile("sti"); }
void stopInterrupts(char* args, CommandLine* cli) { asm volatile("cli"); }
void halt(char* args, CommandLine* cli) { asm volatile("hlt"); }


//assembler for generating executable 
//binary code from assembly file
void assembler(char* args, CommandLine* cli) {

	//generate binary file
	cli->compiler->Assemble(argparse(args, 0), argparse(args, 1));
}


//port commands
void PortRead8(char* args, CommandLine* cli) {

	Port8Bit port8(str2int(args));
	cli->returnVal = port8.Read();
}
void PortWrite8(char* args, CommandLine* cli) {

	Port8Bit port8(str2int(argparse(args, 0)));
	port8.Write(str2int(argparse(args, 1)));
}
void PortRead16(char* args, CommandLine* cli) {

	Port16Bit port16(str2int(args));
	cli->returnVal = port16.Read();
}
void PortWrite16(char* args, CommandLine* cli) {

	Port16Bit port16(str2int(argparse(args, 0)));
	port16.Write(str2int(argparse(args, 1)));
}
void PortRead32(char* args, CommandLine* cli) {

	Port32Bit port32(str2int(args));
	cli->returnVal = port32.Read();
}
void PortWrite32(char* args, CommandLine* cli) {

	Port32Bit port32(str2int(argparse(args, 0)));
	port32.Write(str2int(argparse(args, 1)));
}





//ata driver and filesystem commands
void offsetptr(char* args, CommandLine* cli) {

	uint32_t val = str2int(args);
	cli->offsetVal = val;
	
	cli->PrintCommand("Command offset updated to ");
	cli->PrintCommand(int2str(val));
	cli->PrintCommand(".\n");
}


void wdisk(char* args, CommandLine* cli) {

	uint32_t sector = numOrVar(args, cli, 0);
	
	if (sector < 64) { 
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
	cli->filesystem->ata0m->Write28(sector, (uint8_t*)args, strlen(args), 0);
	cli->filesystem->ata0m->Flush();

	
	cli->PrintCommand("Wrote: ");
	cli->PrintCommand(args);
	cli->PrintCommand(" to disk sector ");
	cli->PrintCommand(int2str(sector));
	cli->PrintCommand(".\n");
}


void rdisk(char* args, CommandLine* cli) {

	uint32_t sector = numOrVar(args, cli, 0);
	uint32_t size = numOrVar(args, cli, 1);
	
	//read data
	uint8_t data[512];
	cli->filesystem->ata0m->Read28(sector, data, size, 0);

	//print and empty data
	cli->PrintCommand((char*)data);
	cli->PrintCommand("\n");
	
	for (int i = 0; i < 512; i++) data[i] = 0;
}



//query filesystem table in memory
void ofs(char* args, CommandLine* cli) {

	cli->PrintCommand("Files allocated in table: ");
	cli->PrintCommand(int2str(cli->filesystem->table->fileCount));
	cli->PrintCommand("\n");

	for (int i = 0; i < cli->filesystem->table->fileCount; i++) {
	
		File* file = (File*)(cli->filesystem->table->files->Read(i));
		
		cli->PrintCommand("Sector: ");
		cli->PrintCommand(int2str(file->Location));
		cli->PrintCommand(" - Name: ");
		cli->PrintCommand(file->Name);
		cli->PrintCommand(" - Entry #");
		cli->PrintCommand(int2str(i));
		cli->PrintCommand("\n");
	}
	cli->PrintCommand("Current allocation sector: ");
	cli->PrintCommand(int2str(cli->filesystem->table->currentOpenSector));
	cli->PrintCommand("\n");
	
	cli->returnVal = cli->filesystem->table->fileCount;
}


void files(char* args, CommandLine* cli) {

	char name[33];
	char tag[33];
	uint32_t fileNum = cli->filesystem->GetFileCount();
	uint8_t tagNum = 0;

	bool filterTags = args[0] != '\0';
	
	uint32_t fileCount = 0;
	uint16_t previousColor = setTextColor(false);

	//list all files
	//go through each file entry and print
	for (int i = 0; i < fileNum; i++) {
		
		uint32_t location = cli->filesystem->GetFileName(i, name);
		bool skip = false;

		//list files with given tag(s)
		//if none given then print all files
		if (filterTags) { skip = cli->filesystem->GetTagFile(args, location, &tagNum) != location; }		

		//print information
		if (!skip) {
			
			cli->PrintCommand(int2str(location));
			cli->PrintCommand("    ");
			cli->PrintCommand(name, 0x09);
			cli->PrintCommand("    ");
	
			for (int j = 0; j < 8; j++) {
		
				cli->filesystem->GetFileTag(name, j, tag);
				cli->PrintCommand(tag, 0x0a);
				cli->PrintCommand(" ");
			}
			cli->PrintCommand("\n", previousColor);
			fileCount++;
		}
	}
	//print file count and give
	//return value of file count
	char* strNum = int2str(fileCount);
	cli->returnVal = fileCount;

	cli->PrintCommand("\n");
	cli->PrintCommand(strNum);
	cli->PrintCommand(" files have been allocated.\n");
}




void catFile(char* args, CommandLine* cli) {

	uint32_t LBAsize = cli->filesystem->GetFileSize(args)/OFS_BLOCK_SIZE;
	uint8_t data[OFS_BLOCK_SIZE];

	if (LBAsize == 0) {
	
		cli->PrintCommand("Nothing here...");
	} else {
		for (int i = 0; i < LBAsize; i++) {
	
			cli->filesystem->ReadLBA(args, data, i);
	
			//print data from file
			for (uint16_t j = 0; j < OFS_BLOCK_SIZE; j++) {
		
				char* str = " ";
				str[0] = (char)(data[j]);
				cli->PrintCommand(str);
			}
		}
	}
	cli->PrintCommand("\n");
}


void size(char* args, CommandLine* cli) {
		
	uint32_t size = cli->filesystem->GetFileSize(args);
	cli->returnVal = size;

	
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


void createFile(char* args, CommandLine* cli) {
	
	if (argcount(args) < 1) {
	
		cli->PrintCommand("Must provide name for file creation.\n");
		return;
	}
	
	//get file name
	int i = 0;
	char fileName[33];
	for (i; i < strlen(argparse(args, 0)); i++) { fileName[i] = args[i];}
	fileName[i] = '\0';

	//get list name
	int j = 0;
	char listName[256];
	for (j; j < strlen(argparse(args, 1)); j++) { listName[j] = args[i+j+1];}
	listName[j] = '\0';
	
	
	
	List* dataList = nullptr;
	uint32_t listIndex = hash(listName) % 1024;
	
	//check if list for data exists
	if (argcount(args) > 1 && cli->varTable[listIndex] < cli->lists->numOfNodes) {
		
		dataList = (List*)(cli->lists->Read(cli->varTable[listIndex]));
	}

	//write data from list to file
	uint8_t data[OFS_BLOCK_SIZE];
		

	if (dataList != nullptr) {
		
		for (int i = 0; i < OFS_BLOCK_SIZE; i++) { 
		
			if (i < dataList->numOfNodes) { data[i] = (uint8_t)(*((uint32_t*)(dataList->Read(i))));
			} else { 	   data[i] = 0x00; }
		}
	} else {
		for (int i = 0; i < OFS_BLOCK_SIZE; i++) { data[i] = 0x00; } 
	}
	

	//create file
	bool created = cli->filesystem->NewFile(fileName, data, OFS_BLOCK_SIZE);

	cli->PrintCommand("'");
	cli->PrintCommand(argparse(args, 0));

	if (created) {  cli->PrintCommand("' was created.\n");
	} else {	cli->PrintCommand("' already exists or couldn't be created for some reason.\n"); }
}


void deleteFile(char* args, CommandLine* cli) {

	bool deleted = cli->filesystem->DeleteFile(args);

	cli->PrintCommand("'");
	cli->PrintCommand(args);
	
	if (deleted) {  cli->PrintCommand("' was deleted.\n");
	} else {	cli->PrintCommand("' isn't a file dumbass.\n"); }
}



void copy(char* args, CommandLine* cli) {

	if (argcount(args) < 2) {
	
		cli->PrintCommand("Atleast 2 files are needed for command.\n");
		return;
	}
	uint32_t location = cli->filesystem->GetFileSector(argparse(args, 0));
	
	if (cli->filesystem->FileIf(location) == false) {
		
		cli->PrintCommand("File to copy doesn't exist.\n");
		return;
	}

	//file block data
	uint8_t data[OFS_BLOCK_SIZE];

	//create and copy file
	bool created = cli->filesystem->NewFile(argparse(args, 1), data, OFS_BLOCK_SIZE);


}


void tag(char* args, CommandLine* cli) {

	for (int i = 1; i < argcount(args); i++) {

		uint32_t location = cli->filesystem->GetFileSector(argparse(args, i));
		bool check = cli->filesystem->NewTag(args, location);

		if (check) {
		
			cli->PrintCommand("'");
			cli->PrintCommand(argparse(args, i));
			cli->PrintCommand("' tagged with '");
			cli->PrintCommand(argparse(args, 0));
			cli->PrintCommand("'.\n");
		} else {
			cli->PrintCommand("File doesn't exist, or already has maximum tags.\n");
		}
	}
}


void detag(char* args, CommandLine* cli) {
	
	for (int i = 1; i < argcount(args); i++) {

		uint32_t location = cli->filesystem->GetFileSector(argparse(args, i));
		uint8_t tagNum = 0;
		bool tagMatches = cli->filesystem->GetTagFile(argparse(args, 0), location, &tagNum) == location;

		if (tagMatches) {
	
			cli->filesystem->DeleteTag(location, tagNum);
			cli->PrintCommand("Tag removed.\n");
		} else {
			cli->PrintCommand("Tag can't be found on given file.\n");
		}
	}
}

void encrypt(char* args, CommandLine* cli) {

	int i = 0;
	
	char fileName[33];
	for (i; i < strlen(argparse(args, 0)); i++) { fileName[i] = args[i];}
	fileName[i] = '\0';

	uint8_t key[16];
	for (int j = i+1; j < strlen(argparse(args, 1)); j++) { key[j] = (uint8_t)args[i+j+1]; }
	
	cli->filesystem->CryptFile(fileName, key, true);
}

void decrypt(char* args, CommandLine* cli) {
	
	int i = 0;
	
	char fileName[33];
	for (i; i < strlen(argparse(args, 0)); i++) { fileName[i] = args[i];}
	fileName[i] = '\0';
	
	uint8_t key[16];
	for (int j = i+1; j < strlen(argparse(args, 1)); j++) { key[j] = (uint8_t)args[i+j+1]; }
	
	
	cli->filesystem->CryptFile(fileName, key, false);
}



//networking commands
void ip(char* args, CommandLine* cli) {

	if (cli->network != 0) {
		
		uint32_t mac0 = (cli->network->mac >> 32);
		uint32_t mac1 = (cli->network->mac & 0xffffffff);

		cli->PrintCommand("MAC Address: ");
		cli->PrintCommand(int2str(mac0));
		cli->PrintCommand(int2str(mac1));
		cli->PrintCommand("\n");
	
		cli->PrintCommand("IP Address: ");
		cli->PrintCommand(int2str(cli->network->ip));
		cli->PrintCommand("\n");
	
		cli->PrintCommand("GATEWAY: ");
		cli->PrintCommand(int2str(cli->network->gateway));
		cli->PrintCommand("\n");
		
		cli->PrintCommand("SUBNET: ");
		cli->PrintCommand(int2str(cli->network->subnet));
		cli->PrintCommand("\n");
	}
}


void ping(char* args, CommandLine* cli) {

	uint32_t ip = str2int(args);
	cli->network->icmp->RequestEchoReply(ip);
	cli->PrintCommand("\n");
}



//graphical commands
void terminal(char* args, CommandLine* cli) {
	
	if (cli->gui) { cli->appWindow->parent->CreateChild(1, "Osaka's Terminal", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}


void kasugapaint(char* args, CommandLine* cli) {
	
	if (cli->gui) { cli->appWindow->parent->CreateChild(2, "KasugaPaint", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}

void journal(char* args, CommandLine* cli) {
	
	if (cli->gui) { cli->appWindow->parent->CreateChild(3, "Journal", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}


void window(char* args, CommandLine* cli) {
		
	if (cli->gui && cli->userWindow == nullptr) {
	
		if (strlen(args) < 2) { cli->userWindow = cli->appWindow->parent->CreateChild(0, "OSaka Window", cli); }
		else { cli->userWindow = cli->appWindow->parent->CreateChild(0, args, cli); }

		cli->targetWindow = true;
	} else {
		if (cli->gui) { cli->PrintCommand("Window is already active.\n"); } 
		else { cli->PrintCommand("This command is not available in text mode.\n"); }
	}
}


//add desktop shortcut
void shortcut(char* args, CommandLine* cli) {

	char* file = argparse(args, 0);
	uint32_t location = cli->filesystem->GetFileSector(argparse(args, 0));

	uint8_t openType = str2int(argparse(args, 1));
	char* imageFile = nullptr;
	//char* imageFile = argparse(args, 2);

	if (cli->filesystem->FileIf(location) == false) {
	
		cli->PrintCommand("'");
		cli->PrintCommand(argparse(args, 0));
		cli->PrintCommand("' wasn't found on the disk :/\n");
		return;
	}

	//make button
	cli->appWindow->parent->CreateButton(argparse(args, 0), openType, imageFile);
}


void targetgui(char* args, CommandLine* cli) {

	if (cli->userWindow == nullptr) { 
		
		cli->PrintCommand("User window is not active.\n");
		return;
	}

	cli->targetWindow ^= 1;

	if (cli->targetWindow) { cli->PrintCommand("Currently targeting user window.\n"); }
	else { cli->PrintCommand("Currently targeting desktop buffer.\n"); }
}

void vgaPalette(char* args, CommandLine* cli) {

	uint8_t vgaIndex = str2int(argparse(args, 0));
	uint8_t r = str2int(argparse(args, 1));
	uint8_t g = str2int(argparse(args, 2));
	uint8_t b = str2int(argparse(args, 3));

	cli->vga->PaletteUpdate(vgaIndex, r, g, b);
}


void putpixel(char* args, CommandLine* cli) {
	
	uint32_t x = numOrVar(args, cli, 0);
	uint32_t y = numOrVar(args, cli, 1);
	uint8_t color = numOrVar(args, cli, 2);

	if (cli->targetWindow) { cli->userWindow->WritePixel(x, y, color); }
	else { cli->appWindow->parent->PutPixel(x, y, color); }
}

void drawrect(char* args, CommandLine* cli) {

	uint32_t x0 = numOrVar(args, cli, 0);
	uint32_t y0 = numOrVar(args, cli, 1);
	uint32_t x1 = numOrVar(args, cli, 2);
	uint32_t y1 = numOrVar(args, cli, 3);
	uint8_t color = numOrVar(args, cli, 4);

	if (cli->targetWindow) { cli->userWindow->DrawRectangle(x0, y0, x1, y1, color, true); }
	else { cli->appWindow->parent->DrawRectangle(x0, y0, x1, y1, color, true); }
}

void drawline(char* args, CommandLine* cli) {
	
	uint32_t x0 = numOrVar(args, cli, 0);
	uint32_t y0 = numOrVar(args, cli, 1);
	uint32_t x1 = numOrVar(args, cli, 2);
	uint32_t y1 = numOrVar(args, cli, 3);
	uint8_t color = numOrVar(args, cli, 4);

	if (cli->targetWindow) { cli->userWindow->DrawLine(x0, y0, x1, y1, color); }
	else { cli->appWindow->parent->DrawLine(x0, y0, x1, y1, color); }
}

void drawtext(char* args, CommandLine* cli) {

	uint32_t x = numOrVar(args, cli, 0);
	uint32_t y = numOrVar(args, cli, 1);
	uint8_t c = (uint8_t)numOrVar(args, cli, 2);

	uint8_t offset = strlen(argparse(args, 0)) + 
			 strlen(argparse(args, 1)) + 
			 strlen(argparse(args, 2)) + 3;	
	
	for (int i = 0; args[i] != '\0'; i++) { args[i] = args[i+offset]; }
	
	if (cli->targetWindow) { cli->userWindow->PutText(args, x, y, c); }
	else { cli->appWindow->parent->PutText(args, x, y, c); }
	
}


void drawpic(char* args, CommandLine* cli) {

	uint32_t x = numOrVar(args, cli, 1);
	uint32_t y = numOrVar(args, cli, 2);
	uint16_t w = 0;
	uint8_t h = 0;
	uint8_t buf[64000]; //ew

	//if we dont do this, filename cant be recognized
	//strings are just so fun to play with right?
	for (int i = 0; args[i] != '\0'; i++) {
	
		if (args[i] == ' ') { args[i] = '\0'; break; }
	}
	

	uint8_t* ptr = nullptr;
	
	if (cli->filesystem->GetTagFile("compressed", cli->filesystem->GetFileSector(args), ptr)) {
	
		//very expensive operations
		cli->filesystem->Read13H(args, buf, &w, &h, true);
	} else {
		cli->filesystem->Read13H(args, buf, &w, &h, false);
	}
	

	if (cli->targetWindow) { cli->userWindow->FillBuffer(x, y, w, h, buf); }
	else { cli->appWindow->parent->FillBuffer(x, y, w, h, buf); }
}


//********************************************************AyumuScript*****************************************************************

uint32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum) {
	
	char* name = argparse(args, argNum);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] != 0xffffffff) { return cli->varTable[hashVar];

	} else if (name[0] == '$' && name[2] == '\0') {

		//given arguments to script
		if (name[1] <= '9' && name[1] >= '0') { return cli->argTable[name[1]-'0']; }

		//value returned from most recent command
		if (name[1] == 'R') { return cli->returnVal; }

	//chars
	} else if (name[0] == '@' && name[2] == '\0') { return (uint8_t)(name[1]);
	
	//input	
	} else if (strcmp("$KEY_CHAR", name)) { return (uint8_t)(cli->Key);
	} else if (strcmp("$KEY_PRESS", name)) { return cli->userWindow->keypress; 
	} else if (strcmp("$LEFT_CLICK", name)) { return cli->userWindow->mouseclick; 
	} else if (strcmp("$MOUSE_X", name)) { return cli->MouseX;
	} else if (strcmp("$MOUSE_Y", name)) { return cli->MouseY;
	
	//str2int
	} else { return str2int(name); }
}


//create variable
void varInt(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);

	if (((uint8_t)(name[0] - '0') < 10) || (name[0] == '$') || (name[0] == '@')) {
		
		char* foo = " "; 
		foo[0] = name[0];
	
		cli->PrintCommand("error: name cannot begin with '");
		cli->PrintCommand(foo);
		cli->PrintCommand("'.\n");
		return;
	}

	uint32_t value = numOrVar(args, cli, 1);
	uint16_t hashVar = hash(name) % 1024;
	
	cli->varTable[hashVar] = value;	
}




//read raw int from table
void rint(char* args, CommandLine* cli) {

	uint32_t index = numOrVar(args, cli, 0);
	
	if (index >= 1024) {
	
		cli->PrintCommand("int error: index is greater than size of table (1024).\n");
		return;
	}
	cli->returnVal = cli->varTable[index];
}

//write raw int from table
void wint(char* args, CommandLine* cli) {

	uint32_t index = numOrVar(args, cli, 0);
	uint32_t value = numOrVar(args, cli, 1);

	if (index >= 1024) {
	
		cli->PrintCommand("int error: index is greater than size of table (1024).\n");
		return;
	}
	cli->varTable[index] = value;
}

//destroy raw int from table
void dint(char* args, CommandLine* cli) {

	uint32_t index = numOrVar(args, cli, 0);
	
	if (index >= 1024) {
	
		cli->PrintCommand("int error: index is greater than size of table (1024).\n");
		return;
	}
	cli->varTable[index] = 0xffffffff;
}


//create list
void varList(char* args, CommandLine* cli) {
	
	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % 1024;

	if (((uint8_t)(name[0] - '0') < 10) || (name[0] == '$') || (name[0] == '@')) {
		
		char* foo = " "; 
		foo[0] = name[0];
	
		cli->PrintCommand("error: name cannot begin with '");
		cli->PrintCommand(foo);
		cli->PrintCommand("'.\n");
		return;
	}

	cli->varTable[hashVar] = cli->lists->numOfNodes;
	
	List* list = (List*)(cli->mm->malloc(sizeof(List)));
	new (list) List(cli->mm);
	cli->lists->Push(list);
}

//print each element in list
void printList(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % 1024;

	if (cli->varTable[hashVar] >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}

	List* list = (List*)(cli->lists->Read(cli->varTable[hash(name) % 1024]));	
	Node* node = list->entryNode;

	for (int i = 0; i < list->numOfNodes; i++) {
	
		cli->PrintCommand(int2str(*(uint32_t*)(node->value)));
		cli->PrintCommand("\n");
		node = node->next;
	}
}


//add to list
void insertList(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(cli->varTable[hashVar]));	

	uint32_t num = numOrVar(args, cli, 1);
	uint32_t index = numOrVar(args, cli, 2);


	uint32_t* newNum = (uint32_t*)(cli->mm->malloc(sizeof(uint32_t)));
	new (newNum) uint32_t;
	*newNum = num;
		
	if (argcount(args) < 3) { list->Push(newNum);
	} else { list->Insert(newNum, index); }
}


//remove from list
void removeList(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(cli->varTable[hashVar]));	

	uint32_t index = numOrVar(args, cli, 1);

	if (argcount(args) < 2) { list->Pop(); }
	else { list->Remove(index); }
}


//read from list index and save into variable
void readList(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(cli->varTable[hashVar]));

	uint32_t index = numOrVar(args, cli, 1);
	uint32_t* value = (uint32_t*)(list->Read(index));

	//save to variable
	char* varName = argparse(args, 2);
	uint16_t readHashVar = hash(varName) % 1024;
	
	if (argcount(args) > 2) { cli->varTable[readHashVar] = *value; }
	cli->returnVal = *value;
}

//write to preexisting node
void writeList(char* args, CommandLine* cli) {

	char* name = argparse(args, 0);
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}
	List* list = (List*)(cli->lists->Read(cli->varTable[hashVar]));
	
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
	uint16_t hashVar = hash(name) % 1024;
	
	if (cli->varTable[hashVar] >= cli->lists->numOfNodes) {
	
		cli->PrintCommand("List not found.\n");
		return;
	}

	//if you remove list that was not most recently
	//allocated, lists index in hash table will be fucked	
	uint32_t index = cli->varTable[hashVar];	
	List* list = (List*)(cli->lists->Read(index));
	list->DestroyList();
	cli->lists->Remove(index);
	cli->varTable[hashVar] = 0xffffffff;
}


//comments
void comment(char* args, CommandLine* cli) {
}


//conditionals

bool trueOrFalse(char* op, uint32_t arg1, uint32_t arg2, CommandLine* cli) {

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


// if and loop statements

void If(char* args, CommandLine* cli) {

	uint32_t arg1 = numOrVar(args, cli, 0);
	
	//not enough args, just eval first arg
	if (argcount(args) < 3) {
	
		cli->conditionIf = (arg1 > 0);
		return;
	}
	
	uint32_t arg2 = numOrVar(args, cli, 2);
	char* op = argparse(args, 1);

	cli->conditionIf = trueOrFalse(op, arg1, arg2, cli);
}
void Else(char* args, CommandLine* cli) { cli->conditionIf ^= 1; }
void Fi(char* args, CommandLine* cli) { cli->conditionIf = true; }



void loop(char* args, CommandLine* cli) {

	uint32_t arg1 = numOrVar(args, cli, 0);
	
	//not enough args, just eval first arg
	if (argcount(args) < 3) {
	
		cli->conditionLoop = (arg1 > 0);
		return;
	}
	
	uint32_t arg2 = numOrVar(args, cli, 2);
	char* op = argparse(args, 1);
	cli->conditionLoop = trueOrFalse(op, arg1, arg2, cli);
}
void pool(char* args, CommandLine* cli) { cli->conditionLoop = true; }


uint32_t mathCMD(char* args, CommandLine* cli, uint8_t op) {

	uint32_t result = 0;
	uint32_t num = 0;

	for (int i = 0; i < (argcount(args)-1); i++) {

		num = numOrVar(args, cli, i+1);

		if (!i) { 
			result += num;
		} else {
			switch (op) {
		
				case 0: result += num; break;
				case 1: result -= num; break;
				case 2: result *= num; break;
				
				//div by 0 lol
				case 3: 
					if (num) { result /= num; }
					break;
				
				case 4: 
					if (num) { result %= num; }
					break;
				
				case 5: result &= num; break;
				case 6: result |= num; break;
				case 7: result ^= num; break;
				default:break;
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


void trig_sin(char* args, CommandLine* cli) {

	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	uint32_t num = numOrVar(args, cli, 0);
	uint32_t amp = numOrVar(args, cli, 1);
	if (amp < 1) { amp = 1; }
	
	uint32_t result = (uint32_t)(sin((double)(num)) * (double)amp);
	cli->varTable[hashVar] = result;
	cli->returnVal = result;
}

void trig_cos(char* args, CommandLine* cli) {
	
	uint16_t hashVar = hash(argparse(args, 0)) % 1024;
	uint32_t num = numOrVar(args, cli, 0);
	uint32_t amp = numOrVar(args, cli, 1);
	if (amp < 1) { amp = 1; }
	
	uint32_t result = (uint32_t)(cos((double)(num)) * (double)amp);
	cli->varTable[hashVar] = result;
	cli->returnVal = result;
}



//execute script
void ex(char* args, CommandLine* cli) {

	char* file = argparse(args, 0);
	uint32_t fileSector = cli->filesystem->GetFileSector(file);

	if (cli->filesystem->FileIf(fileSector)) {

		//parse script arguments
		for (int i = 0; i < 10; i++) {
		
			cli->argTable[i] = str2int(argparse(args, i+1));
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
		for (i = 0; i < 10; i++) {
		
			cli->argTable[i] = str2int(argparse(args, i+1));
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
		cli->CreateTaskForScript(file);
		return;
	}
	cli->PrintCommand("Script file wasn't found.\n");
}

//execute binary
void exb(char* args, CommandLine* cli) {

	char* file = argparse(args, 0);
	uint32_t fileSector = cli->filesystem->GetFileSector(file);
	
	if (cli->filesystem->FileIf(fileSector) == false) {
	
		cli->PrintCommand("Executable wasn't found.\n");
		return;
	}
	
	//read from file and load into memory
	uint8_t binData[OFS_BLOCK_SIZE];
	cli->filesystem->ReadLBA(file, binData, 0);
	
	
	//allocate memory for code
	uint16_t codeSize = (binData[0] << 8) | binData[1];
	uint16_t instructionCount = ((binData[2] << 8) | binData[3]);
	uint32_t entrypoint = (uint32_t)(cli->code);

	//uint8_t* code = (uint8_t*)(cli->mm->malloc(sizeof(uint8_t)*codeSize));
	//new (code) uint8_t;
	
	cli->PrintCommand("Size of code: ");
	cli->PrintCommand(int2str(codeSize));
	cli->PrintCommand(" bytes\n");
	cli->PrintCommand("Number of instructions: ");
	cli->PrintCommand(int2str(instructionCount));
	cli->PrintCommand("\nEntrypoint: ");
	cli->PrintCommand(int2str(entrypoint));
	cli->PrintCommand("\n");
	

	//load program code into memory
	for (int i = 0; i < codeSize; i++) { cli->code[i] = binData[i+4]; }
	
	//allocate and add task
	Task* task = (Task*)(cli->mm->malloc(sizeof(Task)));
	new (task) Task(cli->gdt, (void(*)())(entrypoint), "bin generic", instructionCount);
	//new (task) Task(cli->gdt, (void(*)())(code), "bin generic", instructionCount);

	//yes this is a memory leak, the loader isn't really 
	//being used seriously for now so fuck it, 12 bytes of
	//unfreed memory isnt going to kill you relax


	cli->tm->AddTask(task);
}

//print registers of previous task
void regPrint(char* args, CommandLine* cli) {

	cli->PrintCommand("EAX: ");   cli->PrintCommand(int2str(cli->tm->eaxPrint));
	cli->PrintCommand("\nEBX: "); cli->PrintCommand(int2str(cli->tm->ebxPrint));
	cli->PrintCommand("\nECX: "); cli->PrintCommand(int2str(cli->tm->ecxPrint));
	cli->PrintCommand("\nEDX: "); cli->PrintCommand(int2str(cli->tm->edxPrint));

	cli->PrintCommand("\nESP: "); cli->PrintCommand(int2str(cli->tm->espPrint));
	cli->PrintCommand("\nEBP: "); cli->PrintCommand(int2str(cli->tm->ebpPrint));
	cli->PrintCommand("\n");
}


//********************************************************END*****************************************************************




//misc. commands
void version(char* args, CommandLine* cli) {

	cli->PrintCommand("osakaOS v2.1\n");
	cli->PrintCommand("Copyleft, Leechplus Software\n");
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


void azufetch(char* args, CommandLine* cli) {

	uint16_t previousColor = setTextColor(false);

	//system info
	uint32_t memory = ((cmosDetectMemory()*4)/1024/1024)+1;
	

	//colors for text/vga
	uint8_t w, p, b, r = 0; //white, pink, blue, red
	if (cli->gui) { w = 0x3f, p = 0x3d, b = 0x39, r = 0x3c;
	} else { 	w = 0x0f, p = 0x0d, b = 0x09, r = 0x0c; }
	
	cli->PrintCommand("\n  A-Z-U-F-E-T-C-H\n", w);
	cli->PrintCommand("   ____________   \n", p);
	cli->PrintCommand("  /------------\\   ", p); cli->PrintCommand("OS:  ", b); cli->PrintCommand("osakaOS\n", w);
	cli->PrintCommand(" /---________---\\  ", p); cli->PrintCommand("CPU: ", r); cli->PrintCommand("486 (harcoded btw)\n", w);
	cli->PrintCommand("/---/-v-/\\-v-\\---\\ ", p); cli->PrintCommand("MEM: ", b); cli->PrintCommand(int2str(memory), w); cli->PrintCommand("MB of memory\n", w);
	cli->PrintCommand("|--/-/ v  v \\-\\--| ", p); cli->PrintCommand("VGA: ", r); 
	if (cli->gui == false) { cli->PrintCommand("80x25 Textmode\n", w);} else { cli->PrintCommand("320x200 Mode 13h\n", w); }
	cli->PrintCommand("|--|v        v|--|\n", p);
	cli->PrintCommand("|--|  O    O  |--|\n", p);
	cli->PrintCommand("|--|   ____   |--|\n", p);
	cli->PrintCommand("|--\\_  \\__/  _/--|\n", p);
	cli->PrintCommand("|----\\      /----|\n", p);
	cli->PrintCommand("|-----|    |-----|\n", p);
	cli->PrintCommand("|-v--/\\____/\\--v-|\n", p);
	cli->PrintCommand(" v v/@@@@@@@@\\v v \n", p);

	//set previous color back
	setTextColor(true, previousColor);
}


void date(char* args, CommandLine* cli) {

	cli->cmos->ReadRTC();
	cli->cmos->DumpRTC();
}


void beep(char* args, CommandLine* cli) {

	uint32_t freq = numOrVar(args, cli, 0);
	
	if (freq < 1) {
	
		cli->PrintCommand("No valid frequency was passed.\n");
		return;
	}
	makeBeep(freq);
}


void rng(char* args, CommandLine* cli) {

	uint32_t mod = str2int(argparse(args, 0));
	uint32_t prngNum = prng();
	
	if (mod > 0) { prngNum %= mod; }

	cli->PrintCommand(int2str(prngNum));
	cli->PrintCommand("\n");
	cli->returnVal = prngNum;
}


void PANIC(char* args, CommandLine* cli) {

	Speaker speaker;

	if (cli->gui == false) {
		
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
			cli->cmos->pit->sleep(700);
		
			cli->PrintCommand("\v");
			speaker.NoSound();
			cli->cmos->pit->sleep(700);
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

	if (strcmp("all", argparse(args, 0))) {
	
		for (int i = 0; i < 1024; i++) {
		
			cli->varTable[i] = 0xffffffff;
			cli->conditionIf = true;
			cli->conditionLoop = true;
		}
	}
}


void osaka(char* args, CommandLine* cli) {

	if (cli->gui == false) {

		uint32_t value = numOrVar(args, cli, 0);
		printOsaka(value, 0);
	} else {
		cli->appWindow->Print("This command is only available in text mode.\n");
	}
}


void dad(char* args, CommandLine* cli) {

	if (cli->gui == false) {

		printf("\v");
		cat();
	
		cli->cmos->pit->sleep(3000);
	
		printf("\v");
		god();
	} else {
		cli->appWindow->Print("This command is only available in text mode.\n");
	}
}


CommandLine::CommandLine(GlobalDescriptorTable* gdt, 
			TaskManager* tm, 
			MemoryManager* mm,
			FileSystem* filesystem,
			Compiler* compiler,
			VideoGraphicsArray* vga,
			CMOS* cmos,
			DriverManager* drvManager) {
	
	this->appType = 1;
	
	this->vga = vga;
	this->gdt = gdt;
	this->tm = tm;
	this->mm = mm;
	this->drvManager = drvManager;
	this->cmos = cmos;
	this->filesystem = filesystem;
	this->compiler = compiler;
	
	List* lists = (List*)(mm->malloc(sizeof(List)));
	new (lists) List(mm);
	this->lists = lists;

	this->userTask = nullptr;

	//init bin code
	for (int i = 0; i < 2048; i++) { this->code[i] = 0x00; }


	//init variables for scripts
	for (int i = 0; i < 1024; i++) {
			
		this->varTable[i] = 0xffffffff;
		if (i < 10) { this->argTable[i] = 0; }
	}
	
}

CommandLine::~CommandLine() {

	Task* freeTask = this->userTask;
	this->mm->free(freeTask);
	this->userTask = nullptr;
}



void CommandLine::SaveOutput(char* filename, CompositeWidget* widget, FileSystem* filesystem) {
}
void CommandLine::ReadInput(char* filename, CompositeWidget* widget, FileSystem* filesystem) {

	//when reading in file to command line 
	//execute as multitasking script
	ext(filename, this);
}


void CommandLine::CreateTaskForScript(char* fileName) {

	Task* userTask = (Task*)this->mm->malloc(sizeof(Task));
	
	new (userTask) Task(gdt, &UserScript, fileName, 0);
	this->userTask = userTask;
	this->tm->AddTask(this->userTask);
}


void CommandLine::DeleteTaskForScript(uint8_t taskNum) {

	if (this->userTask != nullptr) {
	
		this->mm->free(this->userTask);
		this->userTask = nullptr;
	}
}



void CommandLine::hash_add(char* cmd, void func(char*, CommandLine* cli)) {

	uint16_t hashIndex = hash(cmd);

	while (CommandLine::cmdTable[hashIndex] != nullptr) {
		
		hashIndex++;
	}
	CommandLine::cmdTable[hashIndex] = func;
}


void CommandLine::hash_cli_init() {

	for (int i = 0; i < 65536; i++) {
	
		CommandLine::cmdTable[i] = nullptr;
	}


	//compute and add functions 
	//to hash table on boot
	
	//general commands
	this->hash_add("say", say);
	this->hash_add("print", print);
	this->hash_add("textcolor", textcolor);
	this->hash_add("help", help);
	this->hash_add("azufetch", azufetch);
	this->hash_add("clear", clear);
	this->hash_add("tasks", tasks);
	this->hash_add("kill", kill);
	this->hash_add("schedule", schedule);
	this->hash_add("ip", ip);
	this->hash_add("ping", ping);
	this->hash_add("reboot", rebootCMD);
	
	//hw ports and asm
	this->hash_add("wport8",  PortWrite8);
	this->hash_add("rport8",  PortRead8);
	this->hash_add("wport16", PortWrite16);
	this->hash_add("rport16", PortRead16);
	this->hash_add("wport32", PortWrite32);
	this->hash_add("rport32", PortRead32);

	this->hash_add("startINT", startInterrupts);
	this->hash_add("stopINT", stopInterrupts);
	this->hash_add("halt", halt);
	
	this->hash_add("asm", assembler);


	//misc.
	this->hash_add("mute", muteNonErrors);
	this->hash_add("delay", delay);
	this->hash_add("sleep", userSleep);
	this->hash_add("rng", rng);
	this->hash_add("osaka", osaka);
	this->hash_add("dad", dad);
	this->hash_add("date", date);
	this->hash_add("beep", beep);
	this->hash_add("PANIC", PANIC);
	this->hash_add("wmem", wmem);
	this->hash_add("rmem", rmem);
	this->hash_add("heap", heap);

	//disk/file
	this->hash_add("wdisk", wdisk);
	this->hash_add("rdisk", rdisk);
	this->hash_add("ofs", ofs);
	this->hash_add("files", files);
	this->hash_add("copy", copy);
	this->hash_add("tag", tag);
	this->hash_add("detag", detag);
	this->hash_add("cat", catFile);
	this->hash_add("size", size);
	this->hash_add("create", createFile);
	this->hash_add("delete", deleteFile);
	this->hash_add("encrypt", encrypt);
	this->hash_add("decrypt", decrypt);

	//vga/graphical
	this->hash_add("terminal", terminal);
	this->hash_add("kasugapaint", kasugapaint);
	this->hash_add("journal", journal);
	this->hash_add("window", window);
	this->hash_add("shortcut", shortcut);
	this->hash_add("targetgui", targetgui);
	this->hash_add("putpixel", putpixel);
	this->hash_add("drawrect", drawrect);
	this->hash_add("drawline", drawline);
	this->hash_add("drawtext", drawtext);
	this->hash_add("drawpic", drawpic);
	this->hash_add("vga", vgaPalette);
	this->hash_add("version", version);

	
	//used for binary programs
	this->hash_add("exb", exb);
	this->hash_add("reg", regPrint);

	
	//special commands for scripting
	this->hash_add("ex", ex);
	this->hash_add("ext", ext);
	
	this->hash_add("int", varInt);
	this->hash_add("rint", rint);
	this->hash_add("wint", wint);
	this->hash_add("dint", dint);

	this->hash_add("list", varList);
	this->hash_add("prlist", printList);
	this->hash_add("insert", insertList);
	this->hash_add("remove", removeList);
	this->hash_add("rlist", readList);
	this->hash_add("wlist", writeList);
	this->hash_add("delist", destroyList);

	this->hash_add("//", comment);

	this->hash_add("if", If);
	this->hash_add("else", Else);
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
	this->hash_add("sin", trig_sin);
	this->hash_add("cos", trig_cos);

	this->hash_add("offset", offsetptr);
	
	this->hash_add("sata", sata);
}


//cli for desktop
void CommandLine::PrintCommand(char* str, uint16_t color) {

	if (this->mute) { return; }

	if (color) {
		if (gui) { this->appWindow->textColor = setTextColor(true, color); }
		else { setTextColor(true, color); }
	}
	
	if (gui) { 
		if (this->targetWindow) { this->userWindow->Print(str);
		} else {		  this->appWindow->Print(str); }
	} else { 
		printf(str); 
	}
}


void CommandLine::OnKeyDown(char ch, CompositeWidget* widget) {

	int i = 0;

	switch (ch) {

		//autofill last command
		case '\xfd':
			for (i; i < 256; i++) { input[i] = lastCmd[i]; }
			for (i = 0; input[i] != '\0'; i++) { widget->PutChar(input[i]); }
			widget->PutChar(128);
			break;
		//empty command
		case '\xfe':
			for (i; input[i] != '\0'; i++) { 
				
				input[i] = 0x00; 
				widget->PutChar('\b');
			}
			widget->PutChar(128);
			break;
		//send command
		case '\n':
			widget->PutChar('\b');
			widget->PutChar('\n');
			
			//process command
			input[index] = '\0';
			
			//save last command
			for (i; i < 256; i++) { lastCmd[i] = input[i]; }

			//execute command
			if (index > 0 && input[0] != ' ') {
				
				this->command(input, index);
			}
			input[0] = 0x00;
			index = 0;
				
			//shell interface
			widget->textColor = 0x24;
			widget->PutChar('$');
			widget->textColor = setTextColor(false);
			widget->PutChar(':');
			widget->PutChar(' ');
			break;
		//backspace
		case '\b':
			if (index > 0) {
				
				index--;
				input[index] = 0x00;
				widget->PutChar('\b');
				widget->PutChar('\b');
				widget->PutChar(128);
			}
			break;
		//input
		default:
			//reject non ascii keys
			if (ch < 32 || ch >= 127) { return; }

			//take input
			input[index] = widget->keyCharWidget;
			widget->PutChar('\b');
			widget->PutChar(widget->keyCharWidget);
			index++;
			
			widget->PutChar(128);
			break;
	}
}
void CommandLine::OnKeyUp(char ch, CompositeWidget* widget) {
}
void CommandLine::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {

	widget->Dragging = true;
}


void CommandLine::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {

	if (this->init == false) {
		
		this->appWindow = widget;
		this->gui = true;
		
		setTextColor(true, 0x3f);
		widget->textColor = 0x24;
		widget->PutChar('$');
		widget->textColor = 0x3f;
		widget->PutChar(':');
		widget->PutChar(' ');

		//execute wakeup script
		if ((CommandLine::WakeupInit) == false) {
		
			if (this->filesystem->FileIf(this->filesystem->GetFileSector("wakeup"))) {
			
				this->command("ex wakeup", strlen("ex wakeup"));
			}
			CommandLine::WakeupInit = true;
		}

		this->init = true;
	}
	App::ComputeAppState(gc, widget);
}


char* CommandLine::command(char* cmd, uint8_t length) {

	char* command = cmd;
	char arguments[length];
	arguments[0] = '\0';

	bool args = false;
	bool pipe = false;
	uint8_t argLength = 0;
	uint8_t cmdLength = length;
	

	for (uint8_t i = 0; i < length; i++) {
		
		if (command[i] == ' ' && argLength == 0) {
			
			argLength = length - i;
			args = (argLength > 0);
			
			cmdLength = i;
		}

		if (args) {
			
			pipe = (cmd[i] == '!' && cmd[i-1] != '\\');
			
			if (pipe) {

				uint8_t j = 0;
				uint8_t recursivePipeOffset = 0;
				char pipeCommand[argLength];

				for (j; cmd[i+j+1] != '\0'; j++) {

					pipeCommand[j] = cmd[i+j+1];

					if (pipeCommand[j] == '!' && recursivePipeOffset == 0) {

						recursivePipeOffset = j;
					}
				}

				//recursively execute piped command
				pipeCommand[j] = '\0';
				this->command(pipeCommand, j);
				break;
			}

			//arguments for command
			arguments[i - (cmdLength + 1)] = cmd[i];
		}
	}
	command[cmdLength] = '\0';
	arguments[argLength-1] = '\0';


	uint16_t result = hash(command);
	
	
	if (this->conditionIf == false && result != hash("fi")) { return "i dont care anymore its 4am lol"; }
	//if (result != hash("else")) { return "i dont care anymore its 4am lol"; }
	if (this->conditionLoop == false && result != hash("pool")) { return "i dont care anymore its 4am lol"; }


	//actual command found
	if (this->cmdTable[result] != nullptr)  {	
	
		(*CommandLine::cmdTable[result])(arguments, this); //execute function from array
		arguments[0] = '\0';
	} else {
		//variable
		if (this->varTable[result % 1024] != 0xffffffff) {
			
			if (mute) { return cmd; }
				
			PrintCommand(int2str(this->varTable[result % 1024]));
			PrintCommand("\n");
				
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
