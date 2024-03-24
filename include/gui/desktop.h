#ifndef __OS__GUI__DESKTOP_H
#define __OS__GUI__DESKTOP_H

#include <gui/widget.h>
#include <gui/sim.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <multitasking.h>
#include <gdt.h>
#include <app.h>


namespace os {

	namespace gui {

		class Desktop : public CompositeWidget, public os::drivers::MouseEventHandler {
		
			//protected:
			public:
				common::GraphicsContext* gc;
				GlobalDescriptorTable* gdt;
				TaskManager* taskManager;
				Simulator* osaka;

				App* apps[10];			

				bool sim = false;

				common::uint32_t MouseX;	
				common::uint32_t MouseY;	
				
				common::uint32_t oldMouseX;	
				common::uint32_t oldMouseY;	
	
				bool mouseMove = false;
				bool click = false;

				common::uint8_t color;
			public:
				Desktop(common::int32_t w, common::int32_t h, common::uint8_t color, 
					common::GraphicsContext* gc, GlobalDescriptorTable* gdt, 
					TaskManager* taskManager, Simulator* osaka);
				
				~Desktop();


				bool AddTaskGUI(Task* task);


				void Draw(common::GraphicsContext* gc);
				void DrawNoMouse(common::GraphicsContext* gc);
				void MouseDraw(common::GraphicsContext* gc);

				void OnMouseDown(common::uint8_t button);
				void OnMouseUp(common::uint8_t button);
				void OnMouseMove(int x, int y);

				void OnKeyDown(char str);
				void OnKeyUp(char str);
		
		};



	}
}


#endif
