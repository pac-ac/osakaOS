#include <gui/widget.h>


using namespace os::common;
using namespace os::gui;


void sleep(uint32_t);
uint16_t prng();


GraphicsContext* Widget::gc = nullptr;


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
		this->buf[Widget::gc->gfxWidth*(y-this->y)+(x-this->x)] = color;
	}
}

void Widget::PutText(char* str, int32_t x, int32_t y, uint8_t color, uint8_t flags) {

	uint16_t length = strlen(str);
	if ((Widget::gc->gfxWidth - x) < (length * FONT_WIDTH-1)) { return; }

	uint8_t* charArr = charset[0];

	for (int i = 0; str[i] != '\0'; i++) {
	
		charArr = charset[str[i]];
	
		switch (flags) {

			case TEXT_ITALIC:
				for (uint16_t w = 0; w < FONT_WIDTH; w++) {
					for (uint16_t h = 0; h < FONT_HEIGHT; h++) {
			
						if (charArr[w] && ((charArr[w] >> h) & 1)) {
				
							this->PutPixel(x+w+(FONT_HEIGHT-h)/2, y+h, color);
						}
					}
				}
				break;

			case TEXT_HEADER:
				for (uint16_t i = 0; i < FONT_WIDTH; i++) {
					for (uint16_t j = 0; j < FONT_HEIGHT; j++) {
				
						if (charArr[i] && ((charArr[i] >> j) & 1)) {
					
							this->PutPixel(x+(i*2),   y+(j*2),   color);
							this->PutPixel(x+(i*2),   y+(j*2)-1, color);
							this->PutPixel(x+(i*2)-1, y+(j*2),   color);
							this->PutPixel(x+(i*2)-1, y+(j*2)-1, color);
						}
					}
				}
				x += FONT_WIDTH;
				break;

			default:
				for (uint16_t w = 0; w < FONT_WIDTH; w++) {
					for (uint16_t h = 0; h < FONT_HEIGHT; h++) {
			
						if (charArr[w] && ((charArr[w] >> h) & 1)) {
				
							this->PutPixel(x+w, y+h, color);
						}
					}
				}

				if (flags == TEXT_BOLD) {
				
					for (uint16_t w = 0; w < FONT_WIDTH; w++) {
						for (uint16_t h = 0; h < FONT_HEIGHT; h++) {
			
							if (charArr[w] && ((charArr[w] >> h) & 1)) {
				
								this->PutPixel(x+w+1, y+h, color);
							}
						}
					}
				}
				
				if (flags == TEXT_UNDERLINE) {
				
					for (uint16_t i = 0; i < FONT_WIDTH; i++) {
						
						this->PutPixel(x+i, y+7, color);
					}
				}
				break;
		}
		x += FONT_WIDTH;
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
	
	this->ID = -1;
	this->windowOffset = 0;

	this->windowBuffer = (uint8_t*)Widget::gc->mm->malloc(sizeof(uint8_t)*Widget::gc->gfxBufferSize);
	this->buf = this->windowBuffer;

	//orignial resolution	
	this->xo = x;
	this->yo = y;
	this->wo = w;
	this->ho = h;

	this->currentTextWidth = w / FONT_WIDTH;
	this->currentTextHeight = h / FONT_HEIGHT;

	/*
	for (int i = 0; i < TEXT_BUF_SIZE; i++) {
	
		this->textBuffer[i] = '\0';
	}
	this->textBufferIndex = 0;
	*/

	this->name = name;
	this->color = color;

	this->Fullscreen = false;
	this->Focussable = window;
	this->Buttons = window;
	this->Resizable = window;
	this->MenuOpen = false;

	this->Wave = false;
	this->Blur = false;
	this->Fire = false;
	this->Rainbow = false;
	this->Pixelize = false;

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

		this->Resizable = false;
		this->Dragging = false;

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

	if (numChildren >= 30) { return false; }
	children[numChildren++] = child;

	return true;
}


bool CompositeWidget::DeleteChild() {

	if (numChildren <= 0) {
		
		return false;

	} else if (numChildren > 1) {
	
		for (int i = 0; i < numChildren-1; i++) {
		
			children[i] = children[i+1];
			children[i]->ID = i;
		}
	} else {
		children[0] = 0; 
		children[0]->ID = 0;
	}
	numChildren--;

	return true;
}


bool CompositeWidget::Maximize() {

	if (	children[0]->w >= Widget::gc->gfxWidth && 
		children[0]->h >= Widget::gc->gfxHeight &&
		children[0]->x > 0    &&
		children[0]->y > 0) {
	
		return false;
	}
	
	children[0]->w = Widget::gc->gfxWidth;
	children[0]->h = Widget::gc->gfxHeight;
	children[0]->x = 0;
	children[0]->y = 0;
	
	children[0]->currentTextWidth = TEXT_MAX_WIDTH;
	children[0]->currentTextHeight = TEXT_MAX_HEIGHT;

	return true;
}

bool CompositeWidget::Minimize() {
	
	CompositeWidget* widget = children[0];

	widget->xo = widget->x;
	widget->yo = widget->y;
	widget->wo = widget->w;
	widget->ho = widget->h;
	
	widget->Min = true;
	widget->x = Widget::gc->gfxWidth;
	widget->y = Widget::gc->gfxHeight;
	widget->parent->minWindows++;
	focussedChild = 0;

	return true;
}




bool CompositeWidget::MenuButton() {return true;}


void CompositeWidget::Resize(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {

	//children[0]->w += (oldx - newx);
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


void CompositeWidget::LoadCursor(uint8_t* buf, uint16_t w, uint16_t h) {
}


void CompositeWidget::UpdateName(char* name) {
}


void CompositeWidget::WindowMenuDraw(GraphicsContext* gc) {}


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

	if (x < Widget::gc->gfxWidth && y < Widget::gc->gfxHeight) { this->buf[Widget::gc->gfxWidth*y+x] = color; }
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
	
	struct math::point pointArr[Widget::gc->gfxWidth];
	uint16_t pixelNum = LineFillArray(x0, y0, x1, y1, pointArr);

	int32_t index = 0;

	for (int i = 0; i < pixelNum; i++) {
			
		index = (Widget::gc->gfxWidth*(pointArr[i].y - this->y)+(pointArr[i].x - this->x));

		/*
		if (pointArr[i].x < this->x+w && pointArr[i].y < this->y+h
		 && pointArr[i].x >= this->x && pointArr[i].y >= this->y
		 && index < Widget::gc->gfxBufferSize && index >= 0) {
		*/
		if (pointArr[i].x >= this->x && pointArr[i].y >= this->y && index < Widget::gc->gfxBufferSize && index >= 0) {
		
			this->buf[Widget::gc->gfxWidth*(pointArr[i].y - this->y)+(pointArr[i].x - this->x)] = color;
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
	uint16_t scrollVert = 0;
	bool scroll = false;

	if (y < 0) {
	
		if (y+h < 0) { return; }
		scrollVert = (y*-1);
		h -= (y*-1);
		y = 0;
		scroll = true;
	}

	for (uint16_t Y = y; Y < y+h; Y++) {
		for (uint16_t X = x; X < x+w; X++) {
		
			pixelColor = newbuf[w*(Y-y)+(X-x)];

			//if (pixelColor == W_EMPTY) { pixelColor = W000000; }
			this->buf[Widget::gc->gfxWidth*Y+X] = pixelColor;
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
			if (button == LEFT_CLICK) {
			
				uint8_t buttonAction = focussedChild->ContainsCoordinateButton(x, y);

				switch (buttonAction) {

					case 1:
						//if (prng() % 1 == 0) { this->Fire = true; } 
						//else { DeleteChild(); }
						DeleteChild();
						break;
					case 2:
						Maximize();
						break;
					case 3:
						Minimize();
						break;
					case 4:
						children[0]->MenuOpen ^= 1;
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


void CompositeWidget::RenderChar(uint8_t* charArr, uint8_t pixelColor, uint8_t flags) {


	switch (flags) {

		case TEXT_HEADER:
		
			for (uint16_t i = 0; i < FONT_WIDTH; i++) {
				for (uint16_t j = 0; j < FONT_HEIGHT; j++) {
				
					if (charArr[i] && ((charArr[i] >> j) & 1)) {
					
						this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+(j*2))+((outx*FONT_WIDTH)+(i*2))] = pixelColor;
						this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+(j*2)-1)+((outx*FONT_WIDTH)+(i*2))] = pixelColor;
						this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+(j*2))+((outx*FONT_WIDTH)+(i*2)-1)] = pixelColor;
						this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+(j*2)-1)+((outx*FONT_WIDTH)+(i*2-1))] = pixelColor;
					}
				}
			}
			outx++;
			break;

		case TEXT_ITALIC:

			for (uint16_t i = 0; i < FONT_WIDTH; i++) {
				for (uint16_t j = 0; j < FONT_HEIGHT; j++) {
		
					if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
						this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+j)+((outx*FONT_WIDTH)+i+(FONT_HEIGHT-j)/2)] = pixelColor;
					}
				}
			}
			break;
	
		case TEXT_BASIC:
		default:
			uint16_t outxSave = this->outx;
			uint16_t outySave = this->outy;
			uint8_t oldColor = pixelColor;
			
			//if bold just draw twice at slightly different x's
			if (flags == TEXT_BOLD) {
				
				//pixelColor = os::drivers::light2dark[pixelColor];
			
				for (uint16_t i = 0; i < FONT_WIDTH; i++) {
					for (uint16_t j = 0; j < FONT_HEIGHT; j++) {
		
						if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
							this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+j)+((outx*FONT_WIDTH)+i+1)] = pixelColor;
						}
					}
				}
				//pixelColor = oldColor;
			}
			
			//draw letter
			for (uint16_t i = 0; i < FONT_WIDTH; i++) {
				for (uint16_t j = 0; j < FONT_HEIGHT; j++) {
		
					if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
						this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+j)+((outx*FONT_WIDTH)+i)] = pixelColor;
					}
				}
			}


			if (flags == TEXT_UNDERLINE) {
			
				outx = outxSave;
				outy = outySave;
			
				for (uint16_t i = 0; i < FONT_WIDTH; i++) {
						
					this->buf[Widget::gc->gfxWidth*((outy*FONT_HEIGHT)+7)+((outx*FONT_WIDTH)+i)] = pixelColor;
				}
			}
		break;
	
	}
}



void CompositeWidget::PutChar(char ch, uint8_t flags) {

	uint8_t* charArr = charset[0];
	uint8_t pixelColor = textColor;
	
	uint16_t adjustedTextHeight = this->currentTextHeight - (1 * this->Fullscreen == false);

	//load font char
	if (ch >= 32) { charArr = charset[ch];
	} else { charArr = font_full; }

	switch (ch) {

		case '\b':
			if (outx > 1) {
				outx--;
				outx += (1 * (flags == TEXT_HEADER));
			} else {
				outy--;
				outy += (1 * (flags == TEXT_HEADER));
				outx = currentTextWidth - 1;
			}
			pixelColor = this->color;
			break;
		case '\n':
			outx = 0;
			outy++;
			outy += (1 * (flags == TEXT_HEADER));
			break;
		defualt:
			break;
	}

	//newline text after full line
	if (outx+1 >= this->currentTextWidth) {
	
		outx = 0;
		outy++;
		outy += (1 * (flags == TEXT_HEADER));
	}



	//scrolling
	if (outy >= adjustedTextHeight) {
	
		for (uint16_t y = 0; y < Widget::gc->gfxHeight; y++) {
			for (uint16_t x = 0; x < Widget::gc->gfxWidth; x++) {
	
				if (y < (Widget::gc->gfxHeight-11)-(10*(flags == TEXT_HEADER))) {
					
						this->buf[Widget::gc->gfxWidth*y+x] = this->buf[Widget::gc->gfxWidth*(y+FONT_HEIGHT+(FONT_HEIGHT*(flags == TEXT_HEADER)))+x];
				} else { 	this->buf[Widget::gc->gfxWidth*y+x] = this->color; }
			}
		}
		outx = 0;
		outy--;
		outy -= (1 * (flags == TEXT_HEADER));

		this->textScroll = true;
	} else {this->textScroll = false; }

	if (ch == '\n') { return; }


	//font is 8x5 monospace
	//write char to buffer
	this->RenderChar(charArr, pixelColor, flags);
	
	/*
	for (uint16_t i = 0; i < font_width; i++) {
		for (uint8_t j = 0; j < font_height; j++) {
		
			if (charArr[i] && ((charArr[i] >> j) & 1)) {
			
				this->buf[Widget::gc->gfxWidth*((outy*9)+j)+((outx*6)+i)] = pixelColor;
			}
		}
	}
	*/
	if (ch != '\b') { outx++; }
}


void CompositeWidget::Print(char* str, uint8_t flags) {
	
	//better if this method doesn't have to call
	//putchar for each char in string
	uint8_t* charArr = charset[0];
	//uint8_t* charArr = charset[0];
	uint8_t pixelColor = textColor;

	uint16_t adjustedTextHeight = this->currentTextHeight - (1 * this->Fullscreen == false);
	
	if (outx+strlen(str) >= this->currentTextWidth && 
		strlen(str) < this->currentTextWidth) {

		this->PutChar('\n', flags);
	}

	//print string
	for (uint8_t ch = 0; str[ch] != '\0'; ch++) {	

		switch (str[ch]) {
		
			case '\v':
				//clear text
				for (uint32_t i = 0; i < Widget::gc->gfxBufferSize; i++) {
					
					this->buf[i] = this->color;
				}
				outx = 0;
				outy = 0;
				
				break;
			case '\n':
				outx = 0;
				outy++;
				outy += (1 * (flags == TEXT_HEADER));
				break;
			default:
				charArr = charset[(uint8_t)str[ch]];
				
				if (outx >= this->currentTextWidth) { 
					
					outx = 0; 
					outy++; 
					outy += (1 * (flags == TEXT_HEADER));
				}
				
				//scrolling
				if (outy >= adjustedTextHeight) {
	
					for (uint16_t y = 0; y < Widget::gc->gfxHeight; y++) {
						for (uint16_t x = 0; x < Widget::gc->gfxWidth; x++) {
	
							if (y < Widget::gc->gfxHeight-11) {  
									this->buf[Widget::gc->gfxWidth*y+x] = this->buf[Widget::gc->gfxWidth*(y+FONT_HEIGHT)+x];
							} else {	this->buf[Widget::gc->gfxWidth*y+x] = this->color; }
						}
					}
					outx = 0;
					outy--;
					outy -= (1 * (flags == TEXT_HEADER));

					this->textScroll = true;
				} else {this->textScroll = false; }
			

				//this->RenderChar(charArr, pixelColor, 1);
				this->RenderChar(charArr, pixelColor, flags);

				if (outy < adjustedTextHeight) { outx++; }
				
				break;
		}
	}
}



void CompositeWidget::PrintTextBuffer() {

	/*
	this->Print("\v");

	for (int i = 0; i < this->textBufferIndex; i++) {
	
		this->PutChar(this->textBuffer[i]);
	}
	*/
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
