#ifndef __OS__APP_H
#define __OS__APP_H

#include <common/types.h>
#include <drivers/vga.h>
#include <gui/widget.h>


namespace os {

	class App {
	
		public:
			App();
			~App();

			virtual void ComputeAppState(common::GraphicsContext* gc, gui::Widget* widget);
	
			virtual void OnKeyDown(char ch, gui::Widget* widget);
			virtual void OnKeyUp(char ch, gui::Widget* widget);

			virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::Widget* widget);
			virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button, gui::Widget* widget);
			virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy,
						gui::Widget* widget);
	};
}

#endif
