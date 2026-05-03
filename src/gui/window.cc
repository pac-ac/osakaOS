#include <gui/window.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::gui;
using namespace os::math;
using namespace os::filesystem;


void sleep(uint32_t);


Window::Window(CompositeWidget* parent, 
int32_t x, int32_t y,
int32_t w, int32_t h,
char* name,
uint8_t color,
App* app,
int8_t ID,
FileSystem* filesystem) 
: CompositeWidget(parent, x, y, w, h, name, color, true) {

	this->filesystem = filesystem;

	this->Dragging = false;
	this->Resizable = false;

	//for dumping buffer/text into file	
	this->FileWindow = false;
	this->Save = false;
	this->Tools = false;
	this->fileNameIndex = 0;
	for (int i = 0; i < 33; i++) { fileName[i] = '\0'; }

	this->name = name;
	//this->UpdateName(name);
	
	this->textColor = WFFFFFF;

	this->app = app;
	this->ID = ID;
	this->winColor = W0041FF;

	//init buttons
	this->buttons = (List*)(filesystem->memoryManager->malloc(sizeof(List))); 
	new (buttons) List(filesystem->memoryManager);


	//fill buffer with some color
	for (int i = 0; i < Widget::gc->gfxBufferSize; i++) { this->buf[i] = color; }

	//this->Blue = true;
	//this->Pixelize = true;
	//this->Wave = true;
}


Window::~Window() {
}



void Window::Draw(GraphicsContext* gc) {


	uint8_t saveBackground[(this->w+2)*(this->h+2)];

	//only really needed for this effect
	if (this->Wave) {
		for (int i = 0; i < h+2; i++) {
			for (int j = 0; j < w+2; j++) {
	
				uint32_t pixelsIndex = ((y-1+i)<<(8+gc->vesa))+((y-1+i)<<(6+gc->vesa))+(x-1+j);
			
				if (gc->pixels[pixelsIndex] >= 0 && gc->pixels[pixelsIndex] < Widget::gc->gfxBufferSize) {	
			
					saveBackground[(w+2)*i+j] = gc->pixels[pixelsIndex];
				}
			}
		}
	}

	if (!Fullscreen) {
	
		//fill in window buffer (actual graphical program)
		gc->FillBufferFull(x+1, y+10, w-1, h-10, this->buf);


		//draw ui stuff
		gc->FillRectangle(x, y, w, 10, this->winColor);
	
		for (int by = 0; by < 10; by++) {
			for (int bx = 0; bx < w; bx++) {
		
				if (bx % (by+1) == 0) { 
					
					gc->PutPixel(x+bx, y+by, light2dark[this->winColor]); 
				}
			}
		}

		//delete button
		gc->DrawLine(x+w-7, y+9, x+w-2, y+4, W000000);
		gc->DrawLine(x+w-6, y+4, x+w-1, y+9, W000000);
		gc->DrawLine(x+w-8, y+8, x+w-3, y+3, WAAAAAA);
		gc->DrawLine(x+w-7, y+3, x+w-2, y+8, WAAAAAA);

		//max button
		gc->DrawRectangle(x+w-17, y+3, 6, 6, W000000);
		gc->DrawRectangle(x+w-18, y+2, 6, 6, WAAAAAA);

		//min button
		gc->DrawLine(x+w-27, y+8, x+w-21, y+8, W000000);
		gc->DrawLine(x+w-28, y+7, x+w-22, y+7, WAAAAAA);

		//menu button
		gc->PutText("<", x+w-37, y+2, W000000);
		gc->PutText("<", x+w-38, y+1, WAAAAAA);
		
		//window ID
		gc->PutText(int2str(this->ID), x+w-47, y+2, W000000);
		gc->PutText(int2str(this->ID), x+w-48, y+1, WAAAAAA);
	
		//name of window
		if (this->altName == nullptr) {
			gc->PutText(this->name, x+2, y+2, W000000);
			gc->PutText(this->name, x+1, y+1, WFFFFFF);
		} else {
			gc->PutText(this->altName, x+2, y+2, W000000);
			gc->PutText(this->altName, x+1, y+1, WFFFFFF);
		}
		//gc->PutText(this->name, x+1, y+1, this->textColor);
	
		//draw menu if active
		if (this->MenuOpen) { this->WindowMenuDraw(gc); }

		//outer rectangle
		gc->DrawRectangle(x, y, w, 10, W555555);

		//window border
		gc->DrawRectangle(x, y, w, h, W555555);
		gc->DrawLineFlat(x, y, x+w, y+h, WAAAAAA, false);
		gc->DrawLineFlat(x, y, x+w, y+h, WAAAAAA, true);
	
		gc->DrawRectangle(x-1, y-1, w+2, h+2, W202020);
		gc->DrawLineFlat(x-1, y-1, x+w+1, y+h+1, W656565, false);
		gc->DrawLineFlat(x-1, y-1, x+w+1, y+h+1, W656565, true);

		//create shadow effect
		uint16_t darkx = x+w+1;
		uint16_t darky = y+h+1;


		for (int i = darky; i < darky+4; i++) {
			for (int j = x+4; j < darkx+4; j++) {
		
				gc->DarkenPixel(j, i);
			}
		}
		for (int i = y+4; i < darky; i++) {
			for (int j = darkx; j < darkx+4; j++) {
		
				gc->DarkenPixel(j, i);
			}
		}
	} else {
		gc->FillBufferFull(x, y, w, h, this->buf);
		if (this->MenuOpen) { this->WindowMenuDraw(gc); }
	}

	
	if (this->app != nullptr) {
	
		this->app->ComputeAppState(gc, this);
		if (this->MenuOpen) { this->app->DrawAppMenu(gc, this); }
	}
	

	//draw buttons
	for (int i = 0; i < this->buttons->numOfNodes; i++) {
	
		WindowButton* button = (WindowButton*)(this->buttons->Read(i));
		button->Draw(gc);
	}
	
	
	//graphical effects
	if (this->Rainbow) { gc->Rainbowize(x, y, w, h); }
	if (this->Pixelize) { gc->Pixelize(x, y, w, h, 2, 2); }
	
	
	if (this->Fire) { 
		
		for (int i = 0; i < h/14; i++) {
		
			gc->Burn(x, y, w, h, i%2);
			this->h--;
			sleep(2);

			if (h <= 16) {
			
				this->DestroyWindow();
				this->DeleteChild();
			}
		}
	}

	static uint8_t waveInc = 0;
	if (this->Wave) {
	
		gc->MakeWave(x-1, y-1, w+2, h+2, 16, 32, waveInc, saveBackground);
		//gc->MakeWave(x, y, w, h, 16, 32, waveInc, saveBackground);
		sleep(2);
		waveInc++;
	}

	//windows need to read/save
	//buffer or text to file
	if (this->FileWindow) { this->FileDraw(gc); }
}

void Window::WindowMenuDraw(GraphicsContext* gc) {

	//draw box for menu
	uint8_t menuDrawOffset = 10 * (this->Fullscreen == false);	
	uint8_t c0 = light2dark[W0041FF];
	uint8_t c1 = light2dark[c0];
	uint8_t c2 = light2dark[c1];
	uint8_t c3 = light2dark[c2];
	gc->FillRectangle(x, y+menuDrawOffset, w, 9, c0);
	gc->FillRectangle(x, y+menuDrawOffset+2, w, 2, c1);
	gc->FillRectangle(x, y+menuDrawOffset+4, w, 2, c2);
	gc->FillRectangle(x, y+menuDrawOffset+6, w, 2, c3);
	gc->DrawRectangle(x, y+menuDrawOffset-1, w, 10, W555555);


	//draw words
	gc->PutText("Read - Save - Tool", x+2, y+menuDrawOffset+1, W000000);
	gc->PutText("Read - Save - Tool", x+1, y+menuDrawOffset, WFFFFFF);
}

void Window::FileDraw(GraphicsContext* gc) {
	
	uint16_t x = 67;
	uint16_t y = 75;
	uint16_t w = 187;
	uint16_t h = 40;
	uint8_t c = 0x09;

	gc->FillRectangle(x, y, w, h, os::drivers::light2dark[c]);

	if (this->Save) {
		
		gc->PutText("Enter file name below. (Save)", x+7, y+9, W000000);
		gc->PutText("Enter file name below. (Save)", x+6, y+8, WFFFFFF);
	} else {
		gc->PutText("Enter file name below. (Read)", x+7, y+9, W000000);
		gc->PutText("Enter file name below. (Read)", x+6, y+8, WFFFFFF);
	}
	
	gc->FillRectangle(x+6, y+22, 175, 12, os::drivers::light2dark[os::drivers::light2dark[c]]);
	gc->PutText(this->fileName, x+7, y+25, W000000);
	gc->PutText(this->fileName, x+6, y+24, WFFFFFF);


	gc->DrawRectangle(x, y, w, h, W555555);
	gc->DrawLineFlat(x, y, x+w, y+h, WAAAAAA, false);
	gc->DrawLineFlat(x, y, x+w, y+h, WAAAAAA, true);
}


void Window::FullScreen() {

	this->Fullscreen ^= 1;
	
	if (Fullscreen) {
	
		x = 0;
		y = 0;
		w = Widget::gc->gfxWidth;
		h = Widget::gc->gfxHeight;
		this->currentTextWidth = Widget::gc->gfxWidth / (FONT_WIDTH-1);
		this->currentTextHeight = Widget::gc->gfxHeight / (FONT_HEIGHT-1);
		this->Dragging = false;
	} else {
		x = xo;
		y = yo;
		w = wo;
		h = ho;
		this->currentTextWidth = wo / (FONT_WIDTH-1);
		this->currentTextHeight = ho / (FONT_HEIGHT-1);
	}
}


void Window::WindowResize(int32_t oldx, int32_t oldy, 
			  int32_t newx, int32_t newy) {

	this->w += (newx - oldx) * (this->w >= MIN_WINDOW_WIDTH);
	this->h += (newy - oldy) * (this->h >= MIN_WINDOW_HEIGHT);

	//if (this->app->appType != APP_TYPE_SHINOSAKA) {	
	
		this->currentTextWidth = this->w / FONT_WIDTH-1;
		this->currentTextHeight = this->h / FONT_HEIGHT-1;
	//}
}


void Window::UpdateName(char* newName) {

	if (this->altName != nullptr) {
	
		buttons->memoryManager->free(this->altName);
	}

	this->altName = (char*)buttons->memoryManager->malloc(strlen(newName)+1);

	int i = 0;
	for (i; newName[i] != '\0'; i++) {
	
		this->altName[i] = newName[i];
	}
	this->altName[i] = '\0';
}


uint8_t Window::ReturnAppType() { return this->app->appType; }


void Window::DestroyWindow() {

	MemoryManager* mm = buttons->memoryManager;
	this->buttons->DestroyList();
	mm->free(this->buttons);
	mm->free(this->app);
	mm->free(this->windowBuffer);
}


void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button) {

	if (x <= this->x+1 
	 || y <= this->y+1 
	 || x >= this->x+this->w-2 
	 || y >= this->y+this->h-2) {
	
		Resizable = true;
		Dragging = false;
	
	//always allow dragging if pressing
	//on window border/ui thingy
	} else if (Fullscreen == false && y < this->y+10) {

		Resizable = false;
		Dragging = true;
		
		//this is fucked up but it works
		if (x >= this->x+w-10) { this->app->Close(); }

	//click on menu below border
	} else if (this->MenuOpen && y < this->y + 20 - (10 * Fullscreen)) {


		if (x < this->x + 30) {
		
			//read file
			this->FileWindow = true;
			this->Save = false;
		
		} else if (x < this->x + 72) {
			
			//save file
			this->FileWindow = true;
			this->Save = true;
		
		} else if (x < this->x + 108) {
		
			//tools
			this->Tools = true;
		}
	}

	uint16_t offsetx = 1 * !Fullscreen;
	uint16_t offsety = 10 * !Fullscreen;

	CompositeWidget::OnMouseDown(x, y, button);
	this->app->OnMouseDown(x-offsetx, y-offsety, button, this);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
	
	uint16_t offsetx = 1 * !Fullscreen;
	uint16_t offsety = 10 * !Fullscreen;

	Dragging = false;
	Resizable = false;
	CompositeWidget::OnMouseUp(x, y, button);
	this->app->OnMouseUp(x-offsetx, y-offsety, button, this);
}




void Window::OnMouseMove(int32_t oldx, int32_t oldy,
		 	 int32_t newx, int32_t newy) {
			
	this->parent->actionDetected =  (newx <= this->x+1 
					|| newy <= this->y+1 
					|| newx >= this->x+this->w-2 
					|| newy >= this->y+this->h-2);


	//resizing window
	if (Resizable) {
		//this->w += (newx - oldx) * (this->w >= 70);
		//this->h += (newy - oldy) * (this->h >= 35);
		this->WindowResize(oldx, oldy, newx, newy);
	} else {
		if (Dragging) {
	
			this->x += newx - oldx;
			this->y += newy - oldy;
		}
	
		uint16_t offsetx = 1 * !Fullscreen;
		uint16_t offsety = 10 * !Fullscreen;
		this->app->OnMouseMove(oldx-offsetx, oldy-offsety, newx-offsetx, newy-offsety, this);
	}
	CompositeWidget::OnMouseMove(oldx, oldy, newx, newy);
}


void Window::OnKeyDown(char str) {

	this->keyCharWidget = str;
	
	//escape
	if (str == '\x1b') {
				
		this->FileWindow = false;
		return;
	}


	//pass keypress to save file window
	if (FileWindow) {
	
		switch (str) {
		
			case '\n':
				fileName[fileNameIndex] = '\0';
				
				//save file
				if (this->Save) { this->app->SaveOutput(this->fileName, this, this->filesystem); } 
				else { this->app->ReadInput(this->fileName, this, this->filesystem); }
				
				for (int i = 0; i < fileNameIndex; i++) { fileName[i] = '\0'; }
				fileNameIndex = 0;
				FileWindow = false;
				break;
			case '\b':
				if (fileNameIndex > 0) {
				
					fileNameIndex--;
					fileName[fileNameIndex] = '\0';
				}
				break;
			default:
				if (fileNameIndex < 32) {
					
					fileName[fileNameIndex] = str;
					fileNameIndex++;
				}
				break;
		}
	//normal
	//pass keypress to window/program
	} else {
		switch (str) {

			case '\v':
				if (this->app->appType != APP_TYPE_JOURNAL) {
					
						this->MenuOpen ^= 1;
				} else {	this->app->OnKeyDown(str, this); }
				break;
			//f3
			case 0x02:
				this->FullScreen();
				break;
			//f4
			case 0x03:
				this->FileWindow = true;
				this->Save = true;
				break;
			//f5
			case 0x04:
				this->FileWindow = true;
				this->Save = false;
				break;
			default:
				this->app->OnKeyDown(str, this);
				break;
		}
	}
	CompositeWidget::OnKeyDown(str);
}

void Window::OnKeyUp(char str) {
	
	this->app->OnKeyUp(str, this);
	CompositeWidget::OnKeyUp(str);
}



/*
//window button class
WindowButton::WindowButton(Window* window, 
int32_t x, int32_t y, 
int32_t w, int32_t h, uint8_t buttonType,
bool offsetX, bool offsetY, bool offsetW, bool offsetH) 
: Widget(x, y, w, h, offsetX, offsetY, offsetW, offsetH) {

	this->buttonType = buttonType;
	this->buf = (uint8_t*)(window->filesystem->memoryManager->malloc(sizeof(w*h)));
	new (this->buf) uint8_t;
}

WindowButton::~WindowButton() {
}

void OnMouseDown(int32_t x, int32_t y, uint8_t button) {}
*/
