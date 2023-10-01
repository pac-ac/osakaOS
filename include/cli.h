#ifndef __OS__CLI_H
#define __OS__CLI_H

#include <common/types.h>
#include <multitasking.h>
#include <gdt.h>


namespace os {

	class CommandLine {

		
		//private:
		public:
			void (*cmdTable[65536])(char*, CommandLine*);	
			common::uint32_t varTable[1024];
			bool conditionIf;
			bool conditionLoop;
			common::uint32_t returnVal = 0;	


			common::uint16_t cmdList[65536];
			common::uint16_t cmdIndex = 0;;


			GlobalDescriptorTable* cli_gdt;
			TaskManager* cli_tm;

			char* scriptFile;
			common::uint16_t cliMode = 0;
		
		public:
			char* commandStr;
			char* command(char *cmd, common::uint8_t length);
			
			void hash_add(char* cmd, void func(char*, CommandLine*));
			void hash_cli_init();


			void script(bool pressed, char key, bool ctrl);
			void getTM(GlobalDescriptorTable* gdt, TaskManager* tm);
	};
}


#endif
