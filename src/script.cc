#include <script.h>

using namespace os;
using namespace os::common;
using namespace os::filesystem;


void printf(char*);
char* argparse(char*, uint8_t);
uint16_t strlen(char*);
bool strcmp(char*, char*);
uint32_t str2int(char*);
char* int2str(uint32_t);




//exi
//not working yet
void AyumuScriptInput(CommandLine* cli, uint8_t* file, 
		uint32_t size, uint32_t &indexF, 
		uint32_t* nestedLoop, uint32_t &indexLoopF) {

	char line[256];

	uint16_t indexLBA = 0;
	uint16_t indexL = 0;
	
	bool start = false;
	bool inLoop = false;


	while (indexLBA < 1920) {
		
		switch (file[indexLBA]) {
			
			case 0x00:
				if (!start) { printf("no line\n"); break; }
				line[indexL] = '\0';
				indexL = 0;


				if (strcmp("loop ", line)) {

					nestedLoop[indexLoopF] = indexLBA - (strlen(line) + 1);
					
					if (!inLoop) {
					
						indexLoopF++;
					}
					inLoop = true;
				}
				
				if (strcmp("pool", line)) {
			
					if (cli->conditionLoop) {
						
						indexLBA = nestedLoop[indexLoopF-(1 * (indexLoopF > 0))];
					} else {	
						indexLoopF--;
						inLoop = false;
					}
				}
			
				//execute as command
				cli->command(line, strlen(line));	
				start = false;
				//printf("eol\n");	
				break;
			default:
				//printf("not eol\n");	
				line[indexL] = file[indexLBA];
				indexL++;	
				start = true;
				break;
		}
		indexLBA++;
		indexF++;

		if (!start && indexL > 0) { return; }
	}
}











//ex
void AyumuScriptCli(char* name, CommandLine* cli) {

	//get file info
	uint32_t size = GetFileSize(argparse(name, 0));
	uint8_t file[1920];
	uint8_t LBA = 0;

	//nested loops and if statements currently not supported

	char line[256];
	
	//number of loops allowed (16)
	uint16_t nestedLoop[16];
	for (uint8_t i = 0; i < 16; i++) { nestedLoop[i] = 0; }

	//file indexes
	uint32_t indexF = 0;
	uint16_t indexLBA = 0;
	
	//loop indexes
	uint8_t indexLoopF = 0;
	
	uint32_t fileIndexLoop = 0;
	uint16_t indexL = 0;
	

	bool start = false;
	bool inLoop = false;


	while (indexF < size) {
	
		if (indexF % 1920 == 0) {
		
			indexLBA = 0;
			//not efficient using argparse
			//but it wont work otherwise
			//because im a hack......
			ReadLBA(argparse(name, 0), file, LBA);
			LBA++;
		}

		switch (file[indexLBA]) {
		
			case 0x00:
				if (!start) { break; }
				
				line[indexL] = '\0';
				indexL = 0;
				
				if (strcmp("loop ", line)) {
						
					//add for every new loop command
					if (!inLoop) {
				
						indexLoopF++;
						nestedLoop[1] = indexLBA - (strlen(line) + 1);
					}
					inLoop = true;
				}
				
				if (strcmp("pool", line)) {
			
					if (cli->conditionLoop) {
						
						indexF -= (indexLBA - nestedLoop[1]);
						indexLBA = nestedLoop[1];
						
					} else {	
						indexLoopF--;
						inLoop = false;
					}
				}
			
				//execute as command
				cli->command(line, strlen(line));	
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

	cli->conditionIf = true;
	cli->conditionLoop = true;
}

