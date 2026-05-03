#include <gui/desktop.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::gui;
using namespace os::net;
using namespace os::filesystem;


void sleep(uint32_t);
uint8_t* memset(uint8_t*, int, size_t);
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
: CompositeWidget(0, 0, 0, w, h, "Desktop", color, false), 
  MouseEventHandler() {

	MouseX = w / 2;
	MouseY = h / 2;
	
	this->color = color;

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
	uint8_t tmp[w*h];
	for (int i = 0; i < w*h; i++) { tmp[i] = color; }
	
	for (uint32_t y = 0; y < h; y++) {	
		for (uint32_t x = 0; x < w; x++) {
		
			this->WritePixel(x, y, tmp[w*y+x]);
		}
	}
}


Desktop::~Desktop() {}


common::GraphicsContext* Desktop::ReturnGraphicsContext() { return this->gc; }


CompositeWidget* Desktop::CreateChild(uint8_t appType, char* name, App* oldApp) {

	App* app = 0;
	uint8_t color = W000000;

	//create app
	switch (appType) {

		//kernel programs
		case APP_TYPE_SCRIPT:
			{
				Script* script = (Script*)memoryManager->malloc(sizeof(Script));
				new (script) Script((CommandLine*)oldApp);
				app = script;
			}
			break;
		case APP_TYPE_KASUGAPAINT:	
			{
				KasugaPaint* paint = (KasugaPaint*)memoryManager->malloc(sizeof(KasugaPaint));
				new (paint) KasugaPaint();
				app = paint;
				color = WFFFFFF;
			}
			break;
		case APP_TYPE_JOURNAL:
			{
				Journal* journal = (Journal*)memoryManager->malloc(sizeof(Journal));
				new (journal) Journal(this->memoryManager);
				app = journal;
				color = WFFFFFF;
			}
			break;
		case APP_TYPE_SHINOSAKA:
			{
				Shinosaka* shinosaka = (Shinosaka*)memoryManager->malloc(sizeof(Shinosaka));
				new (shinosaka) Shinosaka(this->network);
				app = shinosaka;
				color = WFFFFFF;
			}
			break;
		//create command line
		//as deafult
		default:
			if (oldApp != nullptr) {
			
				app = (CommandLine*)oldApp;
			} else {
				CommandLine* newCli = (CommandLine*)memoryManager->malloc(sizeof(CommandLine));
				new (newCli) CommandLine(this->gdt, this->taskManager, this->memoryManager, this->filesystem, 
							 this->network, this->compiler, this->gc, this->cmos, this->drvManager);
				app = newCli;
			}
			break;
	}

	Window* window = (Window*)memoryManager->malloc(sizeof(Window));
	//window->gc = this->gc;
	//new (window) Window(this, prng()%140, prng()%120, 180, 80, name, color, app, this->filesystem);
	
	if (window != 0) {
	
		new (window) Window(this, prng() % (gc->gfxWidth/2), prng() % (gc->gfxHeight/2), 
					(gc->gfxWidth/2)+(gc->gfxWidth/16), (gc->gfxHeight/2)-(gc->gfxHeight/10), 
					name, color, app, this->numChildren, this->filesystem);
		//new (window) Window(this, 70, 50, 180, 80, name, color, app, this->filesystem);
	} else {
		//some error check here
	}


	//add gui buttons and 
	//other app specific inits
	switch (appType) {
	
		case APP_TYPE_KASUGAPAINT:
			break;
		case APP_TYPE_SHINOSAKA:
			window->currentTextWidth = this->gc->gfxWidth / FONT_WIDTH;
			window->currentTextHeight = this->gc->gfxHeight / FONT_HEIGHT;
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
		case APP_TYPE_TERMINAL:
			{
				CommandLine* cliPtr = (CommandLine*)app;
				cliPtr->CleanCommandLine();
			}
			break;

		case APP_TYPE_JOURNAL:
			{
				Journal* ptr = (Journal*)app;
				
				if (ptr->fileBuffer != nullptr) {
				
					ptr->mm->free(ptr->fileBuffer);
				}
				ptr->mm->free(ptr->degreePoints);
			}
			break;
		case APP_TYPE_KASUGAPAINT:
			{
				KasugaPaint* ptr = (KasugaPaint*)app;
				this->gc->mm->free(ptr->backup);
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
		
	//screen saver
	if (this->timer > 0xffff) {
	
		this->DrawSaver(gc, this->saverVal);
		return;
	
	} else { 
		this->setScreenSaver = false;
		this->timer++;
		this->saverVal = prng();
	}


	if (this->keyValue == 0x5b) {

		this->osaka->sim ^= 1;
		this->OnMouseUp(0);
		this->keyValue = 0;
	}
	
	
	if (this->osaka->sim == false) {

		//draw background
		gc->FillBuffer(0, 0, gc->gfxWidth, gc->gfxHeight, this->buf, false);
	
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
	
	gc->MakeDark(this->osaka->darkLevel, 0, 0, gc->gfxWidth, gc->gfxHeight);
	
	if (this->osaka->waveLength != 0) {
	
		gc->MakeWave(0, 0, gc->gfxWidth, gc->gfxHeight, 16, this->osaka->waveLength, this->osaka->ticks, nullptr);
	}


	//cursor
	this->MouseDraw(gc);


	//screenshot screen
	if (takeSS) {
		this->Screenshot();
		this->takeSS = false;
	}
	
	
	//graphical effects
	if (this->Rainbow) { gc->Rainbowize(this->x, this->y, this->w, this->h); }
	if (this->Pixelize) { gc->Pixelize(this->x, this->y, this->w, this->h, 2, 2); }
	
	static uint8_t waveInc = 0;
	
	if (this->Wave) {
	
		gc->MakeWave(this->x, this->y, this->w, this->h, 16, 32, waveInc, nullptr);
		sleep(2);
		waveInc++;
	}


	//write to video memory
	gc->DrawToScreen();
}



void Desktop::DrawSaver(common::GraphicsContext* gc, uint8_t saverType) {
		
	if (saverType % 2) {
	
		static uint16_t pointx1 = prng() % gc->gfxWidth;
		static uint16_t pointy1 = prng() % gc->gfxHeight;
		
		uint16_t pointx2 = (prng() + timer) % gc->gfxWidth;
		uint16_t pointy2 = (prng() + timer) % gc->gfxHeight;
	
		uint16_t color = prng() % 256;
	
		if (!setScreenSaver) { 
		
			for (int i = 0; i < gc->gfxBufferSize; i++) {
				gc->pixels[i] = light2dark[gc->pixels[i]];
			}
			setScreenSaver = true;
		}
	
		gc->DrawLine(pointx1, pointy1, pointx2, pointy2, color);
		pointx1 = pointx2;
		pointy1 = pointy2;
	} else {
		//define cube
		float v0[3] = {  1,  1,  1 };
		float v1[3] = { -1,  1,  1 };
		float v2[3] = { -1, -1,  1 };
		float v3[3] = {  1, -1,  1 };
		float v4[3] = {  1,  1, -1 };
		float v5[3] = { -1,  1, -1 };
		float v6[3] = { -1, -1, -1 };
		float v7[3] = {  1, -1, -1 };
		float* v[] { v0, v1, v2, v3, v4, v5, v6, v7 };

		//vertix indeces + color
		uint8_t t0[4] = { 0, 1, 2, WFFFFFF };
		uint8_t t1[4] = { 0, 2, 3, WFFFFFF };
		uint8_t t2[4] = { 4, 0, 3, W00FF00 };
		uint8_t t3[4] = { 4, 3, 7, W00FF00 };
		uint8_t t4[4] = { 5, 4, 7, W0000FF };
		uint8_t t5[4] = { 5, 7, 6, W0000FF };
		uint8_t t6[4] = { 1, 5, 6, W00FFFF };
		uint8_t t7[4] = { 1, 6, 2, W00FFFF };
		uint8_t t8[4] = { 4, 5, 1, WFF0000 };
		uint8_t t9[4] = { 4, 1, 0, WFF0000 };
		uint8_t t10[4] ={ 2, 6, 7, WFFFF00 };
		uint8_t t11[4] ={ 2, 7, 3, WFFFF00 };
		uint8_t* t[] = { t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11 };
	
		static float rotate = 0.0;
		static uint8_t rotateOption = 1;

		//gc->FillTriangle(34, 45, 240, 90, 310, 190, WFFFFFF);
	
		for (int i = 0; i < 8; i++) {
		
			gc->RotateVertex(v[i], rotate, rotateOption);
			gc->RotateVertex(v[i], rotate, rotateOption-1);
		}
	
		gc->FillRectangle(0, 0, gc->gfxWidth, gc->gfxHeight, W000000);
		gc->RenderObject(v, 8, t, 12);
	
		rotate += 0.01;

		gc->PutText("osakaOS", gc->gfxWidth-90, gc->gfxHeight-25, WFFFFFF, 1);
	}
	//write to video memory
	sleep(16);
	gc->DrawToScreen();
}



void Desktop::DrawTaskBar(common::GraphicsContext* gc) {
	
	if (this->minWindows == 0) { return; }

	gc->FillRectangle(0, (gc->gfxHeight-(gc->gfxHeight/20))-3, this->gc->gfxWidth, 20, WAAAAAA);
	gc->DrawLine(0, (gc->gfxHeight-(gc->gfxHeight/20))-3, this->gc->gfxWidth, 
			(gc->gfxHeight-(gc->gfxHeight/20))-3, WAAAAAA);


	uint16_t ButtonWidth = (this->gc->gfxWidth / this->minWindows);
	uint8_t ButtonNum = 0;

	for (int i = 0; i < numChildren; i++) {

		if (this->children[i]->Min) {
			
			//draw button
			uint16_t x0 = (ButtonWidth * ButtonNum);

			gc->FillRectangle(x0, gc->gfxHeight-(gc->gfxHeight/20), x0+ButtonWidth, gc->gfxHeight-2, ButtonNum+1);
			gc->DrawRectangle(x0, gc->gfxHeight-(gc->gfxHeight/20), x0+ButtonWidth, gc->gfxHeight-2, W555555);
			ButtonNum++;
			
			//draw name of window
			char* name = children[i]->name;
			gc->PutText(name, x0+2, gc->gfxHeight-(gc->gfxHeight/20)+2, W000000);
			gc->PutText(name, x0+1, gc->gfxHeight-(gc->gfxHeight/20)+1, WFFFFFF);
		}
	}
}

void Desktop::TaskBarClick(uint8_t button) {

	if (this->minWindows <= 0) { return; }
	
	uint16_t ButtonWidth = (this->gc->gfxWidth / this->minWindows);
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
			
				case APP_TYPE_KASUGAPAINT:
					cursorArt = cursorChiChi;
					mouseW = 17;
					mouseH = 19;
					offsetY = -3;
					offsetX = -1;
					break;

				case APP_TYPE_JOURNAL:
					cursorArt = cursorPencil;
					mouseW = 16;
					mouseH = 16;
					break;
				default:

					if (this->customCursorBuf != nullptr) {
					
						cursorArt = this->customCursorBuf;
						mouseW = this->customCursorW;
						mouseH = this->customCursorH;
					} else {
						cursorArt = cursorClassic;
						mouseW = 7;
						mouseH = 7;
					}
					break;
			}
		
			//resize cursor	
			if (this->actionDetected) {
						
				cursorArt = cursorResize;
				mouseW = 15;
				mouseH = 15;
				offsetX = -7;
			}
		}
	} else {
		if (this->customCursorBuf != nullptr) {
					
			cursorArt = this->customCursorBuf;
			mouseW = this->customCursorW;			
			mouseH = this->customCursorH;
		} else {
			cursorArt = cursorClassic;
			mouseW = 7;
			mouseH = 7;
		}
	}

	//draw mouse
	gc->FillBuffer(MouseX+offsetX, MouseY+offsetY, mouseW, mouseH, cursorArt, false);
}


void Desktop::LoadCursor(uint8_t* buf, uint16_t w, uint16_t h) {

	if (this->customCursorBuf != nullptr) {
	
		this->memoryManager->free(this->customCursorBuf);
	}

	if (buf == nullptr) {
	
		this->customCursorBuf = nullptr;
		return;
	}


	this->customCursorBuf = (uint8_t*)this->memoryManager->malloc(sizeof(w*h));
	
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
	
			this->customCursorBuf[w*i+j] = buf[this->gc->gfxWidth*i+j];
		}
	}
	this->customCursorW = w;
	this->customCursorH = h;
}



void Desktop::Screenshot() {

	char* fileName = "SS_0000";
	char* hex = "0123456789ABCDEF";
	uint16_t count = (uint16_t)(this->filesystem->GetFileCount());
	fileName[6] = hex[(count & 0xf)];
	fileName[5] = hex[(count >> 4) & 0xf];
	fileName[4] = hex[(count >> 8) & 0xf];
	fileName[3] = hex[(count >> 12)];

	uint8_t buf[gc->gfxBufferSize];

	for (int i = 0; i < gc->gfxBufferSize; i++) {
	
		buf[i] = this->gc->pixels[i];
	}
	
	//this->filesystem->Write13H(fileName, buf, this->gc->gfxWidth, this->gc->gfxHeight, true);
	uint8_t* ptr = nullptr;
	if (this->filesystem->GetTagFile("compressed", filesystem->GetFileSector(fileName), ptr)) {
	
		this->filesystem->Write13H(fileName, buf, this->gc->gfxWidth, this->gc->gfxHeight, true);
	} else {
		this->filesystem->Write13H(fileName, buf, this->gc->gfxWidth, this->gc->gfxHeight, false);
	}
	
}




void Desktop::OnMouseDown(common::uint8_t button) {

	if (this->osaka->sim) { this->osaka->OnMouseDown(MouseX, MouseY, button);
	} else {
		//left click after switching to sim and back
		//and when there are 0 children (windows) causes crash
		if (MouseY >= gc->gfxHeight-(gc->gfxHeight/20) && this->taskbar) {
		
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
	
	this->timer = 0;
}

void Desktop::OnMouseUp(common::uint8_t button) {
	
	if (this->osaka->sim) {
		this->osaka->OnMouseUp(MouseX, MouseY, button);
	} else {
		this->click = false;
		CompositeWidget::OnMouseUp(MouseX, MouseY, button);
	}
	this->timer = 0;
}

void Desktop::OnMouseMove(int x, int y) {
	
	this->actionDetected = false;

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
	
	
	this->timer = 0;
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
		
		if (this->timer < 0xffff) {
		
			CompositeWidget::OnKeyDown(str);
		}
	}

	//take screenshot
	//if (str == 6) { this->takeSS = true; }
	this->timer = 0;
}


void Desktop::OnKeyUp(char str) {
	
	if (this->osaka->sim) { this->osaka->OnKeyUp(str); }
	else { CompositeWidget::OnKeyUp(str); }
	this->timer = 0;
}



void Desktop::CreateButton(char* file, uint8_t openType, uint8_t* imageFile) {

	if (this->buttons->numOfNodes >= gc->gfxWidth/2) { return; }

	DesktopButton* button = (DesktopButton*)(this->memoryManager->malloc(sizeof(DesktopButton)));
	new (button) DesktopButton(file, openType, imageFile, this->buttons->numOfNodes);

	this->buttons->Push(button);
}






//class for desktop icons and shortcuts
DesktopButton::DesktopButton(char* file, uint8_t openType, uint8_t* imageFile, uint8_t index) 
: Widget((index * 20) % this->gc->gfxWidth, (index / 16) * 20, 20, 20) {

	this->openType = openType;

	for (int i = 0; file[i] != '\0'; i++) { 
			
		this->file[i] = file[i]; 
	}

	memset(this->iconBuf, 0x00, 400);

	if (imageFile == nullptr) { 
	
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
		
				this->iconBuf[20*i+j] = fileShortcut[16*i+j];
			}
		}
	} else {
		//20x20 sprite res
		for (int i = 0; i < 300; i++) { this->iconBuf[i] = imageFile[i]; }
		//for (int i = 0; i < 400; i++) { this->iconBuf[i] = imageFile[i]; }
	}
	
	if (this->buf != nullptr) {
	
		this->gc->mm->free(this->buf);
	}
	this->buf = this->iconBuf;
}

DesktopButton::~DesktopButton() {}


void DesktopButton::Draw(GraphicsContext* gc) {

	gc->FillBuffer(this->x, this->y, 20, 20, this->iconBuf, false);
	
	gc->PutText(this->file, this->x+2, this->y+16, W000000);
	gc->PutText(this->file, this->x+1, this->y+15, WFFFFFF);
}


void DesktopButton::OnMouseDown(int32_t x, int32_t y, uint8_t button, Desktop* desktop) {

	if (ContainsCoordinate(x, y) == false) { return; }

	for (int i = 0; i < desktop->numChildren; i++) {
	
		if (desktop->children[i]->ContainsCoordinate(x, y)) { return; }
	}

	CompositeWidget* widget = nullptr;

	switch (this->openType) {

		case APP_TYPE_SCRIPT:
		case APP_TYPE_TERMINAL:
			widget = desktop->CreateChild(APP_TYPE_TERMINAL, "Osaka's Terminal", nullptr);
			break;
		case APP_TYPE_KASUGAPAINT:
			widget = desktop->CreateChild(APP_TYPE_KASUGAPAINT, "KasugaPaint", nullptr);
			break;
		case APP_TYPE_SHINOSAKA:
			widget = desktop->CreateChild(APP_TYPE_SHINOSAKA, "Shinosaka", nullptr);
			break;
		//file will be opened for 
		//text editing as default
		case APP_TYPE_JOURNAL:
		default:
			widget = desktop->CreateChild(APP_TYPE_JOURNAL, "Journal", nullptr);
			break;
	}
	
	if (this->file != nullptr && desktop->filesystem->FileIf(desktop->filesystem->GetFileSector(this->file))) {
	
		Window* win = (Window*)widget;
		win->app->ReadInput(this->file, widget, desktop->filesystem);
	}
}
