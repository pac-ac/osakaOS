#ifndef __OS__FUNCTIONTYPES_H
#define __OS__FUNCTIONTYPES_H

#include <common/types.h>
#include <filesys/ofs.h>
#include <string.h>
#include <math.h>
#include <list.h>
#include <tree.h>


#define TYPE_NULL 0
#define TYPE_INT 1
#define TYPE_BOOL 2
#define TYPE_FLOAT 3
#define TYPE_STRING 4
#define TYPE_FUNCTION 5

#define LITERAL_INT 6
#define LITERAL_BOOL 7
#define LITERAL_FLOAT 8
#define LITERAL_STRING 9


namespace os {

	class FunctionType;
	
	//usually unique for every 
	//function call so needs 
	//its own struct
	struct CallStackUnit {

		common::uint32_t startIndex;
		common::uint32_t returnIndex;
		List* args;
	};

	
	class Type {

		public:
			common::uint8_t type;
			
			//default types
			common::int64_t typeInt = 0;
			float typeFloat = 0.0;
			char* typeStr = nullptr;
			
			//custom types as functions
			FunctionType* typeFunc = nullptr;
		public:
			Type(common::uint8_t type, void* initVal);
			~Type();
	};
	
	class FunctionType {
	
		public:
			char file[32];
			common::uint32_t funcStartIndex = 0;
			common::uint32_t numOfArgs = 0;

			common::uint8_t* buf = nullptr;
			common::uint32_t bufSize = 0;
			List* args = nullptr;
		public:
			FunctionType(char* file, common::uint32_t funcStartIndex, common::uint32_t numOfArgs);
			~FunctionType();
			
			common::uint32_t LoadFunction(filesystem::FileSystem* filesystem);
	};
}



#endif
