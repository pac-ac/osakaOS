#ifndef __OS__DRIVERS__VGA_H
#define __OS__DRIVERS__VGA_H

#include <common/types.h>
#include <memorymanagement.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <gui/font.h>
#include <gui/pixelart.h>
#include <string.h>
#include <math.h>
#include <list.h>



#define WIDTH_13H 320
#define HEIGHT_13H 200
#define BUFFER_SIZE_13H 64000

#define TEXT_BASIC 0x00
#define TEXT_HEADER 0x01
#define TEXT_ITALIC 0x02
#define TEXT_BOLD 0x03
#define TEXT_UNDERLINE 0x04


namespace os {

	namespace drivers {

		class VideoGraphicsArray {

			public:		
			hardwarecommunication::Port8Bit miscPort;
			
			hardwarecommunication::Port8Bit crtcIndexPort;
			hardwarecommunication::Port8Bit crtcDataPort;
			
			hardwarecommunication::Port8Bit sequencerIndexPort;
			hardwarecommunication::Port8Bit sequencerDataPort;
			
			hardwarecommunication::Port8Bit colorPaletteMask;
			hardwarecommunication::Port8Bit colorRegisterRead;
			hardwarecommunication::Port8Bit colorRegisterWrite;
			hardwarecommunication::Port8Bit colorDataPort;
			
			hardwarecommunication::Port8Bit graphicsControllerIndexPort;
			hardwarecommunication::Port8Bit graphicsControllerDataPort;
			
			hardwarecommunication::Port8Bit attributeControllerIndexPort;
			hardwarecommunication::Port8Bit attributeControllerReadPort;
			hardwarecommunication::Port8Bit attributeControllerWritePort;
			hardwarecommunication::Port8Bit attributeControllerResetPort;


			void WriteRegisters(common::uint8_t* registers);
			
			common::uint8_t* GetFrameBufferSegment();
			common::uint8_t* FrameBufferSegment;
			MemoryManager* mm;

			//back buffer
			common::uint32_t gfxWidth;
			common::uint32_t gfxHeight;
			common::uint32_t gfxBufferSize;
			common::uint8_t vesa = 0;
			
			common::uint8_t* pixels;
			//common::uint8_t pixels[BUFFER_SIZE_13H];

			public:
				VideoGraphicsArray(MemoryManager* mm, common::uint8_t* vbeAddr, 
							common::uint32_t width, common::uint32_t height);
				~VideoGraphicsArray();

				virtual bool SupportsMode(common::uint32_t width, common::uint32_t height, common::uint32_t colordepth);
				virtual bool SetMode(common::uint32_t width, common::uint32_t height, common::uint32_t colordepth);
				virtual void PaletteUpdate(common::uint8_t index, common::uint8_t r, common::uint8_t g, common::uint8_t b);	

				void PutPixel(common::int32_t x, common::int32_t y, common::uint8_t colorIndex);
				void PutPixelRaw(common::int32_t x, common::int32_t y, common::uint8_t colorIndex);
				void AlphaWrite(common::int32_t x, common::int32_t y, common::uint8_t color, float alpha);
				void DarkenPixel(common::int32_t x, common::int32_t y);
				common::uint8_t ReadPixel(common::int32_t x, common::int32_t y);

				virtual void PutText(char* str, common::int32_t x, common::int32_t y, common::uint8_t color, common::uint8_t flags = 0x00);
				virtual void PrintData(char* str, common::uint16_t length, common::int32_t x, common::int32_t y, common::uint8_t color);

				virtual void FillBufferFull(common::int32_t x, common::int32_t y,
								common::int32_t w, common::int32_t h,
								common::uint8_t* buf);
				
				virtual void FillBuffer(common::int16_t x, common::int16_t y,
							common::int16_t w, common::int16_t h,
							common::uint8_t* buf, bool mirror);
				
				virtual void FillBufferCoordinate(common::int16_t x, common::int16_t y,
								  common::int16_t w, common::int16_t h,
								  common::uint8_t* buf, bool mirror,
								  common::uint8_t* positionBuf, common::uint8_t positionVal);
				
				virtual void FillRectangle(common::int32_t x, common::int32_t y, 
							   common::int32_t w, common::int32_t h, 
							   common::uint8_t color);
				
				virtual void DrawRectangle(common::int32_t x, common::int32_t y, 
							   common::int32_t w, common::int32_t h, 
							   common::uint8_t color);
				
				virtual void DrawLineFlat(common::int32_t x0, common::int32_t y0, 
							common::int32_t x1, common::int32_t y1, 
							common::uint8_t color, bool x);
				
				virtual void DrawLineLow(common::int32_t x0, common::int32_t y0, 
							common::int32_t x1, common::int32_t y1, 
							common::uint8_t color);
				
				virtual void DrawLineHigh(common::int32_t x0, common::int32_t y0, 
							common::int32_t x1, common::int32_t y1, 
							common::uint8_t color);
				
				virtual void DrawLine(common::int32_t x0, common::int32_t y0, 
							common::int32_t x1, common::int32_t y1, 
							common::uint8_t color);
				
				virtual void FillTriangle(common::int16_t x0, common::int16_t y0, 
							 common::int16_t x1, common::int16_t y1, 
							 common::int16_t x2, common::int16_t y2, 
							 common::uint8_t color);
				
				virtual void FillPolygon(common::uint16_t x[], common::uint16_t y[], 
							 common::uint8_t edgeNum, common::uint8_t color);
				
				virtual common::int16_t ProjectVertexX(float v[3]);
				virtual common::int16_t ProjectVertexY(float v[3]);
				
				virtual void RotateVertex(float v[3], float degrees, common::uint8_t varConst);
				
				virtual void RenderObject(float** vertices, common::uint16_t numOfVertices, 
							  common::uint8_t** triangles, common::uint16_t numOfTriangles);
				
				void FSdither(common::uint32_t* buf, common::uint16_t w, common::uint16_t h);
				
				void ErrorScreen();

				virtual void MakeDark(common::uint8_t darkness, 
						      common::uint16_t x, common::uint16_t y,
						      common::uint16_t w, common::uint16_t h);				
				
				virtual void MakeWave(common::int32_t x, common::int32_t y, 
						      common::int32_t w, common::int32_t h, 
						      common::uint8_t amp, common::uint8_t freq, common::uint8_t waveInc,
						      common::uint8_t* background);

				virtual void Pixelize(common::int32_t x, common::int32_t y, 
						      common::int32_t w, common::int32_t h, 
						      common::uint8_t verticalSize, common::uint8_t horizontalSize);
				
				virtual void Rainbowize(common::int32_t x, common::int32_t y, 
						      common::int32_t w, common::int32_t h);
				
				virtual void Burn(common::int32_t x, common::int32_t y, 
						      common::int32_t w, common::int32_t h, bool mirror);
				void DrawToScreen();
		};	
		
		
		//default vga palette
		//used for palette init and color conversion
		static common::uint32_t defaultPalette[] = {

			0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA, 0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF, 
			0x000000, 0x101010, 0x202020, 0x353535, 0x454545, 0x555555, 0x656565, 0x757575, 0x8A8A8A, 0x9A9A9A, 0xAAAAAA, 0xBABABA, 0xCACACA, 0xDFDFDF, 0xEFEFEF, 0xFFFFFF, 
			0x0000FF, 0x4100FF, 0x8200FF, 0xBE00FF, 0xFF00FF, 0xFF00BE, 0xFF0082, 0xFF0041, 0xFF0000, 0xFF4100, 0xFF8200, 0xFFBE00, 0xFFFF00, 0xBEFF00, 0x82FF00, 0x41FF00,
			0x00FF00, 0x00FF41, 0x00FF82, 0x00FFBE, 0x00FFFF, 0x00BEFF, 0x0082FF, 0x0041FF, 0x8282FF, 0x9E82FF, 0xBE82FF, 0xDF82FF, 0xFF82FF, 0xFF82DF, 0xFF82BE, 0xFF829E, 
			0xFF8282, 0xFF9E82, 0xFFBE82, 0xFFDF82, 0xFFFF82, 0xDFFF82, 0xBEFF82, 0x9EFF82, 0x82FF82, 0x82FF9E, 0x82FFBE, 0x82FFDF, 0x82FFFF, 0x82DFFF, 0x82BEFF, 0x829EFF, 
			0xBABAFF, 0xCABAFF, 0xDFBAFF, 0xEFBAFF, 0xFFBAFF, 0xFFBAEF, 0xFFBADF, 0xFFBACA, 0xFFBABA, 0xFFCABA, 0xFFDFBA, 0xFFEFBA, 0xFFFFBA, 0xEFFFBA, 0xDFFFBA, 0xCAFFBA, 
			0xBAFFBA, 0xBAFFCA, 0xBAFFDF, 0xBAFFEF, 0xBAFFFF, 0xBAEFFF, 0xBADFFF, 0xBACAFF, 0x000071, 0x1C0071, 0x390071, 0x550071, 0x710071, 0x710055, 0x710039, 0x71001C, 
			0x710000, 0x711C00, 0x713900, 0x715500, 0x717100, 0x557100, 0x397100, 0x1C7100, 0x007100, 0x00711C, 0x007139, 0x007155, 0x007171, 0x005571, 0x003971, 0x001C71, 
			0x393971, 0x453971, 0x553971, 0x613971, 0x713971, 0x713961, 0x713955, 0x713945, 0x713939, 0x714539, 0x715539, 0x716139, 0x717139, 0x617139, 0x557139, 0x457139, 
			0x397139, 0x397145, 0x397155, 0x397161, 0x397171, 0x396171, 0x395571, 0x394571, 0x515171, 0x595171, 0x615171, 0x695171, 0x715171, 0x715169, 0x715161, 0x715159, 
			0x715151, 0x715951, 0x716151, 0x716951, 0x717151, 0x697151, 0x617151, 0x597151, 0x517151, 0x517159, 0x517161, 0x517169, 0x517171, 0x516971, 0x516171, 0x515971, 
			0x000041, 0x100041, 0x200041, 0x310041, 0x410041, 0x410031, 0x410020, 0x410010, 0x410000, 0x411000, 0x412000, 0x413100, 0x414100, 0x314100, 0x204100, 0x104100, 
			0x004100, 0x004110, 0x004120, 0x004131, 0x004141, 0x003141, 0x002041, 0x001041, 0x202041, 0x282041, 0x312041, 0x392041, 0x412041, 0x412039, 0x412031, 0x412028, 
			0x412020, 0x412820, 0x413120, 0x413920, 0x414120, 0x394120, 0x314120, 0x284120, 0x204120, 0x204128, 0x204131, 0x204139, 0x204141, 0x203941, 0x203141, 0x202841, 
			0x2D2D41, 0x312D41, 0x352D41, 0x3D2D41, 0x412D41, 0x412D3D, 0x412D35, 0x412D31, 0x412D2D, 0x41312D, 0x41352D, 0x413D2D, 0x41412D, 0x3D412D, 0x35412D, 0x31412D, 
			0x2D412D, 0x2D4131, 0x2D4135, 0x2D413D, 0x2D4141, 0x2D3D41, 0x2D3541, 0x2D3141, 0x0055AA, 0xAA0055, 0xAA5555, 0xAAAA55, 0xFF5500, 0xFF55AA, 0xFFAA00, 0xFFAA55
			//0x2D412D, 0x2D4131, 0x2D4135, 0x2D413D, 0x2D4141, 0x2D3D41, 0x2D3541, 0x2D3141, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
		};
		
		//default vga palette light to dark
		static common::uint8_t light2dark[] = {
		
			0x00, 0x68, 0x78, 0x7c, 0x70, 0x6c, 0xb8, 0x15, 0x11, 0x97, 0x02, 0x03, 0x04, 0x05, 0x74, 0x1a,
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
			0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
			0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
			0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
			0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
			0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
			0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
			0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
			0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7e, 0x6e, 0x88, 0x8c, 0x06, 0x85, 0x73, 0x8a 
		};
		

	}
}

#endif
