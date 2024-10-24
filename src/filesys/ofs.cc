#include <filesys/ofs.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;


void printf(char*);
bool strcmp(char* one, char* two);
char* int2str(uint32_t);



//32 bit FNV-1a hash
//used for deciding file location and finding
uint32_t os::filesystem::fnv1a(char* str) {

		uint32_t hash = 0x811c9dc5;

		for (int i = 0; str[i] != '\0'; i++) {
	
			hash ^= str[i];
			hash *= 0x01000193;
		}

		//hash within sectors available on disk
		//hahahahahahahahahahahahahaha
		return (hash % 4096) + 1024;
}



FileSystem::FileSystem(AdvancedTechnologyAttachment* ata0m) {

	this->ata0m = ata0m;
}

FileSystem::~FileSystem() {
}



bool FileSystem::FileIf(uint32_t sector) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);

	uint8_t file[4];
	ata0m->Read28(sector, file, 4, 0);

	bool fileExists = file[0] == 0xf0 && file[1] == 0x10 && 
			  file[2] == 0x70 && file[3] == 0xe0; 

	return file[0] != 0x00;
}



uint32_t FileSystem::GetFileSize(char* name) {
		
	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint32_t size = 0;
	uint32_t location = fnv1a(name);

	if (FileIf(location)) {
	
		uint8_t data[8];
		ata0m->Read28(location, data, 8, 0);

		size = (data[7] << 24) | (data[6] << 16) | 
			(data[5] << 8) | data[4];
	}
	return size;
}



uint32_t FileSystem::AddTable(char* name, uint32_t location) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);

	//+1 file added to system
	uint32_t fileNum = 0;
	uint8_t numOfFiles[512];
	ata0m->Read28(tableStartSector, numOfFiles, 512, 0);

	int i = 0;
	while (i < 256) {
	
		//first half for file num
		fileNum += numOfFiles[i];

		if (numOfFiles[i] < 0xff) {
		
			numOfFiles[i]++;
			break;
		}
		i++;
	}
	ata0m->Write28(tableStartSector, numOfFiles, 512, 0);
	ata0m->Flush();
	
	//fileNum++;
	

	uint8_t sectorData[512];
	uint16_t index = (fileNum%128)*4;
	
	ata0m->Read28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	sectorData[index] = (location & 0xffffffff)   >> 24;
	sectorData[index+1] = (location & 0xffffff)     >> 16; 
	sectorData[index+2] = (location & 0xffff)       >> 8; 
	sectorData[index+3] = (location & 0xff); 

	ata0m->Write28(fileStartSector+(fileNum/128), sectorData, 512, 0);
	ata0m->Flush();

	return fileNum+1;
}




uint32_t FileSystem::RemoveTable(char* name) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	
	//1 file removed from system
	uint32_t fileNum = 0;
	uint8_t numOfFiles[512];
	ata0m->Read28(tableStartSector, numOfFiles, 512, 0);

	int i = 0;
	while (i < 256) {
	
		//first half for file num
		fileNum += numOfFiles[i];

		if (numOfFiles[i+1] == 0x00) {
		
			numOfFiles[i]--;
			break;
		}
		i++;
	}
	ata0m->Write28(tableStartSector, numOfFiles, 512, 0);
	ata0m->Flush();
	
	fileNum--;
	
	uint8_t sectorData[512];
	uint32_t location = fnv1a(name);
	uint16_t index = (fileNum%128)*4;
	
	//check if removing newest file or not
	ata0m->Read28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	//newest file allocated	
	uint32_t newLocation =  (sectorData[index] << 24) |
				(sectorData[index+1] << 16) | 
				(sectorData[index+2] << 8) | 
				(sectorData[index+3]);
	
	//removing newest file
	for (int j = 0; j < 4; j++) { sectorData[index+j] = 0; }
	ata0m->Write28(fileStartSector+(fileNum/128), sectorData, 512, 0);
	ata0m->Flush();
		


	if (fileNum > 0 || newLocation != location) {
		
		//for each sector
		for (int j = 0; j <= (fileNum/128); j++) {
	
			ata0m->Read28(fileStartSector+j, sectorData, 512, 0);
			
			//go through sector
			for (int k = 0; k < 512; k += 4) {
			
				uint32_t findLocation =  (sectorData[k] << 24) |
							 (sectorData[k+1] << 16) | 
							 (sectorData[k+2] << 8) | 
							 (sectorData[k+3]);

				if (location == findLocation) {
				
					sectorData[k] = (newLocation & 0xffffffff)   >> 24;
					sectorData[k+1] = (newLocation & 0xffffff)     >> 16; 
					sectorData[k+2] = (newLocation & 0xffff)       >> 8; 
					sectorData[k+3] = (newLocation & 0xff); 
					ata0m->Write28(fileStartSector+j, sectorData, 512, 0);
					ata0m->Flush();
				
					return 0;
				}
			}
		}

	}
	return 0;
}



uint32_t FileSystem::GetFileCount() {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint8_t numOfFiles[512];
	uint32_t fileNum = 0;
	
	ata0m->Read28(tableStartSector, numOfFiles, 512, 0);

	for (int i = 0; i < 256; i++) {
	
		fileNum += numOfFiles[i];
	}
	return fileNum;
}



//store name in string, return location
uint32_t FileSystem::GetFileName(uint16_t fileNum, char fileName[33]) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint8_t sectorData[512];
	uint32_t location = 0;
	uint16_t index = (fileNum%128)*4;
	
	ata0m->Read28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	location = (sectorData[index] << 24) |
		   (sectorData[index+1] << 16) | 
		   (sectorData[index+2] << 8) | 
		   (sectorData[index+3]);
	
	if (location) {
		
		ata0m->Read28(location, sectorData, 72, 0);
		int i = 0;
		
		//name
		for (i = 0; (sectorData[i+8] != '\0' || i < 32); i++) {

			fileName[i] = (char)(sectorData[i+8]);
			
		}
		fileName[i] = '\0';
	} else {
		fileName[0] = '\0';
	}
	return location;
}


//return tag string stored in char tag[33]
uint32_t FileSystem::GetFileTag(char* fileName, uint8_t tagNum, char tag[33]) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint8_t sectorData[512];
	uint32_t location = fnv1a(fileName);
	

	if (location && tagNum < 8) {
		
		ata0m->Read28(location, sectorData, 512, 0);
		uint8_t offset = (tagNum * 32);
		int i = 0;
		
		//tags
		for (i = 0; (sectorData[256+offset+i] != '\0' && i < 32); i++) {

			tag[i] = (char)(sectorData[256+offset+i]);
		}
		tag[i] = '\0';
	} else {
		tag[0] = '\0';
	}
	return location;
}


//return file location if it has given tag
uint32_t FileSystem::GetTagFile(char* tagName, uint32_t location, uint8_t* tagNum) {

	char tag[33];
	uint8_t sectorData[512];

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	ata0m->Read28(location, sectorData, 512, 0);

	//go through tag section of sector	
	for (int i = 256; i < 512; i += 32) {

		//get tag
		int j = 0;
		for (j; j < 32 && sectorData[i+j] != '\0'; j++) {

			tag[j] = (char)(sectorData[i+j]);
		}
		tag[j] = '\0';

		if (strcmp(tagName, tag)) { 
			
			*tagNum = ((i>>5)-8);
			return location; 
		}
	}
	return 0;
}



bool FileSystem::NewFile(char* name, uint8_t* file, uint32_t location, uint32_t size) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	AddTable(name, location);
	//uint32_t location = fnv1a(name);
	
	if (FileIf(location)) {

		//collision lol
		printf("Collision detected, file can't be created.\n");
		return false;
	}

	//OFS FILE STRUCTURE BELOW
	
	//first 8 bytes for file nums and size
	uint8_t sectorData[512];

	//init with zeros
	for (int i = 0; i < 512; i++) { sectorData[i] = 0x00; }


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
	int i = 0;
	for (i; name[i] != '\0'; i++) {
	
		sectorData[i+8] = (uint8_t)name[i];
	}
	sectorData[i+8] = '\0';

	
	
	//byte 124 to 128 for image resolution
	//default is full resolution
	sectorData[124] = 255; sectorData[125] = 65;
	sectorData[126] = 200; sectorData[127] = 0;





	//byte 256 to end for tag strings
	/*
	for (i = 0; tag[i] != '\0'; i++) {
	
		sectorData[i+40] = (uint8_t)tag[i];
	}
	*/



	//END OF OFS FILE STRUCTURE


	//first sector is reserved for file metadata	
	ata0m->Write28(location, sectorData, 512, 0);
	ata0m->Flush();

	uint8_t lbaFile[1920];
	uint32_t lbaNum = (size / 1920) - 1;

	//temp
	WriteLBA(name, file, 0);
	return true;
}




bool FileSystem::NewTag(char* name, uint32_t location) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint16_t tagIndex = 0;
	uint8_t sectorData[512];

	//byte 256 to end for tag strings
	ata0m->Read28(location, sectorData, 512, 0);

	//find new tag location
	for (int i = 256; i < 512; i += 32) {
	
		if (sectorData[i] == 0x00) {
		
			tagIndex = i;
			break;
		}
	}
	
	//can't attach any more tags
	if (tagIndex == 0) { return false; }

	//write new tag
	for (int j = 0; (name[j] != '\0' && name[j] != ' ' && j < 32); j++) {
	
		sectorData[j+tagIndex] = (uint8_t)(name[j]);
	}
	
	
	ata0m->Write28(location, sectorData, 512, 0);
	ata0m->Flush();

	return true;
}



//unfinished
bool FileSystem::DeleteTag(common::uint32_t location, common::uint8_t tagNum) {

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint8_t sectorData[512];
	uint16_t sectorTagIndex = 256+(tagNum*32);

	//byte 256 to end for tag strings
	ata0m->Read28(location, sectorData, 512, 0);

	//remove tag by overwriting with zeros
	for (int i = sectorTagIndex; i < sectorTagIndex+32; i++) { 
		
		sectorData[i] = 0x00; 
	}

	ata0m->Write28(location, sectorData, 512, 0);
	ata0m->Flush();

	return true;
}






bool FileSystem::WriteFile(char* name, uint8_t* file, uint32_t size) {

	uint32_t location = fnv1a(name);
	
	if (FileIf(location) == false) {
	
		printf("write what?\n");
		return false;
	}

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	location++;

	uint8_t sectorData[512];
	uint32_t sectorNum = (size / 512) + 1;
	uint32_t dataIndex = 0;



	for (uint16_t i = 0; i < sectorNum; i++) {
		
		for (uint16_t j = 0; j < 512; j++) {
		
			sectorData[j] = file[dataIndex];
			dataIndex++;
		}

		ata0m->Write28(location+i, sectorData, 512, 0);
		ata0m->Flush();
	}	
	
	
	//update size
	if (GetFileSize(name) < size) {
	
		ata0m->Read28(location, sectorData, 512, 0);
		sectorData[4] = (size)       & 0xff;
		sectorData[5] = (size >> 8)  & 0xff;
		sectorData[6] = (size >> 16) & 0xff;
		sectorData[7] = (size >> 24);
	
		ata0m->Write28(location, sectorData, 512, 0);
		ata0m->Flush();	
	}
	return true;
}



bool FileSystem::WriteLBA(char* name, uint8_t* file, uint32_t lba) {

	uint32_t size = 1920 * (lba + 1);
	uint32_t location = fnv1a(name);

	if (FileIf(location) == false) {
	
		printf("write what?\n");
		return false;
	}

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
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
	ata0m->Read28(startSector, sectorData, 512, 0);


	for (uint16_t i = 0; i < sectorNum; i++) {

		if (i == (sectorNum - 1) && (lba % 4 != 3)) {
		
			//read higher data	
			ata0m->Read28(startSector+i, sectorData, 512, 0);
			upperOffset = size % 512;
		}

		for (uint16_t j = lowerOffset; j < upperOffset; j++) {
		
			sectorData[j] = file[dataIndex];
			dataIndex++;
		}
		//ata0m->Write28(startSector+i, sectorData, upperOffset, lowerOffset);
		ata0m->Write28(startSector+i, sectorData, 512, 0);
		ata0m->Flush();
		
		lowerOffset = 0;
	}	


	//update size
	if (GetFileSize(name) < size) {
	
		ata0m->Read28(location, sectorData, 512, 0);
		sectorData[4] = (size)       & 0xff;
		sectorData[5] = (size >> 8)  & 0xff;
		sectorData[6] = (size >> 16) & 0xff;
		sectorData[7] = (size >> 24);
	
		ata0m->Write28(location, sectorData, 512, 0);
		ata0m->Flush();	
	}
	return true;
}





bool FileSystem::ReadFile(char* name, uint8_t* file, uint32_t size) {

	uint32_t location = fnv1a(name);

	if (FileIf(location) == false) {
	
		printf("read what?\n");
		return false;
	}

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	uint32_t startSector = location + 1;
	uint8_t sectorData[512];


	uint32_t sectorNum = (size / 512) + 1;
	uint8_t headerOffset = 8;
	uint32_t dataIndex = 0;


		
	for (uint16_t i = 0; i < sectorNum; i++) {
		
		ata0m->Read28(startSector+i, sectorData, 512, 0);
		
		for (uint16_t j = headerOffset; j < 512; j++) {

			file[dataIndex] = sectorData[j];
			dataIndex++;
		}
		
		headerOffset = 0;
	}
	return true;
}



bool FileSystem::ReadLBA(char* name, uint8_t* file, uint32_t lba) {
	
	uint32_t size = 1920 * (lba + 1);	
	uint32_t location = fnv1a(name);

	if (FileIf(location) == false) {
	
		for (int i = 0; i < 1920; i++) {
		
			file[i] = 0x00;
		}
		
		return false;
	}

	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
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
	
	
	for (uint16_t i = 0; i < sectorNum; i++) {
		
		//if (i == (sectorNum - 1) && (lba % 4) != 3) {
		if (i == (sectorNum - 1) && (lba % 4) != 3) {
		
			upperOffset = size % 512;
		}

		ata0m->Read28(startSector+i, sectorData, upperOffset, lowerOffset);
	
		for (uint16_t j = lowerOffset; j < upperOffset; j++) {
			
			file[dataIndex] = sectorData[j];
			dataIndex++;
		}
		lowerOffset = 0;
	}
	return true;
}





bool FileSystem::DeleteFile(char* name) {
	
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
	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	
	ata0m->Write28(location, zeros, 512, 0);
	ata0m->Flush();
	
	return true;
}



bool FileSystem::Compress(char* name, uint8_t* buffer, uint32_t bufsize) {
}
bool FileSystem::Decompress(char* name, uint8_t* buffer, uint32_t bufsize) {
}



uint32_t FileSystem::GetImageResolution(char* name) {

	uint8_t data[512];
	//AdvancedTechnologyAttachment ata0m->0x1F0, true);
	ata0m->Read28(fnv1a(name), data, 256, 0);
	
	uint16_t width = data[124] + data[125];
	uint16_t height = data[126] + data[127];

	return (width << 16) | height;
}




//read and write VGA mode 13H buffer 
//from disk to memory and vice versa
bool FileSystem::Read13H(char* name, uint8_t* buffer, uint16_t* retWidth, uint8_t* retHeight) {

	if (FileIf(fnv1a(name)) == false) { return false; }

	uint8_t file[1920];
	uint16_t x = 0;
	uint8_t y = 0;
	uint32_t lba = 0;

	//retrieve and save res data
	uint32_t res = GetImageResolution(name);
	uint16_t width = res >> 16;
	uint16_t height = res & 0xffff;

	*retWidth = width;
	*retHeight = height;

	//read file and write buffer
	for (lba; lba < ((width*height)/1920)+1; lba++) {

		ReadLBA(name, file, lba);

		for (uint16_t j = 0; j < 1920; j++) {
	
			//leave if buffer already full	
			if ((lba*1920)+j >= width*height) { return true; }

			buffer[width*y+x] = file[j];
			x++;

			if (x >= width) {

				y++;
				x = 0;
			}
		}
	}
	return true;
}


bool FileSystem::Write13H(char* name, uint8_t* buffer, uint16_t width, uint16_t height) {

	uint32_t lba = 0;
	uint32_t size = width*height;
	uint8_t file[1920];
	for (uint32_t i = 0; i < 1920; i++) { file[i] = 0x00; }

	//make new file if it doesn't exist
	if (FileIf(fnv1a(name)) == false) {

		NewFile(name, file, fnv1a(name), GetFileSize(name));
		NewTag("img", fnv1a(name));
	}

	//write width and height of image
	if (width < 320 && height < 200) {
	
		uint8_t data[512];
		//AdvancedTechnologyAttachment ata0m->0x1F0, true);
		ata0m->Read28(fnv1a(name), data, 512, 0);


		if (width < 256) {
		
			data[124] = width; 
			data[125] = 0;
		} else {
			data[124] = 255; 
			data[125] = width % 256;
		}
		
		data[126] = height; 
		data[127] = 0;
		
		
		ata0m->Write28(fnv1a(name), data, 512, 0);
		ata0m->Flush();
	}


	//read buffer and write file
	for (lba; lba < (size/1920)+1; lba++) {
		
		for (uint16_t j = 0; j < 1920; j++) {
		
			if ((lba*1920)+j < size) { file[j] = buffer[(lba*1920)+j];
			} else { 		   file[j] = 0x00; }
		}
		WriteLBA(name, file, lba);
	}
	return true;
}
