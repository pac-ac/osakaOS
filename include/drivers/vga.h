#ifndef __OS__DRIVERS__VGA_H
#define __OS__DRIVERS__VGA_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <gui/font.h>
#include <gui/pixelart.h>
#include <math.h>



namespace os {

	namespace drivers {

		class VideoGraphicsArray {

			public:		
			hardwarecommunication::Port8Bit miscPort;
			
			hardwarecommunication::Port8Bit crtcIndexPort;
			hardwarecommunication::Port8Bit crtcDataPort;
			
			hardwarecommunication::Port8Bit sequencerIndexPort;
			hardwarecommunication::Port8Bit sequencerDataPort;
			
			hardwarecommunication::Port8Bit graphicsControllerIndexPort;
			hardwarecommunication::Port8Bit graphicsControllerDataPort;
			
			hardwarecommunication::Port8Bit attributeControllerIndexPort;
			hardwarecommunication::Port8Bit attributeControllerReadPort;
			hardwarecommunication::Port8Bit attributeControllerWritePort;
			hardwarecommunication::Port8Bit attributeControllerResetPort;


			void WriteRegisters(common::uint8_t* registers);
			
			common::uint8_t* GetFrameBufferSegment();
			common::uint8_t* FrameBufferSegment;

			//back buffer
			common::uint8_t pixels[64000];

			public:
				VideoGraphicsArray();
				~VideoGraphicsArray();

				virtual bool SupportsMode(common::uint32_t width, common::uint32_t height, common::uint32_t colordepth);
				virtual bool SetMode(common::uint32_t width, common::uint32_t height, common::uint32_t colordepth);
				

				virtual void PutPixel(common::int32_t x, common::int32_t y, common::uint8_t colorIndex);
				virtual void PutPixelRaw(common::int32_t x, common::int32_t y, common::uint8_t colorIndex);
				virtual common::uint8_t ReadPixel(common::int32_t x, common::int32_t y);

				virtual void PutText(char* str, common::int32_t x, common::int32_t y, common::uint8_t color);

				virtual void FillBufferFull(common::int32_t x, common::int32_t y,
								common::int32_t w, common::int32_t h,
								common::uint8_t* buf);
				
				virtual void FillBuffer(common::int16_t x, common::int16_t y,
							common::int16_t w, common::int16_t h,
							common::uint8_t* buf, bool mirror);
				
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
				
				virtual void FillPolygon(common::uint16_t x[], common::uint16_t y[], 
							 common::uint8_t edgeNum, common::uint8_t color);
				
				void FSdither(common::uint32_t* buf, common::uint16_t w, common::uint16_t h);
				
				void ErrorScreen();

				virtual void MakeDark(common::uint8_t darkness);
				virtual void MakeWave(common::uint8_t waveLength);
				
				void DrawToScreen();
		};	

		//default ega palette light to dark
		static common::uint8_t light2dark[] = {
		
			0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x01, 0x18, 0x19, 0x04, 0x21, 0x30, 0x31, 
			0x40, 0x01, 0x22, 0x23, 0x20, 0x12, 0x06, 0x2a, 0x40, 0x11, 0x02, 0x03, 0x04, 0x0d, 0x16, 0x23, 
			0x40, 0x28, 0x30, 0x31, 0x04, 0x0c, 0x34, 0x2e, 0x40, 0x21, 0x10, 0x23, 0x0c, 0x05, 0x14, 0x35,
			0x40, 0x38, 0x02, 0x0a, 0x14, 0x2c, 0x06, 0x0e, 0x40, 0x11, 0x1a, 0x1b, 0x24, 0x2d, 0x36, 0x37,
			0x40
		};
	}
}

#endif
