#include <gui/desktop.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::gui;
using namespace os::filesystem;


void sleep(uint32_t);
uint32_t str2int(char*);


Desktop::Desktop(common::int32_t w, common::int32_t h,
		 common::uint8_t color,
		 common::GraphicsContext* gc,
		 GlobalDescriptorTable* gdt,
		 TaskManager* taskManager,
		 MemoryManager* memoryManager,
		 FileSystem* filesystem,
		 CMOS* cmos, DriverManager* drvManager,
		 Button* buttons,
		 Simulator* osaka)
: CompositeWidget(0, 0, 0, w, h, "desktop", color, false), 
  MouseEventHandler() {

	MouseX = w / 2;
	MouseY = h / 2;
	
	this->color = color;

	this->gc = gc;
	this->taskManager = taskManager;
	this->memoryManager = memoryManager;
	this->filesystem = filesystem;
	this->gdt = gdt;
	this->cmos = cmos;
	this->drvManager = drvManager;
	this->buttons = buttons;
	this->osaka = osaka;


	//setup background
	uint8_t tmp[64000];
	for (int i = 0; i < 64000; i++) { tmp[i] = 0x11; }
	
	uint16_t bgw = 320;
	uint8_t bgh = 200;
	this->filesystem->Read13H("home", tmp, &bgw, &bgh);


	for (uint32_t y = 0; y < 200; y++) {	
		for (uint32_t x = 0; x < 320; x++) {
		
			this->WritePixel(x, y, tmp[320*y+x]);
		}
	}
}


Desktop::~Desktop() {
}


common::GraphicsContext* Desktop::ReturnGraphicsContext() { return this->gc; }


CompositeWidget* Desktop::CreateChild(uint8_t appType, char* name, App* oldApp) {

	App* app = 0;
	uint8_t color = 0x40;

	//create app
	switch (appType) {

		//kernel programs
		case 0:
			{
			Script* script = (Script*)memoryManager->malloc(sizeof(Script));
			new (script) Script((CommandLine*)oldApp);
			app = script;
			}
			break;
		//kasugapaint
		case 2:	
			{
			KasugaPaint* paint = (KasugaPaint*)memoryManager->malloc(sizeof(KasugaPaint));
			new (paint) KasugaPaint();
			app = paint;
			color = 0x3f;
			}
			break;
		//journal
		case 3:
			{
			Journal* journal = (Journal*)memoryManager->malloc(sizeof(Journal));
			new (journal) Journal();
			app = journal;
			color = 0x3f;
			}
			break;
		//create command line
		//as deafult
		default:
			{
			if (oldApp != nullptr) {
			
				app = (CommandLine*)oldApp;
			} else {
				CommandLine* newCli = (CommandLine*)memoryManager->malloc(sizeof(CommandLine));
				new (newCli) CommandLine(this->gdt, this->taskManager, this->memoryManager, 
							 this->filesystem, this->cmos, this->drvManager);
				app = newCli;
			}
			}
			break;
	}


	//create window
	Window* window = (Window*)memoryManager->malloc(sizeof(Window));
	new (window) Window(this, 70, 50, 180, 80, name, color, app, this->filesystem);
	this->AddChild(window);	
	this->GetFocus(window);
	return window;
}



void Desktop::FreeChild(Window* window) {

	uint8_t appType = window->ReturnAppType();

	App* app = window->app;
	window->app = nullptr;
	this->memoryManager->free(app);
	this->memoryManager->free(window);
}



void Desktop::Draw(common::GraphicsContext* gc) {
	
	if (this->keyValue == 0x5b) {

		this->osaka->sim ^= 1;
		this->OnMouseUp(0);
		this->keyValue = 0;
	}
	
	
	if (this->osaka->sim == false) {

		//draw background
		gc->FillBuffer(0, 0, 320, 200, this->buf, false);
	
		//draw buttons
		if (drawButtons) { this->buttons->Draw(gc); }

		//draw windows
		CompositeWidget::Draw(gc);

		//taskbar
		if (this->taskbar) { this->DrawTaskBar(gc); }
	} else {
		//osaka simulator
		this->osaka->DrawRoom(gc);
	}
	gc->MakeDark(this->osaka->darkLevel);
	gc->MakeWave(this->osaka->waveLength);

	

	//cursor
	this->MouseDraw(gc);
	
	//screenshot screen
	if (takeSS) {
		this->Screenshot();
		this->takeSS = false;
	}
		
	//write to video memory
	gc->DrawToScreen();
}


void Desktop::DrawTaskBar(common::GraphicsContext* gc) {
	
	//avoid divide by 0 lol
	if (this->minWindows == 0) { return; }

	gc->FillRectangle(0, 187, 320, 20, 0x07);
	gc->DrawLine(0, 187, 320, 187, 0x3f);


	uint16_t ButtonWidth = (320 / this->minWindows);
	uint8_t ButtonNum = 0;

	for (int i = 0; i < numChildren; i++) {

		if (this->children[i]->Min) {
			
			//draw button
			uint8_t x0 = (ButtonWidth * ButtonNum);

			gc->FillRectangle(x0, 189, x0+ButtonWidth, 198, ButtonNum+1);
			gc->DrawRectangle(x0, 189, x0+ButtonWidth, 198, 0x38);
			ButtonNum++;
			
			//draw name of window
			char* name = children[i]->name;
			//name[ButtonWidth/5] = '\0';
			gc->PutText(name, x0+2, 192, 0x40);
			gc->PutText(name, x0+1, 191, 0x3f);
		}
	}
}

void Desktop::TaskBarClick(uint8_t button) {

	if (this->minWindows == 0) { return; }
	
	uint16_t ButtonWidth = (320 / this->minWindows);
	uint16_t ButtonNum = (MouseX / ButtonWidth) + 1;

	//MouseX;
	int i = 0;
	uint8_t windowIndex = this->minWindows;
	for (int i = numChildren-1; i >= 0; i--) {
	
		if (this->children[i]->Min) {
		
			if (windowIndex == ButtonNum) { 
		
				//unminimize	
				children[i]->x = 80;
				children[i]->y = 55;
				children[i]->w = children[i]->wo;
				children[i]->h = children[i]->ho;
				children[i]->Min = false;
				this->GetFocus(children[i]);
				this->minWindows--;
				return;
			} else { 
				windowIndex--; 
			}
		}
	}
}

void Desktop::DrawNoMouse(common::GraphicsContext* gc) { CompositeWidget::Draw(gc); }


void Desktop::MouseDraw(common::GraphicsContext* gc) {
	
	//mouse icon
	uint16_t cursorIndex = 0;
	uint8_t* cursorArt = nullptr;

	uint8_t mouseW = 13;
	uint8_t mouseH = 20;
	int8_t offsetX = 0;
	int8_t offsetY = 0;


	if (this->osaka->sim == false) {

		if (this->focussedChild == 0) {

			if (click) { cursorArt = cursorClickLeft; } 
			else { cursorArt = cursorNormal; }
		} else {
			//unique cursors for each program
			switch (this->focussedChild->ReturnAppType()) {
			
				case 2:
					cursorArt = cursorChiChi;
					mouseW = 17;
					mouseH = 19;
					offsetY = -3;
					offsetX = -1;
					break;
				case 3:
					cursorArt = cursorPencil;
					mouseW = 16;
					mouseH = 16;
					break;
				default:
					cursorArt = cursorClassic;
					mouseW = 7;
					mouseH = 7;
					break;
			}
		}
	} else {
		cursorArt = cursorClassic;
		mouseW = 7;
		mouseH = 7;
	}

	//draw mouse
	gc->FillBuffer(MouseX+offsetX, MouseY+offsetY, mouseW, mouseH, cursorArt, false);
}


void Desktop::Screenshot() {

	//if filecount over 65535
	//index will not exist in hex array
	//but who cares, if you make that many
	//files in fuckin osakaOS you deserve
	//to have your shit crash
	char* fileName = "ss_0000";
	char* hex = "0123456789abcdef";
	uint16_t count = (uint16_t)(this->filesystem->GetFileCount());
	fileName[6] = hex[(count & 0xf)];
	fileName[5] = hex[(count >> 4) & 0xf];
	fileName[4] = hex[(count >> 8) & 0xf];
	fileName[3] = hex[(count >> 12)];

	uint8_t buf[64000];

	for (int i = 0; i < 64000; i++) {
	
		buf[i] = this->gc->pixels[i];
	}
	this->filesystem->Write13H(fileName, buf, 320, 200);
}




void Desktop::OnMouseDown(common::uint8_t button) {

	if (this->osaka->sim) { this->osaka->OnMouseDown(MouseX, MouseY, button);
	} else {
		//left click after switching to sim and back
		//and when there are 0 children (windows) causes crash
		if (MouseY >= 190 && this->taskbar) {
		
			this->TaskBarClick(button);
		}
		this->click = true;
		if (drawButtons) { this->buttons->OnMouseDown(MouseX, MouseY, button, this); }
		CompositeWidget::OnMouseDown(MouseX, MouseY, button);
	}
}

void Desktop::OnMouseUp(common::uint8_t button) {
	
	if (this->osaka->sim) {
		//this->osaka->OnMouseUp(MouseX, MouseY, button);
	} else {
		this->click = false;
		CompositeWidget::OnMouseUp(MouseX, MouseY, button);
	}
}

void Desktop::OnMouseMove(int x, int y) {
	
		
	int32_t newMouseX = MouseX + x;
	this->oldMouseX = MouseX;

	int32_t newMouseY = MouseY + y;
	this->oldMouseY = MouseY;
	
	
	if (this->osaka->sim) { this->osaka->OnMouseMove(MouseX, MouseY, newMouseX, newMouseY); }


	if (newMouseX < 0) { newMouseX = 0; }
	if (newMouseX >= w) { newMouseX = w - 1; }
	
	if (newMouseY < 0) { newMouseY = 0; }
	if (newMouseY >= h) { newMouseY = h - 1; }
	
	CompositeWidget::OnMouseMove(MouseX, MouseY, newMouseX, newMouseY);
	MouseX = newMouseX;
	MouseY = newMouseY;
	
}


void Desktop::OnKeyDown(char str) {

	if (this->osaka->sim) { this->osaka->OnKeyDown(str);
	} else {
		switch (str) {
	
			//f# keys	
			case 1: this->taskbar ^= 1; break;
			case 7:
				{
					/*
					for (int i = 0; i < this->numChildren; i++) {
					
						this->children[i]->
					}
					*/
				}
				break;
			default:
				break;
		}
		CompositeWidget::OnKeyDown(str);
	}

	//take screenshot
	if (str == 6) { this->takeSS = true; }
}


void Desktop::OnKeyUp(char str) {
	
	if (this->osaka->sim) { this->osaka->OnKeyUp(str); }
	else { CompositeWidget::OnKeyUp(str); }
}
