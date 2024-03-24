#ifndef __OS__CLI_H
#define __OS__CLI_H


#include <common/types.h>
#include <drivers/speaker.h>
#include <drivers/ata.h>
#include <filesys/ofs.h>
#include <multitasking.h>
#include <gdt.h>
#include <art.h>
#include <app.h>


namespace os {

	class CommandLine : public App {

		//private:
		public:
			//command things
			common::uint8_t index = 0;
			char input[256];
			char lastCmd[256];
			common::uint8_t keyChar = 0;
			
			//hash tables and script stuff
			void (*cmdTable[65536])(char*, CommandLine*);	
			common::uint32_t varTable[1024];
			common::uint32_t argTable[10];
			bool conditionIf;
			bool conditionLoop;
			common::uint32_t returnVal = 0;
			common::uint32_t offsetVal = 0;

			common::uint16_t cmdList[65536];
			common::uint16_t cmdIndex = 0;;

			bool mute = false;
			
			char* scriptFile;
			common::uint16_t cliMode = 0;

			//drivers and hardware
			bool desktop = false;
			gui::Widget* appWindow;
			
			GlobalDescriptorTable* gdt;
			TaskManager* tm;
			drivers::AdvancedTechnologyAttachment* ata0m;
		public:
			CommandLine(GlobalDescriptorTable* gdt, 
					TaskManager* tm, 
					drivers::AdvancedTechnologyAttachment* ata0m);
			~CommandLine();

			char* commandStr;
			char* command(char *cmd, common::uint8_t length);
			
			void hash_add(char* cmd, void func(char*, CommandLine*));
			void hash_cli_init();

			void script(bool pressed, char key, bool ctrl);

			void PrintCommand(char* str);

			void ComputeAppState(common::GraphicsContext* gc, gui::Widget* widget);
	
			void OnKeyDown(char ch, gui::Widget* widget);
			void OnKeyUp(char ch, gui::Widget* widget);
			
			void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::Widget* widget);
	};
}


#endif
