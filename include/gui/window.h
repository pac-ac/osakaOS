#ifndef __OS__GUI__WINDOW_H
#define __OS__GUI__WINDOW_H

#include <gui/widget.h>
#include <drivers/mouse.h>
#include <gui/desktop.h>
#include <gui/font.h>
#include <app.h>


namespace os {

	namespace gui {

		class Window : public CompositeWidget {
	
			friend class Desktop;

			public:
			//protected:
				bool Dragging;

				common::uint8_t buf[64000];
				common::uint8_t text[1500];
				common::uint16_t textNum;

				common::uint16_t outx;
				common::uint8_t outy;

				char* name;

				os::App* app;
			public:
				Window(Widget* parent, 
					common::int32_t x, common::int32_t y,
					common::int32_t w, common::int32_t h,
					char* name,
					common::uint8_t color,
					App* app);
				
				~Window();

				void Draw(common::GraphicsContext* gc);

				void Print(char* str);
				void PutChar(char ch);

				void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				void OnMouseMove(common::int32_t oldx, common::int32_t oldy,
						 common::int32_t newx, common::int32_t newy);

				void OnKeyDown(char str);
				void OnKeyUp(char str);
		};



	}
}





#endif
