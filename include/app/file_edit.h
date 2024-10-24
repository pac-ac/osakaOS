#ifndef __OS__APP__FILE_EDIT_H
#define __OS__APP__FILE_EDIT_H


#include <common/types.h>
#include <gui/window.h>
#include <filesys/ofs.h>


namespace os {

	class Journal : public App {

		public:
			common::uint32_t fileOffset = 0;
			common::uint16_t numOfBlocks = 0;

			common::uint8_t LBA[1920];
			common::uint8_t LBA2[1920];

			common::uint16_t cursorx = 0;
			common::uint8_t cursory = 0;

			common::uint16_t index = 0;
			common::uint16_t cursor = 0;
			bool init = false;
		public:
			Journal();
			~Journal();

			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			virtual void SaveOutput(char* fileName, gui::CompositeWidget* widget, filesystem::FileSystem* filesystem);
			virtual void ReadInput(char* fileName, gui::CompositeWidget* widget, filesystem::FileSystem* filesystem);

			virtual void DrawTheme(gui::CompositeWidget* widget);

			virtual void DrawAppMenu(common::GraphicsContext* gc, gui::CompositeWidget* widget);


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
