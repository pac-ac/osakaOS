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
#include <net/network.h>
#include <gdt.h>
#include <app.h>
#include <app/paint.h>
#include <app/file_edit.h>
#include <app/browser.h>
#include <string.h>
#include <cli.h>
#include <list.h>
#include <math.h>
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
				net::Network* network;
				Compiler* compiler;

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

				common::uint8_t* customCursorBuf = nullptr;
				common::uint16_t customCursorW = 0;
				common::uint16_t customCursorH = 0;

				bool setScreenSaver = false;
				common::uint8_t saverVal = 0;
				common::uint32_t timer = 0x10000;


				/*
				float v0[3] = {  1,  1,  1 };
				float v1[3] = { -1,  1,  1 };
				float v2[3] = { -1, -1,  1 };
				float v3[3] = {  1, -1,  1 };
				float v4[3] = {  1,  1, -1 };
				float v5[3] = { -1,  1, -1 };
				float v6[3] = { -1, -1, -1 };
				float v7[3] = {  1, -1, -1 };
				
				common::uint8_t t0[4] = { 0, 1, 2, WFF0000 };
				common::uint8_t t1[4] = { 0, 2, 3, WFF0000 };
				common::uint8_t t2[4] = { 4, 0, 3, W00FF00 };
				common::uint8_t t3[4] = { 4, 3, 7, W00FF00 };
				common::uint8_t t4[4] = { 5, 4, 7, W0000FF };
				common::uint8_t t5[4] = { 5, 7, 6, W0000FF };
				common::uint8_t t6[4] = { 1, 5, 6, W00FFFF };
				common::uint8_t t7[4] = { 1, 6, 2, W00FFFF };
				common::uint8_t t8[4] = { 4, 5, 1, WFF00FF };
				common::uint8_t t9[4] = { 4, 1, 0, WFF00FF };
				common::uint8_t t10[4] ={ 2, 6, 7, WFFFF00 };
				common::uint8_t t11[4] ={ 2, 7, 3, WFFFF00 };
				*/
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
				void DrawSaver(common::GraphicsContext* gc, common::uint8_t saverType);
				void DrawTaskBar(common::GraphicsContext* gc);
				void DrawNoMouse(common::GraphicsContext* gc);
				void MouseDraw(common::GraphicsContext* gc);
				
				void LoadCursor(common::uint8_t* buf, common::uint16_t w, common::uint16_t h);

				void Screenshot();
				
				void CreateButton(char* file, common::uint8_t openType, common::uint8_t* imageFile);
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
				common::uint8_t iconBuf[400];
			public:
				DesktopButton(char* file, common::uint8_t openType, 
						common::uint8_t* imageFile, common::uint8_t index);
				~DesktopButton();
				
				void Draw(common::GraphicsContext* gc);

				void OnMouseDown(common::int32_t x,
						common::int32_t y,
						common::uint8_t button, Desktop* desktop);
		};
	}
}


#endif
