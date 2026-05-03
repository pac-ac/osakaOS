#include <cli.h>
#include <script.h>


using namespace os;
using namespace os::gui;
using namespace os::net;
using namespace os::math;
using namespace os::drivers;
using namespace os::common;
using namespace os::filesystem;
using namespace os::hardwarecommunication;


//the entire command line of osakaOS is stored here
void (*CommandLine::cmdTable[CMD_TABLE_SIZE])(char* str, CommandLine* cli);
bool CommandLine::WakeupInit = false;


//kernel.cc functions used for everything
uint16_t hash(char* str);
	
void TUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void putcharTUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
uint16_t setTextColor(bool set, uint16_t color = WAAAAAA);
void printf(char*);
void printOsaka(uint8_t, bool);

void makeBeep(uint32_t freq);

uint32_t prng();

void reboot();
//void sleep(uint32_t);

void memWrite(uint32_t, uint32_t);
uint32_t memRead(uint32_t);
uint32_t cmosDetectMemory();

uint32_t FileList(CommandLine* cli);

int32_t numOrVar(char* args, CommandLine* cli, uint8_t argNum);		

uint8_t* memset(uint8_t*, int, size_t);

CommandLine* LoadScriptForTask(bool set, CommandLine* cli = 0);




void say(char* args, CommandLine* cli) {

	cli->PrintCommand(strOrVar(args, cli, -1));
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
	cli->returnVar->typeInt = cli->tm->numTasks;
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
	cli->DeleteTaskForScript();

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
	
	cli->returnVar->typeInt = val;
	
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
	cli->returnVar->typeInt = port8.Read();
}
void PortWrite8(char* args, CommandLine* cli) {

	Port8Bit port8(str2int(argparse(args, 0)));
	port8.Write(str2int(argparse(args, 1)));
}
void PortRead16(char* args, CommandLine* cli) {

	Port16Bit port16(str2int(args));
	cli->returnVar->typeInt = port16.Read();
}
void PortWrite16(char* args, CommandLine* cli) {

	Port16Bit port16(str2int(argparse(args, 0)));
	port16.Write(str2int(argparse(args, 1)));
}
void PortRead32(char* args, CommandLine* cli) {

	Port32Bit port32(str2int(args));
	cli->returnVar->typeInt = port32.Read();
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
	
	cli->returnVar->typeInt = cli->filesystem->table->fileCount;
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
		skip = (cli->filesystem->GetTagFile("sys", location, &tagNum) == location) && !(strcmp("sys", args));


		//print information
		if (!skip) {
			
			cli->PrintCommand(int2str(location));
			cli->PrintCommand("    ");
			cli->PrintCommand(name, W0000FF);
			cli->PrintCommand("    ");
	
			for (int j = 0; j < 8; j++) {
		
				cli->filesystem->GetFileTag(name, j, tag);
				cli->PrintCommand(tag, W00AA00);
				cli->PrintCommand(" ");
			}
			cli->PrintCommand("\n", previousColor);
			fileCount++;
		}
	}
	//print file count and give
	//return value of file count
	char* strNum = int2str(fileCount);
	cli->returnVar->typeInt = fileCount;

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
	cli->returnVar->typeInt = size;

	
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
	//char* fileName = strOrVar(args, cli, 0);
	int i = 0;
	char fileName[33];
	for (i; i < strlen(argparse(args, 0)); i++) { fileName[i] = args[i];}
	fileName[i] = '\0';


	char* dataStr = strOrVar(args, cli, 1);

	//write data from list to file
	uint8_t data[OFS_BLOCK_SIZE];
	memset(data, 0x00, OFS_BLOCK_SIZE);

	if (dataStr != nullptr) {
		
		for (int i = 0; i < strlen(dataStr) && i < OFS_BLOCK_SIZE; i++) { 
		
			data[i] = dataStr[i];
		}
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
	for (int j = i+1; j < strlen(argparse(args, 1)); j++) { 
		/*
		char hashStr[2];
		hashStr[0] = args[i+j+1];
		hashStr[1] = args[i+j+2];
		
		
		
		uint16_t hashVal = hash(hashStr);
		key[j] = ; 
		*/
		key[j] = (uint8_t)args[i+j+1]; 
	}
	
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
		cli->PrintCommand(ip2str(cli->network->ip));
		cli->PrintCommand("\n");
	
		cli->PrintCommand("GATEWAY: ");
		cli->PrintCommand(ip2str(cli->network->gateway));
		cli->PrintCommand("\n");
		
		cli->PrintCommand("SUBNET: ");
		cli->PrintCommand(ip2str(cli->network->subnet));
		cli->PrintCommand("\n");
	}
}

void arp(char* args, CommandLine* cli) {

	cli->network->arp->BroadcastMACAddress(cli->network->gateway);
}

void ping(char* args, CommandLine* cli) {

	uint32_t ip = str2ip(args);
	cli->PrintCommand("Pinging ");
	cli->PrintCommand(ip2str(ip));
	cli->PrintCommand("...\n");

	if (argcount(args) < 2) {
	
		cli->network->icmp->RequestEchoReply(ip, nullptr, 0);
	} else {
		//shift string
		char* cmp = argparse(args, 0);
		uint8_t offset = 0;

		for (offset; cmp[offset] != '\0'; offset++) {} offset++;
		for (int i = 0; args[i] != '\0'; i++) {
	
			args[i] = args[i+offset];
		}
		cli->network->icmp->RequestEchoReply(ip, (uint8_t*)args, strlen(args));
	}
}

void control(char* args, CommandLine* cli) {

	cli->network->activateServerCMD ^= 1;
	
	if (cli->network->activateServerCMD) { cli->PrintCommand("Command server mode activated.\n");
	} else {				cli->PrintCommand("Command server mode deactivated.\n"); }
}




void web(char* args, CommandLine* cli) {
		
	if (strcmp(argparse(args, 0), "start")) {

		if (cli->network->activateServerHTTP == true) {
		
			cli->PrintCommand("Web server is already running, relax.\n");
			return;
		}

		//web start 80 index
		uint16_t port = str2int(argparse(args, 1));
		char* indexFile = argparse(args, 2);
		cli->PrintCommand("Starting web server for '");
		cli->PrintCommand(indexFile);
		cli->PrintCommand("' on port ");
		cli->PrintCommand(int2str(port));
		cli->PrintCommand(".\n");


		//read from file
		uint8_t data[OFS_BLOCK_SIZE];
		cli->filesystem->ReadLBA(indexFile, data, 0);
		
		uint16_t responseLength = strlen(RESPONSE_HTTP);
		uint32_t messageLength = 0;
		for (int i = 0; i < OFS_BLOCK_SIZE; i++) {
		
			if (data[i] == 0x00) {
			
				messageLength = i;
				break;
			}
		}

		//allocate and copy data to send
		cli->network->fileDataSize = responseLength+messageLength+2;
		cli->network->fileData = (uint8_t*)cli->mm->malloc(sizeof(uint8_t)*(cli->network->fileDataSize));

		for (int i = 0; i < responseLength+messageLength; i++) {
		
			if (i < responseLength) { cli->network->fileData[i] = RESPONSE_HTTP[i];
			} else {		  cli->network->fileData[i] = data[i-responseLength]; }
		}
		cli->network->fileData[responseLength+messageLength] = '\r';
		cli->network->fileData[responseLength+messageLength+1] = '\n';


		//establish tcp
		//cli->tcpSocket = cli->network->tcp->Listen(80);
		cli->webPort = port;
		cli->tcpSocket = cli->network->tcp->Listen(port);
		cli->network->tcp->Bind(cli->tcpSocket, cli->network);
		//cli->tcpSocket->poll = false;
		cli->network->activateServerHTTP = true;
	
		cli->PrintCommand("Listening for connections.\n");
		cli->PrintCommand("\nMake sure to set up port forwarding for internet access.\n\n");

	} else if (strcmp(argparse(args, 0), "stop")) {

		//kill server	
		if (cli->tcpSocket != nullptr) {
			
			cli->network->tcp->Disconnect(cli->tcpSocket);
			cli->network->activateServerHTTP = false;
		}

		//free file data
		if (cli->network->fileData != nullptr) {
		
			cli->mm->free(cli->network->fileData);
			cli->network->fileData = nullptr;
			cli->network->fileDataSize = 0;
		}
	}
}




void udp(char* args, CommandLine* cli) {

	if (strcmp(argparse(args, 0), "send")) {

		if (cli->udpSocket != nullptr) {
		
			cli->network->udp->Disconnect(cli->udpSocket);
			cli->udpSocket = nullptr;
		}
			
		uint32_t ip = str2ip(argparse(args, 1));
		uint16_t port = str2int(argparse(args, 2));
		cli->udpSocket = cli->network->udp->Connect(ip, port);
		cli->network->udp->Bind(cli->udpSocket, cli->network);
		cli->udpSocket->Send((uint8_t*)(args+5), strlen(args)-5);

	} else if (strcmp(argparse(args, 0), "listen")) {
	
		uint32_t port = str2ip(argparse(args, 1));
		cli->udpSocket = cli->network->udp->Listen(port);
		cli->network->udp->Bind(cli->udpSocket, cli->network);
		cli->PrintCommand("Listening on port ");
		cli->PrintCommand(argparse(args, 1));
		cli->PrintCommand(".\n");
	} else {
		cli->PrintCommand("1st arg must be 'send' or 'listen'.\n");	
	}
}

void tcp(char* args, CommandLine* cli) {

	if (strcmp(argparse(args, 0), "connect")) {
		
		if (cli->tcpSocket != nullptr) {
		
			cli->network->tcp->Disconnect(cli->tcpSocket);
			cli->tcpSocket = nullptr;
		}
	
		uint32_t ip = str2ip(argparse(args, 1));
		uint16_t port = str2int(argparse(args, 2));
		cli->tcpSocket = cli->network->tcp->Connect(ip, port);
		cli->network->tcp->Bind(cli->tcpSocket, cli->network);

	} else if (strcmp(argparse(args, 0), "listen")) {
		
		uint16_t port = str2int(argparse(args, 1));
		cli->tcpSocket = cli->network->tcp->Listen(port);
		cli->network->tcp->Bind(cli->tcpSocket, cli->network);
		cli->PrintCommand("Listening on port ");
		cli->PrintCommand(argparse(args, 1));
		cli->PrintCommand(".\n");
	
	} else if (strcmp(argparse(args, 0), "send")) {

		if (cli->tcpSocket == nullptr) {
		
			cli->PrintCommand("Send where? There's no connection.\n");
		}

		cli->PrintCommand("Sending TCP data...\n");
		cli->tcpSocket->Send((uint8_t*)(args+5), strlen(args)-5);
	
	} else if (strcmp(argparse(args, 0), "disconnect")) {
	
		if (cli->tcpSocket != nullptr) {
		
			cli->network->tcp->Disconnect(cli->tcpSocket);
		}
	} else {
		cli->PrintCommand("1st arg must be 'connect', 'send', 'listen', or 'disconnect'.\n");	
	}
}



void download2file(char* args, CommandLine* cli) {

	createFile(args, cli);
	
	int i = 0;
	for (i; args[i] != '\0' && i < 32; i++) {
	
		cli->externalFile[i] = args[i];
	}
	cli->externalFile[i] = '\0';

	cli->downloadExternalFile = true;

	cli->PrintCommand("Ready to download network data.\n");
	cli->PrintCommand("Remember to do 'tcp listen [port]'.\n");
}


//graphical commands
void terminal(char* args, CommandLine* cli) {
	
	if (cli->gui) { cli->appWindow->parent->CreateChild(APP_TYPE_TERMINAL, "Osaka's Terminal", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}


void kasugapaint(char* args, CommandLine* cli) {
	
	if (cli->gui) { cli->appWindow->parent->CreateChild(APP_TYPE_KASUGAPAINT, "KasugaPaint", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}

void journal(char* args, CommandLine* cli) {
	
	if (cli->gui) { cli->appWindow->parent->CreateChild(APP_TYPE_JOURNAL, "Journal", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}

void shinosaka(char* args, CommandLine* cli) {

	if (cli->gui) { cli->appWindow->parent->CreateChild(APP_TYPE_SHINOSAKA, "Shinosaka", 0); }
	else { cli->PrintCommand("This command is not available in text mode.\n"); }
}


void window(char* args, CommandLine* cli) {
		
	if (cli->gui && cli->userWindow == nullptr) {
	
		if (strlen(args) < 2) { cli->userWindow = cli->appWindow->parent->CreateChild(0, "OSaka Window", cli); }
		else { cli->userWindow = cli->appWindow->parent->CreateChild(APP_TYPE_SCRIPT, args, cli); }

		cli->targetWindow = true;
	} else {
		if (cli->gui) { cli->PrintCommand("Window is already active.\n"); } 
		else { cli->PrintCommand("This command is not available in text mode.\n"); }
	}
}


void charsetPrint(char* args, CommandLine* cli) {

	char* str = "  ";
	cli->PrintCommand("  0 1 2 3 4 5 6 7 8 9 A B C D E F\n", WBABAFF, TEXT_BOLD);

	for (uint8_t i = 0; i < 16; i++) {
			
		char* row = "  ";
	
		if (i < 10) { row[0] = '0'+i;
		} else {      row[0] = 'A'+(i-10); }
		cli->PrintCommand(row, WBABAFF, TEXT_BOLD);

		for (uint8_t j = 0; j < 16; j++) {
		
			str[0] = (char)((i*16)+j);
			if (str[0] == '\v' || str[0] == '\n') { str[0] = ' '; }
			cli->PrintCommand(str, WFFFFFF);
		}
		cli->PrintCommand("\n");
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


void effect(char* args, CommandLine* cli) {
		
	uint8_t windowID = str2int(argparse(args, 1));
	if (argcount(args) < 2) { windowID = -1;}


	if (strcmp(argparse(args, 0), "wave")) {
		
		if (windowID < 0 || windowID > cli->appWindow->parent->numChildren) {
			cli->appWindow->parent->Wave ^= 1;
		} else {cli->appWindow->parent->children[windowID]->Wave ^= 1;}
	
	} else if (strcmp(argparse(args, 0), "pixel")) {
	
		if (windowID < 0 || windowID > cli->appWindow->parent->numChildren) {
			cli->appWindow->parent->Pixelize ^= 1;
		} else {cli->appWindow->parent->children[windowID]->Pixelize ^= 1; }
	
	} else if (strcmp(argparse(args, 0), "rainbow")) {
	
		if (windowID < 0 || windowID > cli->appWindow->parent->numChildren) {
			cli->appWindow->parent->Rainbow ^= 1;
		} else {cli->appWindow->parent->children[windowID]->Rainbow ^= 1; }
	} else {
		cli->PrintCommand("Unknown effect, use the 'wave', 'pixel', or 'rainbow' args.\n");
	}
}


void cursor(char* args, CommandLine* cli) {

	uint16_t w = 0;
	uint16_t h = 0;
	uint8_t buf[cli->vga->gfxBufferSize];
	for (int i = 0; i < cli->vga->gfxBufferSize; i++) { buf[i] = 0x00; }
	uint8_t* ptr = nullptr;
	bool fileFound = false;

	if (cli->filesystem->GetTagFile("compressed", cli->filesystem->GetFileSector(args), ptr)) {
	
		//very expensive operations
		fileFound = cli->filesystem->Read13H(args, buf, &w, &h, true);
	} else {
		fileFound = cli->filesystem->Read13H(args, buf, &w, &h, false);
	}

	if (fileFound) { cli->appWindow->parent->LoadCursor(buf, w, h); }
	else { 		 cli->appWindow->parent->LoadCursor(nullptr, w, h); }
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
	uint16_t h = 0;
	uint8_t buf[cli->vga->gfxBufferSize]; //ew

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
	task->binary = true;
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




//misc. commands
void version(char* args, CommandLine* cli) {

	cli->PrintCommand("osakaOS v3.0\n");
	cli->PrintCommand("Copyleft, Leechplus Software\n");
}


void help(char* args, CommandLine* cli) {
	
	char* ch = " .\n";
	ch[0] = 1;

	cli->PrintCommand("\nWelcome to osakaOS!\n", WFFFFFF, TEXT_ITALIC);
	cli->PrintCommand("This is the command line interface.\n", WFFFFFF);
	cli->PrintCommand("Please take it easy ", WFFFFFF);
	cli->PrintCommand(ch, WFFFFFF);
	
	if (cli->gui) {
		cli->PrintCommand("\n\nClick on the desktop icons to launch a program.\n", WFFFFFF);
		cli->PrintCommand("  'Terminal'    ", WFF5555, TEXT_BOLD);
		cli->PrintCommand("- system command line.\n", WFFFFFF);
		cli->PrintCommand("  'KasugaPaint' ", W55FF55, TEXT_BOLD);
		cli->PrintCommand("- draw and display images.\n", WFFFFFF);
		cli->PrintCommand("  'Journal'     ", W5555FF, TEXT_BOLD);
		cli->PrintCommand("- edit text based files.\n", WFFFFFF);
		cli->PrintCommand("  'Shinosaka'   ", WAA00AA, TEXT_BOLD);
		cli->PrintCommand("- web utility & browser.\n\n", WFFFFFF);
		
		/*
		cli->PrintCommand("  'Terminal'    - system command line you're using now.\n", WFFFFFF);
		cli->PrintCommand("  'KasugaPaint' - simple paint program to draw and display images.\n", WFFFFFF);
		cli->PrintCommand("  'Journal'     - text editor program for all text based files.\n", WFFFFFF);
		cli->PrintCommand("  'Shinosaka'   - web utility for loading simple HTML pages.\n\n", WFFFFFF);
		*/
	} else {
		cli->PrintCommand("In VGA text mode, use the keyboard shortcuts: \n", WFFFFFF);
		cli->PrintCommand("    ctrl+e - file edit mode (ctrl-w to write file)\n", WFFFFFF);
		cli->PrintCommand("    ctrl+p - piano mode (press keys to play notes)\n", WFFFFFF);
		cli->PrintCommand("    ctrl+s - snake mode (wasd to play snake game)\n", WFFFFFF);
		cli->PrintCommand("    ctrl+c - exit mode (return back to cli)\n", WFFFFFF);
	}

	cli->PrintCommand("If you feel lost, try the following commands: \n", WFFFFFF);
	cli->PrintCommand("  say (str)        ", W0000FF, TEXT_BOLD);
	cli->PrintCommand("- print given text.\n", WFFFFFF);
	cli->PrintCommand("  files            ", W0000FF, TEXT_BOLD);
	cli->PrintCommand("- list all existing files.\n", WFFFFFF);
	cli->PrintCommand("  var (str) (type) ", W0000FF, TEXT_BOLD);
	cli->PrintCommand("- create scripting variable.\n", WFFFFFF);
	cli->PrintCommand("  (variable name)  ", W0000FF, TEXT_BOLD);
	cli->PrintCommand("- display value of variable.\n", WFFFFFF);
	cli->PrintCommand("  ext (file)       ", W0000FF, TEXT_BOLD);
	cli->PrintCommand("- run as AyumuScript program.\n", WFFFFFF);
	cli->PrintCommand("  reboot           ", W0000FF, TEXT_BOLD);
	cli->PrintCommand("- restart the machine.\n\n", WFFFFFF);
	
	cli->PrintCommand("Consult online matieral at youtube.com/@dpacarana.\n", WFFFFFF);
}


void azufetch(char* args, CommandLine* cli) {

	//system info
	uint32_t memory = ((cmosDetectMemory()*4)/VAR_TABLE_SIZE/VAR_TABLE_SIZE)+1;

	//colors for text/vga
	uint8_t w, p, b, r = 0; //white, pink, blue, red
	if (cli->gui) { w = WFFFFFF, p = WFF55FF, b = W5555FF, r = WFF5555;
	} else { 	w = WBABAFF, p = WBE00FF, b = W0000FF, r = WAA0055; }
	
	cli->PrintCommand("\n  ");
	cli->PrintCommand("AZUFETCH\n", w, TEXT_HEADER);
	cli->PrintCommand("   ____________   \n", p);
	cli->PrintCommand("  /------------\\   ", p); cli->PrintCommand("OS:  ", b); cli->PrintCommand("osakaOS\n", w, TEXT_BOLD);
	cli->PrintCommand(" /---________---\\  ", p); cli->PrintCommand("CPU: ", r); cli->PrintCommand("486 (harcoded btw)\n", w, TEXT_UNDERLINE);
	cli->PrintCommand("/---/-v-/\\-v-\\---\\ ", p); cli->PrintCommand("MEM: ", b); cli->PrintCommand(int2str(memory), w); cli->PrintCommand("MB of memory\n", w);
	cli->PrintCommand("|--/-/ v  v \\-\\--| ", p); cli->PrintCommand("VID: ", r);
	if (cli->gui == false) { 	cli->PrintCommand("80x25 Textmode\n", w);} 
	else if (cli->vga->vesa) { 	cli->PrintCommand("640x480 VESA 101h\n", w); }
	else { 				cli->PrintCommand("320x200 Mode 13h\n", w); }
	cli->PrintCommand("|--|v        v|--|\n", p);
	cli->PrintCommand("|--|  O    O  |--|\n", p);
	cli->PrintCommand("|--|   ____   |--|\n", p);
	cli->PrintCommand("|--\\_  \\__/  _/--|\n", p);
	cli->PrintCommand("|----\\      /----|\n", p);
	cli->PrintCommand("|-----|    |-----|\n", p);
	cli->PrintCommand("|-v--/\\____/\\--v-|\n", p);
	cli->PrintCommand(" v v/@@@@@@@@\\v v \n", p);
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
	cli->returnVar->typeInt = prngNum;
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
	
		for (int i = 0; i < VAR_TABLE_SIZE; i++) {
		
			if (cli->varTable[i] != nullptr) {
		
				if (cli->varTable[i]->typeStr != nullptr) {
				
					cli->mm->free(cli->varTable[i]->typeStr);
				}
				if (cli->varTable[i]->typeFunc != nullptr) {
				
					cli->mm->free(cli->varTable[i]->typeFunc);
				}
				cli->mm->free(cli->varTable[i]);
			}
			cli->varTable[i] = nullptr;
		}
		cli->conditionIf = true;
		cli->conditionLoop = true;
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
			Network* network,
			Compiler* compiler,
			VideoGraphicsArray* vga,
			CMOS* cmos,
			DriverManager* drvManager) {
	
	this->appType = APP_TYPE_TERMINAL;
	
	this->vga = vga;
	this->gdt = gdt;
	this->tm = tm;
	this->mm = mm;
	this->drvManager = drvManager;
	this->cmos = cmos;
	this->filesystem = filesystem;
	this->network = network;
	this->compiler = compiler;
	
	List* lists = (List*)(mm->malloc(sizeof(List)));
	new (lists) List(mm);
	this->lists = lists;
	
	
	List* funcStates = (List*)(mm->malloc(sizeof(List)));
	new (funcStates) List(mm);
	this->callStack = funcStates;

	this->userTask = nullptr;

	//init bin code
	for (int i = 0; i < 2048; i++) { this->code[i] = 0x00; }


	//init variables for scripts
	for (int i = 0; i < VAR_TABLE_SIZE; i++) {
			
		this->varTable[i] = nullptr;
		if (i < 10) { this->argTable[i] = nullptr; }
	}

	//create generic type for return val
	int32_t initVal = 0;
	this->returnVar = (Type*)mm->malloc(sizeof(Type));
	new (this->returnVar) Type(TYPE_INT, (void*)&initVal);
}

CommandLine::~CommandLine() {

	Task* freeTask = this->userTask;
	this->mm->free(freeTask);
	this->userTask = nullptr;

	if (this->tcpSocket != nullptr) {
		
		this->network->tcp->Disconnect(this->tcpSocket);
	}
}



void CommandLine::CleanCommandLine() {

	if (returnVar->typeStr != nullptr) { mm->free(returnVar->typeStr); }
	if (returnVar->typeFunc != nullptr) {
					     mm->free(returnVar->typeFunc->args); 
					     mm->free(returnVar->typeFunc); }
	this->mm->free(this->returnVar);

	for (int i = 0; i < this->lists->numOfNodes; i++) {
	
		List* list = (List*)(this->lists->Read(i));
		list->DestroyList();
	}

	for (int i = 0; i < VAR_TABLE_SIZE; i++) {
	
		if (this->varTable[i] != nullptr) {

			if (this->varTable[i]->typeStr != nullptr) {
			
				this->mm->free(this->varTable[i]->typeStr);
			}
			if (this->varTable[i]->typeFunc != nullptr) {
			
				this->mm->free(this->varTable[i]->typeFunc);
			}
			this->mm->free(this->varTable[i]);
		}
	}

	this->lists->DestroyList();
	this->mm->free(this->lists);

	this->callStack->DestroyList();
	this->mm->free(this->callStack);

	this->DeleteTaskForScript();
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


void CommandLine::DeleteTaskForScript() {

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

	for (int i = 0; i < CMD_TABLE_SIZE; i++) {
	
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

	//network
	this->hash_add("ip", ip);
	this->hash_add("arp", arp);
	this->hash_add("ping", ping);
	this->hash_add("udp", udp);
	this->hash_add("tcp", tcp);
	this->hash_add("web", web);
	this->hash_add("control", control);
	this->hash_add("download", download2file);

	//vga/graphical
	this->hash_add("terminal", terminal);
	this->hash_add("kasugapaint", kasugapaint);
	this->hash_add("journal", journal);
	this->hash_add("shinosaka", shinosaka);
	this->hash_add("window", window);
	this->hash_add("shortcut", shortcut);
	this->hash_add("effect", effect);
	this->hash_add("cursor", cursor);
	this->hash_add("charset", charsetPrint);
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
	this->hash_add("input", inputStr);
	
	this->hash_add("var", var);
	this->hash_add("dvar", dvar);

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
	
	this->hash_add("function", function);
	this->hash_add("end", Fi);
	this->hash_add("return", setReturn);
	this->hash_add("=", evalMath);
	
	this->hash_add("offset", offsetptr);
	
	this->hash_add("sata", sata);
}


//cli for desktop
void CommandLine::PrintCommand(char* str, uint16_t color, uint8_t flags) {

	if (this->mute) { return; }

	if (color) {
	
		uint16_t previousColor = setTextColor(false);
		
		if (gui) { this->appWindow->textColor = setTextColor(true, color); }
		else { setTextColor(true, color); }
	
		setTextColor(true, previousColor);
	}
	
	if (gui) { 
		if (this->targetWindow) { this->userWindow->Print(str, flags);
		} else {		  this->appWindow->Print(str, flags); }
	} else { 
		printf(str); 
	}
}


void CommandLine::OnKeyDown(char ch, CompositeWidget* widget) {

	if (this->getCurrentInput) {
	
		switch (ch) {
			
			case '\n':
				this->scriptInput[scriptInputIndex] = '\0';
				scriptInputIndex = 0;
				widget->PutChar('\n');
				this->getCurrentInput = false;
				break;
			case '\b':
				if (scriptInputIndex > 0) {
				
					scriptInputIndex--;
					this->scriptInput[scriptInputIndex] = '\0';
					widget->PutChar('\b');
				}
				break;
			default:
				if (scriptInputIndex < 256) {

					this->scriptInput[scriptInputIndex] = ch;
					scriptInputIndex++;
					widget->PutChar(widget->keyCharWidget);
				}
				break;
		}
	} else {


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
			widget->textColor = WFF0000;
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
		
		setTextColor(true, WFFFFFF);
		widget->textColor = WFF0000;
		widget->PutChar('$');
		widget->textColor = WFFFFFF;
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
				
	
	if (this->tcpSocket != nullptr) {
				
		if (this->tcpSocket->handleType != HANDLE_FLAG_EMPTY) {

			uint32_t packetSize = this->tcpSocket->bufferIndex;
			uint8_t* packetData = this->tcpSocket->handleBuffer;
	
			//command and control server mode
			if (this->network->activateServerCMD == true) {
		
				for (int i = 0; i < this->tcpSocket->bufferIndex; i++) {
			
					if ((this->tcpSocket->handleBuffer[i] < 32 && 
				    		this->tcpSocket->handleBuffer[i] > 127) ||
						this->tcpSocket->handleBuffer[i] == '\n') {
				
						this->tcpSocket->bufferIndex = i;
					}
				}
			
				this->tcpSocket->handleBuffer[this->tcpSocket->bufferIndex] = '\0';
				this->command((char*)this->tcpSocket->handleBuffer, this->tcpSocket->bufferIndex);
			
			//hosting web server
			} else if (this->network->activateServerHTTP == true) {
				
				//print full network packet data to terminal
				char* foo = " ";
				for (int i = 0; i < packetSize; i++) {
		
					foo[0] = (char)(packetData[i]);
					this->PrintCommand(foo);
				}

				//get ready for another requests
				if (this->tcpSocket != nullptr) {
				
					this->mm->free(this->tcpSocket);
				}
				this->tcpSocket = this->network->tcp->Listen(this->webPort);
				this->network->tcp->Bind(this->tcpSocket, this->network);
				

				//this->PrintCommand("\nSending web-file data to client...\n", W5555FF, TEXT_BOLD);
			
			//download external file
			} else if (this->downloadExternalFile == true) {
			
				uint8_t data[OFS_BLOCK_SIZE];
				
				for (int i = 0; i < (packetSize/OFS_BLOCK_SIZE)+1; i++) {
				
					for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
					
						data[j] = packetData[i*OFS_BLOCK_SIZE+j];
					}
					this->filesystem->WriteLBA(this->externalFile, data, i);
				}
				this->downloadExternalFile = false;
			} else {
				//print full network packet data to terminal
				char* foo = " ";
				for (int i = 0; i < packetSize; i++) {
		
					foo[0] = (char)(packetData[i]);
					this->PrintCommand(foo);
				}
			}
			this->network->EmptyHandleBufferTCP(this->tcpSocket);
			this->tcpSocket->handleType = HANDLE_FLAG_EMPTY;
		}
		
		if (this->tcpSocket->connectionFail) {
		
			this->PrintCommand("\nFailed to establish TCP connection", WFF5555, TEXT_BOLD);
			this->tcpSocket->connectionFail = false;
		}
	}
	App::ComputeAppState(gc, widget);
}


//returns nullptr if function call return cmd otherwise
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
	
	
	if (this->conditionIf == false && result != hash("fi") && result != hash("end")) { return "i dont care anymore its 4am lol"; }
	if (this->conditionLoop == false && result != hash("pool")) { return "i dont care anymore its 4am lol"; }


	//actual command found
	if (this->cmdTable[result] != nullptr)  {	
	
		(*CommandLine::cmdTable[result])(arguments, this); //execute function from array
		arguments[0] = '\0';
	} else {
		Type* var = this->varTable[result % VAR_TABLE_SIZE];

		//variable
		if (var != nullptr) {
			
			if (mute) { return cmd; }
			
			switch (var->type) {
			
				case TYPE_INT:
					PrintCommand(int2str(var->typeInt));
					PrintCommand("\n");
					break;
				case TYPE_FLOAT:
					PrintCommand(float2str(var->typeFloat));
					PrintCommand("\n");
					break;
				case TYPE_STRING:
					PrintCommand(var->typeStr);
					PrintCommand("\n");
					break;
				case TYPE_FUNCTION:
					{
						setFunctionArgs(arguments, this, var->typeFunc);
					
						//load and call function manually
						if (this->scriptRunning == false) {
					
							uint32_t size = var->typeFunc->LoadFunction(this->filesystem);
							AyumuScriptCli(var->typeFunc->file, this, var->typeFunc->buf, size);
							return cmd;
						}
						return nullptr;
					}
					break;
				default:break;
			}
				
		//unknown command		
		} else {
			if (length < 0xff) {

				PrintCommand("ERROR: ", WFF0000, TEXT_BOLD);
				PrintCommand("'", WFFFFFF);
				PrintCommand(command, WFFFFFF);
				PrintCommand("' is not a command or variable.\n", WFFFFFF);		
				makeBeep(60);

			} else {
				PrintCommand("No command needs to be this long.\n");		
				makeBeep(1);
			}
		}
	}
	return cmd;
}
