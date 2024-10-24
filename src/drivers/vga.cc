#include <drivers/vga.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::drivers;
using namespace os::math;


uint16_t strlen(char*);
uint8_t Web2EGA(uint32_t webColor);


VideoGraphicsArray::VideoGraphicsArray() :
	
	miscPort(0x3c2),
	crtcIndexPort(0x3d4),
	crtcDataPort(0x3d5),
	sequencerIndexPort(0x3c4),
	sequencerDataPort(0x3c5),
	graphicsControllerIndexPort(0x3ce),
	graphicsControllerDataPort(0x3cf),
	attributeControllerIndexPort(0x3c0),
	attributeControllerReadPort(0x3c1),
	attributeControllerWritePort(0x3c0),
	attributeControllerResetPort(0x3da) {


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

	return width == 320 && height == 200 && colordepth == 8;
}


bool VideoGraphicsArray::SetMode(uint32_t width, uint32_t height, uint32_t colordepth) {

	if (!SupportsMode(width, height, colordepth)) {
	
		return false;
	}

	unsigned char g_320x200x256[] = {
	
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


	WriteRegisters(g_320x200x256);

	this->FrameBufferSegment = GetFrameBufferSegment();

	return true;
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

	if (x < 0 || 320 <= x || y < 0 || 200 <= y) { return; }
	pixels[(y<<8)+(y<<6)+x] = color;
	//pixels[320*y+x] = color;
}



//draw directly to vmem
void VideoGraphicsArray::PutPixelRaw(int32_t x, int32_t y, uint8_t colorIndex) {

	if (x < 0 || 320 <= x || y < 0 || 200 <= y) {
		return;
	}

	uint8_t* pixelAddress = this->FrameBufferSegment+((y<<8) + (y<<6) + x);
	*pixelAddress = colorIndex;
}


//read from backbuffer
uint8_t VideoGraphicsArray::ReadPixel(int32_t x, int32_t y) {

	if (x < 0 || 320 <= x || y < 0 || 200 <= y) { return 0; }
	return pixels[320*y+x];
}



void VideoGraphicsArray::PutText(char* str, int32_t x, int32_t y, uint8_t color) {

	uint16_t length = strlen(str);

	if ((320 - x) < (length * 5)) { return; }

	uint8_t* charArr = charset[0];

	for (int i = 0; str[i] != '\0'; i++) {
	
		charArr = charset[(uint8_t)(str[i])-32];

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



//draw from buffer
void VideoGraphicsArray::FillBufferFull(int32_t x, int32_t y, 
					int32_t w, int32_t h, uint8_t* buf) {

	for (int32_t Y = y; Y < y+h; Y++) {
		for (int32_t X = x; X < x+w; X++) {
		
			this->PutPixel(X, Y, buf[320*(Y-y)+(X-x)]);
		}
	}
}


void VideoGraphicsArray::FillBuffer(int16_t x, int16_t y, 
				int16_t w, int16_t h, uint8_t* buf, bool mirror) {

	uint8_t pixelColor = 0;
	uint8_t scrollVert = 0;
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


void VideoGraphicsArray::FillPolygon(uint16_t x[], uint16_t y[], 
				     uint8_t edgeNum, 
				     uint8_t color) {
	int16_t i, j, temp = 0;
	uint16_t xmin = 320; 
	uint16_t xmax = 0;

	for (i = 0; i < edgeNum; i++) {
	
		if (x[i] < xmin) {
			xmin = x[i];
		}
		if (x[i] > xmax) {
			xmax = x[i];
		}
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


void VideoGraphicsArray::MakeDark(uint8_t darkness) {

	if (darkness > 0) {

		for (uint8_t y = 0; y < 200; y++) {
			for (uint16_t x = 0; x < 320; x++) {

				//make pixel darker	
				for (uint8_t i = 0; i < darkness; i++) {
				
					pixels[(y<<8)+(y<<6)+x] = light2dark[pixels[(y<<8)+(y<<6)+x]];
				}
			}
		}
	}
}

void VideoGraphicsArray::MakeWave(uint8_t waveLength) {

	if (!waveLength) { return; }

	uint16_t offset = waveLength;
	bool incOrDec = true;

	for (uint8_t y = 0; y < 200; y++) {
		for (uint16_t x = offset; x < 320; x++) {
		
			pixels[(y<<8)+(y<<6)+(x-offset)] = pixels[(y<<8)+(y<<6)+x];
		}
		
		if (offset > waveLength) { incOrDec = false; }
		if (offset == 0) { incOrDec = true; }
		
		if (incOrDec) { offset++;
		} else { offset--; }
	}
}

void VideoGraphicsArray::DrawToScreen() {

	for (uint8_t y = 0; y < 200; y++) {
		for (uint16_t x = 0; x < 320; x++) {

			uint8_t* pixelAddress = this->FrameBufferSegment+((y<<8) + (y<<6) + x);
			*pixelAddress = pixels[(y<<8)+(y<<6)+x];
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
			newPixel = Web2EGA(oldPixel);
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

	this->FillRectangle(0, 0, 320, 200, 0x09);
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
