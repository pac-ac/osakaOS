#ifndef __OS__STRING_H
#define __OS__STRING_H


#include <common/types.h>


namespace os {

	common::uint16_t strlen(char* args);
	bool strcmp(char* one, char* two);
	char* strcpy(char* one, char* two);
	
	bool findCharInStr(char ch, char* str);
	
	common::int32_t str2int(char* args);
	char* int2str(common::int32_t num);
	
	float str2float(char* str);
	char* float2str(float val);
	
	common::uint32_t str2ip(char* str);
	char* ip2str(common::uint32_t ip);
	
	char* argparse(char* args, common::int8_t num);
	
	common::uint8_t argcount(char* args);

	common::uint8_t arg1len(char* args);
	
	char* argshift(char* args, common::uint16_t size);
}

#endif
