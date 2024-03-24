#ifndef __OS__SCRIPT_H
#define __OS__SCRIPT_H

#include <common/types.h>
#include <cli.h>
#include <filesys/ofs.h>


void AyumuScriptInput(os::CommandLine* cli, 
		os::common::uint8_t* file, os::common::uint32_t size, 
		os::common::uint32_t &indexF, 
		os::common::uint32_t* nestedLoop, os::common::uint32_t &indexLoopF);


void AyumuScriptCli(char* name, os::CommandLine* cli);


#endif
