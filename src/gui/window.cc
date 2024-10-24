#include <gui/window.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::gui;
using namespace os::math;
using namespace os::filesystem;


Window::Window(CompositeWidget* parent, 
int32_t x, int32_t y,
int32_t w, int32_t h,
char* name,
uint8_t color,
App* app,
FileSystem* filesystem) 
: CompositeWidget(parent, x, y, w, h, name, color, true) {

	this->filesystem = filesystem;

	this->Dragging = false;
	this->Resize = false;

	//for dumping buffer/text into file	
	this->FileWindow = false;
	this->Save = false;
	this->Tools = false;
	this->fileNameIndex = 0;
	for (int i = 0; i < 33; i++) { fileName[i] = '\0'; }

	this->name = name;
	this->textColor = 0x3f;

	this->app = app;
	this->winColor = 0x19;

	//fill buffer with some color
	for (int i = 0; i < 64000; i++) { this->buf[i] = color; }
}


Window::~Window() {
}



void Window::Draw(GraphicsContext* gc) {

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
		gc->DrawLine(x+w-7, y+9, x+w-2, y+4, 0x40);
		gc->DrawLine(x+w-6, y+4, x+w-1, y+9, 0x40);
		gc->DrawLine(x+w-8, y+8, x+w-3, y+3, 0x07);
		gc->DrawLine(x+w-7, y+3, x+w-2, y+8, 0x07);

		//max button
		gc->DrawRectangle(x+w-17, y+3, 6, 6, 0x40);
		gc->DrawRectangle(x+w-18, y+2, 6, 6, 0x07);

		//min button
		gc->DrawLine(x+w-27, y+8, x+w-21, y+8, 0x40);
		gc->DrawLine(x+w-28, y+7, x+w-22, y+7, 0x07);

		//menu button
		gc->PutText("<", x+w-37, y+2, 0x40);
		gc->PutText("<", x+w-38, y+1, 0x07);
	

		//name of window
		gc->PutText(this->name, x+2, y+2, 0x40);
		gc->PutText(this->name, x+1, y+1, this->textColor);
	

		//draw menu if active
		if (this->Menu) { this->MenuDraw(gc); }



		//outer rectangle
		gc->DrawRectangle(x, y, w, 10, 0x38);

		//window border
		gc->DrawRectangle(x, y, w, h, 0x38);
		gc->DrawLineFlat(x, y, x+w, y+h, 0x07, false);
		gc->DrawLineFlat(x, y, x+w, y+h, 0x07, true);
	
	} else {
		gc->FillBufferFull(x, y, w, h, this->buf);
		if (this->Menu) { this->MenuDraw(gc); }
	}

	
	if (this->app != nullptr) {	
	
		//compute program associated with window
		this->app->ComputeAppState(gc, this);
		this->app->DrawAppMenu(gc, this);
	}
	

	//windows need to read/save
	//buffer or text to file
	if (this->FileWindow) {
	
		this->FileDraw(gc);
	}
}

void Window::MenuDraw(GraphicsContext* gc) {

	//draw box for menu
	uint8_t menuDrawOffset = 10 * (this->Fullscreen == false);	
	//uint8_t c0 = light2dark[this->winColor];
	uint8_t c0 = light2dark[0x09];
	uint8_t c1 = light2dark[c0];
	uint8_t c2 = light2dark[c1];
	uint8_t c3 = light2dark[c2];
	gc->FillRectangle(x, y+menuDrawOffset, w, 9, c0);
	gc->FillRectangle(x, y+menuDrawOffset+2, w, 2, c1);
	gc->FillRectangle(x, y+menuDrawOffset+4, w, 2, c2);
	gc->FillRectangle(x, y+menuDrawOffset+6, w, 2, c3);
	gc->DrawRectangle(x, y+menuDrawOffset-1, w, 10, 0x38);


	//draw words
	gc->PutText("Read - Save - Tool", x+2, y+menuDrawOffset+1, 0x40);
	gc->PutText("Read - Save - Tool", x+1, y+menuDrawOffset, 0x3f);
}

void Window::FileDraw(GraphicsContext* gc) {
	
	uint16_t x = 67;
	uint8_t y = 75;
	uint16_t w = 187;
	uint8_t h = 40;
	uint8_t c = 0x09;

	gc->FillRectangle(x, y, w, h, os::drivers::light2dark[c]);
	

	if (this->Save) {
		
		gc->PutText("Enter file name below. (Save)", x+7, y+9, 0x40);
		gc->PutText("Enter file name below. (Save)", x+6, y+8, 0x3f);
	} else {
		gc->PutText("Enter file name below. (Read)", x+7, y+9, 0x40);
		gc->PutText("Enter file name below. (Read)", x+6, y+8, 0x3f);
	}
	
	gc->FillRectangle(x+6, y+22, 175, 12, os::drivers::light2dark[os::drivers::light2dark[c]]);
	gc->PutText(this->fileName, x+7, y+25, 0x40);
	gc->PutText(this->fileName, x+6, y+24, 0x3f);


	gc->DrawRectangle(x, y, w, h, 0x38);
	gc->DrawLineFlat(x, y, x+w, y+h, 0x07, false);
	gc->DrawLineFlat(x, y, x+w, y+h, 0x07, true);
}



void Window::FullScreen() {

	this->Fullscreen ^= 1;
	
	if (Fullscreen) {
	
		x = 0;
		y = 0;
		w = 320;
		h = 200;
		this->Dragging = false;
	} else {
		w = wo;
		h = ho;
	}
}




uint8_t Window::ReturnAppType() {

	return this->app->appType;
}




void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button) {

	if (x == this->x || y == this->y 
	 || x == this->x + this->w - 1 
	 || y == this->y + this->h - 1) {
	
		Resize = true;
		Dragging = false;
	
	//always allow dragging if pressing
	//on window border/ui thingy
	} else if (Fullscreen == false && y < this->y + 10) {

		Resize = false;
		Dragging = true;
	
		//this is fucked up but it works
		if (x >= this->x+w-10) { this->app->Close(); }

	//click on menu below border
	} else if (this->Menu && y < this->y + 20 - (10 * Fullscreen)) {


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
	uint8_t offsety = 10 * !Fullscreen;

	CompositeWidget::OnMouseDown(x, y, button);
	this->app->OnMouseDown(x-offsetx, y-offsety, button, this);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
	
	uint16_t offsetx = 1 * !Fullscreen;
	uint8_t offsety = 10 * !Fullscreen;

	Dragging = false;
	Resize = false;
	CompositeWidget::OnMouseUp(x, y, button);
	this->app->OnMouseUp(x-offsetx, y-offsety, button, this);
}

void Window::OnMouseMove(int32_t oldx, int32_t oldy,
		 	 int32_t newx, int32_t newy) {
	
	//resizing window
	if (Resize) {
	
		this->w += (newx - oldx) * (this->w >= 70);
		this->h += (newy - oldy) * (this->h >= 35);
	} else {
		if (Dragging) {
	
			this->x += newx - oldx;
			this->y += newy - oldy;
		}
	
		uint16_t offsetx = 1 * !Fullscreen;
		uint8_t offsety = 10 * !Fullscreen;
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
