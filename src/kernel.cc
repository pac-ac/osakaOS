#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <art.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/amd_am79c973.h>
#include <drivers/ata.h>
#include <drivers/speaker.h>
#include <drivers/pit.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <filesys/ofs.h>
#include <cli.h>
#include <script.h>
#include <mode/piano.h>
#include <mode/snake.h>
#include <mode/file_edit.h>
#include <mode/space.h>



using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;
using namespace os::net;
using namespace os::filesystem;
using namespace os::gui;



void putchar(unsigned char ch, unsigned char forecolor, 
		unsigned char backcolor, uint8_t x, uint8_t y) {

	uint16_t attrib = (backcolor << 4) | (forecolor & 0x0f);
	volatile uint16_t* vidmem;
	vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
	*vidmem = ch | (attrib << 8);
}


void printfTUI(uint8_t forecolor, uint8_t backcolor, 
		uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		bool shadow) {


	for (uint8_t y = 0; y < 25; y++) {
		
		for (uint8_t x = 0; x < 80; x++) {

			putchar(0xff, 0x00, backcolor, x, y);
		}
	}
	
	uint8_t resetX = x1;


	while (y1 < y2) {
	
		while (x1 < x2) {
		
			putchar(0xff, 0x00, forecolor, x1, y1);
			x1++;
		}
		y1++;
		
		//side shadow
		if (shadow) {
		
			putchar(0xff, 0x00, 0x00, x1, y1);
		}
		x1 = resetX;
	}

	//bottom shadow
	if (shadow) {
		
		for (resetX++; resetX < (x2 + 1); resetX++) {
	
			putchar(0xff, 0x00, 0x00, resetX, y1);
		}
	}
}


void printfColor(char* str, uint8_t forecolor, uint8_t backcolor, uint8_t x, uint8_t y) {

	for (int i = 0; str[i] != '\0'; i++) {

		if (str[i] == '\n') {		
		
			y++;
			x = 0;

		} else {
			
			putchar(str[i], forecolor, backcolor, x, y);
			x++;
		}

		if (x >= 80) {
			
			y++;
			x = 0;
		}

		if (y >= 25) {
		
			y = 0;
		}
	}
}



void printf(char* str) {
	
	static uint8_t x = 0, y = 0;
	static bool cliCursor = false;

	//default gray on black text
	uint16_t attrib = 0x07;

	volatile uint16_t* vidmem;

		
	for (int i = 0; str[i] != '\0'; i++) {
		
		vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
		
		switch (str[i]) {
		
			case '\b':
				vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
				*vidmem = ' ' | (attrib << 8);
				vidmem--; *vidmem = '_' | (attrib << 8);
				x--;

				break;

			case '\n':
				*vidmem = ' ' | (attrib << 8);
				y++;
				x = 0;
					
				break;

			case '\t': //$: shell interface
					
				if (!i) {
					cliCursor = true;
					
					if (x < 3) {
		
						x = 3;
					}
					
					vidmem = (volatile uint16_t*)0xb8000 + (80*y);
					*vidmem = '$' | 0xc00;
					vidmem++; *vidmem = ':' | 0xf00;
					vidmem++; *vidmem = ' ';
				
				} else {
					*vidmem = '_' | (attrib << 8);
				}
				break;
				
			case '\v': //clear screen
				
				for (y = 0; y < 25; y++) {
					for (x = 0; x < 80; x++) {
					
						vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
						*vidmem = 0x00;
					}
				}
				x = 0;
				y = 0;
					
				break;

				
			default:
				*vidmem = str[i] | (attrib << 8);
				x++;
					
				break;
		}
	
		if (x >= 80) {
		
			y++;
			x = 0;
		}

		
		//scrolling	
		if (y >= 25) {
			
			uint16_t scroll_temp;

			for (y = 1; y < 25; y++) {	
				for (x = 0; x < 80; x++) {
					
					vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
					scroll_temp = *vidmem;
						
					vidmem -= 80;
					*vidmem = scroll_temp;
					
					if (y == 24) {
						
						vidmem = (volatile uint16_t*)0xb8000 + (1920+x);
						*vidmem = ' ' | (attrib << 8);
					}
				}
			}
			x = 0;
			y = 24;
		}
	}
}





void printfHex(uint8_t key) {

	char* foo = "00 ";
       	char* hex = "0123456789ABCDEF";

        foo[0] = hex[(key >> 4) & 0x0F];
        foo[1] = hex[key & 0x0F];
	printf(foo);
}



uint16_t strlen(char* args) {

        uint16_t length = 0;

        for (length = 0; args[length] != '\0'; length++) {

        }

        return length;
}



bool strcmp(char* one, char* two) {

	uint16_t i = 0;

	for (i; one[i] != '\0'; i++) {
	
		if (one[i] != two[i]) {
		
			return false;
		}
	}

	return true;
}



uint32_t str2int(char* args) {

        uint32_t number = 0;
        uint16_t i = 0;
        bool gotNum = false;

        for (uint16_t i = 0; args[i] != '\0'; i++) {

                if ((args[i] >= 58 || args[i] <= 47) && args[i] != ' ') {

                        return 0;
                }

                if (args[i] != ' ') {

                        number *= 10;
                        number += ((uint32_t)args[i] - 48);
                        gotNum = true;
                        args[i] = ' ';

                } else {
                        if (gotNum) {

                                return number;
                        }
                }
        }
        return number;
}


char* int2str(uint32_t num) {

        uint32_t numChar = 1;
        uint8_t i = 1;

        if (num % 10 != num) {

                while ((num / (numChar)) >= 10) {

                        numChar *= 10;
                        i++;
                }

                char* str = "4294967296";
                uint8_t strIndex = 0;

                while (i) {

                        str[strIndex] = (char)(((num / (numChar)) % 10) + 48);

                        if (numChar >= 10) {

                                numChar /= 10;
                        }
                        strIndex++;
                        i--;
                }

                str[strIndex] = '\0';
                return str;
        }

        char* str = " ";
        str[0] = (num + 48);

        return str;
}




char* argparse(char* args, uint8_t num) {

	char buffer[256];

	bool valid = false;
	uint8_t argIndex = 0;
	uint8_t bufferIndex = 0;


	for (int i = 0; i < (strlen(args) + 1); i++) {
	
		if (args[i] == ' ' || args[i] == '\0') {
		
			if (valid) {
				
				if (argIndex == num) {
				
					buffer[bufferIndex] = '\0';
					char* arg = buffer;
					return arg;
				}
				argIndex++;
			}
			valid = false;

		} else {
			if (argIndex == num) {
				
				buffer[bufferIndex] = args[i];
				bufferIndex++;
			}
			valid = true;
		}
	}

	//       |
	//this   v
	return "wtf";
}

uint8_t argcount(char* args) {

	uint8_t i = 0;
	char* foo = argparse(args, i);
	
	//and this gotta be the same
	while (foo != "wtf") {
	
		foo = argparse(args, i);
		i++;
	}	

	return i-1;
}






//class that connects the keyboard to the rest of the command line

class CLIKeyboardEventHandler : public KeyboardEventHandler, public CommandLine {
	
	//protected:
	public:
		uint8_t index = 0;
		char input[256];

		char keyChar;
		bool pressed;
		
		char lastCmd[256];
	public:
		CLIKeyboardEventHandler(GlobalDescriptorTable* gdt, 
					TaskManager* tm) {
		
			this->getTM(gdt, tm);
			this->cli = true;
		}
	

		void modeSelect(uint16_t mode, bool pressed, char ch, 
				bool ctrl, bool type) {			
	
			switch (this->cliMode) {
			
				case 1:
					//if (type) { this->script(pressed, ch, ctrl); }
					this->script(pressed, ch, ctrl);
					break;
				case 2:
					piano(pressed, ch);
					break;
				case 3:
					snake(ch);
					break;
				case 4:
					if (type) { fileMain(pressed, ch, ctrl); }
					break;
				case 5:
					space(pressed, ch);
					break;
				default:
					break;
			}
		}

		
		void OnKeyDown(char c) {
			
			this->pressed = true;
			this->keyChar = c;


			if (this->ctrl) {
			
				switch (c) {
					case 'c':
						this->resetMode();
						return; break;
					case 'p':
						this->cliMode = 2;
						modeSet(this->cliMode);
						break;
					case 's':
						this->cliMode = 3;
						modeSet(this->cliMode);
						break;
					case 'e':
						this->cliMode = 4;
						modeSet(this->cliMode);
						break;
					case 'i':
						this->cliMode = 5;
						modeSet(this->cliMode);
						break;
					default:
						break;
				}
			}
			
			
			if (this->cliMode == 0) {

				char* foo = " \t";
				foo[0] = c;
				
				switch (c) {

					//scroll command history
					case '\xff':
						break;
					case '\xfc':
						break;
					//up
					case '\xfd':
					
						if (index > 0) {
						
							for (int i = 0; i < index; i++) {
						
								input[index] = 0x00;
								printf("\b");
							}
						}
							
						for (index = 0; lastCmd[index] != '\0'; index++) {
							
							input[index] = lastCmd[index];
						}
						input[index] = '\0';
						printf(input);
						break;
					//down
					case '\xfe':	
							
						for (int i = 0; i < index; i++) {
						
							input[index] = 0x00;
							printf("\b");
						}
						index = 0;
						break;

					//backspace
					case '\b':
						if (index > 0) {
							
							printf(foo);
							index--;
							input[index] = 0;
						}
						break;

					//enter
					case '\n': 
						printf(foo);
						input[index] = '\0';
	

						//execute command input
						if (index > 0 && input[0] != ' ') {
						
							for (int i = 0; input[i] != '\0'; i++) {
								lastCmd[i] = input[i];
							}
							lastCmd[index] = '\0';
					
							command(input, index);
						}

						input[0] = 0x00;
						index = 0;
						break;
					//type
					default:
						printf(foo);
						input[index] = c;
						index++;
						break;
				}
			} else {
			
				//modes
				this->modeSelect(this->cliMode, this->pressed, this->keyChar, this->ctrl, 1);
				
				//lol
				index = 0;
			}
		}
	

		void OnKeyUp() {

			this->pressed = false;
		}

		

		void resetCmd() {
		
			for (uint16_t i = 0; i < 256; i++) {
			
				input[i] = 0x00;
			}
		}
		
		
		//print the ui for mode here
		void modeSet(uint8_t mode) {
		
			//reset mode before entering next one
			this->resetMode();

			//black = 0		//dark gray = 8
			//blue = 1		//light blue = 9
			//green = 2		//light green = A
			//cyan = 3		//light cyan = B
			//red = 4		//light red = C
			//magenta = 5		//light Magenta = D
			//brown = 6		//yellow = E
			//light gray = 7	//white = F
			
			//example: print foreground with light gray, background with blue
			//printTUI(0x07, 0x01, coordinates, shadows, etc)
	


			this->cliMode = mode;


			switch (this->cliMode) {
			
				case 2:
					//piano
					pianoTUI();
					break;
				case 3:
					//snake
					snake('r');
					snakeTUI();
					snakeInit();
					break;
				case 4:
					//file editor
					fileTUI();
					fileMain(0, 'c', 1);
					break;
				case 5:
					//space game
					space(true, 'r');
					spaceTUI();
					break;
				default:
					printf("Mode not found.\n");
					break;
			}
		}


		//print error messagse for modes
		//and other things you want
		void resetMode() {
					
			printf("\v");
	
			switch (this->cliMode) {
		
				case 1:
					printf("\nScript has been terminated.\n\n");
					break;
				case 2:
					printf("\nExiting piano mode...\n\n");
					break;
				case 3:
					printf("\nExiting snake mode...\n\n");
					break;
				case 4:
					fileMain(0, 'c', 1);
					printf("\nExiting file edit mode...\n\n");
					break;
				case 5:
					printf("\nExiting space mode...\n\n");
					break;
				default:
					break;
			}

			this->cliMode = 0;
		}
};


void sleep(uint32_t ms) {

	//sleep 1 = wait 1 ms
	PIT pit;

	for (uint32_t i = 0; i < ms; i++) {
		
		pit.setCount(1193182/1000);
		uint32_t start = pit.readCount();
	
		while ((start - pit.readCount()) < 1000) {}
	}
}


void memWrite(uint32_t memory, uint32_t inputVal) {

	volatile uint32_t* value;
	value = (volatile uint32_t*)memory;
	*value = inputVal;
}

uint32_t memRead(uint32_t memory) {

	volatile uint32_t* value;
	value = (volatile uint32_t*)memory;
	
	return *value;
}




void WmDisk(uint32_t sector, char* data) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	
	ata0m.Write28(sector, (uint8_t*)data, strlen(data), 0);
	ata0m.Flush();

	printf("Wrote: ");
	printf(data);
	printf(" to disk sector ");
	printf(int2str(sector));
	printf(".\n");
}


void RmDisk(uint32_t sector, uint16_t dataLen) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	
	uint8_t bytes[512];
	ata0m.Read28(sector, bytes, dataLen, 0);

	char* foo = " ";
	for (int i = 0; i < dataLen; i++) {
	
		foo[0] = bytes[i];
		printf(foo);
	}
	printf("\n");
}


uint32_t FileList() {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	
	uint32_t fileNum = 0;
	uint8_t numOfFiles[512];
	ata0m.Read28(512, numOfFiles, 512, 0);

	for (int i = 0; i < 512; i++) {

		fileNum += numOfFiles[i];
	}
	

	uint8_t sectorData[512];
	uint8_t fileName[33];

	uint32_t location = 0;

	for (int i = 0; i < fileNum; i++) {
	
		ata0m.Read28(513+i, sectorData, 4, 0);
	
		location = (sectorData[0] << 24) | 
			   (sectorData[1] << 16) | 
			   (sectorData[2] << 8) | 
			   (sectorData[3]);
	
		if (location) {
		
			ata0m.Read28(location, sectorData, 40, 0);
		
			for (int j = 0; j < 32; j++) {

				fileName[j] = sectorData[j+8];
			}
			fileName[32] = '\0';
	

			printf((char*)fileName);
			printf("    ");
			printf(int2str(location));
			printf("\n");
		}

		/*
		for (int j = 8; j < 40; j++) {
		
			fileName[j] = 0x00;
			sectorData[j] = 0x00;
		}
		*/
	}

	return fileNum;
}






void printOsaka(uint8_t num) {
	
	Funny osaka;
	printf("\v");

	
	switch (num) {
	
		case 0:
			osaka.osakaFace();
			break;
		case 1:
			osaka.osakaHead();
			break;
		case 2:
			osaka.god();
			break;
		case 3:
			osaka.osakaKnife();
			break;
		case 4:
			osaka.osakaAscii();
			break;
		default:
			osaka.osakaFace();
			break;
	}
}


	
void makeBeep(uint32_t freq) {

	Speaker speaker;
	speaker.Speak(freq);
}




uint16_t prng() {

	PIT pit;
	uint16_t seed = (uint16_t)pit.readCount();
	uint16_t lfsr = seed;
	uint16_t period = 0;

	do {
		uint16_t lsb = lfsr & 1u;
		lfsr >>= 1;
		lfsr ^= (-lsb) & 0xb400u;
		
		period++;

	} while (period < seed);
	

	return lfsr;
}



void forget() {

	uint32_t start = 0;
	uint32_t end = prng();

	for (start; start < end; start++) {
	
		memWrite(start, 0);
	}
}



void explodeMain() {

	//vga
	VideoGraphicsArray vga;
	vga.SetMode(320, 200, 8);
	
	uint8_t color = 0;
	
	//beeps
	Speaker speaker;
	speaker.PlaySound(1234);

	//memory	
	MemoryManager memoryManager((size_t)0, (size_t)4*1024*1024*1024);


	while (1) {
	
		void* mem = memoryManager.malloc((size_t)4*1024*1024);
	

		speaker.PlaySound(1200);
		speaker.NoSound();
		
		for (uint16_t y = 0; y < 200; y++) {
			for (uint16_t x = 0; x < 320; x++) {
		
				vga.PutPixel(x, y, color);
			}
		}
		color++;
			
		if (color > 0x3f) {
		
			color = 0;
		}	

		speaker.PlaySound(200);
		speaker.NoSound();
	}	
}





typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;



extern "C" void callConstructors() {

	for (constructor* i = &start_ctors; i != &end_ctors; i++) {
		(*i)();
	}
}



extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {

	printf("Hello :^)\n");


	GlobalDescriptorTable* gdt;
	TaskManager* taskManager;
	
	InterruptManager interrupts(0x20, gdt, taskManager);
	printf("Initializing Hardware, Stage 1\n");


	DriverManager drvManager;
	
	//drivers
	CLIKeyboardEventHandler kbhandler(gdt, taskManager);
	KeyboardDriver keyboard(&interrupts, &kbhandler);
	
	drvManager.AddDriver(&keyboard);

	Desktop desktop(320, 200, 0x01);
	MouseDriver mouse(&interrupts, &desktop);

	drvManager.AddDriver(&mouse);

	PeripheralComponentInterconnectController PCIController;
	PCIController.SelectDrivers(&drvManager, &interrupts);

	VideoGraphicsArray vga;

	
	printf("\nInitializing Hardware, Stage 2\n");
	drvManager.ActivateAll();
	
	printf("Initializing Hardware, Stage 3\n");
	interrupts.Activate();
	
	printf("\n\nEverything seems fine.\n");


	//while (1) {}
	//everything beyond this point is no longer testing/initialization


	//the boot screen and very important cube	
	Funny haha;		
	printOsaka(4);
	uint8_t cubeCount = 0;

	makeBeep(600);
	
	do {	
		//cube repeats when cubeCount = 170	
		haha.cubeAscii(cubeCount);
		cubeCount++;
		sleep(10);
	
	} while (kbhandler.pressed == false);
	printf("\v");



	//initialize command line hash table
	kbhandler.cli = true;
	kbhandler.hash_cli_init();

	
		
	//this is the command line :D
	while (keyboard.keyHex != 0x5b) { //0x5b = command/windows key	

		kbhandler.cli = true;

		while (kbhandler.cliMode) {
			
			kbhandler.cli = false;
			kbhandler.modeSelect(kbhandler.cliMode, kbhandler.pressed, 
					kbhandler.keyChar, kbhandler.ctrl, 0);
		}
	}



	//initialize desktop
	KeyboardDriver keyboardDesktop(&interrupts, &desktop);
	drvManager.Replace(&keyboardDesktop, 0);

	vga.SetMode(320, 200, 8);	


	Window win1(&desktop, 10, 10, 40, 20, 0x04);
	desktop.AddChild(&win1);
	
	Window win2(&desktop, 100, 15, 30, 30, 0x19);
	desktop.AddChild(&win2);	
	
	Window win3(&desktop, 60, 45, 80, 65, 0x32);
	desktop.AddChild(&win3);	
	

	//this is the gui :(
	while (keyboardDesktop.keyHex != 0x39) {
		
		desktop.Draw(&vga, 1);
		sleep(17);
	}
}
