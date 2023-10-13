#include <filesys/ofs.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;


void printf(char*);
char* int2str(uint32_t);

//not finished yet

bool os::filesystem::FileIf(uint32_t sector) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);

	uint8_t file[4];
	ata0m.Read28(sector, file, 4, 0);

	bool fileExists = file[0] == 0xf0 && file[1] == 0x10 && 
			  file[2] == 0x70 && file[3] == 0xe0; 

	
	return file[0] != 0x00;
}



//32 bit FNV-1a hash
uint32_t os::filesystem::fnv1a(char* str) {

	uint32_t hash = 0x811c9dc5;


	for (int i = 0; str[i] != '\0'; i++) {
	
		hash ^= str[i];
		hash *= 0x01000193;
	}


	//hash within sectors available on disk
	return (hash % 2048) + 1024;
}



uint32_t os::filesystem::GetFileSize(char* name) {
		
	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	uint32_t size = 0;
	uint32_t location = fnv1a(name);

	if (FileIf(location)) {
	
		uint8_t data[8];
		ata0m.Read28(location, data, 8, 0);

		size = (data[7] << 24) | (data[6] << 16) | 
			(data[5] << 8) | data[4];
	}

	return size;
}




uint32_t os::filesystem::AddTable(char* name) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);

	//+1 file added to system
	uint32_t fileNum = 0;
	uint8_t numOfFiles[512];
	ata0m.Read28(512, numOfFiles, 512, 0);

	int i = 0;
	while (i < 512) {
	
		fileNum += numOfFiles[i];

		if (numOfFiles[i] < 0xff) {
		
			numOfFiles[i]++;
			break;
		}	
		i++;
	}
	ata0m.Write28(512, numOfFiles, 512, 0);
	ata0m.Flush();
	
	fileNum++;
	
	
	uint8_t sectorData[4];
	uint32_t location = fnv1a(name);
	
	if (fileNum) {
		
		sectorData[0] = (location & 0xffffffff)   >> 24;
		sectorData[1] = (location & 0xffffff)     >> 16; 
		sectorData[2] = (location & 0xffff)       >> 8; 
		sectorData[3] = (location & 0xff); 

		ata0m.Write28(512+fileNum, sectorData, 4, 0);
		ata0m.Flush();
	}

	return fileNum;
}




uint32_t os::filesystem::RemoveTable(char* name) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	
	//1 file removed from system
	uint32_t fileNum = 0;
	uint8_t numOfFiles[512];
	ata0m.Read28(512, numOfFiles, 512, 0);

	int i = 0;
	while (i < 512) {
	
		fileNum += numOfFiles[i];

		if (numOfFiles[i] == 0x00) {
		
			numOfFiles[i-1]--;
			break;
		}	
		i++;
	}
	ata0m.Write28(512, numOfFiles, 512, 0);
	ata0m.Flush();
	
	//fileNum--;
	
	
	uint8_t sectorData[4];
	uint32_t location = fnv1a(name);
	
	uint8_t nullData[4];
	for (i = 0; i < 4; i++) { nullData[0] = 0x00; }
	
	if (fileNum) {
		
		uint32_t findLocation = 0;
		uint32_t replace = 0;
		int j;

		for (j = 0; j < fileNum; j++) {
		
			ata0m.Read28(513+j, sectorData, 4, 0);
			
			findLocation = (sectorData[0] << 24) |
				       (sectorData[1] << 16) | 
				       (sectorData[2] << 8) | 
				       (sectorData[3]);
		
			if (location == findLocation) {
			
				//ata0m.Write28(513+j, nullptr, 4, 0);
				ata0m.Write28(513+j, nullData, 4, 0);
				ata0m.Flush();
			
				replace = j;
				j = fileNum;
			}
		}

		ata0m.Read28(513+fileNum, sectorData, 4, 0);

		//get rid of newest
		//ata0m.Write28(513+fileNum, nullptr, 4, 0);
		ata0m.Write28(513+fileNum, nullData, 4, 0);
		ata0m.Flush();

		//put it where the removed was
		ata0m.Write28(513+replace, sectorData, 4, 0);
		ata0m.Flush();
	}
	
	return 0;
}









 







bool os::filesystem::NewFile(char* name, uint8_t* file, uint32_t size) {

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	AddTable(name);
	uint32_t location = fnv1a(name);
	
	if (FileIf(location)) {

		//collision lol
		return false;
	}

	
	//first 8 bytes for file nums and size
	uint8_t sectorData[512];

	//start of file
	sectorData[0] = 0xf0;
	sectorData[1] = 0x10;
	sectorData[2] = 0x70;
	sectorData[3] = 0xe0;
	
	//file size in 32 bits
	sectorData[4] = (size)       & 0xff;
	sectorData[5] = (size >> 8)  & 0xff;
	sectorData[6] = (size >> 16) & 0xff;
	sectorData[7] = (size >> 24);

	//byte 8 to 40 for file name
	for (int i = 0; name[i] != '\0'; i++) {
	
		sectorData[i+8] = (uint8_t)name[i];
	}



	//first sector is reserved for file metadata	
	ata0m.Write28(location, sectorData, 512, 0);
	ata0m.Flush();


	uint8_t lbaFile[1920];
	uint32_t lbaNum = (size / 1920) - 1;


	//temp
	WriteLBA(name, file, 0);
	return true;
}




bool os::filesystem::WriteFile(char* name, uint8_t* file, uint32_t size) {

	uint32_t location = fnv1a(name);
	
	if (FileIf(location) == false) {
	
		printf("write what?\n");
		return false;
	}

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	location++;
	
	uint8_t sectorData[512];
	uint32_t sectorNum = (size / 512) + 1;
	uint32_t dataIndex = 0;



	for (uint16_t i = 0; i < sectorNum; i++) {
		
		for (uint16_t j = 0; j < 512; j++) {
		
			sectorData[j] = file[dataIndex];
			dataIndex++;
		}

		ata0m.Write28(location+i, sectorData, 512, 0);
		ata0m.Flush();
	}	
	
	
	//update size
	if (GetFileSize(name) < size) {
	
		ata0m.Read28(location, sectorData, 512, 0);
		sectorData[4] = (size)       & 0xff;
		sectorData[5] = (size >> 8)  & 0xff;
		sectorData[6] = (size >> 16) & 0xff;
		sectorData[7] = (size >> 24);
	
		ata0m.Write28(location, sectorData, 512, 0);
		ata0m.Flush();	
	}
	
	return true;
}




bool os::filesystem::WriteLBA(char* name, uint8_t* file, uint32_t lba) {

	uint32_t size = 1920 * (lba + 1);
	uint32_t location = fnv1a(name);

	if (FileIf(location) == false) {
	
		printf("write what?\n");
		return false;
	}

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	uint32_t startSector = location + 1 + ((size - 1920) / 512);
	uint8_t sectorData[512];
	uint32_t sectorNum = 4;


	//fix later, end of lba 3 doesn't work
	switch (lba % 4) {
	
		case 0:
			break;
		case 1:
			sectorNum++;
			break;
		case 2:
			sectorNum++;
			break;
		case 3:
			break;
	}



	uint32_t upperOffset = 512;
	uint32_t lowerOffset = (size - 1920) % 512;
	uint32_t dataIndex = 0;
			
	
	//read lower data	
	ata0m.Read28(startSector, sectorData, 512, 0);


	for (uint16_t i = 0; i < sectorNum; i++) {

		if (i == (sectorNum - 1)) {
		
			//read higher data	
			ata0m.Read28(startSector+i, sectorData, 512, 0);
			upperOffset = size % 512;
		}

		for (uint16_t j = lowerOffset; j < upperOffset; j++) {
		
			sectorData[j] = file[dataIndex];
			dataIndex++;
		}
		//ata0m.Write28(startSector+i, sectorData, upperOffset, lowerOffset);
		ata0m.Write28(startSector+i, sectorData, 512, 0);
		ata0m.Flush();
		
		lowerOffset = 0;
	}	


	//update size
	if (GetFileSize(name) < size) {
	
		ata0m.Read28(location, sectorData, 512, 0);
		sectorData[4] = (size)       & 0xff;
		sectorData[5] = (size >> 8)  & 0xff;
		sectorData[6] = (size >> 16) & 0xff;
		sectorData[7] = (size >> 24);
	
		ata0m.Write28(location, sectorData, 512, 0);
		ata0m.Flush();	
	}
	return true;
}





bool os::filesystem::ReadFile(char* name, uint8_t* file, uint32_t size) {

	uint32_t location = fnv1a(name);

	if (FileIf(location) == false) {
	
		printf("read what?\n");
		return false;
	}

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	uint32_t startSector = location + 1;
	uint8_t sectorData[512];


	uint32_t sectorNum = (size / 512) + 1;
	uint8_t headerOffset = 8;
	uint32_t dataIndex = 0;


		
	for (uint16_t i = 0; i < sectorNum; i++) {
		
		ata0m.Read28(startSector+i, sectorData, 512, 0);
		
		for (uint16_t j = headerOffset; j < 512; j++) {

			file[dataIndex] = sectorData[j];
			dataIndex++;
		}
		
		headerOffset = 0;
	}

	return true;
}



bool os::filesystem::ReadLBA(char* name, uint8_t* file, uint32_t lba) {
	
	uint32_t size = 1920 * (lba + 1);	
	uint32_t location = fnv1a(name);

	if (FileIf(location) == false) {
	
		for (int i = 0; i < 1920; i++) {
		
			file[i] = 0x00;
		}
		
		return false;
	}

	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	uint32_t startSector = location + 1 + ((size - 1920) / 512);
	uint8_t sectorData[512];
	uint32_t sectorNum = 4;


	//fix later, end of lba 3 doesn't work
	switch (lba % 4) {
		
		case 0:
			break;
		case 1:
			sectorNum++;
			break;
		case 2:
			sectorNum++;
			//startSector++;
			break;
		case 3:
			//startSector++;
			break;
	}


	uint32_t upperOffset = 512;
	uint32_t lowerOffset = (size - 1920) % 512;
	uint32_t dataIndex = 0;
	
	
	for (uint16_t i = 0; i < sectorNum; i++) {
		
		if (i == (sectorNum - 1)) {
		
			upperOffset = size % 512;
		}

		ata0m.Read28(startSector+i, sectorData, upperOffset, lowerOffset);
	
		for (uint16_t j = lowerOffset; j < upperOffset; j++) {
			
			file[dataIndex] = sectorData[j];
			dataIndex++;
		}
		lowerOffset = 0;
	}

	return true;
}





bool os::filesystem::DeleteFile(char* name) {
	
	uint32_t location = fnv1a(name);
	uint32_t size = GetFileSize(name);
	
	if (FileIf(location) == false) {

		return false;
	}
	
	//remove from table
	RemoveTable(name);
	
	//delete actual file data	
	uint8_t zeros[1920];
	for (int i = 0; i < 1920; i++) { zeros[i] = 0x00; }

	for (uint32_t i = 0; i < (size/1920); i++) {
	
		WriteLBA(name, zeros, i);
	}

	//get rid of magic numbers + other metadata
	AdvancedTechnologyAttachment ata0m(0x1F0, true);
	
	ata0m.Write28(location, zeros, 512, 0);
	ata0m.Flush();

	return true;
}

