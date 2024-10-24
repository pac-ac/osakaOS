#include <mode/space.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;


void TUI(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool);
void printfTUI(char*, uint8_t, uint8_t, uint8_t, uint8_t);
void putcharTUI(unsigned char, unsigned char, 
		unsigned char, uint8_t, uint8_t);

void sleep(uint32_t);
uint16_t prng();
void makeBeep(uint32_t);





void spaceTUI() {

	TUI(0x00, 0x00, 0, 0, 79, 24, false);
	
	//health
	printfTUI("HEALTH [", 0x0f, 0x00, 4, 0);
	
	for (int i = 12; i < 32; i++) {

		putcharTUI(0xff, 0x04, 0x04, i, 0);
	}
	printfTUI("]", 0x0f, 0x00, 32, 0);
	
	printfTUI("Press space to fire.", 0x0f, 0x00, 37, 0);

	//rest of ui
	printfTUI("SCORE = 0000", 0x0f, 0x00, 64, 0);
	
	for (int i = 0; i < 80; i++) {
	
		putcharTUI(0xcd, 0x0f, 0x00, i, 1);
	}
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
		printfTUI("               ", 0x0f, 0x00, x-7, y-1);
		printfTUI("               ", 0x0f, 0x00, x-7, y  );
		printfTUI("               ", 0x0f, 0x00, x-7, y+1);

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
					
						putcharTUI(' ', 0x04, 0x00, x, i+1);
						putcharTUI('|', 0x04, 0x00, x, i);
						sleep(5);
					}
					makeBeep(1200);
					putcharTUI(' ', 0x04, 0x00, x, i+1);
					
					if (x == ax) {
					
						putcharTUI('*', 0x09, 0x00, ax, ay);
						asteroid = false;
						score += 5000;
						makeBeep(50);
						putcharTUI(' ', 0x09, 0x00, ax, ay);
					}
					score += 50;
					makeBeep(500);
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
	
		printfTUI("GAME OVER", 0x0f, 0x00, 36, 12);
		return;
	}

	//new position
	printfTUI("   \\  ___  /   ", 0x0f, 0x00, x-7, y-1);
	printfTUI("____\\/___\\/____", 0x0f, 0x00, x-7, y  );
	printfTUI("     v   v     ", 0x01, 0x00, x-7, y+1);




	//asteroids
	if (ay >= 24) {
	
		putcharTUI(' ', 0x06, 0x00, ax, ay);
		asteroid = false;
		ax = 0;
	}

	if (asteroid) {
	
		putcharTUI(' ', 0x06, 0x00, ax, ay);
		ay++;
		putcharTUI('o', 0x06, 0x00, ax, ay);
	} else {	
		ay = 2;
		ax = 0;
		
		if (prng() % 10 == 0) {
			ax = (gameTicks % 70) + 4;	
			putcharTUI('o', 0x06, 0x00, ax, ay);
		}
	}
	asteroid = (ax > 2);
	

	//collision with player
	if (ay >= y-1 && ((ax > x-7) && (ax <= x+7))) {
	
		ay = 24;
		health -= 10;
		makeBeep(100);
		putcharTUI(0xff, 0x00, 0x00, 12 + (health / 5), 0);
		putcharTUI(0xff, 0x00, 0x00, 12 + (health / 5) + 1, 0);
	}
	gameTicks++;
	if (gameTicks % 50 == 0) { score++; }
	
	putcharTUI((score / 1000) + 48, 0x0f, 0x00, 72, 0);
	putcharTUI((score / 100) + 48, 0x0f, 0x00, 73, 0);
	putcharTUI(((score / 10) % 10) + 48, 0x0f, 0x00, 74, 0);
	putcharTUI((score % 10) + 48, 0x0f, 0x00, 75, 0);
	
	sleep(25);
}


