#ifndef __OS__APP_H
#define __OS__APP_H

#include <common/types.h>
#include <drivers/vga.h>
#include <gui/widget.h>
#include <filesys/ofs.h>


namespace os {

	class App {

		public:
			common::uint8_t PID;
			common::uint8_t appType;
		public:
			App();
			~App();

			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);

			virtual void DrawAppMenu(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			
			virtual void Close();
			
			virtual void SaveOutput(char* fileName, gui::CompositeWidget* widget, os::filesystem::FileSystem* filesystem);
			virtual void ReadInput(char* fileName, gui::CompositeWidget* widget, os::filesystem::FileSystem* filesystem);

			virtual void OnKeyDown(char ch, gui::CompositeWidget* widget);
			virtual void OnKeyUp(char ch, gui::CompositeWidget* widget);

			virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy,
						gui::CompositeWidget* widget);
	};
}

#endif
