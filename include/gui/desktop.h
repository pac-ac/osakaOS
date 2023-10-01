#ifndef __OS__GUI__DESKTOP_H
#define __OS__GUI__DESKTOP_H

#include <gui/widget.h>
#include <drivers/mouse.h>



namespace os {

	namespace gui {

		class Desktop : public CompositeWidget, public os::drivers::MouseEventHandler {
		
			//protected:
			public:
				common::uint32_t MouseX;	
				common::uint32_t MouseY;	
				
				common::uint32_t oldMouseX;	
				common::uint32_t oldMouseY;	
	
				bool mouseMove = false;

			public:
				Desktop(common::int32_t w, common::int32_t h, 
						common::uint8_t r, common::uint8_t g, common::uint8_t b);
				~Desktop();

				void Draw(common::GraphicsContext* gc, bool mode);
				void DrawNoMouse(common::GraphicsContext* gc, bool mode);
				
				void MouseDraw(common::GraphicsContext* gc, bool mode);

				void OnMouseDown(common::uint8_t button);
				void OnMouseUp(common::uint8_t button);
				void OnMouseMove(int x, int y);
		
		};



	}
}










#endif
