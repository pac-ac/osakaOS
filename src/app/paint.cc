#include <app/paint.h>

using namespace os;
using namespace os::common;
using namespace os::filesystem;
using namespace os::gui;


char* int2str(uint32_t);
void BufferSwap(uint8_t* buf1, uint8_t* buf2, uint32_t size = (320*200));



KasugaPaint::KasugaPaint() {

	this->appType = 2;
}


KasugaPaint::~KasugaPaint() {}

void KasugaPaint::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {

	if (widget->Menu) {

		this->menuX = widget->x+widget->w-51;
		this->menuY = widget->y+19-(10 * widget->Fullscreen);
		int8_t offsetFullscreen = 0;
		
		gc->FillRectangle(menuX, menuY, 50, widget->h-20+(10*widget->Fullscreen), 0x07);
	
		
		//size of brush
		gc->FillRectangle(menuX+2, menuY+2, 46, 9, 0x38);
		gc->PutText("Size:", menuX+3, menuY+3, 0x07);
		gc->PutText(int2str(this->size+1), menuX+33, menuY+3, 0x07);
		
		//width of image
		gc->FillRectangle(menuX+2, menuY+13, 46, 9, 0x38);
		gc->PutText("wid:", menuX+3, menuY+14, 0x07);
		gc->PutText(int2str(this->width), menuX+27, menuY+14, 0x07);
		
		//height of image
		gc->FillRectangle(menuX+2, menuY+24, 46, 9, 0x38);
		gc->PutText("hgt:", menuX+3, menuY+25, 0x07);
		gc->PutText(int2str(this->height), menuX+27, menuY+25, 0x07);

		//clickables
		
		//increase or decrease brush size/width/height
		gc->PutText(" + | -", menuX+3, menuY+35, 0x38);

		//different brushes/draw options
		gc->PutText(" \x81 | \x87", menuX+3, menuY+43, 0x38);
		gc->PutText(" \xa1 | \xa2", menuX+3, menuY+51, 0x38);
			
		if (this->compress) { gc->PutText("RLE on", menuX+3, menuY+165, 0x38); }
		
		switch (menuTarget) {
			
			case 0:gc->DrawRectangle(menuX+2,  menuY+2,  46, 9, 0x40);break;
			case 1:gc->DrawRectangle(menuX+2,  menuY+13, 46, 9, 0x40);break;
			case 2:gc->DrawRectangle(menuX+2,  menuY+24, 46, 9, 0x40);break;
			default:break;
		}
		
		switch (drawOption) {
			
			case 0:gc->DrawRectangle(menuX+8,  menuY+43, 7, 9, 0x40);break;
			case 1:gc->DrawRectangle(menuX+32, menuY+43, 7, 9, 0x40);break;
			case 2:gc->DrawRectangle(menuX+8,  menuY+51, 7, 9, 0x40);break;
			case 3:gc->DrawRectangle(menuX+32, menuY+51, 7, 9, 0x40);break;
			default:break;
		}
	
		//color
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 4; j++) {
			
				gc->FillRectangle(menuX+5+(j*10), menuY+61+(i*6), 10, 6, (4*i+j));
			
				if ((4*i+j) == this->paintColor) {
				
					gc->DrawRectangle(menuX+5+(j*10), menuY+61+(i*6), 10, 6, 0x38);
				}
			}
			if (menuY+73+(i*6) > widget->h) { break; }
		}
	}
}


void KasugaPaint::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {
}



void KasugaPaint::SaveOutput(char* file, CompositeWidget* widget, FileSystem* filesystem) {
	
	uint8_t tmp[64000];

	for (uint8_t y = 0; y < this->height; y++) {
		for (uint16_t x = 0; x < this->width; x++) {
		
			uint32_t index = this->width*y+x;
			tmp[index] = widget->ReadPixel(index);
		}
	}
	filesystem->Write13H(file, tmp, this->width, this->height, this->compress);
}



void KasugaPaint::ReadInput(char* file, CompositeWidget* widget, FileSystem* filesystem) {

	if (filesystem->FileIf(filesystem->GetFileSector(file)) == false) { return; }

	uint8_t tmp[64000];

	uint16_t inputw = 320;
	uint8_t inputh = 200;

	//filesystem->Read13H(file, tmp, &inputw, &inputh, false);
	uint8_t* ptr = nullptr;

	if (filesystem->GetTagFile("compressed", filesystem->GetFileSector(file), ptr)) {
	
		filesystem->Read13H(file, tmp, &inputw, &inputh, true);
	} else {
		filesystem->Read13H(file, tmp, &inputw, &inputh, false);
	}

	this->width = inputw;
	this->height = inputh;


	for (uint32_t y = 0; y < this->height; y++) {
		for (uint32_t x = 0; x < this->width; x++) {

			widget->WritePixel(x, y, tmp[this->width*y+x]);
		}
	}
}



void KasugaPaint::OnKeyDown(char ch, CompositeWidget* widget) {

	//f# keys deciding color offset	
	if (ch < 4) { colorOffset = ch * 16; }

	switch (ch) {
	
		case '\v':
			widget->Menu ^= 1;
			this->paintMenu ^= 1;
			break;
		
		//keyboard shortcuts
		//color
		case '0': paintColor = 0x0 + colorOffset; break;
		case '1': paintColor = 0x1 + colorOffset; break;
		case '2': paintColor = 0x2 + colorOffset; break;
		case '3': paintColor = 0x3 + colorOffset; break;
		case '4': paintColor = 0x4 + colorOffset; break;
		case '5': paintColor = 0x5 + colorOffset; break;
		case '6': paintColor = 0x6 + colorOffset; break;
		case '7': paintColor = 0x7 + colorOffset; break;
		case '8': paintColor = 0x8 + colorOffset; break;
		case '9': paintColor = 0x9 + colorOffset; break;	
		case 'a': paintColor = 0xa + colorOffset; break;
		case 'b': paintColor = 0xb + colorOffset; break;
		case 'c': paintColor = 0xc + colorOffset; break;
		case 'd': paintColor = 0xd + colorOffset; break;
		case 'e': paintColor = 0xe + colorOffset; break;
		case 'f': paintColor = 0xf + colorOffset; break;	  

		//adjust drawing
		case '!': drawOption = 0; break;
		case '@': drawOption = 1; break;
		case '#': drawOption = 2; break;
		case '$': drawOption = 3; break;

		//case '=': this->DrawSize(true); break;
		//case '-': this->DrawSize(false); break;
		
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

	//increase or decrease dimensions or brush size	
	if (ch == '-' || ch == '=') {
	
		switch (this->menuTarget) {
			
			case 0:this->DrawSize(ch == '=');break;
			case 1:this->Dimensions(true,  ch == '=');break;
			case 2:this->Dimensions(false, ch == '=');break;
			default:break;
		}
	}
}


void KasugaPaint::Zoom(CompositeWidget* widget, bool increase) {

	if (increase && zoomSize == 1) {
	
		//save full image
		for (uint32_t i = 0; i < 64000; i++) {
	
			this->backup[i] = widget->ReadPixel(i);
		}
	}

	if (increase) { zoomSize += 1 * (zoomSize < 10); 
	} else { zoomSize -= 1 * (zoomSize > 1); }
	
	//zoom in on different 
	//areas of image
	uint8_t menuX = 0;
	uint8_t menuY = 0;

	for (uint32_t y = 0; y < 200; y += zoomSize) {
		for (uint32_t x = 0; x < 320; x += zoomSize) {
			for (uint8_t i = 0; i < zoomSize*zoomSize; i++) {
				
				widget->PutPixel(x+(i%zoomSize), y+(i/zoomSize), this->backup[(320*(y+menuY)+(x+menuX))/zoomSize]);
			}	
		}
	}
}



void KasugaPaint::Fill(CompositeWidget* widget) {

	for (uint8_t y = 0; y < this->height; y++) {
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

	uint8_t color = widget->buf[320*y+x];
	uint16_t xo = x;
	uint8_t yo = y;
		
	for (y = yo; widget->buf[320*y+xo] != color; y++) {
		for (x = xo; widget->buf[320*y+x] != color; x++) {
	
			widget->PutPixel(x, y, paintColor);
		}
	}
}





void KasugaPaint::DrawSize(bool increase) {

	if (increase) { this->size += 1 * (size < 49);
	} else {	this->size -= 1 * (size > 0); }
}


void KasugaPaint::Dimensions(bool width, bool increase) {

	if (width) { 
		if (increase) { this->width += 1 * (this->width < 320); 
		} else { 	this->width -= 1 * (this->width > 10); }
	} else {
		if (increase) { this->height += 1 * (this->height < 200); 
		} else { 	this->height -= 1 * (this->height > 10); }
	}
}




void KasugaPaint::OnKeyUp(char ch, CompositeWidget* widget) {
}


void KasugaPaint::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {

	
	if (widget->ContainsCoordinateButton(x, y+10) == 0) {
		
		widget->Dragging = (button == 1);

	} else if (widget->Menu && y < this->menuY) {
			
		menuTarget = 0;
	
	//paint menu
	} else if (widget->Menu && x >= this->menuX && y >= this->menuY) {

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
				case 1:this->Dimensions(true,  leftOrIncrease);break;
				case 2:this->Dimensions(false, leftOrIncrease);break;
				default:break;
			}

		//choose brushes
		} else if (y < menuY+43) { drawOption = 1 - (1 * leftOrIncrease);
		} else if (y < menuY+51) { drawOption = 3 - (1 * leftOrIncrease);
		
		//choose color
		} else if (y < menuY+167) {
			
			uint8_t colorx = (x-(menuX+5))/10;
			uint8_t colory = (y-(menuY+51))/6;
			
			this->paintColor = (4*colory+colorx);

			if (!paintColor) { this->paintColor = 0x40; }
		}
	} else {
		//drawing
		widget->Dragging = false;
		this->drawing = (button == 1);
	}
}

void KasugaPaint::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
	
	if (drawing) {
		
		if (size == 0) {
			widget->PutPixel(x, y, paintColor);
		} else {
			
			switch (drawOption) {
				
				case 1:
					this->Circle(x, y, size, widget);
					break;
				case 2:
					this->LightBrush(x, y, x, y, widget);
					break;
				case 3:
					this->Bucket(x, y, widget);
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
		if (size == 0) {
					
			widget->DrawLine(oldx, oldy, newx, newy, paintColor);
		} else {
			switch (drawOption) {
			
				case 1:
					this->Circle(newx, newy, size, widget);
					break;
				case 2:
					this->LightBrush(oldx, oldy, newx, newy, widget);
					break;
				case 3:
					this->Bucket(newx, newy, widget);
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
