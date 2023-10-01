#include <mode/space.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;


void printfTUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void printfColor(char*, uint8_t, uint8_t, uint8_t, uint8_t);
void putchar(unsigned char, unsigned char, 
		unsigned char, uint8_t, uint8_t);

void sleep(uint32_t);
uint16_t prng();
void makeBeep(uint32_t);





void spaceTUI() {

	printfTUI(0x00, 0x09, 8, 8, 72, 17, false);
	printfColor("SCORE = 0000", 0x0f, 0x09, 34, 0);


	for (int i = 0; i < 8; i++) { 
	
		putchar('\\', 0x0f, 0x09, i, i); 
		putchar('/', 0x0f, 0x09, 79-i, i); 
		putchar('/', 0x0f, 0x09, i, 24-i); 
		putchar('\\', 0x0f, 0x09, 79-i, 24-i); 
	}
	
}



void spaceInit() {

}


void space(bool pressed, char key) {

	static uint8_t x = 40;
	static uint8_t y = 23;
	
	static uint16_t score = 0;
	static uint16_t gameTicks = 0;


	static uint8_t jumpTicks = 0;
	static bool jump = false;

	volatile uint16_t* vidmem;
	
	//vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
	//replace old character
	if (y < 17) {
		putchar(0xff, 0x00, 0x00, x, y);
	} else {
		putchar(0xff, 0x09, 0x09, x, y);
	}	



	if (pressed) {	
	
		switch (key) {
	
			case 'a':
				x -= 1 * (x > 9);
				break;
			case 'd':
				x += 1 * (x < 71);
				break;
			case ' ':
				if (jumpTicks == 0) {
				
					jump = true;
				}
				break;
			case 'r':
				x = 40;
				y = 23;
				score = 0;
				gameTicks = 0;
				jumpTicks = 0;
				jump = false;
				return;
				break;
			default:
				break;
		}
	}




	//jumping
	if (jump) {
	
		y -= 1 * (y > 10);
		jumpTicks++;
	} else {
		
		y += 1 * (y < 23);	
	}

	if (jumpTicks > 30) {
	
		jump = false;
	}
	if (y == 23) {
	
		jumpTicks = 0;
	}





		





	//draw new character
	if (y < 17) {
		putchar('W', 0x0f, 0x00, x, y);
	} else {
		putchar('W', 0x0f, 0x09, x, y);
	}	


	//vidmem = (volatile uint16_t*)0xb8000 + (80*x+y);
	sleep(25);
}


