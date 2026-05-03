#include <gui/sim.h>

using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;
using namespace os::gui;
using namespace os::math;


//void sleep(uint32_t);
uint16_t prng();
void reboot();
uint8_t* memset(uint8_t*, int, size_t);


Simulator::Simulator(GraphicsContext* gc, FileSystem* ofs, CMOS* cmos) {

	this->lastOsakaX = 155;
	this->lastOsakaY = 65;
	this->osakaX = 155;
	this->osakaY = 65;
	
	this->mouseX = osakaX;
	this->mouseY = osakaY;

	this->stepsNum = 0;
	this->walkCycle = 1;


	for (int i = 0; i < WIDTH_13H; i++) {
		
		if (i < 16) { keylog[i] = 0x00; }
		walkPixels[i].x = 0;
		walkPixels[i].y = 0;
	}

	this->gc = gc;
	this->ofs = ofs;
	this->cmos = cmos;
	cmos->ReadRTC();


	//set object coordinates
	for (int i = 0; i < 16; i++) {
	
		this->ObjectLocations[i].x = 0;
		this->ObjectLocations[i].y = 0;
	}

	this->objectTarget = 0x00;


	if (ofs->FileIf(ofs->GetFileSector("*ROOM"))) {

		this->LoadRoom();
	} else {
		this->ObjectLocations[tvVal].x = 141;
		this->ObjectLocations[tvVal].y = 33;
	
		this->ObjectLocations[gameConsoleVal].x = 122;
		this->ObjectLocations[gameConsoleVal].y = 73;
	
		this->ObjectLocations[computerVal].x = 30;
		this->ObjectLocations[computerVal].y = 78;
	
		this->ObjectLocations[crtPCVal].x = 27;
		this->ObjectLocations[crtPCVal].y = 53;
	
		this->ObjectLocations[bongDirtyVal].x = 6;
		this->ObjectLocations[bongDirtyVal].y = 84;

		this->ObjectLocations[dresserVal].x = 184;
		this->ObjectLocations[dresserVal].y = 63;

		this->ObjectLocations[photoFrameVal].x = 192;
		this->ObjectLocations[photoFrameVal].y = 58;
	
		this->ObjectLocations[lampVal].x = 237;
		this->ObjectLocations[lampVal].y = 33;
	
		this->ObjectLocations[bedVal].x = 241;
		this->ObjectLocations[bedVal].y = 79;
	}

	this->GenObjectPositions(gc);
}

Simulator::~Simulator() {
}



void Simulator::LoadRoom() {

	if (ofs->FileIf(ofs->GetFileSector("*ROOM"))) {

		uint8_t objectVals[] = { tvVal, gameConsoleVal, computerVal, 
					 crtPCVal, bongDirtyVal, dresserVal, 
					 photoFrameVal, lampVal, bedVal };	
		uint8_t LBA[OFS_BLOCK_SIZE];
		ofs->ReadLBA("*ROOM", LBA, 0);
		
		for (int i = 0; i < 9; i++) {
		
			this->ObjectLocations[objectVals[i]].x = (LBA[16+(i*4)] << 8) | LBA[17+(i*4)];
			this->ObjectLocations[objectVals[i]].y = (LBA[18+(i*4)] << 8) | LBA[19+(i*4)];
		}
	}
}



void Simulator::SaveRoom() {
		
	uint8_t objectVals[] = { tvVal, gameConsoleVal, computerVal, 
				 crtPCVal, bongDirtyVal, dresserVal, 
				 photoFrameVal, lampVal, bedVal };	
	uint8_t LBA[OFS_BLOCK_SIZE];
	memset(LBA, 0x00, OFS_BLOCK_SIZE);

	for (int i = 0; i < 9; i++) {
	
		LBA[16+(i*4)] = this->ObjectLocations[objectVals[i]].x >> 8;
		LBA[17+(i*4)] = this->ObjectLocations[objectVals[i]].x & 0xff;
		LBA[18+(i*4)] = this->ObjectLocations[objectVals[i]].y >> 8;
		LBA[19+(i*4)] = this->ObjectLocations[objectVals[i]].y & 0xff;
	}

	
	if (ofs->FileIf(ofs->GetFileSector("*ROOM"))) {
		
		ofs->WriteLBA("*ROOM", LBA, 0);
	} else {
		ofs->NewFile("*ROOM", LBA, OFS_BLOCK_SIZE);
		ofs->NewTag("sys", ofs->GetFileSector("*ROOM"));
	}
}



void Simulator::LoadSkin(char* file) {
	
	if (ofs->FileIf(ofs->GetFileSector(file))) {
	
		uint8_t LBA[OFS_BLOCK_SIZE];
		uint8_t skin[114*49];

		for (int i = 0; i < 4; i++) {
		
			ofs->ReadLBA(file, LBA, i);
		
			for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
			
				if ((OFS_BLOCK_SIZE*i)+j < 114*49) {
				
					skin[(OFS_BLOCK_SIZE*i)+j] = LBA[j];
				}
			}
		}

		
		uint8_t* sprites[] = { 	customFrontRight,     customBackRight, 
					customFrontRightWalk1,customFrontRightWalk2,
					customBackRightWalk1, customBackRightWalk2 };
		
		int widths[] = { 16, 15, 16, 25, 21, 21 };
		int offset = 0;

		for (int w = 0; w < 6; w++) {
			for (int x = 0; x < widths[w]; x++) {
				for (int y = 0; y < 49; y++) {
				
					uint8_t* sprite = sprites[w];
					sprite[(widths[w]*y+x)] = skin[(114*y+x+offset)];
				}
			}
			offset += widths[w];
		}
	}
}


void Simulator::GenObjectPositions(GraphicsContext* gc) {
	//posters
	/* 
	gc->FillBufferCoordinate(208, 15, 20, 30, catPoster, false, ObjectPositionMap, catPosterVal);
	gc->FillBufferCoordinate(267, 26, 20, 30, awakeningPoster, false, ObjectPositionMap, awakeningPosterVal);
	gc->FillBufferCoordinate(9, 29, 20, 30, brazilPoster, true, ObjectPositionMap, brazilPosterVal);
	*/
	memset(this->ObjectPositionMap, 0x00, BUFFER_SIZE_13H);

	for (int i = 0; i < 16; i++) {

		int x = this->ObjectLocations[i].x;
		int y = this->ObjectLocations[i].y;

		switch (i) {

			case tvVal:
				gc->FillBufferCoordinate(x, y, 38, 33, crtTV, false, ObjectPositionMap, tvVal);
				break;
			
			case computerVal:
				gc->FillBufferCoordinate(x, y, 20, 23, computer, false, ObjectPositionMap, computerVal);
				break;

			case crtPCVal:
				gc->FillBufferCoordinate(x, y, 22, 18, crtPC, false, ObjectPositionMap, crtPCVal);
				break;
		
			case bongDirtyVal:
				gc->FillBufferCoordinate(x, y, 11, 18, bongDirty, false, ObjectPositionMap, bongDirtyVal);
				break;

			case dresserVal:
				gc->FillBufferCoordinate(x, y, 49, 26, dresser, true, ObjectPositionMap, dresserVal);
				gc->FillBufferCoordinate(x, y-7, 49, 26, dresser, true, ObjectPositionMap, dresserVal);
				break;
			
			case gameConsoleVal:
				gc->FillBufferCoordinate(x, y, 17, 15, gameConsole, false, ObjectPositionMap, gameConsoleVal);
				break;
			
			case photoFrameVal:
				gc->FillBufferCoordinate(x, y, 10, 10, photoFrame, false, ObjectPositionMap, photoFrameVal);
				break;

			case lampVal:
				gc->FillBufferCoordinate(x, y, 17, 59, lamp, false, ObjectPositionMap, lampVal);
				break;

			case bedVal:
				gc->FillBufferCoordinate(x, y, 76, 61, bed, false, ObjectPositionMap, bedVal);
				break;
			default:
				break;
		}
	}
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

	uint8_t objectValue = this->ObjectPositionMap[WIDTH_13H*newMouseY+newMouseX];


	if (button == LEFT_CLICK) {
	
		switch (objectValue) {
	
			case tvVal:
				this->menu = true;
				this->menuType = 1;
				break;

			case gameConsoleVal:
				this->SaveRoom();
				break;
		
			case crtPCVal:
				this->sim = false;
				break;
		
			case bongDirtyVal:
				this->waveLength ^= 20;
				break;
		
			case dresserVal:
				this->menu = true;
				this->menuType = 2;
				break;
		
			case windowVal:
				break;
		
			case lampVal:
				this->lampOn ^= 1;
				if (lampOn) { darkLevel = 0; }
				else {	      darkLevel = 1; }
				break;
		
			case bedVal:
				this->sleeping ^= 1;
				this->ticks = 0;
				break;
		
			default:
				if (walkCycle == 1 && flying == false && (newMouseY > 70 || 
					(newMouseX > 80 && newMouseX < 240 && newMouseY > 35))) {	
		
					mousePress = true;
					stepsNum = LineFillArray(mouseX, mouseY, newMouseX, newMouseY, walkPixels);

					backwardsX = mouseX > newMouseX;
					backwardsY = mouseY > newMouseY;
					direction = abs(mouseX - newMouseX) > abs(mouseY - newMouseY);
					this->mouseX = newMouseX;
					this->mouseY = newMouseY;
				}
				break;
		}

	} else if (button == 2 && objectValue != 0x00) {

		//this->objectTarget = ObjectLocations[objectValue];
		this->objectTarget = objectValue;
	}
}


void Simulator::OnMouseMove(int32_t MouseX, int32_t MouseY, int32_t newMouseX, int32_t newMouseY) {

	this->moveMouseX = newMouseX - MouseX;
	this->moveMouseY = newMouseY - MouseY;
	
	if (this->objectTarget != 0x00) {
	
		this->ObjectLocations[objectTarget].x = newMouseX;
		this->ObjectLocations[objectTarget].y = newMouseY;
	}
}

void Simulator::OnMouseUp(int32_t newMouseX, int32_t newMouseY, uint8_t button) {

	if (this->objectTarget != 0x00 && button == 2) {
	
		this->GenObjectPositions(this->gc);
		this->objectTarget = 0x00;
	}
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
		
			//home network media
			case 1:
				this->net->activateServerHNM ^= 1;
				
				if (this->HNM_Socket != nullptr) {
				
					this->net->udp->Disconnect(this->HNM_Socket);
				}
				this->HNM_Socket = this->net->udp->Listen(HNM_PORT);
				this->net->udp->Bind(this->HNM_Socket, this->net);
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
		
		switch (this->outfit) {
		
			case 0:break;
			case 1:break;
			
			case 4:this->LoadSkin("*OSAKA_SIM_0");break;
			case 5:this->LoadSkin("*OSAKA_SIM_1");break;
			case 6:this->LoadSkin("*OSAKA_SIM_2");break;
			case 7:this->LoadSkin("*OSAKA_SIM_3");break;

			default:
			       this->LoadSkin("*OSAKA_SIM_0");
			       break;
		}
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
	gc->FillRectangle(0, 0, WIDTH_13H, HEIGHT_13H, W5555FF);
	gc->FillRectangle(60, 50, HEIGHT_13H, 100, W000041);
	gc->DrawRectangle(60, 50, HEIGHT_13H, 100, WAAAAAA);
	
	char* str1 = "null"; char* str2 = "null"; 
	char* str3 = "null"; char* str4 = "null"; 
	char* str5 = "null"; char* str6 = "null"; 
	char* str7 = "null"; char* str8 = "null";

	switch (menuType) {
	
		//tv
		case 1:
			str1 = "HNM";
			str2 = "WIP 1";
			str3 = "WIP 2";
			break;
		//dresser
		case 2:
			str1 = "Winter";
			str2 = "Summer";
			str5 = "Cstm.0";
			str6 = "Cstm.1";
			str7 = "Cstm.2";
			str8 = "Cstm.3";
			break;
		default:
			break;
	}

	uint8_t strIndex = 0;
	char* strArr[8] = {str1, str2, str3, str4, str5, str6, str7, str8};
	
	for (uint8_t i = 0; i < 2; i++) {
		for (uint8_t j = 0; j < 4; j++) {
	
			//draw boxes
			gc->FillRectangle(84+(44*j), 70+(40*i), 20, 20, W000000);
			gc->DrawRectangle(84+(44*j), 70+(40*i), 20, 20, W555555);
	
			//print name
			gc->PutText(strArr[strIndex], 77+(44*j), 94+(40*i), WFFFFFF);
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

	int x = this->ObjectLocations[bedVal].x;
	int y = this->ObjectLocations[bedVal].y;
	bool mirror = x <= 160;

	//first dream bubble
	gc->FillBuffer(x+34, y-6, 4, 4, dreamBubbleMini1, mirror);

	//second dream bubble	
	if (ticks > 780) {
	
		gc->FillBuffer(x+23, y-12, 5, 5, dreamBubbleMini2, mirror);
	}
	
	//third dream bubble	
	if (ticks > 900) {
	
		gc->FillBuffer(x-21, y-39, 21, 29, dreamBubbleHalf, mirror);
		gc->FillBuffer(x-1, y-39, 21, 29, dreamBubbleHalf, !mirror);

		switch (this->ticks / (3600)) {
	
			case 0:
				gc->FillBuffer(x+17, y+35, 17, 19, cursorChiChi, mirror);
				gc->FillBuffer(x+1, y+35, 13, 20, osakaPlatSprite1, !mirror);
				break;
			case 1:
				gc->PutText("=D", x-11, y-29, W000000);
				break;
			case 2:
				break;
			case 3:
				gc->PutText("fuck\nC++", x+16, y-29, W000000);
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
			for (uint8_t i = 9; i < 14; i++) { gc->PutPixel(osakaX+4, osakaY+i, W829EFF); }
			for (uint8_t i = 9; i < 15; i++) { gc->PutPixel(osakaX+9, osakaY+i, W829EFF); }
			for (uint8_t i = 5; i < 9; i++) { gc->PutPixel(osakaX+i, osakaY+12, WFFFFBA);}
			break;
		case 0:
			for (uint8_t i = 9; i < 15; i++) { gc->PutPixel(osakaX+6, osakaY+i, W829EFF); }
			for (uint8_t i = 9; i < 14; i++) { gc->PutPixel(osakaX+11, osakaY+i, W829EFF); }
			for (uint8_t i = 7; i < 11; i++) { gc->PutPixel(osakaX+i, osakaY+12, WFFFFBA);}
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
		
			for (int i = 0; i < WIDTH_13H; i++) {
		
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
				gc->FillRectangle(0, 0, WIDTH_13H, HEIGHT_13H, W000000);
				if (ticks >= 360) { gc->PutText("GAME OVER", 133, 96, WFF0000); }
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
	gc->FillRectangle(0, 0, WIDTH_13H, HEIGHT_13H, W000041);
	
	//floor
	gc->FillRectangle(0, 95, WIDTH_13H, HEIGHT_13H, WAA5500);
	uint16_t xFloor[] = {0, 160, WIDTH_13H};
	uint16_t yFloor[] = {95, 65, 95};
	gc->FillPolygon(xFloor, yFloor, 3, WAA5500);


	//rug
	uint16_t xRug[] = {90, 160, 230, 160};
	uint16_t yRug[] = {120, 100, 120, 140};
	gc->FillPolygon(xRug, yRug, 4, WFFFFFF);
	uint16_t xRug2[] = {105, 160, 215, 160};
	uint16_t yRug2[] = {120, 105, 120, 135};
	gc->FillPolygon(xRug2, yRug2, 4, W00AAAA);

	
	//walls
	//brown
	uint16_t xWall1[] = {0, 160, WIDTH_13H, WIDTH_13H, 160, 0};
	uint16_t yWall1[] = {95, 65, 95, 80, 50, 80};
	gc->FillPolygon(xWall1, yWall1, 6, W410000);

	//green
	uint16_t yWall2[] = {80, 50, 80, 70, 40, 70};
	gc->FillPolygon(xWall1, yWall2, 6, W397145);

	//beige
	uint16_t yWall3[] = {70, 40, 70, 20, 0, 20};
	gc->FillPolygon(xWall1, yWall3, 6, WFFFFBA);
	
	//outlines	
	gc->DrawLine(0, 95, 160, 65, W000000);
	gc->DrawLine(160, 65, WIDTH_13H, 95, W000000);
	gc->DrawLine(0, 80, 160, 50, W000000);
	gc->DrawLine(160, 50, WIDTH_13H, 80, W000000);
	gc->DrawLine(160, 50, WIDTH_13H, 80, W000000);
	gc->DrawLine(160, 40, 0, 70, W000000);
	gc->DrawLine(160, 40, WIDTH_13H, 70, W000000);
	gc->DrawLine(160, 0, 160, 65, W000000);
	gc->DrawLine(0, 20, 160, 0, W000000);
	gc->DrawLine(160, 0, WIDTH_13H, 20, W000000);



	//draw the items inside the room

	//posters
	gc->FillBuffer(208, 15, 20, 30, catPoster, false);
	gc->FillBuffer(267, 26, 20, 30, awakeningPoster, false);
	gc->FillBuffer(9, 29, 20, 30, brazilPoster, true);

	//wall outlets
	gc->FillBuffer(127, 59, 7, 10, wallOutlet, false);
	//gc->FillBuffer(25, 78, 7, 10, wallOutlet, false);
	gc->FillBuffer(263, 73, 7, 10, wallOutlet, true);
	
	
	
	
	//blinds
	for (uint8_t i = 15; i < 42; i += 3) {
		gc->FillBuffer(65, i, 35, 8, blindsRepeat, false);
	}
	gc->DrawLine(67, 23, 67, 42, W000000);
	gc->DrawLine(68, 23, 68, 42, WAAAAAA);
	gc->DrawLine(69, 23, 69, 42, W000000);
	gc->PutPixel(68, 41, W000000);
	gc->FillBuffer(64, 42, 38, 9, windowShelf, false);



	for (int i = 0; i < 16; i++) {

		int x = this->ObjectLocations[i].x;
		int y = this->ObjectLocations[i].y;
		bool mirror = x > 160;
	
		switch(i) {

			case lampVal:
				gc->FillBuffer(x, y, 17, 59, lamp, !mirror);
				break;
			
			case dresserVal:
				gc->FillBuffer(x, y, 49, 26, dresser, mirror);
				gc->FillBuffer(x, y-7, 49, 26, dresser, mirror);
				break;

			case tvVal:
	
				//tv stand
				gc->DrawLine(x-3, y+28, 130, 62, W000000);
				gc->FillRectangle(x, y+35, 38, 7, W410000);
				gc->DrawRectangle(x, y+35, 38, 7, W000000);
				gc->FillRectangle(x-3, y+28, 44, 7, W410000);
				gc->DrawRectangle(x-3, y+28, 44, 7, W000000);
				gc->DrawLine(x+17, y+38, x+21, y+38, W555555);
				
				gc->FillBuffer(x, y, 38, 33, crtTV, false);
		

				if (this->net->activateServerHNM == true) {
					
					gc->DrawLine(x+14, y-8, x+19, y, W000000);
					gc->DrawLine(x+24, y-8, x+19, y, W000000);
				}	

				
				//tv flash
				gc->FillRectangle(x+4, y+4, 30, 22, (((ticks/5)%2)*8)+1);
				break;
			
			case bedVal:
				gc->FillBuffer(x, y, 76, 61, bed, !mirror);

				if (this->sleeping) {
		
					//gc->FillBuffer(241, 79, 76, 61, bed, false);
					//gc->FillBuffer(284, 82, 17, 10, osakaSleep, false);
					//gc->FillBuffer(x+43, y+3, 17, 10, osakaSleep, !mirror);
					gc->FillBuffer(x+43, y+3, 17, 10, osakaSleep, !mirror);
					
					if (this->ticks > 600) { this->Dream(gc); }
				}
				break;
			
			//case computerVal:
				//gc->FillBuffer(x, y, 20, 23, computer, false);
			//	break;

			case crtPCVal:
				mirror = x+24 > 148;
	
				//outlet	
				gc->FillBuffer(x-2, y+25, 7, 10, wallOutlet, mirror);
				//computer
				gc->FillBuffer(x+3, y+25, 20, 23, computer, mirror);
				//desk
				gc->FillBuffer(x-9, y+7, 49, 46, desk, mirror);
				//monitor
				gc->FillBuffer(x, y, 22, 18, crtPC, mirror);
				//keyboard
				gc->FillBuffer(x+8, y+16, 16, 7, keyboard, mirror);
				//keyboard cable
				gc->FillBuffer(x-1, y+13, 9, 8, keyboardCable, mirror);
				//mouse
				gc->FillBuffer(x+24, y+9, 7, 9, mouse, mirror);
				//chair
				gc->FillBuffer(x+16, y+18, 30, 40, chair, mirror);
				break;
		
			case bongDirtyVal:
				gc->FillBuffer(x, y, 11, 18, bongDirty, mirror);
				break;
			
			case gameConsoleVal:
				gc->DrawLine(x+6, y+4, 130, 65, W000000);
				gc->FillBuffer(x, y, 17, 15, gameConsole, false);
				//gc->FillBuffer(122, 73, 17, 15, gameConsole, false);
				//gc->DrawLine(131, 75, 130, 65, W000000);
				break;

			case photoFrameVal:
				gc->FillBuffer(x, y, 10, 10, photoFrame, !mirror);
				break;

			default:
				break;
		}
	}

	


	//walking
	uint8_t walkFrames = 0;
	walkFrames = (1*(backwardsX)) + (2*backwardsY);
	if ((walkCycle % 40 < 5)) {
	} else if ((walkCycle % 40 < 20)) { walkFrames += 4 + walkFrames;
	} else if ((walkCycle % 40 < 25)) {
	} else { walkFrames += 5 + walkFrames; }
	

	//draw osaka
	if (this->sleeping) {
	
		//gc->FillBuffer(284, 82, 17, 10, osakaSleep, false);
		//if (this->ticks > 600) { this->Dream(gc); }
	} else {
		//walking and standing around
		bool facingLeft = false;

		if (walkFrames == 1 || walkFrames == 3 || 
			walkFrames == 6 || walkFrames == 7 || 
			walkFrames == 10 || walkFrames == 11) {
		
			facingLeft = true;
		}

		switch (this->outfit) {
			
			//normal outfit
			case 0:
				gc->FillBuffer(osakaX, osakaY, osakaWidths[walkFrames], 49, osakaSprites[walkFrames], facingLeft);
				break;
			//draw normal outfit then summer overlay on top
			case 1:
				gc->FillBuffer(osakaX, osakaY, osakaWidths[walkFrames], 49, osakaSprites[walkFrames], facingLeft);
				gc->FillBuffer(osakaX, osakaY+18, osakaWidths[walkFrames], 19, osakaSummerSprites[walkFrames], facingLeft);
				break;

			//draw custom outfits
			default:
				gc->FillBuffer(osakaX, osakaY, osakaWidths[walkFrames], 49, this->osakaCustomSprites[walkFrames], facingLeft);
				break;
			
		}
		if (this->flying) { this->Fly(gc, walkFrames); }
	}

	//draw ui stuff
	char* timeDateString = "00/00/00 - 00:00:00";
	TimeAndDate(timeDateString);
	gc->PutText(timeDateString, 6, 191, W000000);
	gc->PutText(timeDateString, 5, 190, WFFFFFF);


	//home network media
	if (this->net->activateServerHNM == true) {

		for (int i = 0; i < BUFFER_SIZE_13H; i++) {
		
			this->net->HNM_Buffer[i] = gc->pixels[i];
		}	
		gc->PutText("Sending HNM", 0, 0, WFFFFFF);
	}
}
