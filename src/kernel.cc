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
#include <drivers/cmos.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <gui/button.h>
#include <gui/widget.h>
#include <gui/sim.h>
#include <gui/raycasting.h>
#include <gui/font.h>
#include <gui/pixelart.h>
#include <multitasking.h>
#include <net/network.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <filesys/ofs.h>
#include <cli.h>
#include <script.h>
#include <app.h>
#include <app/paint.h>
#include <app/file_edit.h>
#include <app/file_browse.h>
#include <mode/piano.h>
#include <mode/snake.h>
#include <mode/file_edit.h>
#include <mode/space.h>
#include <mode/bootscreen.h>
#include <math.h>


using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;
using namespace os::net;
using namespace os::filesystem;
using namespace os::gui;
using namespace os::math;

PeripheralComponentInterconnectController* pcic;



void putcharTUI(unsigned char ch, unsigned char forecolor, 
		unsigned char backcolor, uint8_t x, uint8_t y) {

	uint16_t attrib = (backcolor << 4) | (forecolor & 0x0f);
	volatile uint16_t* vidmem;
	vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
	*vidmem = ch | (attrib << 8);
}


void TUI(uint8_t forecolor, uint8_t backcolor, 
		uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		bool shadow) {

	for (uint8_t y = 0; y < 25; y++) {
		
		for (uint8_t x = 0; x < 80; x++) {

			putcharTUI(0xff, 0x00, backcolor, x, y);
		}
	}
	
	uint8_t resetX = x1;

	while (y1 < y2) {
	
		while (x1 < x2) {
		
			putcharTUI(0xff, 0x00, forecolor, x1, y1);
			x1++;
		}
		y1++;
		
		//side shadow
		if (shadow) { putcharTUI(0xff, 0x00, 0x00, x1, y1); }
		x1 = resetX;
	}

	//bottom shadow
	if (shadow) {
		
		for (resetX++; resetX < (x2 + 1); resetX++) {
	
			putcharTUI(0xff, 0x00, 0x00, resetX, y1);
		}
	}
}


void printfTUI(char* str, uint8_t forecolor, uint8_t backcolor, uint8_t x, uint8_t y) {

	for (int i = 0; str[i] != '\0'; i++) {

		if (str[i] == '\n') {		
		
			y++;
			x = 0;
		} else {
			putcharTUI(str[i], forecolor, backcolor, x, y);
			x++;
		}
		if (x >= 80) {
			
			y++;
			x = 0;
		}
		if (y >= 25) { y = 0; }
	}
}



//set color for printf
uint16_t setTextColor(bool set, uint16_t color = 0x07) {
	
	static uint16_t newColor = 0x07; //default gray on black text
	if (set) { newColor = color; }
	return newColor;
}


//the main print function for textmode
void printf(char* strChr) {
	
	static uint8_t x = 0, y = 0;
	static bool cliCursor = false;

	uint16_t attrib = setTextColor(false);
	volatile uint16_t* vidmem;

	
	uint8_t* str = (uint8_t*)strChr;


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
					
					if (x < 3) { x = 3; }
					
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


void printfLine(const char* str, uint8_t line) {

	for (uint16_t i = 0; str[i] != '\0'; i++) {
	
		volatile uint16_t* vidmem = (volatile uint16_t*)0xb8000 + (80*line+i);
		*vidmem = str[i] | 0x700;
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
	for (length = 0; args[length] != '\0'; length++) {}
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
                        if (gotNum) { return number; }
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

			if (numChar >= 10) { numChar /= 10; }
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



char* int2hex(uint32_t num) {

	char* str = "0x00000000";
	const char* hex = "0123456789ABCDEF";
	
	uint8_t i = 0;
	while (num)
	{
		i++;
		str[10-i] = hex[num & 0x0F];
		num = num >> 4;
	}
	
	
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


void altCode(uint8_t c, uint8_t &numCode) {
	
	static uint8_t count = 0;
	bool bitShift = (count % 2 == 0);
	count++;

	if (c <= '9' && c >= '0') { numCode += (c - '0'); }
	if (c <= 'f' && c >= 'a') { numCode += (c - 'a') + 10; }
	numCode <<= (4 * bitShift);
}




//class that connects the keyboard to the rest of the command line
class CLIKeyboardEventHandler : public KeyboardEventHandler, public CommandLine {
	
	public:
		bool pressed;
	public:
		CLIKeyboardEventHandler(GlobalDescriptorTable* gdt, 
					TaskManager* tm,
					MemoryManager* mm,
					FileSystem* filesystem,
					CMOS* cmos,
					DriverManager* drvManager) 
		: CommandLine(gdt, tm, mm, filesystem, cmos, drvManager) {
		
			this->cli = true;
		}
	
		void modeSelect(uint16_t mode, bool pressed, unsigned char ch, 
				bool ctrl, bool type, FileSystem* filesystem) {			
	
			switch (this->cliMode) {
			
				case 2:
					piano(pressed, ch);
					break;
				case 3:
					snake(ch);
					break;
				case 4:
					if (type) { fileMain(pressed, ch, ctrl, filesystem); }
					break;
				case 5:
					space(pressed, ch);
					break;
				case 6:
					bootScreen(pressed, ch);
					break;
				default:
					break;
			}
		}
		
		void OnKeyDown(char c) {
			
			this->pressed = true;
			keyChar = c;

			//num code
			if (this->alt) {
			
				altCode(c, numCode);
				return;
			}

			if (this->alt == false && this->numCode != 0) {
				
				c = this->numCode;
				keyChar = this->numCode;
			}
			this->numCode = 0;

			//mode shortcut
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
					case 'b':
						this->cliMode = 6;
						modeSet(this->cliMode);
						break;
					default:
						break;
				}
			}
			
			//normal cli
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
						input[index] = keyChar;
						index++;
						break;
				}
			} else {
				//modes
				this->modeSelect(this->cliMode, this->pressed, 
						this->keyChar, this->ctrl, 1, this->filesystem);
				
				//lol
				index = 0;
			}
		}
	

		void OnKeyUp() { this->pressed = false; }
		

		void resetCmd() {
		
			for (uint16_t i = 0; i < 256; i++) {
			
				input[i] = 0x00;
			}
		}
		
		//print the ui for mode here
		void modeSet(uint8_t mode) {
		
			//reset mode before entering next one
			this->resetMode();
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
					fileMain(0, 'c', 1, nullptr);
					break;
				case 5:
					//space game
					space(true, 'r');
					spaceTUI();
					break;
				case 6:
					//initial mode for cube
					bootInit();
					bootScreen(false, 'b');
					break;
				default:
					printf("Mode not found.\n");
					break;
			}
		}
		//print error messages for modes
		//and other things you want
		void resetMode() {
					
			printf("\v");
	
			switch (this->cliMode) {
		
				case 2:
					printf("\nExiting piano mode...\n\n");
					break;
				case 3:
					printf("\nExiting snake mode...\n\n");
					break;
				case 4:
					fileMain(0, 'c', 1, nullptr);
					printf("\nExiting file edit mode...\n\n");
					break;
				case 5:
					printf("\nExiting space mode...\n\n");
					break;
				case 6:
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


double getTicks() {

	PIT pit;
	pit.setCount(1193182/1000);
	return (double)(pit.readCount());
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


void printOsaka(uint8_t num, bool cube) {

	Funny osaka;

	if (cube) {	
		osaka.cubeAscii(num);
		return;
	} else {
		printf("\v");
	}


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


void reboot() {

	asm volatile ("cli");

	uint8_t read = 0x02;
	Port8Bit resetPort(0x64);

	while (read & 0x02) {
	
		read = resetPort.Read();
	}

	resetPort.Write(0xfe);
	asm volatile ("hlt");
}

uint32_t cmosDetectMemory() {

	uint32_t total = 0;
	uint8_t lowmem, highmem = 0;

	Port8Bit WriteCMOS(0x70);
	Port8Bit ReadCMOS(0x71);
	
	//size of low memory
	WriteCMOS.Write(0x15);
	sleep(10);
	lowmem = ReadCMOS.Read();
	
	WriteCMOS.Write(0x16);
	sleep(10);
	highmem = ReadCMOS.Read();
	total += ((lowmem | highmem) << 10);
	
	//total memory between 1M and 16M...or 65M osdev says lol
	WriteCMOS.Write(0x17);
	sleep(10);
	lowmem = ReadCMOS.Read();
	
	WriteCMOS.Write(0x18);
	sleep(10);
	highmem = ReadCMOS.Read();
	total += ((lowmem | highmem) << 10);

	//total memory between 16M and 4G
	WriteCMOS.Write(0x30);
	sleep(10);
	lowmem = ReadCMOS.Read();
	
	WriteCMOS.Write(0x31);
	sleep(10);
	highmem = ReadCMOS.Read();
	total += ((lowmem | highmem) << 16);

	return total;
}



uint8_t Web2EGA(uint32_t color) {

	uint8_t bytes[3];
	bytes[2] = color >> 16;
	bytes[1] = (color >> 8) & 0xff;
	bytes[0] = color & 0xff;

	uint8_t result = 0;

	for (int i = 0; i < 3; i++) {
	
		if        (bytes[i] < 0x2b) { bytes[i] = 0x00;
		} else if (bytes[i] < 0x80) { bytes[i] = 0x55; 
		} else if (bytes[i] < 0xd5) { bytes[i] = 0xaa; 
		} else {		      bytes[i] = 0xff; }
	}

	for (int i = 0; i < 3; i++) {
	
		switch (bytes[i]) {
		
			//both sets of
			//3 bits
			case 0xff:
				result |= (1 << (i+3));
				result |= (1 << i);
				break;
			//most sig 3 bits
			case 0x55: result |= (1 << (i+3)); break;
			//least sig 3 bits
			case 0xaa: result |= (1 << i); break;
			default: break;
		}
	}
	return result;
}




CommandLine* LoadScriptForTask(bool set, CommandLine* cli = 0) {

	static CommandLine* retCli = 0;
	if (set) { retCli = cli; }
	return retCli;
}


Desktop* LoadDesktopForTask(bool set, Desktop* desktop = 0) {

	static Desktop* retDesktop = 0;
	if (set) { retDesktop = desktop; }
	return retDesktop;
}

void DrawDesktopTask() {

	Desktop* desktop = LoadDesktopForTask(false);
	desktop->gc->SetMode(320, 200, 8);

	while (1) { desktop->Draw(desktop->gc); }
}

// Zinchuk. Need for some wierd driver stuff
PeripheralComponentInterconnectController* GetPeripheralComponentInterconnectController(){
	return pcic;
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
	TaskManager taskManager(gdt);
	
	//heap manager
	uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
	size_t heap = 4*1024*1024;
	MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

	
	InterruptManager interrupts(0x20, gdt, &taskManager);
	printf("Initializing Hardware, Stage 1\n");

	DriverManager drvManager;
	
	//drivers and command line
	CMOS cmos;
	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	FileSystem osakaFileSystem(&ata0m);
	
	CLIKeyboardEventHandler* kbhandler = (CLIKeyboardEventHandler*)memoryManager.malloc(sizeof(CLIKeyboardEventHandler));
	new (kbhandler) CLIKeyboardEventHandler(gdt, &taskManager, &memoryManager, &osakaFileSystem, &cmos, &drvManager);
	kbhandler->hash_cli_init(); //init command line
	
	KeyboardDriver keyboard(&interrupts, kbhandler);
	kbhandler->caps = false;
	kbhandler->shift = false;
	kbhandler->ctrl = false;


	//gui driver stuff
	VideoGraphicsArray vga;
	Simulator osaka(&cmos);
	Button buttons;
	Desktop desktop(320, 200, 0x01, &vga, gdt, &taskManager, 
			&memoryManager, &osakaFileSystem, &cmos, 
			&drvManager, &buttons, &osaka);
	MouseDriver mouse(&interrupts, &desktop);


	drvManager.AddDriver(&keyboard);
	drvManager.AddDriver(&mouse);
	drvManager.AddDriver(&ata0m);


	//pci and init
	PeripheralComponentInterconnectController PCIController(&memoryManager);
	PCIController.SelectDrivers(&drvManager, &interrupts);
	pcic = &PCIController;

	//network
	amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[3]);
	//amd_am79c973 eth0(PCIController.PCIdev, &interrupts);
	//drvManager.drivers[3] = &eth0;


	printf("\nInitializing Hardware, Stage 2\n");
	drvManager.ActivateAll();
	
	
	//network init
	//IP Address
	uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
	uint32_t ip_be = ((uint32_t)ip4 << 24) | ((uint32_t)ip3 << 16) 
			| ((uint32_t)ip2 << 8) | (uint32_t)ip1;
	
	eth0->SetIPAddress(ip_be);
	EtherFrameProvider etherframe(eth0);
	AddressResolutionProtocol arp(&etherframe);    


	// IP Address of the default gateway
	uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
	uint32_t gip_be = ((uint32_t)gip4 << 24) | ((uint32_t)gip3 << 16) 
			| ((uint32_t)gip2 << 8) | (uint32_t)gip1;

	uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
	uint32_t subnet_be = ((uint32_t)subnet4 << 24) | ((uint32_t)subnet3 << 16) 
			| ((uint32_t)subnet2 << 8) | (uint32_t)subnet1;
		           
	InternetProtocolProvider ipv4(&etherframe, &arp, gip_be, subnet_be);
	InternetControlMessageProtocol icmp(&ipv4);
	
	
	Network network(eth0, &arp, &ipv4, &icmp, gip_be, subnet_be);
	kbhandler->network = &network;


	printf("Initializing Hardware, Stage 3\n");
	interrupts.Activate();
	
	printf("\n\nEverything seems fine.\n");

	
	interrupts.boot = true;
	makeBeep(600);

	//while (1) {}
	//everything beyond this point is no longer testing/initialization


	//the boot screen and very important cube
	uint8_t cubeNum = 0;
	printOsaka(4, false);
	kbhandler->pressed = false;
	while (kbhandler->pressed == false) {

		printOsaka(cubeNum, true);
		cubeNum++;
		sleep(10);
	}


	//initialize command line hash table
	kbhandler->gui = false;
	kbhandler->cli = true;
	kbhandler->OnKeyDown('\b');
	printf("\v");


	//this is the command line :D		
	while (keyboard.handler->keyValue != 0x5b) { //0x5b = command/windows key	

		kbhandler->cli = true;

		while (kbhandler->cliMode) {

			kbhandler->cli = false;
			kbhandler->modeSelect(kbhandler->cliMode, kbhandler->pressed, 
					kbhandler->keyChar, kbhandler->ctrl, 0, 
					&osakaFileSystem);
		}
	}
	kbhandler->cli = true;
	kbhandler->gui = true;


	//initialize desktop
	KeyboardDriver keyboardDesktop(&interrupts, &desktop);
	drvManager.Replace(&keyboardDesktop, 0);

	desktop.CreateChild(1, "Osaka's Terminal", kbhandler);
	
	
	//add task for drawing desktop
	LoadDesktopForTask(true, &desktop);
	Task guiTask(gdt, DrawDesktopTask, "osakaOS GUI");
	taskManager.AddTask(&guiTask);

	
	//this is the gui :)
	while (1) {
	
		//this is the loop where the kernel
		//exists in, the rest is handed off
		//to the taskmanager, godspeed o7
		//			      /|
		//			      / \

	}
}
