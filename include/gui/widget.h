#ifndef __OS__GUI__WIDGET_H
#define __OS__GUI__WIDGET_H

#include <common/types.h>
#include <common/graphicscontext.h>
#include <drivers/keyboard.h>



namespace os {

	namespace gui {

		class Widget : public os::drivers::KeyboardEventHandler{
		
			protected:
				Widget* parent;
				common::int32_t x;
				common::int32_t y;
				common::int32_t w;
				common::int32_t h;
				
				common::uint8_t color;
				bool Focussable;
			public:

				Widget(Widget* parent,  
					common::int32_t x, common::int32_t y, 
					common::int32_t w, common::int32_t h,
					common::uint8_t color);
				
				~Widget();

				virtual void GetFocus(Widget* widget);
				virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);
				virtual bool ContainsCoordinate(common::int32_t x, common::int32_t y);
				
				virtual void Draw(common::GraphicsContext* gc);
				
				virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						         common::int32_t newx, common::int32_t newy);

		
		};

		class CompositeWidget : public Widget {
		
			//private:
			public:
				Widget* children[100];
				int numChildren;
				Widget* focussedChild;

			public:
				CompositeWidget(Widget* parent,  
					common::int32_t x, common::int32_t y, 
					common::int32_t w, common::int32_t h,
					common::uint8_t color);
				
				~CompositeWidget();
				
				
				virtual void GetFocus(Widget* widget);
				virtual bool AddChild(Widget* child);
				
				virtual void Draw(common::GraphicsContext* gc, bool mode);
				
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
