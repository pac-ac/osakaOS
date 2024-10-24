#ifndef __OS__APP__FILE_BROWSE_H
#define __OS__APP__FILE_BROWSE_H

#include <common/types.h>
#include <gui/window.h>
#include <drivers/ata.h>
#include <filesys/ofs.h>


namespace os {

	class FileBrowse : public App {
	
		public:
			
		public:
			FileBrowse();
			~FileBrowse();

			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
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
