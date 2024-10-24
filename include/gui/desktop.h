#ifndef __OS__GUI__DESKTOP_H
#define __OS__GUI__DESKTOP_H

#include <gui/widget.h>
#include <gui/window.h>
#include <gui/sim.h>
#include <drivers/driver.h>
#include <drivers/mouse.h>
#include <drivers/cmos.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <multitasking.h>
#include <memorymanagement.h>
#include <filesys/ofs.h>
#include <gdt.h>
#include <app.h>
#include <app/paint.h>
#include <app/file_edit.h>
#include <cli.h>
#include <script.h>


namespace os {

	namespace gui {

		class Button;

		class Desktop : public CompositeWidget, public os::drivers::MouseEventHandler {
		
			//protected:
			public:
				GlobalDescriptorTable* gdt;
				TaskManager* taskManager;
				MemoryManager* memoryManager;
				filesystem::FileSystem* filesystem;

				common::GraphicsContext* gc;
				drivers::CMOS* cmos;
				drivers::DriverManager* drvManager;

				Simulator* osaka;

				bool taskbar = true;
				bool drawButtons = true;
				Button* buttons;

				bool takeSS = false;

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
					TaskManager* taskManager, MemoryManager* memoryManager, 
					filesystem::FileSystem* filesystem, drivers::CMOS* cmos,
					drivers::DriverManager* drvManager, Button* buttons, Simulator* osaka);
				
				~Desktop();

				common::GraphicsContext* ReturnGraphicsContext();


				CompositeWidget* CreateChild(common::uint8_t appType, char* name, App* app);
				void FreeChild(Window* window);

				void Draw(common::GraphicsContext* gc);
				void DrawTaskBar(common::GraphicsContext* gc);
				void DrawNoMouse(common::GraphicsContext* gc);
				void MouseDraw(common::GraphicsContext* gc);

				void Screenshot();

				void OnMouseDown(common::uint8_t button);
				void OnMouseUp(common::uint8_t button);
				void OnMouseMove(int x, int y);
				void TaskBarClick(common::uint8_t button);

				void OnKeyDown(char str);
				void OnKeyUp(char str);
		};

		//buttons
		class Button {

			public:
				common::uint8_t numOfButtons = 0;
				common::uint8_t buttonTypes[96];
				char* buttonFiles[96];
			public:
				Button();
				~Button();
				
				void Draw(common::GraphicsContext* gc);

				void OnMouseDown(common::int32_t x,
						common::int32_t y,
						common::uint8_t button,
						Desktop* desktop);

				void AddFile(char* fileName);
		};
	}
}


#endif
