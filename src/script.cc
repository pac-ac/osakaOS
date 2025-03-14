#include <script.h>
#include <gui/desktop.h>

using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;


void printf(char*);
char* argparse(char*, uint8_t);
uint16_t strlen(char*);
bool strcmp(char*, char*);
uint32_t str2int(char*);
char* int2str(uint32_t);
Desktop* LoadDesktopForTask(bool set, Desktop* desktop = 0);
TaskManager* LoadTaskManager(bool set, TaskManager* tm = 0);
//void sleep(uint32_t)

os::CommandLine* LoadScriptForTask(bool set, os::CommandLine* cli = 0);


Script::Script(CommandLine* cli) {

	this->parentCli = cli;
	this->appType = 0;
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

	TaskManager* tm = LoadTaskManager(false);
	CommandLine* cli = LoadScriptForTask(false);
	cli->PrintCommand("\n");
	AyumuScriptCli(cli->scriptName, cli);

	cli->userTask->kill = true;
	while (1) {}
}



//ex
void AyumuScriptCli(char* name, os::CommandLine* cli) {

	//get file info
	uint32_t size = cli->filesystem->GetFileSize(argparse(name, 0));
	uint8_t file[OFS_BLOCK_SIZE];
	uint8_t LBA = 0;
	
	//file indexes
	uint32_t indexF = 0;
	uint16_t indexLBA = 0;
	
	uint32_t fileIndexLoop = 0;

	//store individual command
	char line[256];
	uint16_t indexL = 0;
	bool start = false;
	bool startLoop = false;
	
	//number of loops allowed (16)
	uint16_t nestedLoop[16];
	bool nestedLoopCondition[16];
	
	for (uint8_t i = 0; i < 16; i++) {

		nestedLoop[i] = 0; 
		nestedLoopCondition[i] = false;
	}
	//loop indexes
	uint8_t indexLoopF = 0;

	//input/return pointer
	uint32_t jumpIndexes[8];
	for (uint8_t i = 0; i < 8; i++) { jumpIndexes[i] = size - 1; }
	uint32_t returnPointer = 0;
	uint16_t hashIndex = 0;


	while (indexF < size) {

		//terminate script
		if (cli->scriptKillSwitch) { 
			
			cli->conditionIf = true;
			cli->conditionLoop = true;
			for (int i = 0; i < 10; i++) { cli->argTable[i] = 0; }
			cli->scriptKillSwitch = false;
			return;	
		}

		//read next block
		if (indexF % OFS_BLOCK_SIZE == 0) {

			indexLBA = 0;
			cli->filesystem->ReadLBA(argparse(name, 0), file, LBA);
			LBA++;
		}

		//parse and interpret
		switch (file[indexLBA]) {

			case '\n':
			case '\v':
			case 0x00:
				if (!start) { break; }

				line[indexL] = '\0';
				indexL = 0;
				
				if (strcmp("loop ", line)) {

					//add for every new loop command
					if (indexLoopF < 16) {

						nestedLoop[indexLoopF] = indexLBA - ((strlen(line) + 1));
						indexLoopF++;
					}
					startLoop = true;

				} else if (strcmp("pool", line) && indexLoopF > 0) {

					//outer pool checks inner loop condition instead of outer
					if (nestedLoopCondition[indexLoopF-1] == true) {

						//go back to start of loop if
						//condition is still true
						indexF -= (indexLBA - nestedLoop[indexLoopF-1]);
						indexLBA = nestedLoop[indexLoopF-1];
					} else {
						//remove loop index and
						//proceed with program
						nestedLoop[indexLoopF-1] = 0;
						nestedLoopCondition[indexLoopF-1] = false;
					}
					indexLoopF -= (1 * (indexLoopF > 0));
				} else {
				}

				//execute as command
				cli->command(line, strlen(line));
				
				if (startLoop && indexLoopF > 0) {
				
					nestedLoopCondition[indexLoopF-1] = cli->conditionLoop;
					startLoop = false;
				}
				start = false;
				break;
			default:
				line[indexL] = file[indexLBA];
				indexL++;
				start = true;
				break;
		}
		indexLBA++;
		indexF++;
	}
	//reset conditions and script arguments
	cli->conditionIf = true;
	cli->conditionLoop = true;
	for (int i = 0; i < 10; i++) { cli->argTable[i] = 0; }
	cli->scriptKillSwitch = false;
}
