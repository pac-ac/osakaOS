#ifndef __OS__GUI__WIDGET_H
#define __OS__GUI__WIDGET_H

#include <common/types.h>
#include <common/graphicscontext.h>
#include <drivers/keyboard.h>
#include <gui/pixelart.h>


#define TEXT_MAX_WIDTH 53
#define TEXT_MAX_HEIGHT 22


namespace os {
		
	class App;

	namespace gui {

		class CompositeWidget : public os::drivers::KeyboardEventHandler {
		
			//private:
			public:
				//pointers to related window
				CompositeWidget* parent;
				CompositeWidget* children[30];
				CompositeWidget* focussedChild;
				int numChildren;
				

				//graphical buffer for windows and shit
				common::uint8_t buf[64000];
				

				//data for storing position
				//of window and text etc.
				common::int32_t x;
				common::int32_t y;
				common::int32_t w;
				common::int32_t h;
				
				common::int32_t wo;
				common::int32_t ho;
			
				//text coords	
				common::uint16_t outx;
				common::uint8_t outy;
				
				//check if scrolling
				bool textScroll = false;


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
				bool Menu;
				bool Min;


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
				virtual bool ContainsCoordinate(common::int32_t x, common::int32_t y);
				virtual common::uint8_t ContainsCoordinateButton(common::int32_t x, common::int32_t y);
				
				//add, remove, sub windows
				virtual bool AddChild(CompositeWidget* child);
				virtual CompositeWidget* CreateChild(common::uint8_t appType, char* name, App* app);
				virtual bool DeleteChild();
				
			
				//menu stuff to change window
				virtual bool Maximize();
				virtual bool Minimize();
				virtual void Resize(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy);
				virtual bool MenuButton();
				virtual void ButtonAction(common::uint8_t button);
				
				
				//draw actual window	
				virtual void Draw(common::GraphicsContext* gc);
				virtual void MenuDraw(common::GraphicsContext* gc);
				
				virtual common::uint8_t ReturnAppType();
				
				//drawing shapes n shit
				virtual void PutPixel(common::int32_t x, common::int32_t y, common::uint8_t color);
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
				void PutChar(char ch);
				void Print(char* str);
				void PutText(char* str, common::int32_t x, common::int32_t y, common::uint8_t color);
			
				//input
				virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy);

				virtual void OnKeyDown(char);
				virtual void OnKeyUp(char);
		
		};
	}
}
#endif
