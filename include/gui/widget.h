#ifndef __OS__GUI__WIDGET_H
#define __OS__GUI__WIDGET_H

#include <common/types.h>
#include <common/graphicscontext.h>
#include <drivers/keyboard.h>
#include <drivers/vga.h>
#include <gui/pixelart.h>
#include <string.h>
#include <list.h>



#define TEXT_MAX_WIDTH 53
#define TEXT_MAX_HEIGHT 22

#define TEXT_BUF_SIZE 2048

#define LEFT_CLICK 1


namespace os {
		
	class App;

	namespace gui {

		class Widget : public os::drivers::KeyboardEventHandler {

			public:
				static common::GraphicsContext* gc;

				common::int32_t x;
				common::int32_t y;
				common::int32_t w;
				common::int32_t h;
				
				common::uint8_t* buf = nullptr;
			public:
				Widget(common::int32_t x, common::int32_t y, 
					common::int32_t w, common::int32_t h);
				~Widget();
				
				virtual bool ContainsCoordinate(common::int32_t x, common::int32_t y);
				virtual void PutPixel(common::int32_t x, common::int32_t y, common::uint8_t color);
				void PutText(char* str, common::int32_t x, common::int32_t y, common::uint8_t color, common::uint8_t flags = 0x00);
		};
	
		


		class CompositeWidget : public Widget {
		
			//private:
			public:
				//pointers to related window
				CompositeWidget* parent;
				CompositeWidget* children[30];
				CompositeWidget* focussedChild;
				common::int8_t ID;
				common::int8_t numChildren;

				List* buttons;
				
				//graphical buffer for windows and shit
				common::uint8_t* windowBuffer;
				//common::uint8_t windowBuffer[BUFFER_SIZE_13H];

				//data for storing position
				//of window and text etc.
				common::int32_t xo;
				common::int32_t yo;
				common::int32_t wo;
				common::int32_t ho;
			
				//text coords	
				common::uint16_t outx;
				common::uint16_t outy;
				
				//check if scrolling
				bool textScroll = false;

				//text max x and y
				common::uint16_t currentTextWidth;
				common::uint16_t currentTextHeight;

				//other stuff we need
				char* name;
				common::uint8_t color;
				common::uint8_t textColor;
				
				common::uint8_t windowOffset;
				common::uint8_t minWindows = 0;

				//window attributes
				bool Focussable;
				bool Fullscreen;
				bool Buttons;
				bool Resizable;
				bool Dragging;
				bool MenuOpen;
				bool Min;
				
				bool actionDetected = false;	
				
				//draw options
				bool Wave;
				bool Blur;
				bool Fire;
				bool Rainbow;
				bool Pixelize;

				//input
				bool keypress = false;
				bool mouseclick = false;
				char keyCharWidget = 0;
			public:
				CompositeWidget(CompositeWidget* parent,  
					common::int32_t x, common::int32_t y, 
					common::int32_t w, common::int32_t h,
					char* name,
					common::uint8_t color, bool window);
				
				~CompositeWidget();
				
				//identify and check position of windows
				virtual void GetFocus(CompositeWidget* widget);
				virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);
				virtual common::uint8_t ContainsCoordinateButton(common::int32_t x, common::int32_t y);
				
				//add, remove, sub windows
				virtual bool AddChild(CompositeWidget* child);
				virtual CompositeWidget* CreateChild(common::uint8_t appType, char* name, App* app);
				virtual void CreateButton(char* file, common::uint8_t openType, char* imageFile);
				virtual bool DeleteChild();
			
				//menu stuff to change window
				virtual bool Maximize();
				virtual bool Minimize();
				virtual void Resize(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy);
				virtual bool MenuButton();
				virtual void ButtonAction(common::uint8_t button);
				virtual void UpdateName(char* name);
			
				virtual void LoadCursor(common::uint8_t* buf, common::uint16_t w, common::uint16_t h);	
				
				//draw actual window	
				virtual void Draw(common::GraphicsContext* gc);
				virtual void WindowMenuDraw(common::GraphicsContext* gc);
				
				virtual common::uint8_t ReturnAppType();
				
				
				//drawing shapes n shit
				virtual void WritePixel(common::int32_t x, common::int32_t y, common::uint8_t color);
				virtual common::uint8_t ReadPixel(common::uint32_t i);	
				virtual void DrawRectangle(common::int32_t x0, common::int32_t y0, 
							common::int32_t x1, common::int32_t y1,
							common::uint8_t color, bool fill);
				virtual void DrawLine(common::int32_t x0, common::int32_t y0, 
							common::int32_t x1, common::int32_t y1,
							common::uint8_t color);
				virtual void DrawCircle(common::int32_t x, common::int32_t y, 
							common::int32_t r, common::uint8_t color);
				
				virtual void FillBuffer(common::int32_t x, common::int32_t y, 
							common::int16_t w, common::int16_t h, 
							common::uint8_t* buf);

				//words and shit
				void RenderChar(common::uint8_t* charArr, common::uint8_t pixelColor, common::uint8_t flags);
				void PutChar(char ch, common::uint8_t flags = 0x00);
				void Print(char* str, common::uint8_t flags = 0x00);
				void PrintTextBuffer();
			
				//input
				virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy);

				virtual void OnKeyDown(char);
				virtual void OnKeyUp(char);
		
		};
		
		
		
		class WindowButton : public Widget {
		
			public:
				CompositeWidget* window;
				common::uint8_t buttonType;
			
				common::int32_t savex;
				common::int32_t savey;

				bool stretchW;
				bool stretchH;
				
				bool offsetX;
				bool offsetY;
				bool offsetW;
				bool offsetH;
			public:
				WindowButton(CompositeWidget* window, 
						common::int32_t x, common::int32_t y, 
						common::int32_t w, common::int32_t h, common::uint8_t buttonType,
						bool offsetX, bool offsetY, bool offsetW, bool offsetH);
				~WindowButton();

				void Draw(common::GraphicsContext* gc);

				void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
		};
	}
}
#endif
