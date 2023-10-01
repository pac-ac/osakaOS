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












void AyumuScriptCli(char* name, CommandLine* cli) {

	//get file info
	uint32_t size = GetFileSize(name);
	uint8_t file[1920];
	uint8_t LBA = 0;



	char line[256];
	//number of loops allowed (16)
	uint32_t nestedLoop[16];

	uint32_t indexF = 0;
	uint16_t indexLBA = 0;
	uint32_t indexLoopF = 0;
	uint16_t indexL = 0;
	

	bool start = false;
	bool inLoop = false;


	while (indexF < size) {
	
		if (indexF % 1920 == 0) {
		
			indexLBA = 0;
			ReadLBA(name, file, LBA);
			LBA++;
		}

		switch (file[indexLBA]) {
		
			case 0x00:
				if (!start) { break; }
				
				line[indexL] = '\0';
				indexL = 0;


				if (strcmp("loop ", line)) {

					nestedLoop[indexLoopF] = indexLBA - (strlen(line) + 1);
					
					if (!inLoop) {
						//add for every new loop command
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

