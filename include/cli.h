#ifndef __OS__CLI_H
#define __OS__CLI_H


#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/speaker.h>
#include <drivers/ata.h>
#include <drivers/cmos.h>
#include <drivers/vga.h>
#include <filesys/ofs.h>
#include <gui/window.h>
#include <net/network.h>
#include <multitasking.h>
#include <code/asm.h>
#include <string.h>
#include <memorymanagement.h>
#include <functiontypes.h>
#include <list.h>
#include <gdt.h>
#include <art.h>
#include <app.h>


#define CMD_TABLE_SIZE 65536
#define VAR_TABLE_SIZE 1024
#define ARG_TABLE_SIZE 10


namespace os {

	class CommandLine : public App {

		public:
			//input and other things
			common::uint8_t index = 0;
			char input[256];
			char lastCmd[256];
			common::uint8_t keyChar = 0;
			bool init = false;

			//hash table for all commands
			//(table for all command lines)
			static void (*cmdTable[CMD_TABLE_SIZE])(char*, CommandLine*);
			static bool WakeupInit;

			//script data	
			char scriptName[32];
			bool scriptRunning = false;
			bool scriptKillSwitch = false;
			Type* returnVar = nullptr;
			
			//common::uint32_t varTable[1024];
			//common::uint32_t argTable[10];
			
			Type* varTable[VAR_TABLE_SIZE];
			Type* argTable[ARG_TABLE_SIZE];
			
			//list of lists
			List* lists;
	
			//script input (cin)
			common::uint8_t scriptInputIndex = 0;	
			char scriptInput[256];
			bool getCurrentInput = false;

			//function
			List* callStack;
			common::uint32_t setStartIndex = 0;
			common::uint32_t setReturnIndex = 0;

			//bools
			bool conditionIf = true;
			bool conditionLoop = true;
			common::int32_t offsetVal = 0;
		
			//mouse/key input
			common::int32_t MouseX = 0;
			common::int32_t MouseY = 0;
			char Key = ' ';

			//other things
			common::uint16_t cmdIndex = 0;;
			common::uint16_t cliMode = 0;
			bool mute = false;

			//gui stuff
			common::uint8_t* backgroundBuffer = nullptr;
			bool gui = true;
			bool targetWindow = false;
			gui::CompositeWidget* appWindow = nullptr;
			gui::CompositeWidget* userWindow = nullptr;

			//network stuff
			net::UserDatagramProtocolSocket* udpSocket = nullptr;
			net::TransmissionControlProtocolSocket* tcpSocket = nullptr;
			bool downloadExternalFile = false;
			char externalFile[32];
			common::uint16_t webPort = 80;

			//binary code
			common::uint8_t code[OFS_BLOCK_SIZE];

			//drivers and hardware
			GlobalDescriptorTable* gdt;
			TaskManager* tm;
			Compiler* compiler;
			Task* userTask = nullptr;
			MemoryManager* mm;
			drivers::DriverManager* drvManager;
			filesystem::FileSystem* filesystem;
			drivers::VideoGraphicsArray* vga;
			drivers::CMOS* cmos = 0;
			net::Network* network = 0;
		public:
			CommandLine(GlobalDescriptorTable* gdt, 
					TaskManager* tm, 
					MemoryManager* mm,
					filesystem::FileSystem* filesystem,
					net::Network* network,
					Compiler* compiler,
					drivers::VideoGraphicsArray* vga,
					drivers::CMOS* cmos,
					drivers::DriverManager* drvManager);
			~CommandLine();

			char* commandStr;
			char* command(char *cmd, common::uint8_t length);
			
			void hash_add(char* cmd, void func(char*, CommandLine*));
			void hash_cli_init();

			
			void ScriptMouseDown();
			void ScriptKeyDown();
	
			void SaveOutput(char* filename, gui::CompositeWidget* widget, 
					filesystem::FileSystem* filesystem);	
			void ReadInput(char* filename, gui::CompositeWidget* widget, 
					filesystem::FileSystem* filesystem);	

			void PrintCommand(char* str, common::uint16_t color = 0, common::uint8_t flags = 0);

			void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			void CreateTaskForScript(char* fileName);
			void DeleteTaskForScript();
			void CleanCommandLine();
	
			void OnKeyDown(char ch, gui::CompositeWidget* widget);
			void OnKeyUp(char ch, gui::CompositeWidget* widget);
			
			void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
	};
}


#endif
