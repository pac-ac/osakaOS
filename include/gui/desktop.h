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
#include <code/asm.h>
#include <memorymanagement.h>
#include <filesys/ofs.h>
#include <gdt.h>
#include <app.h>
#include <app/paint.h>
#include <app/file_edit.h>
#include <cli.h>
#include <list.h>
#include <script.h>


namespace os {

	namespace gui {

		class Desktop : public CompositeWidget, public os::drivers::MouseEventHandler {
		
			//protected:
			public:
				GlobalDescriptorTable* gdt;
				TaskManager* taskManager;
				MemoryManager* memoryManager;
				filesystem::FileSystem* filesystem;
				Compiler* compiler;

				common::GraphicsContext* gc;
				drivers::CMOS* cmos;
				drivers::DriverManager* drvManager;

				Simulator* osaka;

				bool taskbar = true;
				bool drawButtons = true;
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
					filesystem::FileSystem* filesystem, Compiler* compiler, 
					drivers::CMOS* cmos, drivers::DriverManager* drvManager, 
					Simulator* osaka);
				
				~Desktop();

				common::GraphicsContext* ReturnGraphicsContext();


				CompositeWidget* CreateChild(common::uint8_t appType, char* name, App* app);
				void FreeChild(Window* window);

				void Draw(common::GraphicsContext* gc);
				void DrawTaskBar(common::GraphicsContext* gc);
				void DrawNoMouse(common::GraphicsContext* gc);
				void MouseDraw(common::GraphicsContext* gc);

				void Screenshot();
				
				void CreateButton(char* file, common::uint8_t openType, char* imageFile);
				void RemoveButton(char* file);

				void OnMouseDown(common::uint8_t button);
				void OnMouseUp(common::uint8_t button);
				void OnMouseMove(int x, int y);
				void TaskBarClick(common::uint8_t button);

				void OnKeyDown(char str);
				void OnKeyUp(char str);
		};

		//buttons
		class DesktopButton : public Widget {

			public:
				char file[33];
				common::uint8_t openType;

				common::uint8_t buffer[400];
			public:
				DesktopButton(char* file, common::uint8_t openType, 
						char* imageFile, common::uint8_t index);
				~DesktopButton();
				
				void Draw(common::GraphicsContext* gc);

				void OnMouseDown(common::int32_t x,
						common::int32_t y,
						common::uint8_t button, Desktop* desktop);
		};
	}
}


#endif
