#ifndef __OS__GUI__BUTTON_H
#define __OS__GUI__BUTTON_H

#include <common/types.h>
#include <memorymanagement.h>
#include <gui/window.h>
#include <gui/desktop.h>
#include <app.h>
#include <cli.h>


namespace os {

	namespace gui {
	
		class Button : public CompositeWidget {
		
			//protected:
			public:
				common::uint8_t buf[400];
				App* app;
			public:
				Button(Desktop* desktop, 
					common::int32_t x, common::int32_t y,
					common::uint8_t appType,
					common::uint8_t* buf);
				~Button();
		
				
				void Draw(common::GraphicsContext* gc);

				void OnMouseDown(common::int32_t x, 
						common::int32_t y, 
						common::uint8_t button);
				
				void OnMouseUp(common::int32_t x, 
						common::int32_t y, 
						common::uint8_t button);
				
				void OnMouseMove(common::int32_t oldx, 
						common::int32_t oldy, 
						common::int32_t newx, 
						common::int32_t newy);
		};
	}
}


#endif
