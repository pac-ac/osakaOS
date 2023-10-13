#include <mode/snake.h>

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



//this was not made very well but I just wanted to get it done



void snakeTUI() {

	printfTUI(0x00, 0x00, 0, 0, 0, 0, false);
	printfColor("(Snake mode, press ctrl+c to exit.)", 0x0f, 0x00, 0, 0);
	printfColor("SCORE = ", 0x0f, 0x00, 40, 0);

	for (int i = 0; i < 80; i++) {
	
		putchar(0xcd, 0x0f, 0x00, i, 1);
	}
}



void snakeInit() {

	//tail
	putchar(0xff, 0x06, 0x06, 40, 9);
	
	putchar(0xff, 0x0a, 0x0a, 40, 10);
	putchar(0xff, 0x0a, 0x0a, 40, 11);
	
	//head
	putchar(0xff, 0x02, 0x02, 40, 12);
}


void snake(char key) {

	static uint8_t x = 40, y = 12;
	static uint8_t tailX = 40, tailY = 9;
	volatile uint16_t* vidmem;
	

	static uint8_t bodyChar = 0;	
	static uint8_t bodyFore = 0;	
	static uint8_t tail = 0;
	static char lastChar = 'w';


	static bool vertical = 1;
	static bool up = 0;
	static bool right = 0;


	static uint8_t fruitX = 0;	
	static uint8_t fruitY = 0;	
	static bool fruit = false;	
	

	static uint8_t score = 0;
	static bool dead = false;
	static uint16_t speed = 100;


	if (dead && key != 'r') {	
			return;
	}
	


	//if (tail <= tailUpdate) {
	if (score <= tail) {

		//remove snake body from previous tail
		putchar(0xff, 0x00, 0x00, tailX, tailY);
	

							   //some light green char in vidmem i guess
		vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY-(1 * tailY != 25))+(tailX));
		uint16_t findU = *vidmem;

		vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY+(1))+(tailX));
		uint16_t findD = *vidmem;
	
		vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY)+(tailX-1));
		uint16_t findL = *vidmem;
	
		vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY)+(tailX+1));
		uint16_t findR = *vidmem;
	
	
		uint16_t find = 0xffff;
	


		for (uint8_t i = 0; i < 4; i++) {

			switch (i) {
		
				case 0:
					vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY-1)+(tailX));
					break;
				case 1:
					vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY+1)+(tailX));
					break;
				case 2:
					vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY)+(tailX-1));
					break;
				case 3:
					vidmem = (volatile uint16_t*)0xb8000 + (80*(tailY)+(tailX+1));
					break;
				default:
					break;
		}

			if ((find > *vidmem) && (*vidmem >> 12) == 0xa) {
		
				find = *vidmem;
			}
		}
	
		tailY -= (find == findU);
		tailY += (find == findD);
		tailX -= (find == findL);
		tailX += (find == findR);

		//print snake tail
		putchar(0xff, 0x06, 0x06, tailX, tailY);
	}
	
	tail = score;



	//print snake body
	putchar(bodyChar, bodyFore, 0x0a, x, y);
	

	if (lastChar != key) {
	
		switch (key) {

			case 'w':
				up = 1;
				vertical = 1;
				lastChar = 's';
				break;
			case 'a':
				right = 0;
				vertical = 0;
				lastChar = 'd';
				break;
			case 's':
				up = 0;
				vertical = 1;
				lastChar = 'w';
				break;
			case 'd':
				right = 1;
				vertical = 0;
				lastChar = 'a';
				break;
			case 'r':
				
				x = 40;
				y = 12;
				
				tailX = 40;
				tailY = 9;
				lastChar = 'w';
	
				bodyChar = 0;	
				bodyFore = 0;	

				vertical = 1;
				up = 0;
				right = 0;
				
				score = 0;
				tail = 0;
				speed = 100;
				
				dead = false;
				fruit = false;

				snakeTUI();
				snakeInit();
				sleep(250);
				
				return;
				break;

			default:
				break;
		}
	}


	//move
	if (vertical) { 
		if (up) {y--;} else {y++;} 
	} else { 
		if (right) {x++;} else {x--;} 
	}


	
	//game over	
	vidmem = (volatile uint16_t*)0xb8000 + (80*y+x);
	dead = ((*vidmem >> 12) == 0xa);
	
	
	if (x >= 80 || y <= 1 || y >= 25 || dead) {

		dead = true;
			
		printfTUI(0x04, 0x04, 0, 0, 0, 0, false);
		printfColor("-GAME OVER-", 0x0f, 0x04, 35, 10);	
		
		printfColor("Score was ", 0x0f, 0x04, 34, 12);	
		char* foo = "   ";
		foo[0] = (score / 100) + 48;
		foo[1] = ((score / 10) % 10) + 48;
		foo[2] = (score % 10) + 48;
		printfColor(foo, 0x0f, 0x04, 44, 12);	
		
		printfColor("Press 'r' to restart.", 0x0f, 0x04, 30, 13);	
		
		makeBeep(294);
		sleep(300);

		makeBeep(220);
		sleep(300);

		makeBeep(208);
		sleep(300);

		return;
	}


	//spawn new fruit
	if (!fruit) {

		do {		
			fruitX = (((uint8_t)prng()) % 78) + 2;
			fruitY = (((uint8_t)prng()) % 21) + 4;
	
			vidmem = (volatile uint16_t*)0xb8000 + (80*fruitY+fruitX);
		
		} while (*vidmem != 0x00ff);

		putchar(0xff, 0x04, 0x04, fruitX, fruitY);
	}

	fruit = true;	


	//player gains point
	if (x == fruitX && y == fruitY) {

		score++;
		fruit = false;
	
		if (score % 5 == 0 && speed > 20) {
		
			speed -= 2;
		}
		
		makeBeep(1000);
	}


	//print snake head
	putchar(0xff, 0x02, 0x02, x, y);
	bodyChar++;
	
	if (bodyChar == 0xff) {
		
		if (bodyFore >= 0x0f) {
	
			bodyFore = 0;
		}
		bodyFore++;
	}


	//print score
	putchar((score / 100) + 48, 0x0f, 0x00, 48, 0);
	putchar(((score / 10) % 10) + 48, 0x0f, 0x00, 49, 0);
	putchar((score % 10) + 48, 0x0f, 0x00, 50, 0);


	sleep(speed);
}


