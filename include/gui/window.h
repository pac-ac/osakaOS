#ifndef __OS__GUI__WINDOW_H
#define __OS__GUI__WINDOW_H

#include <gui/widget.h>
#include <drivers/mouse.h>
#include <filesys/ofs.h>
#include <gui/font.h>
#include <string.h>
#include <list.h>
#include <app.h>


#define MIN_WINDOW_WIDTH 70
#define MIN_WINDOW_HEIGHT 35


namespace os {

	namespace gui {

		class Window : public CompositeWidget {

			friend class Desktop;

			public:
				char* altName = nullptr;

				bool FileWindow;
				bool Save;
				bool Tools;
				char fileName[33];
				common::uint8_t fileNameIndex;
				common::uint8_t winColor;

				os::App* app;
				os::filesystem::FileSystem* filesystem;
			public:
				Window(CompositeWidget* parent, 
					common::int32_t x, common::int32_t y,
					common::int32_t w, common::int32_t h,
					char* name,
					common::uint8_t color,
					App* app,
					common::int8_t ID,
					filesystem::FileSystem* filesystem);
				~Window();

				void FullScreen();

				void Draw(common::GraphicsContext* gc);
				void WindowMenuDraw(common::GraphicsContext* gc);
				void FileDraw(common::GraphicsContext* gc);
				void WindowResize(common::int32_t oldx, common::int32_t oldy,
						  common::int32_t newx, common::int32_t newy);
				void UpdateName(char* name);

				common::uint8_t ReturnAppType();

				void DestroyWindow();
				
				void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
				void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
				void OnMouseMove(common::int32_t oldx, common::int32_t oldy,
						 common::int32_t newx, common::int32_t newy);

				void OnKeyDown(char str);
				void OnKeyUp(char str);
		};
		
		/*
		class WindowButton : public Widget {
	
			public:
				common::uint8_t buttonType;
				common::uint8_t* buf;
			public:
				WindowButton(Window* window,
						common::int32_t x, common::int32_t y, 
						common::int32_t w, common::int32_t h, 
						common::uint8_t buttonType,
						bool offsetX, bool offsetY,
						bool offsetW, bool offsetH);
				~WindowButton();
				
				void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
		};
		*/
	}
}





#endif
