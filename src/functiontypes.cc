#include <functiontypes.h>

using namespace os;
using namespace os::common;
using namespace os::math;
using namespace os::filesystem;

void printf(char*);
uint8_t* memset(uint8_t*, int, size_t);


Type::Type(uint8_t type, void* initVal) {

	this->type = type;

	switch (type) {

		case TYPE_INT:	   this->typeInt   = *((uint32_t*)initVal);	break;
		//case TYPE_BOOL:	   this->typeBool  = *((bool)initVal);		break;
		case TYPE_FLOAT:   this->typeFloat = *((float*)initVal);	break;
		case TYPE_STRING:  this->typeStr   = (char*)initVal;		break;
		case TYPE_FUNCTION:this->typeFunc  = (FunctionType*)initVal;	break;
	}
}

Type::~Type() {
}


FunctionType::FunctionType(char* file, uint32_t funcStartIndex, uint32_t numOfArgs) {

	for (int i = 0; i < 32; i++) { this->file[i] = '\0'; }
	strcpy(this->file, file);
	this->funcStartIndex = funcStartIndex;
	this->numOfArgs = numOfArgs;
}

FunctionType::~FunctionType() {
}

/*

uint32_t FunctionType::LoadFunction(FileSystem* filesystem) {

	if (this->buf != nullptr) { filesystem->memoryManager->free(this->buf); }

	uint32_t beginBlock = this->funcStartIndex / OFS_BLOCK_SIZE;
	uint32_t totalBlock = (filesystem->GetFileSize(this->file) / OFS_BLOCK_SIZE) - beginBlock;
	
	this->bufSize = totalBlock*OFS_BLOCK_SIZE;
	this->buf = (uint8_t*)filesystem->memoryManager->malloc(this->bufSize);

	for (int i = beginBlock; i < totalBlock; i++) {
		
		uint8_t data[OFS_BLOCK_SIZE];
		filesystem->ReadLBA(this->file, data, i);
		
		for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
		
			this->buf[(i*OFS_BLOCK_SIZE)+j] = data[j];
		}
	}


}
*/

uint32_t FunctionType::LoadFunction(FileSystem* filesystem) {

	if (this->buf != nullptr) { return this->bufSize; }

	uint32_t beginBlock = this->funcStartIndex / OFS_BLOCK_SIZE;
	uint32_t totalBlock = (filesystem->GetFileSize(this->file) / OFS_BLOCK_SIZE) - beginBlock;

	uint8_t* temp = (uint8_t*)filesystem->memoryManager->malloc(totalBlock*OFS_BLOCK_SIZE);
	memset(temp, 0x00, totalBlock*OFS_BLOCK_SIZE);

	uint32_t offset = 0;
	uint32_t endIndex = 0;

	//load file
	for (int i = beginBlock; i < totalBlock; i++) {
	
		uint8_t data[OFS_BLOCK_SIZE];
		filesystem->ReadLBA(this->file, data, i);

		
		if (i == beginBlock) { offset = this->funcStartIndex % OFS_BLOCK_SIZE;
		} else { 	       offset = 0; }

		for (int j = offset; j < OFS_BLOCK_SIZE; j++) {
		
			temp[(i*OFS_BLOCK_SIZE)+j] = data[j];
		}
		
		//find end
		for (int j = 0; j < OFS_BLOCK_SIZE-4; j++) {
			
			if (data[0] == 'e' && data[1] == 'n' & data[2] == 'd' && 
				(data[3] == '\n' || data[3] == '\0')) {
			
				endIndex = (i*OFS_BLOCK_SIZE)+j+4;
				this->bufSize = endIndex;
			}
		}
	}

	//save in buffer
	this->buf = (uint8_t*)filesystem->memoryManager->malloc(this->bufSize);
	memset(this->buf, 0x00, this->bufSize);
	offset = (this->funcStartIndex % OFS_BLOCK_SIZE) + (beginBlock*OFS_BLOCK_SIZE);

	for (int i = offset; i < this->bufSize; i++) {
	
		this->buf[i-offset] = temp[i];
	}
	filesystem->memoryManager->free(temp);

	return (this->bufSize - offset);
}
