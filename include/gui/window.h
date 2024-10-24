#ifndef __OS__GUI__WINDOW_H
#define __OS__GUI__WINDOW_H

#include <gui/widget.h>
#include <drivers/mouse.h>
#include <filesys/ofs.h>
#include <gui/font.h>
#include <app.h>

		
namespace os {

	namespace gui {

		class Window : public CompositeWidget {

			friend class Desktop;

			public:
				char* name;
				bool Resize;

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
					filesystem::FileSystem* filesystem);
				~Window();

				void FullScreen();


				void Draw(common::GraphicsContext* gc);
				void MenuDraw(common::GraphicsContext* gc);
				void FileDraw(common::GraphicsContext* gc);


				common::uint8_t ReturnAppType();

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
