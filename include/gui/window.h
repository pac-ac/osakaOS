#ifndef __OS__GUI__WINDOW_H
#define __OS__GUI__WINDOW_H

#include <gui/widget.h>
#include <drivers/mouse.h>


namespace os {

	namespace gui {

		class Window : public CompositeWidget {
		
			protected:
				bool Dragging;

			public:
				Window(Widget* parent, 
					common::int32_t x, common::int32_t y,
					common::int32_t w, common::int32_t h,
					common::uint8_t color);
				
				~Window();


				void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				void OnMouseMove(common::int32_t oldx, common::int32_t oldy,
							 common::int32_t newx, common::int32_t newy);

		};



	}
}





#endif
