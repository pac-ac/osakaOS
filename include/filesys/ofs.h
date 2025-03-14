#ifndef __OS__FILESYS__OFS_H
#define __OS__FILESYS__OFS_H

#include <common/types.h>
#include <drivers/ata.h>
#include <memorymanagement.h>
#include <list.h>
#include <math.h>


/*
#define tableStartSector 2048
#define tagStartSector 2049
#define fileStartSector 2049
*/


#define tableStartSector 512
#define tagStartSector 513
#define fileStartSector 513

#define OFS_BLOCK_SIZE 2048


namespace os {

	namespace filesystem {

		struct OFS_Table {
		
			common::uint32_t fileCount;
			common::uint32_t currentOpenSector;
			List* files;
			
		} __attribute__((packed));


		class File {

			public:
				common::uint32_t Location;
				common::uint32_t Size;
				char Name[33];
			public:
				File(common::uint32_t location, common::uint32_t size, char name[33]);
				~File();
		};
		
		class FileSystem {
		
			public:
				common::uint32_t newestLocation;
				drivers::AdvancedTechnologyAttachment* ata0m;
				OFS_Table* table;
				MemoryManager* memoryManager;
			public:
				FileSystem(drivers::AdvancedTechnologyAttachment* ata0m, 
						MemoryManager* memoryManager, OFS_Table* table);
				~FileSystem();


				common::uint32_t GetFileSector(char* name);
				common::uint32_t GetFileSectorTable(char* name);
				common::uint32_t GetFragmentFromLBA(common::uint32_t location, common::uint8_t lba);
				bool FileIf(common::uint32_t sector);

		
				common::uint32_t AddTable(char* name, common::uint32_t location);
				common::uint32_t RemoveTable(char* name, common::uint32_t location);
				common::uint32_t UpdateTable();
		
				void UpdateSize(common::uint32_t location, common::uint32_t size);
				common::uint32_t GetFileCount();
		
				common::uint32_t GetFileSize(char* name);
				common::uint32_t SetFileSize(char* name, common::uint32_t size);
				common::uint32_t GetDataSize(char* name);
				common::uint32_t GetFileName(common::uint16_t fileNum, char fileName[33]);
		
				common::uint32_t GetFileTag(char* file, common::uint8_t tagNum, char tag[33]);
				common::uint32_t GetTagFile(char* tagName, common::uint32_t location, common::uint8_t* tagNum);

				bool NewFile(char* name, common::uint8_t* file, common::uint32_t size);
				bool DeleteFile(char* name);

				bool WriteLBA(char* name, common::uint8_t* file, 
						  	common::uint32_t lba);
				bool ReadLBA(char* name, common::uint8_t* file,	
							  common::uint32_t lba);
		
				bool NewTag(char* tagName, common::uint32_t location);
				bool DeleteTag(common::uint32_t location, common::uint8_t tagNum);
		
				void Compress(char* name, common::uint8_t* buffer, common::uint32_t bufsize);
				void Decompress(char* name, common::uint8_t* buffer, common::uint32_t bufsize);

				
				
				void EncryptBlock(common::uint8_t data[16], common::uint8_t key[16]);
				void DecryptBlock(common::uint8_t data[16], common::uint8_t key[16]);
				
				void CryptFile(char* name, common::uint8_t key[16], bool encrypt);

				
				
				common::uint32_t GetImageResolution(char* name);

				bool Read13H(char* name, common::uint8_t* buffer, 
						common::uint16_t* retWidth, common::uint8_t* retHeight, 
						bool compress);
				bool Write13H(char* name, common::uint8_t* buffer, 
						common::uint16_t width, common::uint16_t height, 
						bool compress);
		
		};
	}
}



#endif
