#include <gui/window.h>

using namespace os::common;
using namespace os::gui;


Window::Window(Widget* parent, 
int32_t x, int32_t y,
int32_t w, int32_t h,
char* name,
uint8_t color,
App* app) 
: CompositeWidget(parent, x, y, w, h, name, color, true) {

	this->Dragging = false;

	this->name = name;
	this->textColor = 0x3f;

	this->outx = 1;
	this->outy = 10;

	this->app = app;
	
	//fill buffer with some color
	for (int i = 0; i < 64000; i++) {
	
		this->buf[i] = color;
	}
}


Window::~Window() {
}



void Window::Draw(GraphicsContext* gc) {


	//fill window buffer
	gc->FillBufferFull(x, y, w, h, this->buf);

	//draw ui stuff
	gc->FillRectangle(x, y, w, 10, 0x00);

	//delete button
	gc->DrawLine(x+w-8, y+8, x+w-3, y+3, 0x38);
	gc->DrawLine(x+w-7, y+3, x+w-2, y+8, 0x38);

	//max button
	gc->DrawRectangle(x+w-18, y+2, 6, 6, 0x38);

	//min button
	gc->DrawLine(x+w-28, y+7, x+w-22, y+7, 0x38);


	//name of window
	gc->PutText(this->name, x+1, y+1, this->textColor);


	//outer rectangle
	gc->DrawRectangle(x, y, w, 10, 0x38);

	//window border
	gc->DrawRectangle(x, y, w, h, 0x38);


	//compute program associated with window
	this->app->ComputeAppState(gc, this);
}



void Window::PutChar(char ch) {

	uint8_t* charArr = charset[0];
	uint8_t pixelColor = textColor;

	//load font char
	if (ch >= 32 && ch <= 122) {
	
		charArr = charset[ch-32];
	} else {
		charArr = font_full;
	}

	switch(ch) {
	
		case '\b':
			if (outx > 0) {
				outx -= font_width;
			} else {
				outy -= font_height;
				outx = 313;
			}
			pixelColor = this->color;
			break;
		case '\n':
			outx = 1;
			outy += font_height;
			this->text[textNum++] = ch;
			return;
			break;
		defualt:
			//add char to window
			this->text[textNum++] = ch;
			break;
	}


	//font is 8x5 monospace
	for (uint16_t i = 0; i < font_width; i++) {
		for (uint8_t j = 0; j < font_height; j++) {
		
			if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
				this->buf[320*(outy+j)+(outx+i)] = pixelColor;
			}
		}
	}		
	outx += font_width;


	//crashes if y overflows
	if (outx >= 318) {
	
		outx = 1;
		outy += font_height;
	}

	//53x21 resolution for font i think??
}


void Window::Print(char* str) {

	//better if this method doesn't have to call
	//putchar for each char in string
	uint8_t* charArr = charset[0];
	uint8_t pixelColor = textColor;
	
	//print string
	for (uint8_t ch = 0; str[ch] != '\0'; ch++) {	
		
		this->text[textNum++] = str[ch];
	
		switch (str[ch]) {
		
			case '\v':
				for (uint16_t i = 0; i < textNum; i++) { 
				
					this->text[i] = 0x00; 
				}
				textNum = 0;

				for (uint16_t i = 0; i < 64000; i++) {
					
					this->buf[i] = this->color;
				}
				outx = 1;
				outy = 10;
				
				break;
			case '\n':
				outx = 1;
				outy += font_height;
				break;
			default:
				charArr = charset[str[ch]-32];
				
				for (uint8_t i = 0; i < font_width; i++) {
					for (uint8_t j = 0; j < font_height; j++) {
		
						if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
							this->buf[320*(outy+j)+(outx+i)] = pixelColor;
						}
					}
				}
				outx += font_width;
	
				if (outx >= 318) {
	
					outx = 1;
					outy += font_height;
				}

				//scroll
				if (outy >= 187) {

					uint16_t scrollIndex = 0;
				
					for (uint8_t y = 0; y < 200; y++) {	
						for (uint16_t x = 0; x < 320; x++) {
					
							scrollIndex = 320*(y+font_height)+x;

							if (scrollIndex < 64000) {
								
								this->buf[320*y+x] = this->buf[scrollIndex];
							}
						}
					}
				}
				break;
		}
	}
}



void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button) {

	Dragging = (button == 1);
	CompositeWidget::OnMouseDown(x, y, button);
	this->app->OnMouseDown(x, y, button, this);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button) {

	Dragging = false;
	CompositeWidget::OnMouseUp(x, y, button);
	this->app->OnMouseUp(x, y, button, this);
}

void Window::OnMouseMove(int32_t oldx, int32_t oldy,
		 	 int32_t newx, int32_t newy) {

	if (Dragging) {
	
		this->x += newx - oldx;
		this->y += newy - oldy;
	}
	CompositeWidget::OnMouseMove(oldx, oldy, newx, newy);
	this->app->OnMouseMove(oldx, oldy, newx, newy, this);
}


void Window::OnKeyDown(char str) {

	this->keyCharWidget = str;
	this->keyPressed = true;

	CompositeWidget::OnKeyDown(str);
	this->app->OnKeyDown(str, this);
}

void Window::OnKeyUp(char str) {
	
	this->keyPressed = false;
	
	CompositeWidget::OnKeyUp(str);
	this->app->OnKeyUp(str, this);
}
