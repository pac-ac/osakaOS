#ifndef __OS__FILESYS__OFS_H
#define __OS__FILESYS__OFS_H

#include <common/types.h>
#include <drivers/ata.h>



#define tableStartSector 512
#define tagStartSector 513
#define fileStartSector 513


namespace os {

	namespace filesystem {

		struct OFS_Table {
		
			common::uint32_t fileCount = 0;

		} __attribute__((packed));

		common::uint32_t fnv1a(char*);

		class FileSystem {
		
			public:
				drivers::AdvancedTechnologyAttachment* ata0m;
			
			public:
				FileSystem(drivers::AdvancedTechnologyAttachment* ata0m);
				~FileSystem();

				bool FileIf(common::uint32_t sector);

		
				common::uint32_t AddTable(char* name, common::uint32_t location);
				common::uint32_t RemoveTable(char* name);
		
				common::uint32_t GetFileCount();
		
				common::uint32_t GetFileSize(char* name);
				common::uint32_t GetFileName(common::uint16_t fileNum, char fileName[33]);
		
				common::uint32_t GetFileTag(char* file, common::uint8_t tagNum, char tag[33]);
				common::uint32_t GetTagFile(char* tagName, common::uint32_t location, common::uint8_t* tagNum);

				bool NewFile(char* name, common::uint8_t* file, 
						 	common::uint32_t location, 
						 	common::uint32_t size);
				bool DeleteFile(char* name);

				bool WriteFile(char* name, common::uint8_t* file, 
						   	common::uint32_t size);
				bool WriteLBA(char* name, common::uint8_t* file, 
						  	common::uint32_t lba);
		
				bool ReadFile(char* name, common::uint8_t* file,	
						  	common::uint32_t size);
				bool ReadLBA(char* name, common::uint8_t* file,	
							  common::uint32_t lba);
		
				bool NewTag(char* tagName, common::uint32_t location);
				bool DeleteTag(common::uint32_t location, common::uint8_t tagNum);
		
				bool Compress(char* name, common::uint8_t* buffer, common::uint32_t bufsize);
				bool Decompress(char* name, common::uint8_t* buffer, common::uint32_t bufsize);

				common::uint32_t GetImageResolution(char* name);

				bool Read13H(char* name, common::uint8_t* buffer, 
						common::uint16_t* retWidth, common::uint8_t* retHeight);
				bool Write13H(char* name, common::uint8_t* buffer, 
						common::uint16_t width, common::uint16_t height);
		
		};
	}
}



#endif
