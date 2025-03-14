#include <filesys/ofs.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;


void printf(char*);
bool strcmp(char* one, char* two);
uint16_t strlen(char*);
char* int2str(uint32_t);
void sleep(uint32_t);


File::File(uint32_t location, uint32_t size, char name[33]) {

	this->Location = location;
	this->Size = size;

	for (int i = 0; i < 33; i++) { this->Name[i] = name[i]; }
}
File::~File() {}




FileSystem::FileSystem(AdvancedTechnologyAttachment* ata0m, 
		MemoryManager* memoryManager, OFS_Table* table) {

	this->ata0m = ata0m;
	this->table = table;
	this->memoryManager = memoryManager;

	this->table->fileCount = GetFileCount();
	this->table->files = (List*)(this->memoryManager->malloc(sizeof(List)));
	new (this->table->files) List(this->memoryManager);
	
	
	//cache sectors for every file currently allocated
	uint8_t sectorData[512];
	uint32_t fileNum = 0;	
	uint32_t fileSize = 0;
	uint32_t location = 0;
	uint32_t totalSize = 0;
	uint16_t index = 0;

	
	//init table in memory	
	for (uint32_t fileNum = 0; fileNum < this->table->fileCount; fileNum++) {
	
		char fileName[33];
		location = this->GetFileName(fileNum, fileName);
		
		//add to memory table
		File* file = (File*)(this->memoryManager->malloc(sizeof(File)));
		new (file) File(location, OFS_BLOCK_SIZE, fileName);
		this->table->files->Push(file);

		//this uses mem table info so we do it after
		fileSize = this->GetFileSize(fileName);
		file->Size = fileSize;
		totalSize += file->Size;
	}
	this->newestLocation = location;
	
	if (this->table->fileCount > 0) { this->table->currentOpenSector = location + (fileSize / 512) + 1;
	} else { 			  this->table->currentOpenSector = tableStartSector + 512; }
}

FileSystem::~FileSystem() {
}




uint32_t FileSystem::GetFileSectorTable(char* name) {
//uint32_t FileSystem::GetFileSector(char* name) {

	uint32_t location = 0;
	char fileName[33];

	//if file is already here
	for (int i = 0; i < this->table->fileCount; i++) {
	
		location = this->GetFileName(i, fileName);
		if (strcmp(name, fileName)) { return location; }
	}
	//if not allocate new file
	return this->table->currentOpenSector;
}


//query the ofs table
uint32_t FileSystem::GetFileSector(char* name) {
//uint32_t FileSystem::GetFileSectorTable(char* name) {

	uint32_t location = 0;
	
	//if file is already here
	for (int i = 0; i < this->table->fileCount; i++) {
	
		File* readFile = (File*)(this->table->files->Read(i));
		if (strcmp(name, readFile->Name)) { return readFile->Location; }
	}

	//if not allocate new file
	return this->table->currentOpenSector;
}


//return 0 if no fragmentation
//1st is for lba num for when fragment starts
//2nd is for offset of sector where fragments is located	
uint32_t FileSystem::GetFragmentFromLBA(uint32_t location, uint8_t lba) {

	uint8_t sectorData[512];
	ata0m->Read28(location, sectorData, 512, 0);

	if (sectorData[64] == 0) { return location + 1 + ((lba*OFS_BLOCK_SIZE)/512); }


	for (int i = 64; i < 96; i += 2) {
	
		if (lba >= sectorData[i] && lba <= sectorData[i+2]) {
		
			return location + sectorData[i+1];
		}
	}

	return 0;
}



bool FileSystem::FileIf(uint32_t sector) {

	uint8_t file[2];
	ata0m->Read28(sector, file, 2, 0);

	bool fileExists = file[0] == 0xf1 && file[1] == 0x7e;
	
	return file[0] != 0x00;
}


uint32_t FileSystem::SetFileSize(char* name, uint32_t size) {

	uint32_t location = this->GetFileSector(name);

	if (FileIf(location)) {
	
		uint8_t sectorData[512];
		ata0m->Read28(location, sectorData, 512, 0);
		
		sectorData[4] = (size)       & 0xff;
		sectorData[5] = (size >> 8)  & 0xff;
		sectorData[6] = (size >> 16) & 0xff;
		sectorData[7] = (size >> 24);
	
		ata0m->Write28(location, sectorData, 512, 0);
	}
	return size;
}


uint32_t FileSystem::GetFileSize(char* name) {
		
	uint32_t size = 0;
	uint32_t location = this->GetFileSector(name);

	if (FileIf(location)) {
	
		uint8_t data[8];
		ata0m->Read28(location, data, 8, 0);

		size = (data[7] << 24) | (data[6] << 16) | 
			(data[5] << 8) | data[4];
	}
	return size;
}

uint32_t FileSystem::GetDataSize(char* name) {
	
	uint32_t size = 0;
	uint32_t location = this->GetFileSector(name);

	if (FileIf(location)) {
	
		uint8_t data[44];
		ata0m->Read28(location, data, 44, 0);

		size = (data[40] << 24) | (data[41] << 16) | 
			(data[42] << 8) | data[43];
	}
	return size;
}


uint32_t FileSystem::AddTable(char* name, uint32_t location) {

	//+1 file added to system
	uint32_t fileNum = 0;
	uint8_t sectorData[512];
	ata0m->Read28(tableStartSector, sectorData, 512, 0);

	for (int i = 0; i < 256; i++) {

		//first half for file num
		fileNum += sectorData[i];

		if (sectorData[i] < 0xff) {
		
			sectorData[i]++;
			break;
		}
	}
	ata0m->Write28(tableStartSector, sectorData, 512, 0);
	
	uint16_t index = (fileNum%128)*4;

	ata0m->Read28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	sectorData[index] = (location & 0xffffffff)   >> 24;
	sectorData[index+1] = (location & 0xffffff)     >> 16; 
	sectorData[index+2] = (location & 0xffff)       >> 8; 
	sectorData[index+3] = (location & 0xff); 

	ata0m->Write28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	return fileNum+1;
}



uint32_t FileSystem::RemoveTable(char* name, uint32_t location) {

	//1 file removed from system
	//remove from list by index here
	uint32_t fileNum = 0;
	uint8_t sectorData[512];
	ata0m->Read28(tableStartSector, sectorData, 512, 0);

	for (int i = 0; i < 256; i++) {
	
		//first half for file num
		fileNum += sectorData[i];

		if (sectorData[i+1] == 0) {
		
			sectorData[i]--;
			break;
		}
	}
	//we will have one less file
	fileNum--;
	ata0m->Write28(tableStartSector, sectorData, 512, 0);
	
	uint16_t index = (fileNum%128)*4;

	//check if removing newest file or not
	ata0m->Read28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	//get the newest file allocated	to replace one being deleted
	uint32_t newLocation =  (sectorData[index] << 24) |
				(sectorData[index+1] << 16) | 
				(sectorData[index+2] << 8) | 
				(sectorData[index+3]);

	//printf(int2str(newLocation));

	//removing newest file
	for (int j = 0; j < 4; j++) { sectorData[index+j] = 0x00; }
	ata0m->Write28(fileStartSector+(fileNum/128), sectorData, 512, 0);
		
	
	//move newest entry to deleted table entry 
	if (fileNum > 0 || newLocation != location) {
					
		//for each sector
		for (int j = 0; j < (fileNum/128)+1; j++) {
	
			ata0m->Read28(fileStartSector+j, sectorData, 512, 0);
		
			//go through sector
			for (int k = 0; k < 512; k += 4) {
			
				uint32_t findLocation =  (sectorData[k] << 24) |
							 (sectorData[k+1] << 16) | 
							 (sectorData[k+2] << 8) | 
							 (sectorData[k+3]);
			
				if (location == findLocation) {
				
					sectorData[k] = newLocation >> 24;
					sectorData[k+1] = (newLocation >> 16) & 0xff; 
					sectorData[k+2] = (newLocation >> 8) & 0xff; 
					sectorData[k+3] = (newLocation & 0xff);

					ata0m->Write28(fileStartSector+j, sectorData, 512, 0);
				
					return fileNum;
				}
			}
		}
	}
	return fileNum;
}


//update table (rewrite table using memory cache)
//this is done because im not spending anymore time on this
//i got other shit to work to make the deadline for the video
uint32_t FileSystem::UpdateTable() {

	uint8_t sectorData[512];
	uint32_t sectorOffset = 0;
	uint32_t location = 0;
	uint32_t index = 0;

	for (int i = 0; i < this->table->fileCount; i++) {
	
		File* file = (File*)(this->table->files->Read(i));
		location = file->Location;
		index = (i%128)*4;

		sectorData[index] = (location & 0xffffffff)   >> 24;
		sectorData[index+1] = (location & 0xffffff)     >> 16; 
		sectorData[index+2] = (location & 0xffff)       >> 8; 
		sectorData[index+3] = (location & 0xff); 				
	
		if ((i % 128) == 0) {
			ata0m->Write28(fileStartSector+sectorOffset, sectorData, 512, 0);
			sectorOffset++;
		}
	}

	return 0;
}


uint32_t FileSystem::GetFileCount() {

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

	uint8_t sectorData[512];
	uint32_t location = 0;
	uint16_t index = (fileNum%128)*4;
	
	ata0m->Read28(fileStartSector+(fileNum/128), sectorData, 512, 0);

	location = (sectorData[index] << 24) |
		   (sectorData[index+1] << 16) | 
		   (sectorData[index+2] << 8) | 
		   (sectorData[index+3]);
	
	if (FileIf(location) == false) { 
		
		//I WANT TO FUCKING KILL MYSELF
		printf("Failed to get file location.\n");
		//return 0; 
	}

	if (fileName != nullptr) {
		
		ata0m->Read28(location, sectorData, 72, 0);
		int i = 0;
		
		//name
		for (i; (sectorData[i+8] != '\0' || i < 32); i++) {

			fileName[i] = (char)(sectorData[i+8]);
		}
		fileName[i] = '\0';
	}
	return location;
}


//return tag string stored in char tag[33]
uint32_t FileSystem::GetFileTag(char* fileName, uint8_t tagNum, char tag[33]) {

	uint8_t sectorData[512];
	uint32_t location = this->GetFileSector(fileName);
	
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



bool FileSystem::NewFile(char* name, uint8_t* file, uint32_t size) {

	uint32_t location = this->table->currentOpenSector;
	AddTable(name, location);
	
	if (FileIf(location)) {

		//collision lol
		printf("Collision detected, file can't be created.\n");
		return false;
	}
	
	//add to memory table	
	this->table->fileCount++;

	File* newFile = (File*)(this->memoryManager->malloc(sizeof(File)));
	new (newFile) File(location, size, name);
	this->table->files->Push(newFile);
	this->newestLocation = location;
	this->table->currentOpenSector += (size/512) + 1;
	
	
	//OFS FILE STRUCTURE BELOW
	
	//first 8 bytes for file nums and size
	uint8_t sectorData[512];

	//init with zeros
	for (int i = 0; i < 512; i++) { sectorData[i] = 0x00; }


	//start of file
	sectorData[0] = 0xf1;
	sectorData[1] = 0x7e;

	//file information and flags
	sectorData[2] = 0x00;
	sectorData[3] = 0x00;
	
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
	

	//size of file data in 32 bits
	sectorData[40] = (size)       & 0xff;
	sectorData[41] = (size >> 8)  & 0xff;
	sectorData[42] = (size >> 16) & 0xff;
	sectorData[43] = (size >> 24);


	//fragmentation byte 64 to 96
	//16 different 2 byte encodings, 
	//1st is for lba num for when fragment starts
	//2nd is for offset of sector where fragments is located	
	sectorData[97] = 0xff;
	
	
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

	//write initial block of data
	WriteLBA(name, file, 0);
	
	return true;
}



bool FileSystem::DeleteFile(char* name) {
	
	uint32_t location = this->GetFileSector(name);
	uint32_t size = GetFileSize(name);

	if (FileIf(location) == false) { return false; }
	
	//remove from table
	RemoveTable(name, location);
	
	//delete actual file data	
	uint8_t zeros[OFS_BLOCK_SIZE];
	for (int i = 0; i < OFS_BLOCK_SIZE; i++) { zeros[i] = 0x00; }

	for (uint32_t i = 0; i < (size/OFS_BLOCK_SIZE); i++) {
	
		WriteLBA(name, zeros, i);
	}
	
	//get rid of magic numbers + other metadata
	ata0m->Write28(location, zeros, 512, 0);
	

	//remove from memory table
	if (location+(size/512)+1 == this->table->currentOpenSector) { 
	
		this->table->currentOpenSector -= ((size/512)+1); 
	}

	for (int i = 0; i < this->table->fileCount; i++) {
	
		File* file = (File*)(this->table->files->Read(i));

		if (location == file->Location) {
			
			this->table->files->Remove(i);
			break;
		}
	}
	this->table->fileCount--;
	
	return true;
}



bool FileSystem::NewTag(char* name, uint32_t location) {

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

	return true;
}



//unfinished
bool FileSystem::DeleteTag(common::uint32_t location, common::uint8_t tagNum) {

	uint8_t sectorData[512];
	uint16_t sectorTagIndex = 256+(tagNum*32);

	//byte 256 to end for tag strings
	ata0m->Read28(location, sectorData, 512, 0);

	//remove tag by overwriting with zeros
	for (int i = sectorTagIndex; i < sectorTagIndex+32; i++) { 
		
		sectorData[i] = 0x00; 
	}
	ata0m->Write28(location, sectorData, 512, 0);

	return true;
}


void FileSystem::UpdateSize(uint32_t location, uint32_t size) {

	uint8_t sectorData[512];

	ata0m->Read28(location, sectorData, 512, 0);
	sectorData[4] = (size)       & 0xff;
	sectorData[5] = (size >> 8)  & 0xff;
	sectorData[6] = (size >> 16) & 0xff;
	sectorData[7] = (size >> 24);
	
	ata0m->Write28(location, sectorData, 512, 0);
}


//fragmentation byte 64 to 96
bool FileSystem::WriteLBA(char* name, uint8_t* file, uint32_t lba) {

	uint32_t size = OFS_BLOCK_SIZE * (lba + 1);
	uint32_t location = this->GetFileSector(name);

	if (FileIf(location) == false) {
	
		printf("write what?\n");
		return false;
	}
	uint32_t startSector = location + 1 + ((size - OFS_BLOCK_SIZE) / 512);
	uint8_t sectorData[512];
	

	//check for fragmentation
	bool addFragment = false;
	int fragmentCheck = 0;
	while (FileIf(startSector) == true && size > GetFileSize(name)) {
	
		startSector += OFS_BLOCK_SIZE;
		fragmentCheck++;
		addFragment = true;

		//check for max of 64 blocks for availability
		if (fragmentCheck >= 64) { break; }
	}

	//add a new fragment to file
	if (addFragment) {
	
		uint8_t frIndex = 64;
		uint8_t fragmentData[512];
		ata0m->Read28(location, fragmentData, 512, 0);

		while (fragmentData[frIndex] != 0 && frIndex < 96) { frIndex += 2; }

		if (frIndex < 96) {
		
			fragmentData[frIndex] = (size-location)/512;
			fragmentData[frIndex+1] = (startSector-location)/512;
		} else {
			printf("fragmentation error.\n");
			return false;
		}
		ata0m->Write28(location, fragmentData, 512, 0);
	}




	//write block sector by sector
	for (int i = 0; i < OFS_BLOCK_SIZE/512; i++) {

		//fill in sector data with file data
		for (int j = 0; j < 512; j++) {
		
			sectorData[j] = file[(512*i)+j];
		}

		//write sector to file
		ata0m->Write28(startSector+i, sectorData, 512, 0);
	}

	//update size
	if (GetFileSize(name) < size) {
	
		ata0m->Read28(location, sectorData, 512, 0);
		sectorData[4] = (size)       & 0xff;
		sectorData[5] = (size >> 8)  & 0xff;
		sectorData[6] = (size >> 16) & 0xff;
		sectorData[7] = (size >> 24);
	
		ata0m->Write28(location, sectorData, 512, 0);
	
		//incase file is newest allocated and size exceeds sector target
		
		//if (location == newestLocation) {
		
		//	this->table->currentOpenSector = location + (size/512);
		//}
		
		if (startSector >= this->table->currentOpenSector) {
		
			this->table->currentOpenSector = startSector + OFS_BLOCK_SIZE;
		}
	}
	

	return true;
}



bool FileSystem::ReadLBA(char* name, uint8_t* file, uint32_t lba) {
	
	uint32_t size = OFS_BLOCK_SIZE * (lba + 1);
	uint32_t location = this->GetFileSector(name);

	if (FileIf(location) == false) {
	
		for (int i = 0; i < OFS_BLOCK_SIZE; i++) { file[i] = 0x00; }
		printf("Can't read file that doesn't exist.\n");
		return false;
	}

	uint32_t startSector = location + 1 + ((size - OFS_BLOCK_SIZE) / 512);
	uint8_t sectorData[512];


	//check if fragmented
	//read from fragment if needed
	if (GetFragmentFromLBA(location, lba) != 0) {
	
		startSector = GetFragmentFromLBA(location, lba);
	}


	//read block sector by sector
	for (int i = 0; i < OFS_BLOCK_SIZE/512; i++) {
		
		//read sector from file
		ata0m->Read28(startSector+i, sectorData, 512, 0);

		//fill in sector data with file data
		for (int j = 0; j < 512; j++) {
		
			file[(512*i)+j] = sectorData[j];
		}

	}
	return true;
}




//buffer will contain data from file to compress
void FileSystem::Compress(char* name, uint8_t* buffer, uint32_t bufsize) {

	//uint16_t blockNum = bufsize / OFS_BLOCK_SIZE;
	
	//write compressed data here
	//List newBuffer(this->memoryManager);
	uint8_t newBuffer[bufsize];
	uint8_t instructionBuffer[OFS_BLOCK_SIZE*8];
	
	uint32_t newBufIndex = 0;
	uint32_t intBufIndex = 2;

	//run length
	uint8_t runLengthVal = 0;
	uint16_t runLength = 0;
	uint32_t runLengthIndex = 0;
	uint16_t runLengthInstructionNum = 0;

	/*
	float valueFrequencyTable[256];
	for (int i = 0; i < 256; i++) { valueFrequencyTable[i] = 0.0; }

	//frequency table
	for (int i = 0; i < bufsize; i++) {
	
		valueFrequencyTable[buffer[i]]++;
	}
	*/

		
	//2 indexes, i for old buf
	//newBufIndex for new buf
	for (int i = 0; i < bufsize; i++) {

		//write uncompressed data to new buffer
		newBuffer[newBufIndex] = buffer[i];

		if (i < (bufsize-6)) {
		
			int j = i+1;
			
			while (buffer[i] == buffer[j] && j < bufsize) { j++; }

			if (j > 6) {
				runLengthIndex = i;
				runLengthVal = buffer[i];
				runLength = j-i;

				instructionBuffer[intBufIndex] = (runLengthIndex >> 16) & 0xff;
				instructionBuffer[intBufIndex+1] = (runLengthIndex >> 8) & 0xff;
				instructionBuffer[intBufIndex+2] = runLengthIndex & 0xff;
				instructionBuffer[intBufIndex+3] = runLengthVal;
				instructionBuffer[intBufIndex+4] = runLength >> 8;
				instructionBuffer[intBufIndex+5] = runLength & 0xff;
				
				intBufIndex += 6;
				runLengthInstructionNum++;
				
				i += j;
			}
		}
		newBufIndex++;
	}
	//add num of run length instructions encoded
	instructionBuffer[0] = runLengthInstructionNum >> 8;
	instructionBuffer[1] = runLengthInstructionNum & 0xff;


	//done with compression, now we write data


	//write list data to disk
	uint8_t data[OFS_BLOCK_SIZE];
	for (int j = 0; j < OFS_BLOCK_SIZE; j++) { data[j] = 0x00; }

	bool setSize = false;
	uint32_t dataIndex = 0;
	
	//write file with compressed data
	for (int i = 0; i < ((newBufIndex+intBufIndex)/OFS_BLOCK_SIZE)+1; i++) {
	
		for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
		
			if (dataIndex < intBufIndex) {
			
				data[j] = instructionBuffer[dataIndex];
			
			} else if (dataIndex < intBufIndex+newBufIndex) {
			
				data[j] = newBuffer[dataIndex-intBufIndex];
			} else {
				data[j] = 0x00;
				setSize = true;
			}
			dataIndex++;
		}
		//write compressed data
		WriteLBA(name, data, i);

		//set size and exit
		if (setSize) {
		
			//this->UpdateSize(this->GetFileSector(name), i*OFS_BLOCK_SIZE);
			break;
		}
	}
}


//buffer have uncompressed data written to it
void FileSystem::Decompress(char* name, uint8_t* buffer, uint32_t bufsize) {

	uint8_t oldBuffer[bufsize];
	uint32_t oldBufIndex = 0;
	
	uint8_t readData[OFS_BLOCK_SIZE];

	for (int i = 0; i < GetFileSize(name)/OFS_BLOCK_SIZE; i++) {
	
		this->ReadLBA(name, readData, i);
		
		for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
		
			oldBuffer[oldBufIndex] = readData[j];
			oldBufIndex++;
		}
	}

	//run length
	uint8_t runLengthInstructionNum = (oldBuffer[0] << 8) | oldBuffer[1];
	uint32_t encodingInstructionBytes = (runLengthInstructionNum*6)+2;
	

	uint32_t newBufIndex = 0;
	uint16_t locationIndex = 1;
	
	uint32_t location = (oldBuffer[2] << 16) 
			  | (oldBuffer[3] << 8) 
			   | oldBuffer[4];
	
	uint8_t runLengthVal = oldBuffer[5];
	uint16_t runLength = (oldBuffer[6] << 8) 
			   | (oldBuffer[7]);

	//the more run length instructions there are, 
	//there more memory corruption at top of image

		
	//start from after instructions to actual data
	for (int i = encodingInstructionBytes; i < oldBufIndex; i++) {
			
		
		//add uncompressed data to file
		if (newBufIndex < bufsize) {

			buffer[newBufIndex] = oldBuffer[i];
			newBufIndex++;
		} else {
			return;
		}
	
		
		//add compressed data to file
		if (i == location+encodingInstructionBytes) {
			
			for (int j = 0; j < runLength; j++) {
			
				if (newBufIndex < bufsize) {
					buffer[newBufIndex] = runLengthVal;
					newBufIndex++;
				} else {
					return;
				}
			}
				
			location = (oldBuffer[(locationIndex*6)+2] << 16) 
				 | (oldBuffer[(locationIndex*6)+3] << 8) 
				  | oldBuffer[(locationIndex*6)+4];
				
			runLengthVal = oldBuffer[(locationIndex*6)+5];
				
			runLength = (oldBuffer[(locationIndex*6)+6] << 8) 
				  | (oldBuffer[(locationIndex*6)+7]);
				
			locationIndex++;
			
			/*
			//update location
			if (locationIndex < runLengthInstructionNum) {
				
				location = (oldBuffer[(locationIndex*6)+2] << 16) 
					 | (oldBuffer[(locationIndex*6)+3] << 8) 
					  | oldBuffer[(locationIndex*6)+4];
				
				runLengthVal = oldBuffer[(locationIndex*6)+5];
				
				runLength = (oldBuffer[(locationIndex*6)+6] << 8) 
					  | (oldBuffer[(locationIndex*6)+7]);
				
				locationIndex++;
			} else {
				location = 0xffffffff;
			}
			*/
			
			runLength = 0x00;
		}
	}
}




void FileSystem::EncryptBlock(uint8_t data[16], uint8_t key[16]) {

	uint8_t copyData[16];
	for (int i = 0; i < 16; i++) { copyData[i] = data[i]; }

	for (int i = 0; i < 10; i++) {
	
		//rotation
		for (int j = 0; j < 16; j++) { data[j] = copyData[(j+i)%16]; }
		
		//addition
		for (int j = 0; j < 16; j++) { data[j] += (uint8_t)(key[j] * key[i]); }

		//xor
		for (int j = 0; j < 16; j++) { data[j] ^= key[j]; }
	}
}
void FileSystem::DecryptBlock(uint8_t data[16], uint8_t key[16]) {
	
	uint8_t copyData[16];
	for (int i = 0; i < 16; i++) { copyData[i] = data[i]; }
		
	for (int i = 0; i < 10; i++) {
		
		//xor
		for (int j = 0; j < 16; j++) { data[j] ^= key[j]; }
		
		//addition
		for (int j = 0; j < 16; j++) { data[j] -= (uint8_t)(key[j] * key[i]); }
		
		//rotation
		for (int j = 0; j < 16; j++) { 
			
			//????
			if (j-i < 0) {
				data[j] = copyData[16+(j-i)];
			} else {
				data[j] = copyData[(j-i)%16];
			}
		}
	}
}


void FileSystem::CryptFile(char* name, uint8_t key[16], bool encrypt) {

	uint8_t data[OFS_BLOCK_SIZE];
	uint8_t block[16];
	uint8_t prev_block[16];
	uint8_t init_vector[16];
			
	//need to hash key to fixed sized length before anything
	//do this later ok? thanks future self, ur welcume man

	//for each ofs file lba
	for (int i = 0; i < GetFileSize(name)/OFS_BLOCK_SIZE; i++) {
	
		//read from file
		ReadLBA(name, data, i);

		
		//init the init vector
		for (int j = 0; j < 16; j++) { init_vector[j] = (uint8_t)(key[j] * 0xb3); }
		

		//for each 128 bit encryption block
		for (int j = 0; j < OFS_BLOCK_SIZE/16; j++) {
		
			//get block data from file
			for (int k = 0; k < 16; k++) { block[k] = data[16*j+k]; }
			

			if (encrypt) {
			
				//xor with init vector for cbc mode
				for (int k = 0; k < 16; k++) { block[k] ^= init_vector[k]; }

				//encrypt
				this->EncryptBlock(block, key);
			
				//save new init vector (next blocks previous ciphertext)
				for (int k = 0; k < 16; k++) { init_vector[k] = block[k]; }
			} else {
				//save ciphertext for next init vector
				for (int k = 0; k < 16; k++) { prev_block[k] = block[k]; }
				
				//decrypt
				this->DecryptBlock(block, key);
				
				//xor with init vector to get plaintext
				for (int k = 0; k < 16; k++) { block[k] ^= init_vector[k]; }
				
				//put ciphertext data into init vector
				for (int k = 0; k < 16; k++) { init_vector[k] = prev_block[k]; }
			}
		
			
			
			//put encrypted data instead
			for (int k = 0; k < 16; k++) { data[16*j+k] = block[k]; }
		}

		//write to file
		WriteLBA(name, data, i);
	}
}




uint32_t FileSystem::GetImageResolution(char* name) {

	uint8_t data[512];
	ata0m->Read28(this->GetFileSector(name), data, 256, 0);
	
	uint16_t width = data[124] + data[125];
	uint16_t height = data[126] + data[127];

	return (width << 16) | height;
}




//read and write VGA mode 13H buffer 
//from disk to memory and vice versa
bool FileSystem::Read13H(char* name, uint8_t* buffer, uint16_t* retWidth, uint8_t* retHeight, bool compress) {

	if (FileIf(this->GetFileSector(name)) == false) { return false; }

	uint8_t file[OFS_BLOCK_SIZE];
	uint16_t x = 0;
	uint8_t y = 0;
	uint32_t lba = 0;

	//retrieve and save res data
	uint32_t res = GetImageResolution(name);
	uint16_t width = res >> 16;
	uint16_t height = res & 0xffff;

	*retWidth = width;
	*retHeight = height;


	if (compress) {

		this->Decompress(name, buffer, width*height);
		//this->Decompress(name, buffer, 320*200);
		
	//read bitmap
	} else {
		
		//read file and write buffer
		for (lba; lba < ((width*height)/OFS_BLOCK_SIZE)+1; lba++) {

			ReadLBA(name, file, lba);

			for (uint16_t j = 0; j < OFS_BLOCK_SIZE; j++) {
	
				//leave if buffer already full	
				if ((lba*OFS_BLOCK_SIZE)+j >= width*height) { return true; }

				buffer[width*y+x] = file[j];
				x++;

				if (x >= width) {

					y++;
					x = 0;
				}
			}
		}
	}
	return true;
}


bool FileSystem::Write13H(char* name, uint8_t* buffer, uint16_t width, uint16_t height, bool compress) {

	uint32_t lba = 0;
	uint32_t size = width*height;
	uint8_t file[OFS_BLOCK_SIZE];
	for (uint32_t i = 0; i < OFS_BLOCK_SIZE; i++) { file[i] = 0x00; }

	//make new file if it doesn't exist
	if (FileIf(this->GetFileSector(name)) == false) {

		this->NewFile(name, file, OFS_BLOCK_SIZE);
		this->NewTag("img", this->GetFileSector(name));
	}

	//write width and height of image
	if (width < 320 && height < 200) {
	
		uint8_t data[512];
		ata0m->Read28(this->GetFileSector(name), data, 512, 0);

		if (width < 256) {
		
			data[124] = width; 
			data[125] = 0;
		} else {
			data[124] = 255; 
			data[125] = width % 256;
		}
		data[126] = height; 
		data[127] = 0;
		
		ata0m->Write28(this->GetFileSector(name), data, 512, 0);
	}



	if (compress) {

		this->Compress(name, buffer, width*height);
		this->NewTag("compressed", this->GetFileSector(name));
		//this->Compress(name, buffer, 320*200);
	
	//write bitmap
	} else {
		//read buffer and write file
		for (lba; lba < (size/OFS_BLOCK_SIZE)+1; lba++) {
		
			for (uint16_t j = 0; j < OFS_BLOCK_SIZE; j++) {
		
				if ((lba*OFS_BLOCK_SIZE)+j < size) { file[j] = buffer[(lba*OFS_BLOCK_SIZE)+j];
				} else { 		   file[j] = 0x00; }
			}
			WriteLBA(name, file, lba);
		}
	}
	return true;
}
