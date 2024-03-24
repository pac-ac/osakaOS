#ifndef __OS__GUI__WIDGET_H
#define __OS__GUI__WIDGET_H

#include <common/types.h>
#include <common/graphicscontext.h>
#include <drivers/keyboard.h>
#include <gui/pixelart.h>


namespace os {

	namespace gui {

		class Widget : public os::drivers::KeyboardEventHandler {
		
			//protected:
			public:
				Widget* parent;
				common::int32_t x;
				common::int32_t y;
				common::int32_t w;
				common::int32_t h;
				
				common::uint8_t windowOffset;
				
				common::int32_t wo;
				common::int32_t ho;
				
				char* name;
				common::uint8_t color;
				common::uint8_t textColor;
				
				bool Focussable;
				bool Buttons;
				bool Resizable;

				bool keyPressed = false;
				char keyCharWidget = 0;
			public:

				Widget(Widget* parent,  
					common::int32_t x, common::int32_t y, 
					common::int32_t w, common::int32_t h,
					char* name,
					common::uint8_t color, bool window);
				
				~Widget();

				virtual void GetFocus(Widget* widget);
				virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);
				
				virtual bool ContainsCoordinate(common::int32_t x, common::int32_t y);
				virtual common::uint8_t ContainsCoordinateButton(common::int32_t x, common::int32_t y);
				
				virtual bool AddChild(Widget* child);

				virtual void Draw(common::GraphicsContext* gc);
				
				virtual void Print(char* str);
				virtual void PutChar(char ch);

				virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						         common::int32_t newx, common::int32_t newy);
				
				virtual void OnKeyDown(char);
				virtual void OnKeyUp(char);

		};

		class CompositeWidget : public Widget {
		
			//private:
			public:
				Widget* children[150];
				int numChildren;
				Widget* focussedChild;

			public:
				CompositeWidget(Widget* parent,  
					common::int32_t x, common::int32_t y, 
					common::int32_t w, common::int32_t h,
					char* name,
					common::uint8_t color, bool window);
				
				~CompositeWidget();
				
				
				virtual void GetFocus(Widget* widget);
				virtual bool AddChild(Widget* child);
				virtual bool DeleteChild();
				virtual bool Maximize();
				virtual bool Minimize();
				
				virtual void Resize(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy);
				
				virtual void ButtonAction(common::uint8_t button);
				
				virtual void Draw(common::GraphicsContext* gc);
				
				virtual void Print(char* str);
				virtual void PutChar(char ch);
				
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
