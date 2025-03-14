#include <gui/sim.h>

using namespace os::common;
using namespace os::drivers;
using namespace os::gui;
using namespace os::math;


char* int2str(uint32_t);
//void sleep(uint32_t);
uint16_t prng();
void reboot();



Simulator::Simulator(CMOS* cmos) {

	this->lastOsakaX = 155;
	this->lastOsakaY = 65;
	this->osakaX = 155;
	this->osakaY = 65;
	
	this->mouseX = osakaX;
	this->mouseY = osakaY;

	this->stepsNum = 0;
	this->walkCycle = 1;


	for (int i = 0; i < 320; i++) {
		
		if (i < 16) { keylog[i] = 0x00; }
		walkPixels[i].x = 0;
		walkPixels[i].y = 0;
	}

	this->cmos = cmos;
	cmos->ReadRTC();
}

Simulator::~Simulator() {
}



void Simulator::OnKeyDown(char ch) {

	keyPress = true;
	bool keyFound = false;
	
	for (uint8_t i = 0; i < logIndex; i++) {
	
		keyFound = (keylog[i] == ch);
	}
	
	if (keyFound == false) {

		this->keylog[logIndex] = ch;
		logIndex++;
	}
	
	//universal key inputs
	switch (ch) {
	
		case 0x1b: //escape
			this->menu = false;

			//cleanup and resest games
			if (this->mode) { 
				
				osakaX = lastOsakaX;
				osakaY = lastOsakaY;
			}
			this->mode = 0;
			break;
		case 'f': //fly mode
			if (this->mode == 0 && walkCycle == 1 && flying == false) {
			
				lastOsakaX = osakaX;
				lastOsakaY = osakaY;
				this->flying = true;
			}
			break;
		default:
			break;
	}
}


void Simulator::OnKeyUp(char ch) {

	for (uint8_t i = 0; i < logIndex; i++) {
	
		if (keylog[i] == ch) {
		
			keylog[i] = 0x00;
			logIndex--;
		}
	}

	if (logIndex == 0) { keyPress = false; }
}


void Simulator::OnMouseDown(int32_t newMouseX, int32_t newMouseY, uint8_t button) {

	if (mode != 0) { return; }

	//menu select
	if (this->menu) {
	
		this->MenuSelect(newMouseX, newMouseY, button);
		return;
	}
	
	//tv select
	if (newMouseX > 142 && newMouseX < 179 
	&& newMouseY > 40 && newMouseY < 70) {
	
		this->menu = true;
		this->menuType = 1;
	
	//dresser	
	} else if (newMouseX > 184 && newMouseX <= 233 
		&& newMouseY > 46 && newMouseY <= 88) {
	
		this->menu = true;
		this->menuType = 2;

	//bed select
	} else if ((newMouseX > 241 && newMouseX < 281 && newMouseY > 105 && newMouseY < 140) || 
		   (newMouseX > 270 && newMouseX < 317 && newMouseY > 79 && newMouseY < 106)) {
	
		this->sleeping ^= 1;
		this->ticks = 0;
	//lamp
	} else if (newMouseX > 235 && newMouseX <= 252 && newMouseY > 34 && newMouseY <= 50) {
		
		this->lampOn ^= 1;
		if (lampOn) { darkLevel = 0; }
		else {	      darkLevel = 1; }

	//computer
	} else if (newMouseX > 31 && newMouseX <= 49 && newMouseY > 54 && newMouseY <= 70) {
	
		this->sim = false;
	//bong	
	} else if (newMouseX > 6 && newMouseX <= 17 && newMouseY > 84 && newMouseY <= 102) {
	
		this->waveLength ^= 20;
	//walking
	} else {
		if (walkCycle == 1 && flying == false 
			&& (newMouseY > 70 || 
			(newMouseX > 80 && newMouseX < 240 && newMouseY > 35))) {	
		
			mousePress = true;
			stepsNum = LineFillArray(mouseX, mouseY, newMouseX, newMouseY, walkPixels);

			backwardsX = mouseX > newMouseX;
			backwardsY = mouseY > newMouseY;
			direction = abs(mouseX - newMouseX) > abs(mouseY - newMouseY);
			this->mouseX = newMouseX;
			this->mouseY = newMouseY;
		}
	}
}


void Simulator::OnMouseMove(int32_t MouseX, int32_t MouseY, int32_t newMouseX, int32_t newMouseY) {

	this->moveMouseX = newMouseX - MouseX;
	this->moveMouseY = newMouseY - MouseY;
	//this->moveMouseX = newMouseX;
	//this->moveMouseY = newMouseY;
}




uint8_t Simulator::MenuSelect(int32_t newMouseX, int32_t newMouseY, uint8_t button) {

	uint8_t select = 0;	

	//find menu option user selected
	for (uint8_t i = 0; i < 2; i++) {
		for (uint8_t j = 0; j < 4; j++) {
		
			int32_t x = newMouseX-(84+(44*j));
			int32_t y = newMouseY-(70+(40*i));

			if (x >= 0 && x < 20 && y >= 0 && y < 20) {
				
				select = (4*i+j)+1;
				break;
			}
		}
	}

	//not a thing yet lol
	uint8_t gameData[] = {
	};
	
	//tv
	if (menuType == 1) {

		lastOsakaX = osakaX;
		lastOsakaY = osakaY;
	
		switch (select) {
		
			//cable
			case 1:
				break;
			//platformer
			case 2:
				this->mode = 1;
				this->platformer.LoadData();
				break;
			//shooter
			case 3:
				this->mode = 2;
				this->shooter.LoadData();
				break;
			default:
				break;
		}
	//dresser
	} else if (menuType == 2) {

		this->outfit = select - 1;
	}

	//turn off menu
	//when option is selected	
	if (select) {
	
		this->menu = false;
		this->menuType = 0;
	}
	return select;
}


void Simulator::DrawMenuSelect(GraphicsContext* gc) {

	//background
	gc->FillRectangle(0, 0, 320, 200, 0x09);
	gc->FillRectangle(60, 50, 200, 100, 0x08);
	gc->DrawRectangle(60, 50, 200, 100, 0x07);
	
	char* str1 = "null"; char* str2 = "null"; 
	char* str3 = "null"; char* str4 = "null"; 
	char* str5 = "null"; char* str6 = "null"; 
	char* str7 = "null"; char* str8 = "null";

	switch (menuType) {
	
		//tv
		case 1:
			str1 = "TBA";
			str2 = "WIP 1";
			str3 = "WIP 2";
			break;
		//dresser
		case 2:
			str1 = "Winter";
			str2 = "Summer";
			break;
		default:
			break;
	}

	uint8_t strIndex = 0;
	char* strArr[8] = {str1, str2, str3, str4, str5, str6, str7, str8};
	
	for (uint8_t i = 0; i < 2; i++) {
		for (uint8_t j = 0; j < 4; j++) {
	
			//draw boxes
			gc->FillRectangle(84+(44*j), 70+(40*i), 20, 20, 0x40);
			gc->DrawRectangle(84+(44*j), 70+(40*i), 20, 20, 0x38);
	
			//print name
			gc->PutText(strArr[strIndex], 77+(44*j), 94+(40*i), 0x3f);
			strIndex++;
		}
	}
}



//normal room
void Simulator::TimeAndDate(char* timeDateString) {

	static uint8_t ticks = 0;	
	
	if (ticks >= 60) {
	
		cmos->ReadRTC();
		ticks = 0;
	}
	
	timeDateString[0] = (cmos->timeData.month / 10) + '0';
	timeDateString[1] = (cmos->timeData.month % 10) + '0';
	
	timeDateString[3] = (cmos->timeData.day / 10) + '0';
	timeDateString[4] = (cmos->timeData.day % 10) + '0';
	
	timeDateString[6] = (cmos->timeData.year / 10) + '0';
	timeDateString[7] = (cmos->timeData.year % 10) + '0';
	

	timeDateString[11] = (cmos->timeData.hour / 10) + '0';
	timeDateString[12] = (cmos->timeData.hour % 10) + '0';
	
	timeDateString[14] = (cmos->timeData.minute / 10) + '0';
	timeDateString[15] = (cmos->timeData.minute % 10) + '0';

	timeDateString[17] = (cmos->timeData.second / 10) + '0';
	timeDateString[18] = (cmos->timeData.second % 10) + '0';

	ticks++;
}


void Simulator::Dream(GraphicsContext* gc) {


	//gc->FillBuffer(241, 79, 76, 61, bed, false);

	//first dream bubble
	gc->FillBuffer(275, 73, 4, 4, dreamBubbleMini1, false);

	//second dream bubble	
	if (ticks > 780) {
	
		gc->FillBuffer(264, 67, 5, 5, dreamBubbleMini2, false);
	}
	
	//third dream bubble	
	if (ticks > 900) {
	
		gc->FillBuffer(220, 40, 21, 29, dreamBubbleHalf, false);
		gc->FillBuffer(240, 40, 21, 29, dreamBubbleHalf, true);

		switch (this->ticks / (3600)) {
	
			case 0:
				gc->FillBuffer(224, 44, 17, 19, cursorChiChi, false);
				gc->FillBuffer(242, 44, 13, 20, osakaPlatSprite1, true);
				break;
			case 1:
				gc->PutText("=D", 230, 50, 0x40);
				break;
			case 2:
				break;
			case 3:
				gc->PutText("fuck\nC++", 225, 50, 0x40);
				break;
			default:
				break;
		}
	}
}


void Simulator::Fly(GraphicsContext* gc, uint16_t osakaSpriteNum) {
	
	//draw crying face
	switch (osakaSpriteNum) {
	
		case 1:
			for (uint8_t i = 9; i < 14; i++) { gc->PutPixel(osakaX+4, osakaY+i, 0x2b); }
			for (uint8_t i = 9; i < 15; i++) { gc->PutPixel(osakaX+9, osakaY+i, 0x2b); }
			for (uint8_t i = 5; i < 9; i++) { gc->PutPixel(osakaX+i, osakaY+12, 0x37);}
			break;
		case 0:
			for (uint8_t i = 9; i < 15; i++) { gc->PutPixel(osakaX+6, osakaY+i, 0x2b); }
			for (uint8_t i = 9; i < 14; i++) { gc->PutPixel(osakaX+11, osakaY+i, 0x2b); }
			for (uint8_t i = 7; i < 11; i++) { gc->PutPixel(osakaX+i, osakaY+12, 0x37);}
			break;
		default:
			break;
	}
	osakaY--;

	if ((osakaY % 20) < 10) {
	
		gc->FillBuffer(osakaX-6, osakaY-6, 9, 10, pigtailUp, false);
		gc->FillBuffer(osakaX+13, osakaY-6, 9, 10, pigtailUp, true);
	} else {
		gc->FillBuffer(osakaX-7, osakaY, 11, 8, pigtailDown, true);
		gc->FillBuffer(osakaX+12, osakaY, 11, 8, pigtailDown, false);
	}

	if (osakaY < -150) {
	
		osakaX = lastOsakaX;
		osakaY = lastOsakaY;
		this->flying = false;
	
		//event here
		this->event = (prng() % 3);
		this->ticks = 0;
	}
}



void Simulator::Walk() {

	uint16_t walkIndex = walkCycle;
	
	//choose reverse or forward walking animations
	//depending on direction of new coordinate
	if (((backwardsX & backwardsY)
	 || ((backwardsX & direction)) 
	 || ((backwardsY ^ direction))) 
	 && (backwardsX | backwardsY))  {
	
		walkIndex = stepsNum - walkCycle;
	}


	if (mousePress && walkCycle < stepsNum) {	

		osakaX = walkPixels[walkIndex].x;
		osakaY = walkPixels[walkIndex].y;

		walkCycle++;
	} else {	
		if (mousePress) {
		
			for (int i = 0; i < 320; i++) {
		
				walkPixels[i].x = 0;
				walkPixels[i].y = 0;
			}
			this->osakaX = mouseX;
			this->osakaY = mouseY;

			mousePress = false;
			walkCycle = 1;
			stepsNum = 0;
		}
	}
}



void Simulator::ComputeGameState() {
	
	switch (mode) {
	
		case 1: //platformer
			this->platformer.ComputePlatformer(keylog, logIndex, keyPress);
			break;
		case 2: //shooter
			this->shooter.ComputeShooter(keylog, logIndex, keyPress, moveMouseX, moveMouseY);
			break;
		
		default://normal room
			this->Walk();
			break;
	}
	this->ticks++;

	//60 frames
	this->cmos->pit->sleep(15);
}


void Simulator::DrawRoom(GraphicsContext* gc) {

	this->ComputeGameState();
	
	//draw menu select
	if (this->menu) {
	
		DrawMenuSelect(gc);
		return;
	}

	//event draw
	if (this->event) {
				
		switch (event) {
		
			case 1:
				gc->FillRectangle(0, 0, 320, 200, 0x40);
				if (ticks >= 360) { gc->PutText("GAME OVER", 133, 96, 0x24); }
				if (ticks >= 720) { reboot(); }
				ticks++;
				break;
			case 2:
				if (ticks >= 360) { gc->ErrorScreen(); }
				if (ticks >= 1080) { reboot(); }
				ticks++;
				break;
			default:
				ticks = 0;
				break;
		}
		return;
	}

	//draw games/tv
	if (this->mode) {
	
		switch (mode) {
		
			case 1: //platformer
				this->platformer.Draw(gc, ticks);
				break;
			case 2: //shooter
				this->shooter.Draw(gc, ticks, keylog, logIndex, keyPress, moveMouseX, moveMouseY);
				break;
			default:
				break;
		}
		return;
	}


	//background
	gc->FillRectangle(0, 0, 320, 200, 0x08);
	
	//floor
	gc->FillRectangle(0, 95, 320, 200, 0x14);
	uint16_t xFloor[] = {0, 160, 320};
	uint16_t yFloor[] = {95, 65, 95};
	gc->FillPolygon(xFloor, yFloor, 3, 0x14);


	//rug
	uint16_t xRug[] = {90, 160, 230, 160};
	uint16_t yRug[] = {120, 100, 120, 140};
	gc->FillPolygon(xRug, yRug, 4, 0x3f);
	uint16_t xRug2[] = {105, 160, 215, 160};
	uint16_t yRug2[] = {120, 105, 120, 135};
	gc->FillPolygon(xRug2, yRug2, 4, 0x23);

	
	//walls
	//brown
	uint16_t xWall1[] = {0, 160, 320, 320, 160, 0};
	uint16_t yWall1[] = {95, 65, 95, 80, 50, 80};
	gc->FillPolygon(xWall1, yWall1, 6, 0x20);

	//green
	uint16_t yWall2[] = {80, 50, 80, 70, 40, 70};
	gc->FillPolygon(xWall1, yWall2, 6, 0x2a);

	//beige
	uint16_t yWall3[] = {70, 40, 70, 20, 0, 20};
	gc->FillPolygon(xWall1, yWall3, 6, 0x37);
	
	//outlines	
	gc->DrawLine(0, 95, 160, 65, 0x40);
	gc->DrawLine(160, 65, 320, 95, 0x40);
	gc->DrawLine(0, 80, 160, 50, 0x40);
	gc->DrawLine(160, 50, 320, 80, 0x40);
	gc->DrawLine(160, 50, 320, 80, 0x40);
	gc->DrawLine(160, 40, 0, 70, 0x40);
	gc->DrawLine(160, 40, 320, 70, 0x40);
	gc->DrawLine(160, 0, 160, 65, 0x40);
	gc->DrawLine(0, 20, 160, 0, 0x40);
	gc->DrawLine(160, 0, 320, 20, 0x40);




	//draw the items inside the room

	//posters
	gc->FillBuffer(208, 15, 20, 30, catPoster, false);
	gc->FillBuffer(267, 26, 20, 30, awakeningPoster, false);
	gc->FillBuffer(9, 29, 20, 30, brazilPoster, true);

	//wall outlets
	gc->FillBuffer(127, 59, 7, 10, wallOutlet, false);
	gc->FillBuffer(25, 78, 7, 10, wallOutlet, false);
	gc->FillBuffer(263, 73, 7, 10, wallOutlet, true);


	//tv stand
	gc->FillRectangle(141, 68, 38, 7, 0x20);
	gc->DrawRectangle(141, 68, 38, 7, 0x40);
	gc->FillRectangle(138, 61, 44, 7, 0x20);
	gc->DrawRectangle(138, 61, 44, 7, 0x40);
	gc->DrawLine(158, 71, 162, 71, 0x38);
	//tv
	gc->FillBuffer(141, 33, 38, 33, crtTV, false);
	gc->DrawLine(130, 62, 138, 65, 0x40);
	//tv flash
	gc->FillRectangle(145, 37, 30, 22, (((ticks/5)%2)*8)+1);
	
	//game console
	gc->FillBuffer(122, 73, 17, 15, gameConsole, false);
	gc->DrawLine(131, 75, 130, 65, 0x40);
	
	
	//computer
	gc->FillBuffer(30, 78, 20, 23, computer, false);
	//desk
	gc->FillBuffer(18, 60, 49, 46, desk, false);
	
	//monitor
	gc->FillBuffer(27, 53, 22, 18, crtPC, false);
	//keyboard
	gc->FillBuffer(35, 69, 16, 7, keyboard, false);
	//keyboard cable
	gc->FillBuffer(26, 66, 9, 8, keyboardCable, false);
	//mouse
	gc->FillBuffer(51, 62, 7, 9, mouse, false);
	//bong
	gc->FillBuffer(6, 84, 11, 18, bongDirty, false);
	


	//chair
	gc->FillBuffer(43, 71, 30, 40, chair, false);


	//dresser
	gc->FillBuffer(184, 63, 49, 26, dresser, true);
	gc->FillBuffer(184, 56, 49, 26, dresser, true);

	//photo frame
	gc->FillBuffer(192, 58, 10, 10, photoFrame, false);


	//blinds
	for (uint8_t i = 15; i < 42; i += 3) {
		gc->FillBuffer(65, i, 35, 8, blindsRepeat, false);
	}
	gc->DrawLine(67, 23, 67, 42, 0x40);
	gc->DrawLine(68, 23, 68, 42, 0x07);
	gc->DrawLine(69, 23, 69, 42, 0x40);
	gc->PutPixel(68, 41, 0x40);
	gc->FillBuffer(64, 42, 38, 9, windowShelf, false);


	//lamp
	gc->FillBuffer(237, 33, 17, 59, lamp, false);
	//bed
	gc->FillBuffer(241, 79, 76, 61, bed, false);
	
	/*
	//files
	for (uint16_t i = 0; i < fileNum; i++) {
	
		switch (i%6) {
		
			case 0:
				gc->FillBuffer(false);
				break;
			default:
				break;
		}
	}
	*/

	//walking
	uint8_t walkFrames = 0;
	walkFrames = (1*(backwardsX)) + (2*backwardsY);
	if ((walkCycle % 40 < 5)) {
	} else if ((walkCycle % 40 < 20)) { walkFrames += 4 + walkFrames;
	} else if ((walkCycle % 40 < 25)) {
	} else { walkFrames += 5 + walkFrames; }
	

	//draw osaka
	if (this->sleeping) {
	
		gc->FillBuffer(284, 82, 17, 10, osakaSleep, false);
		if (this->ticks > 600) { this->Dream(gc); }
	} else {
		bool facingLeft = false;

		if (walkFrames == 1 || walkFrames == 3 || 
			walkFrames == 6 || walkFrames == 7 || 
			walkFrames == 10 || walkFrames == 11) {
		
			facingLeft = true;
		}
		gc->FillBuffer(osakaX, osakaY, osakaWidths[walkFrames], 49, osakaSprites[walkFrames], facingLeft);

		switch (this->outfit) {
		
			case 1:
				gc->FillBuffer(osakaX, osakaY+18, osakaWidths[walkFrames], 19, osakaSummerSprites[walkFrames], facingLeft);
				break;
			default:
				break;
		}
		if (this->flying) { this->Fly(gc, walkFrames); }
	}

	//draw ui stuff
	char* timeDateString = "00/00/00 - 00:00:00";
	TimeAndDate(timeDateString);
	gc->PutText(timeDateString, 6, 191, 0x40);
	gc->PutText(timeDateString, 5, 190, 0x3f);
}
