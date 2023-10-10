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

	printfTUI(0x00, 0x00, 0, 0, 79, 24, false);
	
	//health
	printfColor("HEALTH [", 0x0f, 0x00, 4, 0);
	
	for (int i = 12; i < 32; i++) {

		putchar(0xff, 0x04, 0x04, i, 0);
	}
	printfColor("]", 0x0f, 0x00, 32, 0);
	
	printfColor("Press space to fire.", 0x0f, 0x00, 37, 0);

	//rest of ui
	printfColor("SCORE = 0000", 0x0f, 0x00, 64, 0);
	
	for (int i = 0; i < 80; i++) {
	
		putchar(0xcd, 0x0f, 0x00, i, 1);
	}
}



void spaceInit() {

}


void space(bool pressed, char key) {

	static uint8_t x = 40;
	static uint8_t y = 23;
	
	static uint8_t ax = 0;
	static uint8_t ay = 2;
	
	static uint8_t cx = 0;
	static uint8_t cy = 2;
	
	static uint8_t ex = 0;

	static int8_t health = 100;
	static uint16_t score = 0;
	static uint16_t gameTicks = 0;
	
	static bool asteroid = false;
	static bool fire = false;
	uint8_t i = 0;
		
	if (key != ' ' || !pressed) {
		fire = false;
	}
	

	if (pressed) {	

		//replace old position
		printfColor("               ", 0x0f, 0x00, x-7, y-1);
		printfColor("               ", 0x0f, 0x00, x-7, y  );
		printfColor("               ", 0x0f, 0x00, x-7, y+1);

		switch (key) {
	
			case 'a':
				x -= 1 * (x > 10);
				break;
			case 'd':
				x += 1 * (x < 70);
				break;
			case ' ':
				if (!fire) {
				
					for (i = 21; i > 2; i--) {
					
						putchar(' ', 0x04, 0x00, x, i+1);
						putchar('|', 0x04, 0x00, x, i);
						sleep(5);
					}
					makeBeep(1200);
					putchar(' ', 0x04, 0x00, x, i+1);
					
					if (x == ax) {
					
						putchar('*', 0x09, 0x00, ax, ay);
						asteroid = false;
						score += 50;
						makeBeep(50);
						putchar(' ', 0x09, 0x00, ax, ay);
					}
				}
				fire = true;
				break;
			case 'r':
				x = 40;
				y = 23;
				ax = 0;
				ay = 2;
				ex = 0;
				health = 100;
				score = 0;
				gameTicks = 0;
				asteroid = false;
				fire = false;
				spaceTUI();
				return;
				break;
			default:
				break;
		}
	}
	
	//game over
	if (health < 1) {
	
		printfColor("GAME OVER", 0x0f, 0x00, 36, 12);
		return;
	}

	//new position
	printfColor("   \\  ___  /   ", 0x0f, 0x00, x-7, y-1);
	printfColor("____\\/___\\/____", 0x0f, 0x00, x-7, y  );
	printfColor("     v   v     ", 0x01, 0x00, x-7, y+1);




	//asteroids
	if (ay >= 24) {
	
		putchar(' ', 0x06, 0x00, ax, ay);
		asteroid = false;
		ax = 0;
	}

	if (asteroid) {
	
		putchar(' ', 0x06, 0x00, ax, ay);
		ay++;
		putchar('o', 0x06, 0x00, ax, ay);
	} else {	
		ay = 2;
		ax = 0;
		
		if (prng() % 10 == 0) {
			ax = (gameTicks % 70) + 4;	
			putchar('o', 0x06, 0x00, ax, ay);
		}
	}
	asteroid = (ax > 2);
	

	//collision with player
	if (ay >= y-1 && ((ax > x-7) && (ax <= x+7))) {
	
		ay = 24;
		health -= 10;
		makeBeep(100);
		putchar(0xff, 0x00, 0x00, 12 + (health / 5), 0);
		putchar(0xff, 0x00, 0x00, 12 + (health / 5) + 1, 0);
	}



	//vidmem = (volatile uint16_t*)0xb8000 + (80*x+y);
	gameTicks++;
	
	putchar((score / 1000) + 48, 0x0f, 0x00, 72, 0);
	putchar((score / 100) + 48, 0x0f, 0x00, 73, 0);
	putchar(((score / 10) % 10) + 48, 0x0f, 0x00, 74, 0);
	putchar((score % 10) + 48, 0x0f, 0x00, 75, 0);
	
	sleep(25);
}


