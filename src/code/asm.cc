#include <code/asm.h>

using namespace os;
using namespace os::common;
using namespace os::filesystem;


void printf(char* str);
char* argparse(char* args, uint8_t num);
bool strcmp(char* one, char* two);
uint32_t str2int(char* str);
char* int2str(uint32_t);


Compiler::Compiler(FileSystem* filesystem) {

	this->filesystem = filesystem;

	for (int i = 0; i < 16; i++) { this->code[i] = 0x00; }
	this->codeLength = 1;
}


Compiler::~Compiler() {}


//turn assembly into machine code
void Compiler::Assemble(char* asmFile, char* outFile) {
	
	


	uint8_t readData[OFS_BLOCK_SIZE];
	this->filesystem->ReadLBA(asmFile, readData, 0);
	

	//no real binary format for now
	//first 2 bytes will be length of code
	uint16_t instructionNum = 0;
	uint16_t writeIndex = 4;
	uint8_t writeData[OFS_BLOCK_SIZE];
	for (int i = 0; i < OFS_BLOCK_SIZE; i++) { writeData[i] = 0x00; }

	
	//array for storing assembly instructions in file
	char instruction[256];
	for (int i = 0; i < 256; i++) { instruction[i] = '\0'; }
	uint8_t strIndex = 0;
	

	for (int i = 0; i < OFS_BLOCK_SIZE; i++) {
	
		switch (readData[i]) {
			
			//store instruction as
			//label in hash table or sum
			//(finish later)
			case ':':
				break;
		
			//ignore commas to make
			//string functions work easier
			case ',':
				break;

			case 0x00:
			case '\n':
				{
				if (instruction[0] == '\0') { break; }

				//terminate string
				instruction[strIndex] = '\0';
				strIndex = 0;

				
				//get binary data and put into code array
				this->EncodeAsm(instruction);
				
				//write that binary data to file buffer
				for (int j = 0; j < this->codeLength; j++) {
				
					writeData[writeIndex] = this->code[j];
					writeIndex++;
				}

				//reset code
				//for (int j = 0; j < 16; j++) { this->code[j] = 0x00; }
				this->codeLength = 1;
	
				//reset instruction string
				for (int j = 0; j < 256; j++) { instruction[j] = '\0'; }

				instructionNum++;
				}
				break;
			default:
				instruction[strIndex] = readData[i];
				strIndex++;
				break;
		}
	}

	//store length of executable code
	writeData[0] = writeIndex >> 8;
	writeData[1] = writeIndex & 0xff;
	
	//store # of instructions
	writeData[2] = instructionNum >> 8;
	writeData[3] = instructionNum & 0xff;


	//write to file
	if (this->filesystem->FileIf(this->filesystem->GetFileSector("a.out"))) {
	
		this->filesystem->WriteLBA("a.out", writeData, 0);
	} else {
		this->filesystem->NewFile("a.out", writeData, OFS_BLOCK_SIZE);
		//this->filesystem->NewFile(outFile, writeData, OFS_BLOCK_SIZE);
	}
	
}



uint32_t Compiler::EncodeAsm(char* instruction) {


	// 00 | 000 | 000
	// mod  reg   R/M
	uint8_t modrm = 0x00;
	uint8_t operandNum = 0;

	char* reg1str = argparse(instruction, 1);
	char* reg2str = instruction;
	
	for (int i = 4; instruction[i] != '\0'; i++) {
	
		if (instruction[i] == ' ') {
		
			reg2str += (i+1);
			break;
		}
	}
	
	
	/*
	uint8_t reg1 = 0; uint8_t regSize1 = 0;
	uint8_t reg2 = 0; uint8_t regSize2 = 0;
	//uint32_t imm = 0; bool isConstant = false;
	uint32_t imm = str2int(reg2str); 
	bool isConstant = false;
	*/

	
	//get operand values for instructions	
	OperandData operands;
	operands.reg1 = 0;
	operands.regSize1 = 0;
	operands.reg2 = 0;
	operands.regSize1 = 0;
	operands.imm = str2int(reg2str);
	operands.isConstant = false;


	operands.reg1 = this->GetRegisterBitsAndOffset(reg1str, &(operands.regSize1));

	//error: cant move val into non register
	if (!(operands.regSize1)) { return -1; }

	operands.reg2 = this->GetRegisterBitsAndOffset(reg2str, &(operands.regSize2));

	//must be same sized registers
	if (operands.regSize1 != operands.regSize2 && operands.regSize2 != 0) { return -2; }
	


	if (!(operands.regSize2)) {
			
		if (operands.imm > 0 || reg2str[0] == '0') {
		
			//operand 2 is imm val
			//printf("constant\n");
			//operands.imm = str2int(reg2str);
			operands.isConstant = true;
		}
	}

	/*
	if (this->GetOperandValues(reg1str, reg2str, 
				&reg1, &regSize1, 
				&reg2, &regSize2, 
				&imm, &isConstant) > 0) {
		return 0;
	}
	*/
	

	if (strcmp("mov", instruction)) { this->GenMov(&operands); }
	
	//else if (strcmp("add", instruction)) { this->GenAdd(reg1, reg2); }
	//else if (strcmp("sub", instruction)) { this->GenSub(reg1, reg2); }
	//else if (strcmp("and", instruction)) { this->GenAnd(reg1, reg2); }
	
	else if (strcmp("add", instruction)) { this->GenOperation(&operands, 0x00, 0x04, 0xc0); }
	else if (strcmp("sub", instruction)) { this->GenOperation(&operands, 0x28, 0x2c, 0xe8); }
	else if (strcmp("and", instruction)) { this->GenOperation(&operands, 0x20, 0x24, 0xe0); }
	
	else if (strcmp("or", instruction)) {  this->GenOperation(&operands, 0x0c, 0x08, 0xc8); }
	else if (strcmp("xor", instruction)) { this->GenOperation(&operands, 0x34, 0x08, 0xc0); } 
	
	else if (strcmp(instruction, "int")) { 
		
		code[0] = 0xf4; 
		code[1] = (uint8_t)(str2int(argparse(instruction, 1)));
		this->codeLength = 2;
	}

	/*
	if (strcmp(instruction, "test")) { 
		
		code[0] = 0; 
	
	} else if (strcmp(instruction, "add")) { 
		
		code[0] = 0x00; operandNum = 2; 
	
	} else if (strcmp(instruction, "sub")) { 
		
		code[0] = 0x28; operandNum = 2; 
	
	} else if (strcmp(instruction, "mul")) { 
	
		//f7 /4
		code[0] = 0xf7; operandNum = 2; 
		modrm | (1 << 5);
	
	} else if (strcmp(instruction, "div")) { 
	
		//f7 /6
		code[0] = 0xf7; operandNum = 2; 
		modrm | (1 << 4);
		modrm | (1 << 5);

	} else if (strcmp(instruction, "and")) { 
	
		code[0] = 0x20; operandNum = 2; 
	
	} else if (strcmp(instruction, "not")) { 
		
		//f6 /2
		code[0] = 0xf6; operandNum = 2; 
		modrm | (1 << 4);
	
	} else if (strcmp(instruction, "or")) { 
		
		code[0] = 0x28; operandNum = 2; 

	} else if (strcmp(instruction, "xor")) { 
		
		code[0] = 0x30; operandNum = 2; 
	
	} else if (strcmp(instruction, "cmp")) { 
		
		code[0] = 0x38; operandNum = 2; 
	
	} else if (strcmp(instruction, "inc")) { 
		
		//fe /0
		code[0] = 0xfe; operandNum = 1;

	} else if (strcmp(instruction, "dec")) { 
		
		//fe /1
		code[0] = 0xfe; operandNum = 1; 
		modrm | (1 << 3);

	} else if (strcmp(instruction, "mov")) { 
		
		code[0] = 0; operandNum = 2;
	
	} else if (strcmp(instruction, "nop")) { 
	
		code[0] = 0; operandNum = 0; 
	
	} else if (strcmp(instruction, "pop")) { 
		
		code[0] = 0; operandNum = 1; 
	
	} else if (strcmp(instruction, "push")) { 
		
		code[0] = 0; operandNum = 1; 
	}

	*/

	else if (strcmp(instruction, "hlt")) { code[0] = 0xf4; }
	
	//clear flags
	else if (strcmp(instruction, "clc")) { code[0] = 0xf8; }
	else if (strcmp(instruction, "cld")) { code[0] = 0xfc; }
	else if (strcmp(instruction, "cli")) { code[0] = 0xfa; }
	else if (strcmp(instruction, "cmc")) { code[0] = 0xf5; }
	
	else if (strcmp(instruction, "nop")) { code[0] = 0x90; }
	
	
	else { code[0] = 0xff; }

	return code[0];
}




uint8_t Compiler::GetRegisterBitsAndOffset(char* operand, uint8_t* regBits) {
	
	uint8_t regOffset = 0;

	for (int i = 0; i < 8; i++) {
	
		if (strcmp(operand, reg32[i])) {
		
			*regBits = 32;
			regOffset = i;
			break;

		} else if (strcmp(operand, reg16[i])) {
		
			*regBits = 16;
			regOffset = i;
			break;

		} else if (strcmp(operand, reg8[i])) {
		
			*regBits = 8;
			regOffset = i;
			break;
		}
	}
	return regOffset;
}



int8_t Compiler::GetOperandValues(char* operand1, char* operand2,
				uint8_t* reg1, uint8_t* regSize1, 
				uint8_t* reg2, uint8_t* regSize2, 
				uint32_t* imm, bool* isConstant) {
	
	*reg1 = this->GetRegisterBitsAndOffset(operand1, regSize1);

	//error: cant move val into non register
	if (!regSize1) { return -1; }

	*reg2 = this->GetRegisterBitsAndOffset(operand2, regSize2);

	//must be same sized registers
	if (*regSize1 != *regSize2 && *regSize2 != 0) { return -2; }


	if (!(*regSize2)) {
		if (str2int(operand2) > 0 && operand2[0] != '0') {
		
			//operand 2 is imm val
			printf(operand2);
			printf("\n");

			*imm = str2int(operand2);
			*isConstant = true;
		}
	}

	return 0;
}



//mov instruction
void Compiler::GenMov(OperandData* operands) {
//void Compiler::GenMov(char* operand1, char* operand2) {

	/*
	uint8_t reg1 = 0; uint8_t regSize1 = 0;
	uint8_t reg2 = 0; uint8_t regSize2 = 0;
	uint32_t imm = 0; bool isConstant = false;

	if (this->GetOperandValues(operand1, operand2, 
				&reg1, &regSize1, 
				&reg2, &regSize2, 
				&imm, &isConstant) > 0) {
		return;
	}
	*/


	//array for actual binary code
	//uint8_t code[16];
	//for (int i = 0; i < 16; i++) { code[i] = 0x00; }

	switch (operands->regSize1) {
	
		case 8:
			if (operands->isConstant) { 
				this->code[0] = 0xb0 + operands->reg1;
				this->code[1] = (uint8_t)(operands->imm);
				this->codeLength = 2;
			} else {	   
				this->code[0] = 0x88; 
				this->code[1] = 0xc0 + operands->reg1 + (operands->reg2*8);
				this->codeLength = 2;
			}
			break;
		case 16:
			if (operands->isConstant) { 
				this->code[0] = 0x66; 
				this->code[1] = 0xb8 + operands->reg1; 
				this->code[2] = ((uint16_t)(operands->imm)) & 0xff; 
				this->code[3] = ((uint16_t)(operands->imm)) >> 8; 
				this->codeLength = 4;	
			} else { 
				this->code[0] = 0x66;
				this->code[1] = 0x89;
				this->code[2] = 0xc0 + operands->reg1 + (operands->reg2*8); 
				this->codeLength = 3;
			}
			break;
		case 32:
			if (operands->isConstant) { 
				this->code[0] = 0xb8 + operands->reg1;
				this->code[1] = operands->imm & 0xff;
				this->code[2] = (operands->imm >> 8) & 0xff;
				this->code[3] = (operands->imm >> 16) & 0xff;
				this->code[4] = (operands->imm >> 24) & 0xff;
				this->codeLength = 5;
			} else {
				this->code[0] = 0x89;
				this->code[1] = 0xc0 + operands->reg1 + (operands->reg2*8);
				this->codeLength = 2;
			}
			break;
	}
}


//math operations (add, sub, and, or, etc.)
void Compiler::GenOperation(OperandData* operands,
				uint8_t opcode,
				uint8_t opcodeEax, 
				uint8_t regBaseOffset) {
	
	switch (operands->regSize1) {
	
		case 8:
			if (operands->isConstant) { 
				if (operands->reg1 == 0) {
					this->code[0] = opcodeEax + operands->reg1;
					this->code[1] = (uint8_t)(operands->imm);
					this->codeLength = 2;
				} else {
					this->code[0] = 0x80;
					this->code[1] = regBaseOffset + operands->reg1;
					this->code[2] = (uint8_t)(operands->imm);
					this->codeLength = 3;
				}
			} else {	   
				this->code[0] = opcode;
				this->code[1] = 0xc0 + operands->reg1 + (operands->reg2*8);
				this->codeLength = 2;
			}
			break;
		case 16:
			if (operands->isConstant) { 
				
				if (operands->imm < 256) {
					this->code[0] = 0x66; 
					this->code[1] = 0x83; 
					this->code[1] = regBaseOffset + operands->reg1;
					this->code[3] = (uint8_t)(operands->imm);
					this->codeLength = 4;
				} else {
					this->code[0] = 0x66; 
					
					if (operands->reg1 == 0) {
					
						this->code[1] = opcodeEax+1;
						this->code[2] = ((uint16_t)(operands->imm)) & 0xff; 
						this->code[3] = ((uint16_t)(operands->imm)) >> 8; 
						this->codeLength = 4;
					} else {
						this->code[1] = 0x81; 
						this->code[2] = regBaseOffset + operands->reg1; 
						this->code[3] = ((uint16_t)(operands->imm)) & 0xff; 
						this->code[4] = ((uint16_t)(operands->imm)) >> 8; 
						this->codeLength = 5;
					}
				}
				//this->codeLength = 4;
			} else { 
				this->code[0] = 0x66;
				this->code[1] = opcode+1;
				this->code[2] = 0xc0 + operands->reg1 + (operands->reg2*8); 
				this->codeLength = 3;	
			}
			break;
		case 32:
			if (operands->isConstant) {
				if (operands->imm < 256) {
						
					this->code[0] = 0x83;
					this->code[2] = regBaseOffset + operands->reg1;
					this->code[2] = (uint8_t)(operands->imm);
					this->codeLength = 3;
				} else {
					if (operands->reg1 == 0) {
					
						this->code[0] = opcodeEax+1;
						this->code[1] = operands->imm & 0xff;
						this->code[2] = (operands->imm >> 8) & 0xff;
						this->code[3] = (operands->imm >> 16) & 0xff;
						this->code[4] = (operands->imm >> 24) & 0xff;
						this->codeLength = 5;
					} else {
						this->code[0] = 0x81;
						this->code[1] = regBaseOffset + operands->reg1;
						this->code[2] = operands->imm & 0xff;
						this->code[3] = (operands->imm >> 8) & 0xff;
						this->code[4] = (operands->imm >> 16) & 0xff;
						this->code[5] = (operands->imm >> 24) & 0xff;
						this->codeLength = 6;
					}
				}	
			} else {
				this->code[0] = opcode+1;
				this->code[1] = 0xc0 + operands->reg1 + (operands->reg2*8);
				this->codeLength = 2;
			}
			break;
	}
}


/*
//add instruction
void Compiler::GenAdd(char* operand1, char* operand2) {
	
	uint8_t reg1 = 0; uint8_t regSize1 = 0;
	uint8_t reg2 = 0; uint8_t regSize2 = 0;
	uint32_t imm = 0; bool isConstant = false;

	if (this->GetOperandValues(operand1, operand2, 
				&reg1, &regSize1, 
				&reg2, &regSize2, 
				&imm, &isConstant) > 0) {
		return;
	}


	switch (regSize1) {
	
		case 8:
			if (isConstant) { 
				if (reg1 == 0) {
					this->code[0] = 0x04 + reg1;
					this->code[1] = (uint8_t)imm;
					this->codeLength = 2;
				} else {
					this->code[0] = 0x80;
					this->code[1] = 0xc0 + reg1;
					this->code[2] = (uint8_t)imm;
					this->codeLength = 3;
				}
			} else {	   
				this->code[0] = 0x00; 
				this->code[1] = 0xc0 + reg1 + (reg2*8);
				this->codeLength = 2;	
			}
			break;
		case 16:
			if (isConstant) { 
				
				if (imm < 256) {
						this->code[0] = 0x66; 
						this->code[1] = 0x83; 
						this->code[2] = 0xc0 + reg1;
						this->code[3] = (uint8_t)imm;
						this->codeLength = 4;
				} else {
					this->code[0] = 0x66; 
					
					if (reg1 == 0) {
					
						this->code[1] = 0x05; 
						this->code[2] = ((uint16_t)imm) & 0xff; 
						this->code[3] = ((uint16_t)imm) >> 8; 
						this->codeLength = 4;
					} else {
						this->code[1] = 0x81; 
						this->code[2] = 0xc0 + reg1; 
						this->code[3] = ((uint16_t)imm) & 0xff; 
						this->code[4] = ((uint16_t)imm) >> 8; 
						this->codeLength = 5;
					}
				}
				//this->codeLength = 4;
			} else { 
				this->code[0] = 0x66;
				this->code[1] = 0x01;
				this->code[2] = 0xc0 + reg1 + (reg2*8); 
				this->codeLength = 3;	
			}
			break;
		case 32:
			if (isConstant) {
				if (imm < 256) {
						
					this->code[0] = 0x83;
					this->code[1] = 0xc0 + reg1; 
					this->code[2] = (uint8_t)imm;
					this->codeLength = 3;
				} else {
					if (reg1 == 0) {
					
						this->code[0] = 0x05;
						this->code[1] = imm & 0xff;
						this->code[2] = (imm >> 8) & 0xff;
						this->code[3] = (imm >> 16) & 0xff;
						this->code[4] = (imm >> 24) & 0xff;
						this->codeLength = 5;
					} else {
						this->code[0] = 0x81;
						this->code[1] = 0xc0 + reg1;
						this->code[2] = imm & 0xff;
						this->code[3] = (imm >> 8) & 0xff;
						this->code[4] = (imm >> 16) & 0xff;
						this->code[5] = (imm >> 24) & 0xff;
						this->codeLength = 6;
					}
				}	
			} else {
				this->code[0] = 0x01;
				this->code[1] = 0xc0 + reg1 + (reg2*8);
				this->codeLength = 2;	
			}
			break;
	}
}

//sub instruction
void Compiler::GenSub(char* operand1, char* operand2) {
	
	uint8_t reg1 = 0; uint8_t regSize1 = 0;
	uint8_t reg2 = 0; uint8_t regSize2 = 0;
	uint32_t imm = 0; bool isConstant = false;

	if (this->GetOperandValues(operand1, operand2, 
				&reg1, &regSize1, 
				&reg2, &regSize2, 
				&imm, &isConstant) > 0) {
		return;
	}


	switch (regSize1) {
	
		case 8:
			if (isConstant) { 
				if (reg1 == 0) {
					this->code[0] = 0x2c + reg1;
					this->code[1] = (uint8_t)imm;
					this->codeLength = 2;
				} else {
					this->code[0] = 0x80;
					this->code[1] = 0xe8 + reg1;
					this->code[2] = (uint8_t)imm;
					this->codeLength = 3;
				}
			} else {	   
				this->code[0] = 0x28; 
				this->code[1] = 0xc0 + reg1 + (reg2*8);
				this->codeLength = 2;	
			}
			break;
		case 16:
			if (isConstant) { 
				
				if (imm < 256) {
						this->code[0] = 0x66; 
						this->code[1] = 0x83; 
						this->code[2] = 0xe8 + reg1;
						this->code[3] = (uint8_t)imm;
						this->codeLength = 4;
				} else {
					this->code[0] = 0x66; 
					
					if (reg1 == 0) {
					
						this->code[1] = 0x2d; 
						this->code[2] = ((uint16_t)imm) & 0xff; 
						this->code[3] = ((uint16_t)imm) >> 8; 
						this->codeLength = 4;
					} else {
						this->code[1] = 0x81; 
						this->code[2] = 0xe8 + reg1; 
						this->code[3] = ((uint16_t)imm) & 0xff; 
						this->code[4] = ((uint16_t)imm) >> 8; 
						this->codeLength = 5;
					}
				}
				//this->codeLength = 4;
			} else { 
				this->code[0] = 0x66;
				this->code[1] = 0x29;
				this->code[2] = 0xc0 + reg1 + (reg2*8); 
				this->codeLength = 3;	
			}
			break;
		case 32:
			if (isConstant) {
				if (imm < 256) {
						
					this->code[0] = 0x83;
					this->code[1] = 0xe8 + reg1; 
					this->code[2] = (uint8_t)imm;
					this->codeLength = 3;
				} else {
					if (reg1 == 0) {
					
						this->code[0] = 0x2d;
						this->code[1] = imm & 0xff;
						this->code[2] = (imm >> 8) & 0xff;
						this->code[3] = (imm >> 16) & 0xff;
						this->code[4] = (imm >> 24) & 0xff;
						this->codeLength = 5;
					} else {
						this->code[0] = 0x81;
						this->code[1] = 0xe8 + reg1;
						this->code[2] = imm & 0xff;
						this->code[3] = (imm >> 8) & 0xff;
						this->code[4] = (imm >> 16) & 0xff;
						this->code[5] = (imm >> 24) & 0xff;
						this->codeLength = 6;
					}
				}	
			} else {
				this->code[0] = 0x29;
				this->code[1] = 0xc0 + reg1 + (reg2*8);
				this->codeLength = 2;	
			}
			break;
	}
}



//and instruction
void Compiler::GenAnd(char* operand1, char* operand2) {

	uint8_t reg1 = 0; uint8_t regSize1 = 0;
	uint8_t reg2 = 0; uint8_t regSize2 = 0;
	uint32_t imm = 0; bool isConstant = false;

	if (this->GetOperandValues(operand1, operand2, 
				&reg1, &regSize1, 
				&reg2, &regSize2, 
				&imm, &isConstant) > 0) {
		return;
	}


	switch (regSize1) {
	
		case 8:
			if (isConstant) { 
				if (reg1 == 0) {
					this->code[0] = 0x24;
					this->code[1] = (uint8_t)imm;
					this->codeLength = 2;
				} else {
					this->code[0] = 0x80;
					this->code[1] = 0xe0 + reg1;
					this->code[2] = (uint8_t)imm;
					this->codeLength = 3;
				}
			} else {	   
				this->code[0] = 0x20; 
				this->code[1] = 0xc0 + reg1 + (reg2*8);
				this->codeLength = 2;	
			}
			break;
		case 16:
			if (isConstant) { 
				
				if (imm < 256) {
						this->code[0] = 0x66; 
						this->code[1] = 0x83; 
						this->code[2] = 0xe0 + reg1;
						this->code[3] = (uint8_t)imm;
						this->codeLength = 4;
				} else {
					this->code[0] = 0x66; 
					
					if (reg1 == 0) {
					
						this->code[1] = 0x25; 
						this->code[2] = ((uint16_t)imm) & 0xff; 
						this->code[3] = ((uint16_t)imm) >> 8; 
						this->codeLength = 4;
					} else {
						this->code[1] = 0x81; 
						this->code[2] = 0xe0 + reg1; 
						this->code[3] = ((uint16_t)imm) & 0xff; 
						this->code[4] = ((uint16_t)imm) >> 8; 
						this->codeLength = 5;
					}
				}
				//this->codeLength = 4;
			} else { 
				this->code[0] = 0x66;
				this->code[1] = 0x21;
				this->code[2] = 0xc0 + reg1 + (reg2*8); 
				this->codeLength = 3;	
			}
			break;
		case 32:
			if (isConstant) {
				if (imm < 256) {
						
					this->code[0] = 0x83;
					this->code[1] = 0xe0 + reg1; 
					this->code[2] = (uint8_t)imm;
					this->codeLength = 3;
				} else {
					if (reg1 == 0) {
					
						this->code[0] = 0x25;
						this->code[1] = imm & 0xff;
						this->code[2] = (imm >> 8) & 0xff;
						this->code[3] = (imm >> 16) & 0xff;
						this->code[4] = (imm >> 24) & 0xff;
						this->codeLength = 5;
					} else {
						this->code[0] = 0x81;
						this->code[1] = 0xe0 + reg1;
						this->code[2] = imm & 0xff;
						this->code[3] = (imm >> 8) & 0xff;
						this->code[4] = (imm >> 16) & 0xff;
						this->code[5] = (imm >> 24) & 0xff;
						this->codeLength = 6;
					}
				}	
			} else {
				this->code[0] = 0x21;
				this->code[1] = 0xc0 + reg1 + (reg2*8);
				this->codeLength = 2;	
			}
			break;
	}
}
*/
