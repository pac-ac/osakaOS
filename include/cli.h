#ifndef __OS__CLI_H
#define __OS__CLI_H


#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/speaker.h>
#include <drivers/ata.h>
#include <drivers/cmos.h>
#include <filesys/ofs.h>
#include <gui/window.h>
#include <net/network.h>
#include <multitasking.h>
#include <memorymanagement.h>
#include <gdt.h>
#include <art.h>
#include <app.h>


namespace os {

	class CommandLine : public App {

		//private:
		public:
			//input and other things
			common::uint8_t index = 0;
			char input[256];
			char lastCmd[256];
			common::uint8_t keyChar = 0;
			bool init = false;

			//hash table for all commands
			//(table for all command lines)
			static void (*cmdTable[65536])(char*, CommandLine*);
			static bool WakeupInit;

			//script data	
			char scriptName[32];
			bool scriptKillSwitch = false;
			common::uint32_t varTable[1024];
			common::uint32_t argTable[10];
			bool conditionIf = true;
			bool conditionLoop = true;
			common::uint32_t returnVal = 0;
			common::uint32_t offsetVal = 0;
		
			//input
			common::int32_t MouseX = 0;
			common::int32_t MouseY = 0;
			char Key = ' ';

			//other things
			common::uint16_t cmdIndex = 0;;
			common::uint16_t cliMode = 0;
			bool mute = false;


			//gui stuff
			bool gui = true;
			bool targetWindow = false;
			gui::CompositeWidget* appWindow = nullptr;
			gui::CompositeWidget* userWindow = nullptr;


			//drivers and hardware
			GlobalDescriptorTable* gdt;
			TaskManager* tm;
			Task* userTask = nullptr;
			MemoryManager* mm;
			drivers::DriverManager* drvManager;
			filesystem::FileSystem* filesystem;
			drivers::CMOS* cmos = 0;
			net::Network* network = 0;
		public:
			CommandLine(GlobalDescriptorTable* gdt, 
					TaskManager* tm, 
					MemoryManager* mm,
					filesystem::FileSystem* filesystem,
					drivers::CMOS* cmos,
					drivers::DriverManager* drvManager);
			~CommandLine();

			char* commandStr;
			char* command(char *cmd, common::uint8_t length);
			
			void hash_add(char* cmd, void func(char*, CommandLine*));
			void hash_cli_init();

			
			void ScriptMouseDown();
			void ScriptKeyDown();
		

			void PrintCommand(char* str, common::uint16_t color = 0);

			void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			void CreateTaskForScript(char* fileName);
			void DeleteTaskForScript(common::uint8_t taskNum);
	
			void OnKeyDown(char ch, gui::CompositeWidget* widget);
			void OnKeyUp(char ch, gui::CompositeWidget* widget);
			
			void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
	};
}


#endif
