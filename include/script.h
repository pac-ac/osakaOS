#ifndef SCRIPT_H
#define SCRIPT_H

#include <common/types.h>
#include <cli.h>
#include <filesys/ofs.h>
/*
namespace os {

	void AyumuScript(CommandLine* cli, 
			common::uint8_t* file, common::uint32_t size, 
			common::uint32_t &indexF, common::uint32_t &indexLoopF);
	//void AyumuScript(char* name, os::CommandLine* cli);

}
*/

void AyumuScriptInput(os::CommandLine* cli, 
		os::common::uint8_t* file, os::common::uint32_t size, 
		os::common::uint32_t &indexF, 
		os::common::uint32_t* nestedLoop, os::common::uint32_t &indexLoopF);
	
void AyumuScriptCli(char* name, os::CommandLine* cli);


#endif
