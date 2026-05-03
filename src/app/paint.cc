#include <app/paint.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;
using namespace os::gui;


void BufferSwap(uint8_t* buf1, uint8_t* buf2, uint32_t size = BUFFER_SIZE_13H);


KasugaPaint::KasugaPaint() {

	this->appType = APP_TYPE_KASUGAPAINT;
	for (int i = 0; i < 64; i++) { textDrawStr[i] = '\0';}
}


KasugaPaint::~KasugaPaint() {}

void KasugaPaint::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {

	App::ComputeAppState(gc, widget);

	if (!initBuffer) {

		this->backup = (uint8_t*)gc->mm->malloc(sizeof(uint8_t)*gc->gfxBufferSize);	
		this->width = gc->gfxWidth;
		this->height = gc->gfxHeight;
		initBuffer = true;
	}
	
	switch (drawOption) {
	
		//draw paint text before its set
		case DRAW_OPTION_TEXT:	
			gc->PutText(textDrawStr, 
					widget->x+textDrawX, 
					widget->y+textDrawY, 
					paintColor, this->textDrawFlags);
					//light2dark[paintColor], this->textDrawFlags);
			break;
		default:
			break;
	}
}


void KasugaPaint::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {

	this->menuX = widget->x+widget->w-51;
	this->menuY = widget->y+19-(10 * widget->Fullscreen);
	int8_t offsetFullscreen = 0;
		
	gc->FillRectangle(menuX, menuY, 50, widget->h-20+(10*widget->Fullscreen), WAAAAAA);
	
		
	//size of brush
	gc->FillRectangle(menuX+2, menuY+2, 46, 9, W555555);
	gc->PutText("Size:", menuX+3, menuY+3, WAAAAAA);
	gc->PutText(int2str(this->size+1), menuX+33, menuY+3, WAAAAAA);
		
	//width of image
	gc->FillRectangle(menuX+2, menuY+13, 46, 9, W555555);
	gc->PutText("wid:", menuX+3, menuY+14, WAAAAAA);
	gc->PutText(int2str(this->width), menuX+27, menuY+14, WAAAAAA);
		
	//height of image
	gc->FillRectangle(menuX+2, menuY+24, 46, 9, W555555);
	gc->PutText("hgt:", menuX+3, menuY+25, WAAAAAA);
	gc->PutText(int2str(this->height), menuX+27, menuY+25, WAAAAAA);

	//clickables
		
	//increase or decrease brush size/width/height
	gc->PutText(" + | -", menuX+3, menuY+35, W555555);

	//different brushes/draw options
	gc->PutText(" \xe0 | \x05", menuX+3, menuY+43, W555555);
	gc->PutText(" \xe1 | \xe2", menuX+3, menuY+51, W555555);
	
	//text options
	gc->PutText(" T | 0", menuX+3, menuY+60, W555555);
		

	if (this->compress) { gc->PutText("RLE on", menuX+3, menuY+165, W555555); }
		
	switch (menuTarget) {
			
		case 0:gc->DrawRectangle(menuX+2,  menuY+2,  46, 9, W000000);break;
		case 1:gc->DrawRectangle(menuX+2,  menuY+13, 46, 9, W000000);break;
		case 2:gc->DrawRectangle(menuX+2,  menuY+24, 46, 9, W000000);break;
		default:break;
	}
	
	//draw button select
	gc->DrawRectangle(menuX+8+(24*(drawOption % 2 == 1)),  
			  menuY+43 + (8*(drawOption/2)), 7, 9, W000000);

	//color
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			
			gc->FillRectangle(menuX+5+(j*5), menuY+70+(i*5), 5, 5, (8*i+j));
			
			if ((8*i+j) == this->paintColor) {
				
				gc->DrawRectangle(menuX+5+(j*5), menuY+70+(i*5), 5, 5, W555555);
			}
		}
		if (menuY+82+((i-1)*5) > widget->h) { break; }
	}

	/*
	gc->FillRectangle(menuX+5, menuY+70, 5, 5, 0x);
			case 0: tmp[i] = 0x14; break;
			case 1: tmp[i] = 0x18; break;
			case 2: tmp[i] = 0x1c; break;
	*/
}



void KasugaPaint::SaveOutput(char* file, CompositeWidget* widget, FileSystem* filesystem) {
	
	uint8_t tmp[widget->gc->gfxBufferSize];

	for (uint16_t y = 0; y < widget->gc->gfxHeight; y++) {
		for (uint16_t x = 0; x < widget->gc->gfxWidth; x++) {
		
			tmp[widget->gc->gfxWidth*y+x] = widget->ReadPixel(widget->gc->gfxWidth*y+x);
		}
	}
	filesystem->Write13H(file, tmp, this->width, this->height, this->compress);
}



void KasugaPaint::ReadInput(char* file, CompositeWidget* widget, FileSystem* filesystem) {

	if (filesystem->FileIf(filesystem->GetFileSector(file)) == false) { 
		
		//read special system graphics into editor
		if (strcmp(file, "OSAKA_SIM_WINTER") || strcmp(file, "OSAKA_SIM_SUMMER")) {
		
			widget->FillBuffer(0, 0, 16, 49, osakaFrontRight);
			widget->FillBuffer(16, 0, 15, 49, osakaBackRight);
			widget->FillBuffer(31, 0, 16, 49, osakaFrontRightWalk1);
			widget->FillBuffer(47, 0, 25, 49, osakaFrontRightWalk2);
			widget->FillBuffer(72, 0, 21, 49, osakaBackRightWalk1);
			widget->FillBuffer(93, 0, 21, 49, osakaBackRightWalk2);	
		
			if (strcmp(file, "OSAKA_SIM_SUMMER")) {
			
				widget->FillBuffer(0, 0, 16, 19, osakaSummerFrontRight);
				widget->FillBuffer(16, 0, 15, 19, osakaSummerBackRight);
				widget->FillBuffer(31, 0, 16, 19, osakaSummerFrontRightWalk1);
				widget->FillBuffer(47, 0, 25, 19, osakaSummerFrontRightWalk2);
				widget->FillBuffer(72, 0, 21, 19, osakaSummerBackRightWalk1);
				widget->FillBuffer(93, 0, 21, 19, osakaSummerBackRightWalk2);
			}
		}
		this->width = 114;
		this->height = 49;
	} else {
		//read image files in
		uint8_t tmp[widget->gc->gfxBufferSize];
		for (int i = 0; i < widget->gc->gfxBufferSize; i++) {
	
			switch (i % 3) {

				case 0: tmp[i] = 0x14; break;
				case 1: tmp[i] = 0x18; break;
				case 2: tmp[i] = 0x1c; break;
			}
		}

		uint16_t inputw = widget->gc->gfxWidth;
		uint16_t inputh = widget->gc->gfxHeight;

		//filesystem->Read13H(file, tmp, &inputw, &inputh, false);
		uint8_t* ptr = nullptr;

		if (filesystem->GetTagFile("compressed", filesystem->GetFileSector(file), ptr)) {
	
			filesystem->Read13H(file, tmp, &inputw, &inputh, true);
		} else {
			filesystem->Read13H(file, tmp, &inputw, &inputh, false);
		}

		this->width = inputw;
		this->height = inputh;

		for (uint32_t y = 0; y < widget->gc->gfxHeight; y++) {
			for (uint32_t x = 0; x < widget->gc->gfxWidth; x++) {
				
				widget->WritePixel(x, y, tmp[widget->gc->gfxWidth*y+x]);
			}
		}
	}
}



void KasugaPaint::OnKeyDown(char ch, CompositeWidget* widget) {

	//draw text in image
	if (drawOption == DRAW_OPTION_TEXT) {
	
		switch (ch) {

			case '\xfd':
				this->textDrawFlags++;
				this->textDrawFlags %= 5;
				break;
			case '\b':	
				if (textDrawIndex > 0) {
					textDrawIndex--;
					textDrawStr[textDrawIndex] = '\0';
				}
				break;
			case '\n':
				{
					widget->PutText(textDrawStr, 
							textDrawX, textDrawY-(10*(widget->Fullscreen == false)), 
							paintColor, this->textDrawFlags);
					for (int i = 0; i < 64; i++) { textDrawStr[i] = '\0'; }
					for (int i = 0; i < 64; i++) { textDrawStr[i] = '\0'; }
					this->textDrawX = 0;
					this->textDrawY = 0;
					this->textDrawFlags = TEXT_BASIC;
				}
				break;
			default:
				if (textDrawIndex < 64) {
					textDrawStr[textDrawIndex] = ch;
					textDrawIndex++;
					textDrawStr[textDrawIndex] = '\0';
				}
				break;
		}
		return;
	}



	switch (ch) {
	
		//adjust drawing
		case '!': drawOption = DRAW_OPTION_DEFAULT; break;
		case '@': drawOption = DRAW_OPTION_CIRCLE; break;
		case '#': drawOption = DRAW_OPTION_BRUSH; break;
		case '$': drawOption = DRAW_OPTION_BUCKET; break;
		case '%': drawOption = DRAW_OPTION_TEXT; break;
		case '^': drawOption = DRAW_OPTION_TEXT_STYLE; break;

		//adjust zoom
		case '+': this->Zoom(widget, true); break;
		case '_': this->Zoom(widget, false); break;
			
		//draw tools
		case 'F': this->Fill(widget); break;
		
		//compress image
		case 'C': this->compress ^= 1; break;
		
		default:
			break;
	}

	//change size/dimensions
	if (ch >= '0' && ch < '9') {
		
		switch (this->menuTarget) {
			
			case 0: this->size = (ch-'0')*10; break;
			case 1: this->width = (ch-'0')*10; break;
			case 2: this->height = (ch-'0')*10; break;
			default:break;
		}
	}

	//increase or decrease dimensions or brush size	
	if (ch == '-' || ch == '=') {
	
		switch (this->menuTarget) {
			
			case 0:this->DrawSize(ch == '='); break;
			case 1:this->Dimensions(true,  ch == '=', widget); break;
			case 2:this->Dimensions(false, ch == '=', widget); break;
			default:break;
		}
	}
}


void KasugaPaint::Zoom(CompositeWidget* widget, bool increase) {

	if (increase && zoomSize == 1) {
	
		//save full image
		for (uint32_t i = 0; i < widget->gc->gfxBufferSize; i++) {
	
			this->backup[i] = widget->ReadPixel(i);
		}
	}

	if (increase) { zoomSize += 1 * (zoomSize < 10); 
	} else { zoomSize -= 1 * (zoomSize > 1); }
	
	//zoom in on different 
	//areas of image
	uint8_t menuX = 0;
	uint8_t menuY = 0;

	for (uint32_t y = 0; y < widget->gc->gfxHeight; y += zoomSize) {
		for (uint32_t x = 0; x < widget->gc->gfxWidth; x += zoomSize) {
			for (uint8_t i = 0; i < zoomSize*zoomSize; i++) {
				
				widget->PutPixel(x+(i%zoomSize), y+(i/zoomSize), this->backup[(widget->gc->gfxWidth*(y+menuY)+(x+menuX))/zoomSize]);
			}	
		}
	}
}



void KasugaPaint::Fill(CompositeWidget* widget) {

	for (uint16_t y = 0; y < this->height; y++) {
		for (uint16_t x = 0; x < this->width; x++) {
			  	
			widget->PutPixel(x, y, paintColor);
		}
	}
}


void KasugaPaint::Rectangle(int32_t x0, int32_t y0, 
			int32_t x1, int32_t y1, 
			CompositeWidget* widget) {
}


void KasugaPaint::Circle(int32_t x, int32_t y, 
			int32_t r, CompositeWidget* widget) {

	widget->DrawCircle(x, y, r, paintColor);
}

void KasugaPaint::LightBrush(int32_t oldx, int32_t oldy, 
			     int32_t newx, int32_t newy, CompositeWidget* widget) {

	bool pixelOffset = false;
	
	for (int16_t h = -size; h < size; h++) {
		for (int16_t w = -size; w < size; w++) {
	
			if (w % size == pixelOffset) {
			
				widget->DrawLine(oldx+w, oldy+h, newx+w, newy+h, paintColor);
			}
		}
		pixelOffset ^= 1;
	}
}

void KasugaPaint::Bucket(int32_t x, int32_t y, CompositeWidget* widget) {

	if (widget->buf[widget->gc->gfxWidth*y+x] != paintColor 
		&& widget->buf[widget->gc->gfxWidth*y+x] == replaceColor
		&& x < widget->gc->gfxWidth && x >= 0 
		&& y < widget->gc->gfxHeight && y >= 0) {
			
		widget->PutPixel(x, y, paintColor);
		
		this->Bucket(x+1, y, widget);
		this->Bucket(x, y+1, widget);
		this->Bucket(x-1, y, widget);
		this->Bucket(x, y-1, widget);
		
		this->Bucket(x-1, y-1, widget);
		this->Bucket(x-1, y+1, widget);
		this->Bucket(x+1, y-1, widget);
		this->Bucket(x+1, y+1, widget);
	}
}





void KasugaPaint::DrawSize(bool increase) {

	if (increase) { this->size += 1 * (size < 49);
	} else {	this->size -= 1 * (size > 0); }
}


void KasugaPaint::Dimensions(bool width, bool increase, CompositeWidget* widget) {

	if (width) { 
		if (increase) { this->width += 1 * (this->width < widget->gc->gfxWidth); 
		} else { 	this->width -= 1 * (this->width > 10); }
	} else {
		if (increase) { this->height += 1 * (this->height < widget->gc->gfxHeight); 
		} else { 	this->height -= 1 * (this->height > 10); }
	}
}




void KasugaPaint::OnKeyUp(char ch, CompositeWidget* widget) {
}


void KasugaPaint::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {

	
	if (widget->ContainsCoordinateButton(x, y+10) == 0) {
		
		widget->Dragging = (button == 1);

	} else if (widget->MenuOpen && y < this->menuY) {
			
		menuTarget = 0;
	
	//paint menu
	} else if (widget->MenuOpen && x >= this->menuX && y >= this->menuY) {

		y -= (10 * widget->Fullscreen);

		//true if clicked on left object
		bool leftOrIncrease = (x < menuX+22);


		//menu target
		if (y < menuY+2) { menuTarget = 0;
		} else if (y < menuY+14) { menuTarget = 1;
		} else if (y < menuY+24) { menuTarget = 2;

		//increase decrease
		} else if (y < menuY+35) {
		
			switch (this->menuTarget) {
			
				case 0:this->DrawSize(leftOrIncrease);break;
				case 1:this->Dimensions(true,  leftOrIncrease, widget);break;
				case 2:this->Dimensions(false, leftOrIncrease, widget);break;
				default:break;
			}

		//choose brushes
		} else if (y < menuY+43) { drawOption = DRAW_OPTION_CIRCLE - (1 * leftOrIncrease);
		} else if (y < menuY+51) { drawOption = DRAW_OPTION_BUCKET - (1 * leftOrIncrease);
		} else if (y < menuY+59) { drawOption = DRAW_OPTION_TEXT_STYLE - (1 * leftOrIncrease);
		
		//choose color
		} else if (y < menuY+167) {
		
			uint8_t colorx = (x-(menuX+5))/5;
			uint8_t colory = (y-(menuY+60))/5;
			
			this->paintColor = (8*colory+colorx);

			//if (!paintColor) { this->paintColor = W000000; }

			//paint color picker window
			//widget->parent->CreateChild(1, "test", nullptr);
		}
	} else {
		//drawing
		widget->Dragging = false;
		this->drawing = (button == 1);
	}
}

void KasugaPaint::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
	
	if (drawing && x < this->width && y < this->height && x >= 0 && y >= 0) {
		
		if (size == 0 && drawOption != DRAW_OPTION_TEXT) {
			
			widget->PutPixel(x, y, paintColor);
		} else {
			switch (drawOption) {
				
				case DRAW_OPTION_CIRCLE:
					this->Circle(x, y, size, widget);
					break;
				case DRAW_OPTION_BRUSH:
					this->LightBrush(x, y, x, y, widget);
					break;
				case DRAW_OPTION_BUCKET:
					this->Bucket(x, y, widget);
					break;
				case DRAW_OPTION_TEXT:
					this->textDrawX = x;
					this->textDrawY = y;
					break;
				default:
					for (int16_t h = -size; h < size; h++) {
						for (int16_t w = -size; w < size; w++) {
					
							widget->PutPixel(x+w, y+h, paintColor);
						}
					}
					break;
			}
		}
	}
	this->drawing = false;
}


void KasugaPaint::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy, CompositeWidget* widget) {
	
	if (drawing && newx < this->width && newy < this->height 
			&& newx >= 0 && newy >= 0) {
		
		//brush mode
		if (size == 0 && drawOption != DRAW_OPTION_TEXT) {
					
			widget->DrawLine(oldx, oldy, newx, newy, paintColor);
		} else {
			switch (drawOption) {
			
				case DRAW_OPTION_CIRCLE:
					this->Circle(newx, newy, size, widget);
					break;
				case DRAW_OPTION_BRUSH:
					this->LightBrush(oldx, oldy, newx, newy, widget);
					break;
				case DRAW_OPTION_BUCKET:
					this->replaceColor = widget->buf[widget->gc->gfxWidth*newy+newx];
					if (this->paintColor != this->replaceColor) {
					
						this->Bucket(newx, newy, widget);
					}
					break;
				case DRAW_OPTION_TEXT:
					this->textDrawX = newx;
					this->textDrawY = newy;
					break;
				default:
					for (int16_t h = -size; h < size; h++) {
						for (int16_t w = -size; w < size; w++) {
	
							widget->DrawLine(oldx+w, oldy+h, newx+w, newy+h, paintColor);
						}
					}
					break;
			}
		}
	}
}
