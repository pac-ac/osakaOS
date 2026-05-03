#include <drivers/vga.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::drivers;
using namespace os::math;


uint8_t Web2VGA(uint32_t webColor);
void sleep(uint32_t);


VideoGraphicsArray::VideoGraphicsArray(MemoryManager* mm, uint8_t* vbeAddr, uint32_t width, uint32_t height) :
	
	miscPort(0x3c2),
	crtcIndexPort(0x3d4),
	crtcDataPort(0x3d5),
	sequencerIndexPort(0x3c4),
	sequencerDataPort(0x3c5),
	
	colorPaletteMask(0x3c6),
	colorRegisterRead(0x3c7),
	colorRegisterWrite(0x3c8),
	colorDataPort(0x3c9),
	
	graphicsControllerIndexPort(0x3ce),
	graphicsControllerDataPort(0x3cf),
	attributeControllerIndexPort(0x3c0),
	attributeControllerReadPort(0x3c1),
	attributeControllerWritePort(0x3c0),
	attributeControllerResetPort(0x3da) {

	this->gfxWidth = width;
	this->gfxHeight = height;
	this->gfxBufferSize = width*height;
	this->pixels = (uint8_t*)mm->malloc(sizeof(uint8_t)*(this->gfxBufferSize));
	this->mm = mm;

	if (vbeAddr != nullptr) {
	
		this->FrameBufferSegment = vbeAddr;
		this->vesa = 1;
	}
}


VideoGraphicsArray::~VideoGraphicsArray() {
}



void VideoGraphicsArray::WriteRegisters(uint8_t* registers) {

	//misc
	miscPort.Write(*(registers++));
	
	//sequencer
	for (uint8_t i = 0; i < 5; i++) {
	
		sequencerIndexPort.Write(i);
		sequencerDataPort.Write(*(registers++));
	}


	//cathode ray tube controller
	crtcIndexPort.Write(0x03);
	crtcDataPort.Write(crtcDataPort.Read() | 0x80);
	crtcIndexPort.Write(0x11);
	crtcDataPort.Write(crtcDataPort.Read() & ~0x80);
	

	registers[0x03] = registers[0x03] | 0x80;
	registers[0x11] = registers[0x11] & ~0x80;

	for (uint8_t i = 0; i < 25; i++) {
	
		crtcIndexPort.Write(i);
		crtcDataPort.Write(*(registers++));
		
	}
	
	//graphics controller
	for (uint8_t i = 0; i < 9; i++) {
	
		graphicsControllerIndexPort.Write(i);
		graphicsControllerDataPort.Write(*(registers++));
		
	}

	//attribute controller
	for (uint8_t i = 0; i < 21; i++) {
	
		attributeControllerResetPort.Read();
		attributeControllerIndexPort.Write(i);
		attributeControllerWritePort.Write(*(registers++));
	}

	attributeControllerResetPort.Read();
	attributeControllerIndexPort.Write(0x20);
}


bool VideoGraphicsArray::SupportsMode(uint32_t width, uint32_t height, uint32_t colordepth) {

	return width == WIDTH_13H && height == HEIGHT_13H && colordepth == 8;
}


bool VideoGraphicsArray::SetMode(uint32_t width, uint32_t height, uint32_t colordepth) {

	if (SupportsMode(width, height, colordepth) == false) {
	
		return false;
	}
	

	unsigned char g_gfxWidthxgfxHeightx256[] = {
	
	/* misc */
		0x63,
	/* seq */
		0x03, 0x01, 0x0f, 0x00, 0x0e,
	/* crtc */
		0x5f, 0x4f, 0x50, 0x82, 0x54, 0x80, 0xbf, 0x1f,
		0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x9c, 0x0e, 0x8f, 0x28, 0x40, 0x96, 0xb9, 0xa3,
		0xff,
	/* gc */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0f,
		0xff,
	/* ac */
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x41, 0x00, 0x0f, 0x00, 0x00
	};

	WriteRegisters(g_gfxWidthxgfxHeightx256);
	this->FrameBufferSegment = GetFrameBufferSegment();

	//pallete init
	this->colorPaletteMask.Write(0xff);
	this->colorRegisterWrite.Write(0);
	
	for (uint16_t color = 0; color < 256; color++) {
	
		this->colorDataPort.Write(((defaultPalette[color] >> 16) & 0xff) >> 2);
		this->colorDataPort.Write(((defaultPalette[color] >> 8) & 0xff) >> 2);
		this->colorDataPort.Write((defaultPalette[color] & 0xff) >> 2);
	}
	return true;
}

void VideoGraphicsArray::PaletteUpdate(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {

	this->colorRegisterWrite.Write(index);
	
	//color is in 18 bits with 6 bits for each channel
	this->colorDataPort.Write(r); //Red
	this->colorDataPort.Write(g); //Green
	this->colorDataPort.Write(b); //Blue
}


uint8_t* VideoGraphicsArray::GetFrameBufferSegment() {

	graphicsControllerIndexPort.Write(0x06);
	uint8_t segmentNumber = graphicsControllerDataPort.Read() & (3 << 2);

	switch (segmentNumber) {
	
		default:
		case 0 << 2: return (uint8_t*)0x00000;
		case 1 << 2: return (uint8_t*)0xa0000;
		case 2 << 2: return (uint8_t*)0xb0000;
		case 3 << 2: return (uint8_t*)0xb8000;
	}
}



//place in backbuffer
void VideoGraphicsArray::PutPixel(int32_t x, int32_t y, uint8_t color) {

	if (x >= 0 && gfxWidth > x && y >= 0 && gfxHeight > y && color != 0x00) {
		
		pixels[(y<<(8+vesa))+(y<<(6+vesa))+x] = color;
		return;
	}

	if (color == 0x00) {
		
		uint32_t index = (y<<(8+vesa))+(y<<(6+vesa))+x;

		switch (index%3) {
		
			case 0: pixels[index] = 0x14; break;
			case 1: pixels[index] = 0x18; break;
			case 2: pixels[index] = 0x1c; break;
		}
		
	}
}



//draw directly to vmem
void VideoGraphicsArray::PutPixelRaw(int32_t x, int32_t y, uint8_t colorIndex) {

	if (x >= 0 && gfxWidth > x && y >= 0 && gfxHeight > y) {

		uint8_t* pixelAddress = this->FrameBufferSegment+((y<<(8+vesa)) + (y<<(6+vesa)) + x);
		*pixelAddress = colorIndex;
	}
}



//write transparent pixels
void VideoGraphicsArray::AlphaWrite(int32_t x, int32_t y, uint8_t color, float alpha) {

	uint32_t finalColor = ((uint32_t)((double)defaultPalette[color]*(double)alpha)) 
				+ (uint32_t)((double)defaultPalette[this->pixels[(y<<(8+vesa))+(y<<(6+vesa))+x]] * (double)(1.0 - alpha));
	
	this->pixels[(y<<(8+vesa))+(y<<(6+vesa))+x] = Web2VGA(finalColor);
}


//darken pixel in buffer
void VideoGraphicsArray::DarkenPixel(int32_t x, int32_t y) {

	if (x >= 0 && gfxWidth > x && y >= 0 && gfxHeight > y) {
	
		pixels[(y<<(8+vesa))+(y<<(6+vesa))+x] = light2dark[pixels[(y<<(8+vesa))+(y<<(6+vesa))+x]];
	}
}


//read from backbuffer
uint8_t VideoGraphicsArray::ReadPixel(int32_t x, int32_t y) {

	if (x < 0 || gfxWidth <= x || y < 0 || gfxHeight <= y) { return 0; }
	return pixels[gfxWidth*y+x];
}


void VideoGraphicsArray::PutText(char* str, int32_t x, int32_t y, uint8_t color, uint8_t flags) {

	uint16_t length = strlen(str);

	if ((gfxWidth - x) < (length * 5)) { return; }

	uint8_t* charArr = charset[0];

	for (int i = 0; str[i] != '\0'; i++) {
	
		charArr = charset[(uint8_t)(str[i])];

		switch (flags) {

			case TEXT_ITALIC:
				for (uint16_t w = 0; w < FONT_WIDTH; w++) {
					for (uint16_t h = 0; h < FONT_HEIGHT; h++) {
			
						if (charArr[w] && ((charArr[w] >> h) & 1)) {
				
							this->PutPixel(x+w+((FONT_HEIGHT-h)/2), y+h, color);
						}
					}
				}
				x += FONT_WIDTH;
				break;
		
			case TEXT_HEADER:
				for (uint16_t w = 0; w < FONT_WIDTH; w++) {
					for (uint16_t h = 0; h < FONT_HEIGHT; h++) {
			
						if (charArr[w] && ((charArr[w] >> h) & 1)) {
				
							this->PutPixel(x+(w*2), y+(h*2), color);
							this->PutPixel(x+(w*2)+1, y+(h*2), color);
							this->PutPixel(x+(w*2), y+(h*2)+1, color);
							this->PutPixel(x+(w*2)+1, y+(h*2)+1, color);
						}
					}
				}
				x += (FONT_WIDTH*2);
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
				
					for (uint16_t w = 0; w < FONT_WIDTH; w++) {
					
						this->PutPixel(x+w, y+FONT_HEIGHT+1, color);
					}
				}


				x += FONT_WIDTH;
				break;
		}
	}
}

void VideoGraphicsArray::PrintData(char* str, uint16_t length, int32_t x, int32_t y, uint8_t color) {

	uint8_t* charArr = charset[0];
	
	for (int i = 0; i < length; i++) {
			
		if (str[i] == '\n' || x+FONT_WIDTH > gfxWidth) {
		
			x = 0;
			y += FONT_HEIGHT;
		}
		if ((gfxHeight - y) < FONT_HEIGHT) { return; }

		charArr = charset[(uint8_t)str[i]];
		
		for (uint16_t w = 0; w < FONT_WIDTH; w++) {
			for (uint16_t h = 0; h < FONT_HEIGHT; h++) {
			
				if (charArr[w] && ((charArr[w] >> h) & 1)) {
					
					this->PutPixel(x+w, y+h, color);
				}
			}
		}
		x += FONT_WIDTH;
	}
}


//draw from buffer
void VideoGraphicsArray::FillBufferFull(int32_t x, int32_t y, 
					int32_t w, int32_t h, uint8_t* buf) {

	for (int32_t Y = y; Y < y+h; Y++) {
		for (int32_t X = x; X < x+w; X++) {
		
			this->PutPixel(X, Y, buf[gfxWidth*(Y-y)+(X-x)]);
		}
	}
}


void VideoGraphicsArray::FillBuffer(int16_t x, int16_t y, 
				int16_t w, int16_t h, uint8_t* buf, bool mirror) {

	uint8_t pixelColor = 0;
	uint16_t scrollVert = 0;
	bool scroll = false;

	if (y < 0) {
		
		if (y+h < 0) { return; }
		scrollVert = (y * -1);
		h -= (y * -1);
		y = 0;
		scroll = true;
	}

	for (int16_t Y = y; Y < y+h; Y++) {
		for (int16_t X = x; X < x+w; X++) {
	
			if (mirror) { pixelColor = buf[w*(Y-y+scrollVert)+(x+w-X-1)];
			} else {      pixelColor = buf[w*(Y-y+scrollVert)+(X-x)]; }
			
			if (pixelColor) {
			
				this->PutPixel(X, Y, pixelColor);
			}
		}
	}
}


void VideoGraphicsArray::FillBufferCoordinate(int16_t x, int16_t y, 
						int16_t w, int16_t h, 
						uint8_t* buf, bool mirror,
						uint8_t* positionBuf, uint8_t positionVal) {
	uint8_t pixelColor = 0;
	uint16_t scrollVert = 0;
	bool scroll = false;

	if (y < 0) {

		if (y+h < 0) { return; }
		scrollVert = (y * -1);
		h -= (y * -1);
		y = 0;
		scroll = true;
	}

	for (int16_t Y = y; Y < y+h; Y++) {
		for (int16_t X = x; X < x+w; X++) {
	
			if (mirror) { pixelColor = buf[w*(Y-y+scrollVert)+(x+w-X-1)];
			} else {      pixelColor = buf[w*(Y-y+scrollVert)+(X-x)]; }
			
			if (pixelColor) {
			
				positionBuf[(Y<<(8+vesa))+(Y<<(6+vesa))+X] = positionVal;
			}
		}
	}

}



void VideoGraphicsArray::FillRectangle(int32_t x, int32_t y, 
		int32_t w, int32_t h, uint8_t color) {

	for (int32_t Y = y; Y < y+h; Y++) {
		for (int32_t X = x; X < x+w; X++) {
			
			this->PutPixel(X, Y, color);
		}
	}	
}



void VideoGraphicsArray::DrawRectangle(int32_t x, int32_t y, 
		int32_t w, int32_t h, uint8_t color) {
		
	for (int32_t X = x; X < x+w; X++) {
			
		this->PutPixel(X, y,     color);
		this->PutPixel(X, y+h-1, color);
	}

	for (int32_t Y = y; Y < y+h; Y++) {
			
		this->PutPixel(x,     Y, color);
		this->PutPixel(x+w-1, Y, color);
	}	
}

void VideoGraphicsArray::DrawLineFlat(int32_t x0, int32_t y0, 
				int32_t x1, int32_t y1,
				uint8_t color,
				bool x) {
	if (x) {
		for (int32_t X = x0; X < x1; X++) {
			
			this->PutPixel(X, y0, color);
		}
	} else {
		for (int32_t Y = y0; Y < y1; Y++) {
			
			this->PutPixel(x0, Y, color);
		}
	}
}

void VideoGraphicsArray::DrawLineLow(int32_t x0, int32_t y0, 
				int32_t x1, int32_t y1,
				uint8_t color) {
	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int16_t yi = 1;
	
	if (dy < 0) {
	
		yi = -1;
		dy = -dy;
	}
	int16_t D = (2*dy) - dx;
	int16_t y = y0;

	for (int x = x0; x < x1; x++) {
	
		this->PutPixel(x, y, color);
	
		if (D > 0) {
			y += yi;
			D += (2*(dy-dx));
		} else {
			D += 2*dy;
		}
	}
}


void VideoGraphicsArray::DrawLineHigh(int32_t x0, int32_t y0, 
				int32_t x1, int32_t y1,
				uint8_t color) {
	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int16_t xi = 1;
	
	if (dx < 0) {
	
		xi = -1;
		dx = -dx;
	}
	int16_t D = (2*dx) - dy;
	int16_t x = x0;

	for (int y = y0; y < y1; y++) {
	
		this->PutPixel(x, y, color);
	
		if (D > 0) {
			x += xi;
			D += (2*(dx-dy));
		} else {
			D += 2*dx;
		}
	}
}


void VideoGraphicsArray::DrawLine(int32_t x0, int32_t y0, 
				int32_t x1, int32_t y1,
				uint8_t color) {
	
	if (abs(y1 - y0) < abs(x1 - x0)) {
	
		if (x0 > x1) {  DrawLineLow(x1, y1, x0, y0, color);
		} else {	DrawLineLow(x0, y0, x1, y1, color); }
	} else {

		if (y0 > y1) {  DrawLineHigh(x1, y1, x0, y0, color);
		} else {	DrawLineHigh(x0, y0, x1, y1, color); }
	}
}


void VideoGraphicsArray::FillTriangle(int16_t x0, int16_t y0, 
				     int16_t x1, int16_t y1, 
				     int16_t x2, int16_t y2,
				     uint8_t color) {
	int16_t tmpx = 0;
	int16_t tmpy = 0;
	
	if (y1 < y0) { tmpx = x0; tmpy = y0; x0 = x1;   y0 = y1; x1 = tmpx; y1 = tmpy; }
	if (y2 < y0) { tmpx = x0; tmpy = y0; x0 = x2;   y0 = y2; x2 = tmpx; y2 = tmpy; }
	if (y2 < y1) { tmpx = x1; tmpy = y1; x1 = x2;   y1 = y2; x2 = tmpx; y2 = tmpy; }

	if (y0 == y2) { return; }

	int16_t mx = x0 + (int16_t)(((float)((x2 - x0) * (y1 - y0))) / ((float)(y2 - y0)));
	int16_t my = y1;

	if (y0 != y1) {	

		float invSlopeb0 = ((float)(x1 - x0)) / ((float)(y1 - y0));
		float invSlopeb1 = ((float)(mx - x0)) / ((float)(my - y0));
		float startxb = x0;
		float endxb = x0;

		for (int y = y0; y <= my; y++) {
			
			this->DrawLine((int)startxb, y, (int)endxb, y, color);
			//this->DrawLineFlat((int)startxb, y, (int)endxb, y, color, true);
			startxb += invSlopeb0;
			endxb += invSlopeb1;
		}
	}


	if (y1 != y2) {	
	
		float invSlopet0 = ((float)(x2 - x1)) / ((float)(y2 - y1));
		float invSlopet1 = ((float)(x2 - mx)) / ((float)(y2 - my));
		float startxt = x2;
		float endxt = x2;

		for (int y = y2; y >= my; y--) {
			
			this->DrawLine((int)startxt, y, (int)endxt, y, color);
			//this->DrawLineFlat((int)startxt, y, (int)endxt, y, color, true);
			startxt -= invSlopet0;
			endxt -= invSlopet1;
		}
	}
}

void VideoGraphicsArray::FillPolygon(uint16_t x[], uint16_t y[], 
				     uint8_t edgeNum, 
				     uint8_t color) {
	int16_t i, j, temp = 0;
	uint16_t xmin = gfxWidth; 
	uint16_t xmax = 0;

	for (i = 0; i < edgeNum; i++) {
	
		if (x[i] < xmin) { xmin = x[i]; }
		if (x[i] > xmax) { xmax = x[i]; }
	}

	for (i = xmin; i <= xmax; i++) {
	
		uint16_t interPoints[edgeNum];
		uint16_t count = 0;

		for (j = 0; j < edgeNum; j++) {
		
			uint16_t next = (j + 1) % edgeNum;

			if ((y[j] > i && y[next] <= i) || (y[next] > i && y[j] <= i)) {
			
				interPoints[count++] = x[j] + (i - y[j]) 
					* (x[next] - x[j]) / (y[next] - y[j]);
			}
		}	

		for (j = 0; j < count-1; j++) {
			for (int k = 0; k < count-j-1; k++) {
			
				if (interPoints[k] > interPoints[k+1]) {
				
					temp = interPoints[k];
					interPoints[k] = interPoints[k+1];
					interPoints[k+1] = temp;
				}
			}
		}

		//draw line
		for (j = 0; j < count; j += 2) {

			this->DrawLine(interPoints[j], i, interPoints[j+1], i, color);
		}
	}
}



common::int16_t VideoGraphicsArray::ProjectVertexX(float v[3]) {

	//viewport w and h (FOV)
	float Vw = 1.0;

	//camera distance
	float d = 2.0;
	float px = ((v[0] * d) / v[2]);
	//return px + (gfxWidth / 2);
	return (px + 1) * 0.5 * gfxWidth;
	//return (px * gfxWidth / (uint16_t)Vw);
}


common::int16_t VideoGraphicsArray::ProjectVertexY(float v[3]) {
	
	//viewport w and h (FOV)
	float Vh = 1.0;

	//camera distance
	float d = 2.0;
	float py = ((v[1] * d) / v[2]);
	//return -py + (gfxHeight / 2);
	return (1 - (py + 1)) * 0.5 * gfxHeight;
	//return (py * gfxHeight / (uint16_t)Vh);
}



void VideoGraphicsArray::RotateVertex(float v[3], float degrees, uint8_t varConst) {

	float newx, newy, newz;

	switch (varConst) {
		
		case 0:
			newy = cos(degrees)*v[1] - sin(degrees)*v[2];
			newz = sin(degrees)*v[1] + cos(degrees)*v[2];
			v[1] = newy;
			v[2] = newz;
			break;
		case 1:
			newx = cos(degrees)*v[0] + sin(degrees)*v[2];
			newz = -sin(degrees)*v[0] + cos(degrees)*v[2];
			v[0] = newx;
			v[2] = newz;
			break;
		case 2:
			newx = cos(degrees)*v[0] - sin(degrees)*v[1];
			newy = sin(degrees)*v[0] + cos(degrees)*v[1];
			v[0] = newx;
			v[1] = newy;
			break;
	}
}



void VideoGraphicsArray::RenderObject(float** vertices, uint16_t numOfVertices, 
					uint8_t** triangles, uint16_t numOfTriangles) {

	//float translate[3] = { -1.5, 0, 7 };
	float translate[3] = { 0, -2.0, 6 };
	int16_t projected[numOfVertices][2];

	for (int i = 0; i < numOfVertices; i++) {

		vertices[i][0] += translate[0];
		vertices[i][1] += translate[1];
		vertices[i][2] += translate[2];
		
		projected[i][0] = ProjectVertexX(vertices[i]);
		projected[i][1] = ProjectVertexY(vertices[i]);
	}
	for (int i = 0; i < numOfTriangles; i++) {
	
		uint8_t* triangle = triangles[i];

		int16_t x0 = projected[(triangle[0])][0];
		int16_t y0 = projected[(triangle[0])][1];
		int16_t x1 = projected[(triangle[1])][0];
		int16_t y1 = projected[(triangle[1])][1];
		int16_t x2 = projected[(triangle[2])][0];
		int16_t y2 = projected[(triangle[2])][1];
		uint8_t color = triangle[3];

		this->FillTriangle(x0, y0, x1, y1, x2, y2, color);
		this->PutText(int2str(x0), 0, 20*i, WFFFFFF);
	}
}



void VideoGraphicsArray::MakeDark(uint8_t darkness, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

	if (darkness > 0) {

		for (int Y = y; Y < y+h; Y++) {
			for (int X = x; X < x+w; X++) {

				//make pixel darker	
				for (uint8_t i = 0; i < darkness; i++) {
			
					uint32_t pixelIndex = (Y<<(8+vesa))+(Y<<(6+vesa))+X;	
					if (pixelIndex >= 0 && pixelIndex < this->gfxBufferSize) {
					
						pixels[pixelIndex] = light2dark[pixels[pixelIndex]];
					}
				}
			}
		}
	}
}


void VideoGraphicsArray::MakeWave(int32_t x, int32_t y, 
				  int32_t w, int32_t h, 
				  uint8_t amp, uint8_t freq, uint8_t waveInc,
				  uint8_t* background) {

	for (int i = 0; i < w; i++) {
	
		uint8_t bufStrip[h];
		int16_t indexAmp = (int16_t)(((float)amp)*sin(((float)((float)i/((float)freq) + 2.0*pi*((float)waveInc/256.0)))));

		if (indexAmp != 0) {
		
			for (int j = 0; j < h; j++) { 
			
				uint32_t pixelsIndex = ((y+j)<<(8+vesa))+((y+j)<<(6+vesa))+(x+i);

				if (pixelsIndex >= 0 && pixelsIndex < this->gfxBufferSize) {
			
					bufStrip[j] = this->pixels[pixelsIndex]; 
					this->pixels[pixelsIndex] = W_EMPTY;
				}
			}
			
			for (int j = 0; j < h; j++) {
				
				uint32_t pixelsIndex = ((y+j+indexAmp)<<(8+vesa))+((y+j+indexAmp)<<(6+vesa))+(x+i);
			
				if (pixelsIndex >= 0 && pixelsIndex < this->gfxBufferSize) {
				
					this->pixels[pixelsIndex] = bufStrip[j];
				}
			}
		}
	}

	if (background != nullptr) {
	
		for (int i = y; i < y+h; i++) { 
			for (int j = x; j < x+w; j++) {
			
				uint32_t pixelsIndex = (i<<(8+vesa))+(i<<(6+vesa))+j;
				
				if (pixelsIndex >= 0 && pixelsIndex < this->gfxBufferSize 
					&& this->pixels[pixelsIndex] == W_EMPTY) {
				
					this->pixels[pixelsIndex] = background[w*(i-y)+(j-x)];
				}
			}
		}
	}
}


void VideoGraphicsArray::Pixelize(int32_t x, int32_t y, 
				  int32_t w, int32_t h, 
				  uint8_t verticalSize, uint8_t horizontalSize) {

	uint16_t pixelSize = verticalSize*horizontalSize;

	for (int i = y; i < y+h; i += verticalSize) {
		for (int j = x; j < x+w; j += horizontalSize) {
		
			uint16_t meanR = 0;
			uint16_t meanG = 0;
			uint16_t meanB = 0;

			for (int k = 0; k < verticalSize; k++) {
				for (int l = 0; l < horizontalSize; l++) {
					
					uint32_t pixelsIndex = gfxWidth*(i+k)+(j+l);

					if (pixelsIndex >= 0 && pixelsIndex < this->gfxBufferSize) {
				
						meanR += ((defaultPalette[this->pixels[pixelsIndex]]) >> 16);
						meanG += ((defaultPalette[this->pixels[pixelsIndex]]) >> 8) & 0xff;
						meanB += ((defaultPalette[this->pixels[pixelsIndex]]) & 0xff);
					}
				}
			}

			meanR /= pixelSize;
			meanG /= pixelSize;
			meanB /= pixelSize;
			this->FillRectangle(j, i, horizontalSize, verticalSize, Web2VGA((meanR << 16) | (meanG << 8) | (meanB)));
		}
	}
}


void VideoGraphicsArray::Rainbowize(int32_t x, int32_t y, 
				  int32_t w, int32_t h) {

	for (int i = y; i < y+h; i++) {
		for (int j = x; j < x+w; j++) {
		
			this->pixels[gfxWidth*y+x]++;
		}
	}
	sleep(1);
}


void VideoGraphicsArray::Burn(int32_t x, int32_t y, 
				  int32_t w, int32_t h, bool mirror) {

	for (int i = 0; i < w/16; i++) {

		this->FillBuffer(x+(i*16), y+h-14, 16, 14, fireEffect1, mirror);
	}
}


//actually draw to the screen
void VideoGraphicsArray::DrawToScreen() {
	
	for (uint16_t y = 0; y < gfxHeight; y++) {
		for (uint16_t x = 0; x < gfxWidth; x++) {

			uint8_t* pixelAddress = this->FrameBufferSegment+((y<<(8+vesa)) + (y<<(6+vesa)) + x);
			*pixelAddress = pixels[(y<<(8+vesa))+(y<<(6+vesa))+x];
		}
	}
}



void VideoGraphicsArray::FSdither(uint32_t* buf, uint16_t w, uint16_t h) {

	uint8_t oldPixel = 0;
	uint8_t newPixel = 0;
	uint8_t quantError = 0;

	for (uint16_t y = 0; y < h; y++) {
		for (uint16_t x = 0; x < w; x++) {

			oldPixel = buf[w*y+x];
			newPixel = Web2VGA(oldPixel);
			buf[w*y+x] = newPixel;
			quantError = oldPixel - newPixel;
			buf[w*(y)+(x+1)]   += quantError*7 / 16;
			buf[w*(y+1)+(x-1)] += quantError*3 / 16;
			buf[w*(y+1)+(x)]   += quantError*5 / 16;
			buf[w*(y+1)+(x+1)] += quantError / 16;
		}
	}
}


void VideoGraphicsArray::ErrorScreen() {

	this->FillRectangle(0, 0, gfxWidth, gfxHeight, 0x09);
	this->FillRectangle(0, 29, 300, 101, 0x3f);
	
	this->PutText("Sorry, osakaOS experienced a critical error. :(", 1, 11, 0x40);
	this->PutText("Sorry, osakaOS experienced a critical error. :(", 0, 10, 0x3f);

	this->PutText("ERROR CODE: ", 0, 30, 0x09);
	this->PutText("0x61 0x72 0x65 0x20 0x79 0x6F 0x75 0x20 0x66 0x75 0x63 0x6B 0x69", 0, 40, 0x09);
	this->PutText("0x6E 0x67 0x20 0x72 0x65 0x74 0x61 0x72 0x74 0x65 0x64 0x3F 0x20", 0, 50, 0x09);
	this->PutText("0x64 0x6F 0x20 0x79 0x6F 0x75 0x20 0x73 0x65 0x72 0x69 0x6F 0x75", 0, 60, 0x09);
	this->PutText("0x73 0x6C 0x79 0x20 0x74 0x68 0x69 0x6E 0x6B 0x20 0x64 0x6F 0x69", 0, 70, 0x09);
	this->PutText("0x6E 0x67 0x20 0x74 0x68 0x61 0x74 0x20 0x73 0x68 0x69 0x74 0x20", 0, 80, 0x09);
	this->PutText("0x74 0x6F 0x20 0x79 0x6F 0x75 0x72 0x20 0x63 0x6F 0x6D 0x70 0x75", 0, 90, 0x09);
	this->PutText("0x74 0x65 0x72 0x20 0x77 0x61 0x73 0x20 0x61 0x20 0x67 0x6F 0x6F", 0, 100, 0x09);
	this->PutText("0x64 0x20 0x69 0x64 0x65 0x61 0x3F 0x20 0x6B 0x69 0x6C 0x6C 0x20", 0, 110, 0x09);
	this->PutText("0x79 0x6F 0x75 0x72 0x73 0x65 0x6C 0x66", 0, 120, 0x09);
	
	this->PutText("Sending all your data to our remote servers...", 1, 151, 0x40);
	this->PutText("Sending all your data to our remote servers...", 0, 150, 0x3f);
	this->PutText("Don't turn off your kitchen lights.", 1, 161, 0x40);
	this->PutText("Don't turn off your kitchen lights.", 0, 160, 0x3f);
	
	for (int i = 20; i < 180; i += 20) {
	
		this->FillBuffer(304, i, 13, 20, cursorClickLeft, false);
	}
}
