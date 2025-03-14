#include <gui/widget.h>


using namespace os::common;
using namespace os::gui;


uint16_t strlen(char* str);
void sleep(uint32_t);


Widget::Widget(int32_t x, int32_t y, int32_t w, int32_t h) 
: KeyboardEventHandler() {

	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}


Widget::~Widget() {
}


bool Widget::ContainsCoordinate(int32_t x, int32_t y) {

	return this->x <= x && x < this->x + this->w
	    && this->y <= y && y < this->y + this->h;
}

void Widget::PutPixel(int32_t x, int32_t y, uint8_t color) {

	if (x < this->x+w && y < this->y+h && 
	    x >= this->x && y >= this->y) {
	
		//this->buf[this->w*(y-this->y)+(x-this->x)] = color;
		this->buf[WIDTH_13H*(y-this->y)+(x-this->x)] = color;
	}
}

void Widget::PutText(char* str, int32_t x, int32_t y, uint8_t color) {

	uint16_t length = strlen(str);

	if ((WIDTH_13H - x) < (length * 5)) { return; }

	uint8_t* charArr = charset[0];

	for (int i = 0; str[i] != '\0'; i++) {
	
		charArr = charset[str[i]-32];

		for (uint8_t w = 0; w < font_width; w++) {
			for (uint8_t h = 0; h < font_height; h++) {
			
				if (charArr[w] && ((charArr[w] >> h) & 1)) {
				
					this->PutPixel(x+w, y+h, color);
				}
			}
		}
		x += font_width;
	}
}




WindowButton::WindowButton(CompositeWidget* window, 
		int32_t x, int32_t y, int32_t w, int32_t h, 
		uint8_t buttonType,
		bool offsetX, bool offsetY, 
		bool offsetW, bool offsetH) 
: Widget(x, y, w, h) {

	this->window = window;
	this->buttonType = buttonType;
	
	this->savex = x;
	this->savey = y;

	this->offsetX = offsetX;
	this->offsetY = offsetY;
	this->offsetW = offsetW;
	this->offsetH = offsetH;
}

WindowButton::~WindowButton() {}

void WindowButton::Draw(GraphicsContext* gc) {

	if (this->offsetX) { this->x = window->w - this->savex; }
	if (this->offsetY) { this->x = window->h - this->savey; }

	//draw to buffer
	gc->FillBufferFull(this->x+window->x, this->y+window->y, 
			this->w, this->h, this->buf);
}

void OnMouseDown(int32_t x, int32_t y, uint8_t button) {}






CompositeWidget::CompositeWidget(CompositeWidget* parent,  int32_t x, int32_t y, 
						int32_t w, int32_t h, 
						char* name,
						uint8_t color,
						bool window) 
: Widget(x, y, w, h) {

	this->parent = parent;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	
	this->windowOffset = 0;

	this->buf = this->windowBuffer;

	//orignial resolution	
	this->xo = x;
	this->yo = y;
	this->wo = w;
	this->ho = h;

	this->name = name;
	this->color = color;

	this->Fullscreen = false;
	this->Focussable = window;
	this->Buttons = window;
	this->Resizable = window;
	this->Menu = false;

	focussedChild = 0;
	numChildren = 0;
	outx = 0;
	outy = 0;
}

CompositeWidget::~CompositeWidget() {}



void CompositeWidget::GetFocus(CompositeWidget* widget) {
	
	if (parent != 0) { parent->GetFocus(widget); }

	this->focussedChild = widget;
	int i = windowOffset;

	//find widget index
	for (i; i < numChildren; i++) {
		if (children[i] == focussedChild) { break; }
	}
	
	//push children back by 1
	for (i; i > 0; i--) {
	
		children[i] = children[i-1];
	}
	children[0] = focussedChild;

	if (parent != 0) { parent->GetFocus(this); }
}                 




uint8_t CompositeWidget::ContainsCoordinateButton(int32_t x, int32_t y) {

	if (this->x <= x && x < this->x + this->w
	    && this->y <= y && y < this->y + 10 
	    && this->Fullscreen == false) {

		uint8_t buttonDistance = this->w - (x - this->x);

		if (buttonDistance <= 10) {	   return 1; //close
		} else if (buttonDistance <= 20) { return 2; //maximize
		} else if (buttonDistance <= 30) { return 3; //minimize
		} else if (buttonDistance <= 40) { return 4; //menu
		} else { return 0; }
	}
	return 0xff;
}



CompositeWidget* CompositeWidget::CreateChild(uint8_t appType, char* name, App* app) {}
void CompositeWidget::CreateButton(char* file, uint8_t appType, char* imageFile) {}


bool CompositeWidget::AddChild(CompositeWidget* child) {

	if (numChildren >= 30) {
		return false;
	}
	children[numChildren++] = child;

	return true;
}


bool CompositeWidget::DeleteChild() {

	if (numChildren <= 0) {
		
		return false;

	} else if (numChildren > 1) {
	
		for (int i = 0; i < numChildren-1; i++) {
		
			children[i] = children[i+1];
		}
	} else {
		children[0] = 0; 
	}
	numChildren--;

	return true;
}


bool CompositeWidget::Maximize() {

	if (	children[0]->w >= WIDTH_13H && 
		children[0]->h >= HEIGHT_13H &&
		children[0]->x > 0    &&
		children[0]->y > 0) {
	
		return false;
	}
	
	children[0]->w = WIDTH_13H;
	children[0]->h = HEIGHT_13H;
	children[0]->x = 0;
	children[0]->y = 0;

	return true;
}

bool CompositeWidget::Minimize() {
	
	CompositeWidget* widget = children[0];

	widget->xo = widget->x;
	widget->yo = widget->y;
	widget->wo = widget->w;
	widget->ho = widget->h;
	
	widget->Min = true;
	widget->x = WIDTH_13H;
	widget->y = HEIGHT_13H;
	widget->parent->minWindows++;
	focussedChild = 0;

	return true;
}




bool CompositeWidget::MenuButton() {return true;}


void CompositeWidget::Resize(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {

	children[0]->w += (oldx - newx);
}


void CompositeWidget::ModelToScreen(int32_t &x, int32_t &y) {

	if (parent != 0) { parent->ModelToScreen(x, y); }
	x += this->x;
	y += this->y;
}


void CompositeWidget::Draw(GraphicsContext* gc) {

	int X = 0;
	int Y = 0;
	this->ModelToScreen(X, Y);

	for (int i = numChildren - 1; i >= windowOffset; i--) {
	
		if (children[i]->Min == false && children[i] != nullptr) {	
		
			children[i]->Draw(gc);
		}
	}
}


void CompositeWidget::MenuDraw(GraphicsContext* gc) {}


void CompositeWidget::ButtonAction(uint8_t button) {

	switch (button) {
	
		case 1:
			DeleteChild();
			break;
		case 2:
			Maximize();
			break;
		case 3:
			Minimize();
			break;
		case 4:
			break;
		default:
			break;
	}
}

uint8_t CompositeWidget::ReturnAppType() {}



void CompositeWidget::WritePixel(int32_t x, int32_t y, uint8_t color) {

	if (x < WIDTH_13H && y < HEIGHT_13H) { this->buf[WIDTH_13H*y+x] = color; }
}

uint8_t CompositeWidget::ReadPixel(uint32_t i) { return this->buf[i]; }


void CompositeWidget::DrawRectangle(int32_t x0, int32_t y0, 
				    int32_t x1, int32_t y1, 
				    uint8_t color, bool fill) {

	if (fill) {
		for (int y = y0; y < y0+y1; y++) {
			for (int x = x0; x < x0+x1; x++) {
		
				this->WritePixel(x, y, color);
			}
		}
	} else {
		for (int x = x0; x < x0+x1; x++) { 
			this->WritePixel(x, y0, color); 
			this->WritePixel(x, y1, color); 
		}
		for (int y = y0; y < y0+y1; y++) { 
			this->WritePixel(x0, y, color); 
			this->WritePixel(x1, y, color); 
		}
	}
}


void CompositeWidget::DrawLine(int32_t x0, int32_t y0, 
			       int32_t x1, int32_t y1, 
			       uint8_t color) {
	
	struct math::point pointArr[WIDTH_13H];
	uint16_t pixelNum = LineFillArray(x0, y0, x1, y1, pointArr);

	int32_t index = 0;

	for (int i = 0; i < pixelNum; i++) {
			
		index = (WIDTH_13H*(pointArr[i].y - this->y)+(pointArr[i].x - this->x));

		/*
		if (pointArr[i].x < this->x+w && pointArr[i].y < this->y+h
		 && pointArr[i].x >= this->x && pointArr[i].y >= this->y
		 && index < 64000 && index >= 0) {
		*/
		if (pointArr[i].x >= this->x && pointArr[i].y >= this->y && index < 64000 && index >= 0) {
		
			this->buf[WIDTH_13H*(pointArr[i].y - this->y)+(pointArr[i].x - this->x)] = color;
		}
	}
}


void CompositeWidget::DrawCircle(int32_t x, int32_t y, 
				 int32_t r, uint8_t color) {
	
	int xc = 0, yc = r;
	int d = 3 - 2 * r;

	this->PutPixel(x+xc, y+yc, color);
	this->PutPixel(x-xc, y+yc, color);
	this->PutPixel(x+xc, y-yc, color);
	this->PutPixel(x-xc, y-yc, color);
	this->PutPixel(x+yc, y+xc, color);
	this->PutPixel(x-yc, y+xc, color);
	this->PutPixel(x+yc, y-xc, color);
	this->PutPixel(x-yc, y-xc, color);

	while (yc >= xc) {
	
		xc++;

		if (d > 0) {
		
			yc--;
			d = d + 4 * (x - y) + 10;
		} else {
		
			d = d + 4 * x + 6;
		}
	
		this->PutPixel(x+xc, y+yc, color);
		this->PutPixel(x-xc, y+yc, color);
		this->PutPixel(x+xc, y-yc, color);
		this->PutPixel(x-xc, y-yc, color);
		this->PutPixel(x+yc, y+xc, color);
		this->PutPixel(x-yc, y+xc, color);
		this->PutPixel(x+yc, y-xc, color);
		this->PutPixel(x-yc, y-xc, color);
	}
}



void CompositeWidget::FillBuffer(int32_t x, int32_t y, 
			       int16_t w, int16_t h, 
			       uint8_t* newbuf) {
	uint8_t pixelColor = 0;
	uint8_t scrollVert = 0;
	bool scroll = false;

	if (y < 0) {
	
		if (y+h < 0) { return; }
		scrollVert = (y*-1);
		h -= (y*-1);
		y = 0;
		scroll = true;
	}

	for (int16_t Y = y; Y < y+h; Y++) {
		for (int16_t X = x; X < x+w; X++) {
		
			pixelColor = newbuf[w*Y+X];

			if (pixelColor) {
			
				this->buf[WIDTH_13H*Y+X] = pixelColor; 
			}
		}
	}
}




void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
	
	for (int i = windowOffset; i < numChildren; i++) {

		if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {

			if (children[i] != focussedChild) {

				GetFocus(children[i]);
			}

			//left click
			if (button == 1) {
			
				uint8_t buttonAction = focussedChild->ContainsCoordinateButton(x, y);

				switch (buttonAction) {

					case 1:
						DeleteChild();
						break;
					case 2:
						Maximize();
						break;
					case 3:
						Minimize();
						break;
					case 4:
						children[0]->Menu ^= 1;
						break;
					default:
						break;
				}
			}

			if (focussedChild != 0) {

				focussedChild->OnMouseDown(x - this->x, y - this->y, button);
			}
			break;
		}
	}
	this->mouseclick = true;
}	

void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {

	for (int i = windowOffset; i < numChildren; i++) {
			
		children[i]->OnMouseUp(x - this->x, y - this->y, button);
	}
	this->mouseclick = false;
}                                

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {


	int firstchild = -1;
	for (int i = windowOffset; i < numChildren; i++) {
		
		if (children[i]->ContainsCoordinate(oldx - this->x, oldy - this->y)) {
			
			children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x, newy - this->y);
			firstchild = i;
			break;
		}
	}

	for (int i = windowOffset; i < numChildren; i++) {
		
		if (children[i]->ContainsCoordinate(newx - this->x, newy - this->y)) {
	
			if (firstchild != i) {	
			
				children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x, newy - this->y);
			}
			break;
		}
	}
}




void CompositeWidget::PutChar(char ch) {

	//53x22 text res

	uint8_t* charArr = charset[0];
	uint8_t pixelColor = textColor;
	
	uint8_t adjustedTextHeight = TEXT_MAX_HEIGHT - (1 * this->Fullscreen == false);


	//load font char
	if (ch >= 32 && ch <= 127) { charArr = charset[ch-32];
	} else { charArr = font_full; }

	switch(ch) {

		case '\b':
			if (outx > 1) {
				outx--;
			} else {
				outy--;
				outx = TEXT_MAX_WIDTH - 1;
			}
			pixelColor = this->color;
			break;
		case '\n':
			outx = 0;
			outy++;
			return;
			break;
		defualt:
			break;
	}
	//scrolling
	if (outy >= adjustedTextHeight) {
	
		for (uint8_t y = 0; y < HEIGHT_13H; y++) {
			for (uint16_t x = 0; x < WIDTH_13H; x++) {
	
				if (y < 189) {  this->buf[WIDTH_13H*y+x] = this->buf[WIDTH_13H*(y+font_height)+x];
				} else { 	this->buf[WIDTH_13H*y+x] = this->color; }
			}
		}
		outx = 0;
		outy--;

		this->textScroll = true;
	} else {this->textScroll = false; }


	//font is 8x5 monospace
	for (uint16_t i = 0; i < font_width; i++) {
		for (uint8_t j = 0; j < font_height; j++) {
		
			if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
				this->buf[WIDTH_13H*((outy*9)+j)+((outx*6)+i)] = pixelColor;
			}
		}
	}		
	if (ch != '\b') { outx++; }


	if (outx >= TEXT_MAX_WIDTH) {
	
		outx = 0;
		outy++;
	}
}


void CompositeWidget::Print(char* str) {

	//better if this method doesn't have to call
	//putchar for each char in string
	uint8_t* charArr = charset[0];
	uint8_t pixelColor = textColor;

	
	//uint8_t TEXT_MAX_WIDTH = 53;
	//uint8_t TEXT_MAX_HEIGHT = 22;
	uint8_t adjustedTextHeight = TEXT_MAX_HEIGHT - (1 * this->Fullscreen == false);
	

	//print string
	for (uint8_t ch = 0; str[ch] != '\0'; ch++) {	

		switch (str[ch]) {
		
			case '\v':
				//clear text
				for (uint16_t i = 0; i < 64000; i++) {
					
					this->buf[i] = this->color;
				}
				outx = 0;
				outy = 0;
				
				break;
			case '\n':
				outx = 0;
				outy++;
				break;
			default:
				charArr = charset[str[ch]-32];
				
				//scrolling
				if (outy >= adjustedTextHeight) {
	
					for (uint8_t y = 0; y < HEIGHT_13H; y++) {
						for (uint16_t x = 0; x < WIDTH_13H; x++) {
	
							if (y < 189) {  this->buf[WIDTH_13H*y+x] = this->buf[WIDTH_13H*(y+font_height)+x];
							} else {	this->buf[WIDTH_13H*y+x] = this->color; }
						}
					}
					outx = 0;
					outy--;

					this->textScroll = true;
				} else {this->textScroll = false; }
			

				for (uint8_t i = 0; i < font_width; i++) {
					for (uint8_t j = 0; j < font_height; j++) {
		
						if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
							this->buf[WIDTH_13H*((outy*9)+j)+((outx*6)+i)] = pixelColor;
						}
					}
				}
				if (outx >= TEXT_MAX_WIDTH) { outx = 0; outy++; }
				if (outy < adjustedTextHeight) { outx++; }
				
				break;
		}
	}
}


void CompositeWidget::OnKeyDown(char str) {

	if (focussedChild != 0 && windowOffset == 0) {

		focussedChild->OnKeyDown(str);
	}
	this->keypress = true;
}

void CompositeWidget::OnKeyUp(char str) {

	if (focussedChild != 0 && windowOffset == 0) {

		focussedChild->OnKeyUp(str);
	}
	this->keypress = false;
}
