#ifndef __OS__SCRIPT_H
#define __OS__SCRIPT_H

#include <common/types.h>
#include <filesys/ofs.h>
#include <gui/widget.h>
#include <cli.h>
#include <app.h>


namespace os {

	class Script : public App {
	
		public:
			bool init = false;
			CommandLine* parentCli;
		public:
			
			Script(CommandLine* cli);
			~Script();
	
			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			virtual void DrawAppMenu(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			
			virtual void Close();
			
			virtual void SaveOutput(char* fileName, gui::CompositeWidget* widget);
			virtual void ReadInput(char* fileName, gui::CompositeWidget* widget);
			
			virtual void OnKeyDown(char ch, gui::CompositeWidget* widget);
			virtual void OnKeyUp(char ch, gui::CompositeWidget* widget);
			
			virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						 common::int32_t newx, common::int32_t newy, 
						 gui::CompositeWidget* widget);
	};
}


void UserScript();
void AyumuScriptCli(char* name, os::CommandLine* cli);


#endif
