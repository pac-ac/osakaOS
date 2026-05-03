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
#include <drivers/ac97.h>
#include <drivers/speaker.h>
#include <drivers/pit.h>
#include <drivers/cmos.h>
#include <gui/desktop.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/widget.h>
#include <gui/sim.h>
#include <gui/raycasting.h>
#include <gui/font.h>
#include <gui/pixelart.h>
#include <multitasking.h>
#include <code/asm.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/network.h>
#include <filesys/ofs.h>
#include <cli.h>
#include <script.h>
#include <app.h>
#include <list.h>
#include <string.h>
#include <app/paint.h>
#include <app/file_edit.h>
#include <app/file_browse.h>
#include <app/browser.h>
#include <mode/piano.h>
#include <mode/snake.h>
#include <mode/file_edit.h>
#include <mode/space.h>
#include <math.h>


using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;
using namespace os::net;
using namespace os::filesystem;
using namespace os::gui;
using namespace os::math;



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


uint8_t* memset(uint8_t* buf, int value, size_t n) {

	for (size_t i = 0; i < n; i++) {
	
		buf[i] = value;
	}
	return buf;
}

uint32_t valCount(uint8_t* buf, uint8_t value, size_t n) {

	uint32_t retVal = 0;
	
	for (size_t i = 0; i < n; i++) {
	
		retVal += (1 * (buf[i] == value));
	}
	return retVal;
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
					Network* network,
					Compiler* compiler,
					VideoGraphicsArray* vga,
					CMOS* cmos,
					DriverManager* drvManager) 
		: CommandLine(gdt, tm, mm, filesystem, network, compiler, vga, cmos, drvManager) {
		
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





uint32_t readCount() {

	uint32_t count = 0;
	Port8Bit channel0(0x40);
	Port8Bit commandPort(0x43);
	
	asm("cli");
	commandPort.Write(0);
	count = channel0.Read();
	count |= channel0.Read() << 8;
	asm("sti");
	
	return count;
}

void sleep(uint32_t ms) {
	
	Port8Bit channel0(0x40);

	for (uint32_t i = 0; i < ms; i++) {
	
		asm("cli");
		channel0.Write(1193182/1000);
		channel0.Write((1193182/1000) >> 8);
		asm("sti");
		
		uint32_t start = readCount();
		while ((start - readCount() < 1000)) {}
	}

}


double getTicks() {

	Port8Bit cmdPort(0x43);
	Port8Bit channel0(0x40);

	asm("cli");

	channel0.Write(1193182/1000);
	channel0.Write((1193182/1000) >> 8);
	
	asm("sti");
	
	
	asm("cli");
	
	cmdPort.Write(0x00);
	
	uint32_t count = channel0.Read();
	count |= channel0.Read() << 8;
	
	asm("sti");

	return (double)(count);
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

	if (cube) {	return;
	} else {	printf("\v"); }

	switch (num) {
	
		case 0:	osakaFace();	break;
		case 1:	osakaHead();	break;
		case 2:	god();		break;
		case 3:	osakaKnife();	break;
		case 4:	osakaAscii();	break;
		default:osakaFace();	break;
	}
}


	
void makeBeep(uint32_t freq) {

	Speaker speaker;
	speaker.Speak(freq);
}


uint16_t prng() {

	//PIT pit;
	
	asm("cli");
	
	Port8Bit cmdPort(0x43);
	Port8Bit channel0(0x40);
	cmdPort.Write(0x00);
	
	uint32_t seed = channel0.Read();
	seed |= channel0.Read() << 8;
	
	asm("sti");


	
	uint16_t lfsr = (uint16_t)seed;
	uint16_t period = 0;

	do {
		uint16_t lsb = lfsr & 1u;
		lfsr >>= 1;
		lfsr ^= (-lsb) & 0xb400u;
		
		period++;

	} while (period < seed);
	

	return lfsr;
}


uint16_t hash(char* str) {

	uint32_t val = 0x811c9dc5;

	for (int i = 0; str[i] != '\0'; i++) {
	
		val ^= str[i];
		val *= 0x01000193;
	}
	val++;

	return (val >> 16) ^ (val & 0xffff);
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


//find minimal difference in web values
uint8_t Web2VGA(uint32_t color) {

	float Rc = (color >> 16);
	float Gc = (color >> 8) & 0xff;
	float Bc = (color & 0xff);

	float closest = (255.0*255.0)*3.0 + 1.0;
	uint8_t index = 0;

	for (int i = 0; i < 256; i++) {
	
		float Rp = (defaultPalette[i] >> 16);
		float Gp = (defaultPalette[i] >> 8) & 0xff;
		float Bp = (defaultPalette[i] & 0xff);

		float euclidDist = ((Rp-Rc)*(Rp-Rc)) + 
				   ((Gp-Gc)*(Gp-Gc)) +
				   ((Bp-Bc)*(Bp-Bc));

		if (closest > euclidDist) {
		
			closest = euclidDist;
			index = i;
		
		} else if (euclidDist < 0.05) {
		
			index = i;
			break;
		}
	}
	
	if (index == W_EMPTY) { index = W000000; }
	
	return index;
}


class PrintfTCPHandler : public TransmissionControlProtocolHandler {

	public:
		bool HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, 
							uint8_t* data, uint16_t size) {
	
			printf("received message: ");

			char* foo = " ";
			for (int i = 0; i < size; i++) {
			
				foo[0] = data[i];
				printf(foo);
			}
			printf("\n");
			
			return true;
		}
};



TaskManager* LoadTaskManager(bool set, TaskManager* tm = 0) {

	static TaskManager* manager = 0;
	if (set) { manager = tm; }
	return manager;
}

CommandLine* LoadScriptForTask(bool set, CommandLine* cli = 0) {

	static CommandLine* script = 0;
	if (set) { script = cli; }
	return script;
}

Desktop* LoadDesktopForTask(bool set, Desktop* desktop = 0) {

	static Desktop* retDesktop = 0;
	if (set) { retDesktop = desktop; }
	return retDesktop;
}

void DrawDesktopTask() {

	Desktop* desktop = LoadDesktopForTask(false);

	while (1) { desktop->Draw(desktop->gc); }
}


typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void callConstructors() {

	for (constructor* i = &start_ctors; i != &end_ctors; i++) {
		(*i)();
	}
}


struct MultibootInfo {

	uint32_t flags;
	uint32_t memLower;
	uint32_t memUpper;
	uint32_t bootDevice;
	uint32_t commandLine;
	uint32_t moduleCount;
	uint32_t moduleAddress;
	uint32_t syms[4];
	uint32_t memMapLength;
	uint32_t memMapAddress;
	uint32_t drivesLength;
	uint32_t drivesAddress;
	uint32_t configTable;
	uint32_t bootLoaderName;
	uint32_t apmTable;
	uint32_t vbeControlInfo;
	uint32_t vbeModeInfo;
	uint16_t vbeMode;
	uint16_t vbeInterfaceSeg;
	uint16_t vbeInterfaceOff;
	uint16_t vbeInterfaceLength;
	
	//gfx
	uint64_t frameBufferAddress;
	uint32_t frameBufferPitch;
	uint32_t frameBufferWidth;
	uint32_t frameBufferHeight;
	uint8_t frameBufferBPP;
	uint8_t frameBufferType;

} __attribute__((packed));



extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {

	printf("Hello :^)\n");

	GlobalDescriptorTable* gdt;
	
	struct MultibootInfo* mbi = (struct MultibootInfo*)multiboot_structure;
	
	//heap manager
	size_t heap = 8*1024*1024;
	MemoryManager memoryManager(heap, mbi->memUpper*1024 - heap - 10*1024);
	
	TaskManager taskManager(gdt, &memoryManager);
	
	InterruptManager interrupts(0x20, gdt, &taskManager);
	printf("Initializing Hardware, Stage 1\n");

	DriverManager drvManager;
	
	//graphics
	uint32_t graphicsWidth = WIDTH_13H;
	uint32_t graphicsHeight = HEIGHT_13H;
	uint8_t* graphicsAddress = nullptr;
	
	//vbe is set and not text mode
	if ((mbi->flags & 0x1000) && mbi->frameBufferAddress != 0xb8000) {
	
		graphicsWidth = mbi->frameBufferWidth;
		graphicsHeight = mbi->frameBufferHeight;
		graphicsAddress = (uint8_t*)mbi->frameBufferAddress;
	}
	
	//drivers and command line
	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	VideoGraphicsArray vga(&memoryManager, graphicsAddress, graphicsWidth, graphicsHeight);
	OFS_Table table;
	FileSystem osakaFileSystem(&ata0m, &memoryManager, &vga, &table);
	Compiler compiler(&osakaFileSystem);
	PIT pit(&interrupts);
	CMOS cmos;
	cmos.pit = &pit;

	CLIKeyboardEventHandler* kbhandler = (CLIKeyboardEventHandler*)memoryManager.malloc(sizeof(CLIKeyboardEventHandler));
	new (kbhandler) CLIKeyboardEventHandler(gdt, &taskManager, &memoryManager, &osakaFileSystem, nullptr, &compiler, &vga, &cmos, &drvManager);
	kbhandler->hash_cli_init(); //init command line
	
	KeyboardDriver keyboard(&interrupts, kbhandler);
	kbhandler->caps = false;
	kbhandler->shift = false;
	kbhandler->ctrl = false;


	//gui driver stuff
	Simulator osaka(&vga, &osakaFileSystem, &cmos);
	Widget setGC(0,0,0,0);
	setGC.gc = &vga;
	Desktop desktop(graphicsWidth, graphicsHeight, 0x7f, &vga, gdt, &taskManager, 
			&memoryManager, &osakaFileSystem, &compiler, 
			&cmos, &drvManager, &osaka);
	MouseDriver mouse(&interrupts, &desktop);


	drvManager.AddDriver(&keyboard);
	drvManager.AddDriver(&mouse);
	drvManager.AddDriver(&ata0m);


	//pci and init
	PeripheralComponentInterconnectController PCIController(&memoryManager);
	PCIController.SelectDrivers(&drvManager, &interrupts);


	printf("\nInitializing Hardware, Stage 2\n");
	drvManager.ActivateAll();
	
	//AC97* ac97 = (AC97*)(drvManager.drivers[4]);

	//sort pci drivers
	AC97* ac97 = nullptr;
	amd_am79c973* eth0 = nullptr;

	for (int i = 0; i < drvManager.numDrivers; i++) {
	
		switch (drvManager.drivers[i]->driverType) {
		
			case 1:
				//network
				eth0 = (amd_am79c973*)(drvManager.drivers[i]);
				break;
			case 2:
				//sound
				ac97 = (AC97*)(drvManager.drivers[i]);
				break;
			default:
				break;
		}
	}


	//AC97* ac97 = (AC97*)(drvManager.drivers[4]);
	//eth0->verbose = true;
	//amd_am79c973 eth0(PCIController.PCIdev, &interrupts);
	//drvManager.drivers[3] = &eth0;
	
	
	//network init
	//IP Address of local machine
	//uint8_t ip1 = 192, ip2 = 168, ip3 = 86, ip4 = 210;
	uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
	uint32_t ip_be = ((uint32_t)ip4 << 24) | ((uint32_t)ip3 << 16) 
			| ((uint32_t)ip2 << 8) | (uint32_t)ip1;
	
	// IP Address of the default gateway
	//uint8_t gip1 = 192, gip2 = 168, gip3 = 86, gip4 = 28;
	uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
	uint32_t gip_be = ((uint32_t)gip4 << 24) | ((uint32_t)gip3 << 16) 
			| ((uint32_t)gip2 << 8) | (uint32_t)gip1;

	uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
	uint32_t subnet_be = ((uint32_t)subnet4 << 24) | ((uint32_t)subnet3 << 16) 
			| ((uint32_t)subnet2 << 8) | (uint32_t)subnet1;

	//eth0->verbose = true;
	eth0->SetIPAddress(ip_be);
	EtherFrameProvider etherframe(eth0, &memoryManager);
	AddressResolutionProtocol arp(&etherframe);
	InternetProtocolProvider ipv4(&etherframe, &arp, gip_be, subnet_be);
	InternetControlMessageProtocol icmp(&ipv4);
	UserDatagramProtocolProvider udp(&ipv4);
	TransmissionControlProtocolProvider tcp(&ipv4);
	
	//NetworkHandler handler;
	Network network(eth0, &arp, &ipv4, &icmp, &udp, &tcp, &osakaFileSystem, gip_be, subnet_be);
	kbhandler->network = &network;
	desktop.network = &network;
	osaka.net = &network;

	printf("Initializing Hardware, Stage 3\n");
	interrupts.Activate();

	
	arp.BroadcastMACAddress(gip_be);

	//request ping
	//icmp.RequestEchoReply(gip_be, nullptr, 0);

	/*
	UserDatagramProtocolSocket* udpsocket = udp.Connect(str2ip("8.8.8.8"), 53);
	udp.Bind(udpsocket, &network);
	uint8_t dns_msg[] = { 0x12, 0x34, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				'p','o','r','t','q','u','i','z', 8, 'n','e','t', 3, 0x00, 0x00, 0x01, 0x00, 0x01 };
	udpsocket->Send(dns_msg, 30);
	*/

	/*
	TransmissionControlProtocolSocket* tcpsocket = network.tcp->Connect(str2ip("35.180.139.74"), 80);
	network.tcp->Bind(tcpsocket, &network);
	sleep(1000);
	char* request = "GET / HTTP/1.1\r\nHost: portquiz.net\r\n\r\n";
	tcpsocket->Send((uint8_t*)request, strlen(request));
	network.tcp->Disconnect(tcpsocket);
	*/

	printf("\n\nEverything seems fine.\n");
	interrupts.boot = true;
	
	//makeBeep(600);
	
	

	//while (1) {}
	//everything beyond this point is no longer testing/initialization

	//initialize command line hash table
	kbhandler->gui = false;
	kbhandler->cli = true;
	//kbhandler->OnKeyDown('\b');
	//printf("\v");

	//printf("vbe mode info: ");
	//printf(int2str(mbi->vbeModeInfo));
	//printf("\n");

	//if booted in vesa mode, skip to gui
	if (graphicsAddress == nullptr) {
		
		//this is the text-mode command line
		while (keyboard.handler->keyValue != 0x5b) { //0x5b = command/windows key	

			kbhandler->cli = true;

			while (kbhandler->cliMode) {

				kbhandler->cli = false;
				kbhandler->modeSelect(kbhandler->cliMode, kbhandler->pressed, 
						kbhandler->keyChar, kbhandler->ctrl, 0, 
						&osakaFileSystem);
			}
		}
	} else {
		//set vbe palette
		//uint32_t vbePMI = ((uint32_t)mbi->vbeInterfaceSeg*16) + ((uint32_t)mbi->vbeInterfaceOff);
		//uint16_t paletteOffset = (&vbePMI)+4;

		vga.colorPaletteMask.Write(0xff);
		vga.colorRegisterWrite.Write(0);

		for (uint16_t i = 0; i < 256; i++) {
		
			vga.colorDataPort.Write(((defaultPalette[i] >> 16) & 0xff) >> 2);
			vga.colorDataPort.Write(((defaultPalette[i] >> 8) & 0xff) >> 2);
			vga.colorDataPort.Write((defaultPalette[i] & 0xff) >> 2);
		}
	}
	kbhandler->cli = true;
	kbhandler->gui = true;


	//initialize desktop
	KeyboardDriver keyboardDesktop(&interrupts, &desktop);
	drvManager.Replace(&keyboardDesktop, 0);

	desktop.CreateButton("CLI", APP_TYPE_TERMINAL, cliButton);
	desktop.CreateButton("PNT", APP_TYPE_KASUGAPAINT, paintButton);
	desktop.CreateButton("JNL", APP_TYPE_JOURNAL, journalButton);
	desktop.CreateButton("WEB", APP_TYPE_SHINOSAKA, browserButton);
	desktop.CreateChild(1, "Osaka's Terminal", kbhandler);
		

	desktop.gc->SetMode(graphicsWidth, graphicsHeight, 8);
	
	//add task for drawing desktop
	LoadDesktopForTask(true, &desktop);
	Task guiTask(gdt, DrawDesktopTask, "osakaOS GUI", 0);
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
