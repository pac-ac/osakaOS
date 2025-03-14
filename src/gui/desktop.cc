#include <gui/desktop.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::gui;
using namespace os::filesystem;


void sleep(uint32_t);
uint32_t str2int(char*);
uint16_t prng();


Desktop::Desktop(common::int32_t w, common::int32_t h,
		 common::uint8_t color,
		 common::GraphicsContext* gc,
		 GlobalDescriptorTable* gdt,
		 TaskManager* taskManager,
		 MemoryManager* memoryManager,
		 FileSystem* filesystem,
		 Compiler* compiler,
		 CMOS* cmos, DriverManager* drvManager,
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
	this->compiler = compiler;
	this->gdt = gdt;
	this->cmos = cmos;
	this->drvManager = drvManager;
	this->osaka = osaka;

	this->buttons = (List*)(memoryManager->malloc(sizeof(List)));
	new (buttons) List(memoryManager);


	//setup background
	uint8_t tmp[64000];
	for (int i = 0; i < 64000; i++) { tmp[i] = 0x11; }
	
	uint16_t bgw = WIDTH_13H;
	uint8_t bgh = HEIGHT_13H;
	this->filesystem->Read13H("home", tmp, &bgw, &bgh, true);
	//this->filesystem->Read13H("home", tmp, &bgw, &bgh, false);


	for (uint32_t y = 0; y < HEIGHT_13H; y++) {	
		for (uint32_t x = 0; x < WIDTH_13H; x++) {
		
			this->WritePixel(x, y, tmp[WIDTH_13H*y+x]);
		}
	}
}


Desktop::~Desktop() {}


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
							 this->filesystem, this->compiler, this->gc, this->cmos, this->drvManager);
				app = newCli;
			}
			}
			break;
	}


	//create window
	Window* window = (Window*)memoryManager->malloc(sizeof(Window));
	new (window) Window(this, prng()%140, prng()%120, 180, 80, name, color, app, this->filesystem);
	//new (window) Window(this, 70, 50, 180, 80, name, color, app, this->filesystem);
	

	//add gui buttons
	switch (appType) {
	
		//kasugapaint
		case 2:
			break;
		default:
			break;
	}
	
	
	this->AddChild(window);	
	this->GetFocus(window);
	return window;
}



void Desktop::FreeChild(Window* window) {

	uint8_t appType = window->ReturnAppType();
	App* app = window->app;

	switch (appType) {
	
		//free lists from cli
		case 1:
			{
			CommandLine* cliPtr = (CommandLine*)app;
			
			for (int i = 0; i < cliPtr->lists->numOfNodes; i++) {
			
				List* list = (List*)(cliPtr->lists->Read(i));
				list->DestroyList();
			}
			cliPtr->lists->DestroyList();
			cliPtr->mm->free(cliPtr->lists);
			cliPtr->DeleteTaskForScript(0);
			}
			break;
		default:
			break;
	}

	//this->memoryManager->free(app);
	
	window->DestroyWindow();
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
		gc->FillBuffer(0, 0, WIDTH_13H, HEIGHT_13H, this->buf, false);
	
		//draw buttons
		for (int i = 0; i < this->buttons->numOfNodes; i++) {
		
			((DesktopButton*)(this->buttons->Read(i)))->Draw(gc);
		}

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

	gc->FillRectangle(0, 187, WIDTH_13H, 20, 0x07);
	gc->DrawLine(0, 187, WIDTH_13H, 187, 0x3f);


	uint16_t ButtonWidth = (WIDTH_13H / this->minWindows);
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

	if (this->minWindows <= 0) { return; }
	
	uint16_t ButtonWidth = (WIDTH_13H / this->minWindows);
	uint16_t ButtonNum = (MouseX / ButtonWidth) + 1;

	//MouseX;
	int i = 0;
	uint8_t windowIndex = this->minWindows;
	for (int i = 0; i < numChildren; i++) {
	//for (int i = numChildren-1; i >= 0; i--) {

		Window* window = (Window*)(children[numChildren-(i+1)]);

		if (window->Min) {
		
			if (windowIndex == ButtonNum) { 
		
				//unminimize	
				window->x = window->xo;
				window->y = window->yo;
				window->w = window->wo;
				window->h = window->ho;
				window->Min = false;
				this->GetFocus(window);
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

	char* fileName = "SS_0000";
	char* hex = "0123456789ABCDEF";
	uint16_t count = (uint16_t)(this->filesystem->GetFileCount());
	fileName[6] = hex[(count & 0xf)];
	fileName[5] = hex[(count >> 4) & 0xf];
	fileName[4] = hex[(count >> 8) & 0xf];
	fileName[3] = hex[(count >> 12)];

	uint8_t buf[64000];

	for (int i = 0; i < 64000; i++) {
	
		buf[i] = this->gc->pixels[i];
	}
	
	//this->filesystem->Write13H(fileName, buf, WIDTH_13H, HEIGHT_13H, true);
	uint8_t* ptr = nullptr;
	if (this->filesystem->GetTagFile("compressed", filesystem->GetFileSector(fileName), ptr)) {
	
		this->filesystem->Write13H(fileName, buf, WIDTH_13H, HEIGHT_13H, true);
	} else {
		this->filesystem->Write13H(fileName, buf, WIDTH_13H, HEIGHT_13H, false);
	}
	
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
		//if (drawButtons) { this->buttons->OnMouseDown(MouseX, MouseY, button, this); }
	
		//mouse down for desktop shortcuts
		for (int i = 0; i < this->buttons->numOfNodes; i++) {	
		
			DesktopButton* dbutton = (DesktopButton*)(this->buttons->Read(i));
			dbutton->OnMouseDown(MouseX, MouseY, button, this);
		}
	
		//mouse down for the windows
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
			case 6: this->takeSS = true; break;
			default:
				break;
		}
		CompositeWidget::OnKeyDown(str);
	}

	//take screenshot
	//if (str == 6) { this->takeSS = true; }
}


void Desktop::OnKeyUp(char str) {
	
	if (this->osaka->sim) { this->osaka->OnKeyUp(str); }
	else { CompositeWidget::OnKeyUp(str); }
}



void Desktop::CreateButton(char* file, uint8_t openType, char* imageFile) {

	if (this->buttons->numOfNodes >= 160) { return; }

	DesktopButton* button = (DesktopButton*)(this->memoryManager->malloc(sizeof(DesktopButton)));
	new (button) DesktopButton(file, openType, imageFile, this->buttons->numOfNodes);

	this->buttons->Push(button);
}






//class for desktop icons and shortcuts
DesktopButton::DesktopButton(char* file, uint8_t openType, char* imageFile, uint8_t index) 
: Widget((index * 20) % 320, (index / 16) * 20, 20, 20) {
//: Widget(0, 0, 20, 20) {

	this->openType = openType;
	for (int i = 0; file[i] != '\0'; i++) { this->file[i] = file[i]; }

	//20x20 sprite res
	for (int i = 0; i < 400; i++) { this->buffer[i] = 0x00; }
	this->buf = this->buffer;

	//no image provided
	if (imageFile == nullptr) { 
	
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
		
				this->buffer[20*i+j] = fileShortcut[16*i+j];
			}
		}
	}
}

DesktopButton::~DesktopButton() {}


void DesktopButton::Draw(GraphicsContext* gc) {

	gc->FillBuffer(this->x, this->y, 20, 20, this->buffer, false);
	
	gc->PutText(this->file, this->x+2, this->y+13, 0x40);
	gc->PutText(this->file, this->x+1, this->y+12, 0x3f);
}


void DesktopButton::OnMouseDown(int32_t x, int32_t y, uint8_t button, Desktop* desktop) {

	if (ContainsCoordinate(x, y) == false) { return; }

	for (int i = 0; i < desktop->numChildren; i++) {
	
		if (desktop->children[i]->ContainsCoordinate(x, y)) { return; }
	}

	CompositeWidget* widget = nullptr;

	switch (this->openType) {

		//script
		case 0:
		case 1:
			widget = desktop->CreateChild(1, "Osaka's Terminal", nullptr);
			break;
		//paint
		case 2:
			widget = desktop->CreateChild(2, "KasugaPaint", nullptr);
			break;
		//journal
		case 3:
		default:
			widget = desktop->CreateChild(3, "Journal", nullptr);
			break;
	}
	Window* win = (Window*)widget;
	win->app->ReadInput(this->file, widget, desktop->filesystem);
}
