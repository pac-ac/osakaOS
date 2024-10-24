#include <mode/file_edit.h>


using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;

void printf(char*);
void putcharTUI(unsigned char, unsigned char, unsigned char, uint8_t, uint8_t);
void TUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void printfTUI(char*, uint8_t, uint8_t, uint8_t, uint8_t);

char* int2str(uint32_t);
void sleep(uint32_t);



void fileTUI() {
 
	TUI(0x07, 0x01, 10, 5, 70, 19, true);
	printfTUI("osakaOS File Editor :)", 0x00, 0x07, 29, 7);
	printfTUI("Search file name up to 32 characters.", 0x00, 0x07, 22, 8);
        printfTUI("Enter file name: ", 0x00, 0x07, 12, 11);
	printfTUI("0123456789abcdef0123456789abcdef", 0x00, 0x00, 29, 11);
}



void file(bool pressed, char key, bool ctrl, bool reset, FileSystem* filesystem) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	static char fileName[32];
	static bool search = true;
	
	static bool start = false;


	//search things		
	static uint8_t index = 0;
	uint32_t clusterNum = 0;
	static char searchStr[32];
	searchStr[index] = '\0';

	
	//edit things
	static uint8_t x = 0;
	static uint8_t y = 0;

	static uint8_t file[1920];
	static uint32_t fileSize = 1920;
	static uint8_t lba = 0;

	static uint8_t copyLine[80];


	if (reset) {
	
		start = false;
		search = true;
		index = 0;
		clusterNum = 0;
		
		for (int i = 0; i < 32; i++) {

			fileName[i] = 0x00;
			searchStr[i] = 0x00;
		}
		searchStr[0] = '\0';	

		x = 0;
		y = 0;
		
		for (int i = 0; i < 1920; i++) {
		
			file[i] = 0x00;
		}
		fileSize = 1920;
		lba = 0;
	}

	
	if (pressed) {

		if (!start) {
		
			start = true;
			return;
		}

		//FILE SEARCH
		if (search) {
	
			switch (key) {
	
				case '\x1b':
					break;
				case '\b':
					if (index > 0) {
				
						searchStr[index] = ' ';
						index--;
						searchStr[index] = '\0';
					}
					break;
				case '\n':
			
					if (index == 0) {
				
						printfTUI("0123456789abcdef0123456789abcdef", 0x07, 0x07, 12, 14);
						printfTUI("Must enter valid file name.", 0x00, 0x07, 12, 14);
						return;
					} else {
						//allocating new file 
						//or
						//edit already existing file 
		
						index = 0;
						clusterNum = 0;
						
						for (int i = 0; searchStr[i] != '\0'; i++) {

							fileName[i] = searchStr[i];
							searchStr[i] = 0x00;
						}	
						searchStr[0] = '\0';	
						search = false;	
						TUI(0x0f, 0x01, 0, 23, 79, 24, false);
						printfTUI(fileName, 0x0f, 0x01, 0, 24);	
						printfTUI("LBA:", 0x0f, 0x01, 72, 24);	
						printfTUI(int2str(lba), 0x0f, 0x01, 76, 24);	


						//read file if it already exists
						fileSize = filesystem->GetFileSize(fileName);
						filesystem->ReadLBA(fileName, file, lba);

						for (int y = 0; y < 24; y++) {
							for (int x = 0; x < 80; x++) {
							
								putcharTUI(file[80*y+x], 0x0f, 0x00, x, y);
							}
						}
						return;
					}
					break;

				default:
					if (index < 32) {
			
						searchStr[index] = key;
						index++;
						searchStr[index] = '\0';
					}
					break;
				}
			char* displayStr = searchStr;
			displayStr[index] = '_';
			printfTUI(displayStr, 0xff, 0x00, 29, 11);
	
		//FILE EDITOR	
		} else {
			volatile uint16_t* vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);

			//ctrl-key shortcuts
			if (ctrl) {	
				
				//remove previous message		
				printfTUI("0123456789abcdef0123456789abcdef", 0x01, 0x01, 33, 24);
				
				switch (key) {
				
					//save file to disk
					case 'w':
						if (filesystem->FileIf(fnv1a(fileName))) {

							filesystem->WriteLBA(fileName, file, lba);
							printfTUI("File has been saved.", 0x0f, 0x01, 33, 24);
						} else {
				
							filesystem->NewFile(fileName, file, fnv1a(fileName), (lba + 1) * 1920);
							printfTUI("File was created.", 0x0f, 0x01, 33, 24);
						}
						return;	
						break;
					
					//next lba
					case '\xff':
						lba += (1 * (lba < 127));	
						break;
					//previous lba
					case '\xfc':
						lba -= (1 * (lba > 0));
						break;
					default:
						break;
				}
				
				if (key == '\xff' || key == '\xfc') {
				
					filesystem->ReadLBA(fileName, file, lba);
					
					for (int y = 0; y < 24; y++) {
						for (int x = 0; x < 80; x++) {	
							putcharTUI(file[80*y+x], 0x0f, 0x00, x, y);
						}
					}
					printfTUI("   ", 0x01, 0x01, 76, 24);
					printfTUI(int2str(lba), 0x0f, 0x01, 76, 24);
				}	
				return;
			}
			//unhighlight old char	
			vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
			*vidmem = ((*vidmem & 0x0f) | (*vidmem & 0xf0)) | 0xf00;

			//typing
			switch (key) {
			
				//left
				case '\xfc':
					x -= (1 * (x > 0));
					break;
				//up
				case '\xfd':
					y -= (1 * (y > 0));
					break;
				//down
				case '\xfe':
					y += (1 * (y < 23));
					break;
				//right
				case '\xff':
					x += (1 * (x < 80));
					break;
				case '\b':
					file[(80*y+x) - 1] = 0x00;
					
					x -= (1 * (x > 0));

					vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
					*vidmem = 0x00;
					break;
				case '\n':
					if (y < 24) { y++; x = 0; }
					break;
				case '\v':
					x += (4 * (x < 76));
					break;
				//escape, file is not saved
				case '\x1b':

					x = 0;
					y = 0;
					//lba = 0;

					for (uint32_t i = 0; i < 1920; i++) {
				
						file[i] = 0x00;
					}

					for (int i = 0; fileName[i] != '\0'; i++) {
						
						fileName[i] = 0x00;
					}
					fileName[0] = '\0';	

					fileSize = 1920;
					search = true;

					fileTUI();	
					return;
				
					break;
				default:
					
					putcharTUI(key, 0xff, 0x00, x, y);
					file[80*y+x] = key;
					x++;

					if (x >= 80) {
					
						x = 0;
						y += (1 * (y < 23));

						if (y >= 24) {
							//do later
						}
					}
					break;
			}
			//highlight new char
			vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
			*vidmem = ((*vidmem & 0x0f) | (*vidmem & 0xf0)) | 0xff000;
		}
	}
}


void fileMain(bool pressed, char key, bool ctrl, FileSystem* filesystem) {

	if (key == 'c' && ctrl) {
	
		file(pressed, key, ctrl, 1, filesystem);
		return;
	}
	file(pressed, key, ctrl, 0, filesystem);
}	
