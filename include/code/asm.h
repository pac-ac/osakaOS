#ifndef __OS__CODE__ASM_H
#define __OS__CODE__ASM_H

#include <common/types.h>
#include <filesys/ofs.h>

namespace os {

	static char* reg32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", nullptr };
	static char* reg16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", nullptr };
	static char* reg8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", nullptr };



	typedef struct {

		common::uint8_t reg1;
		common::uint8_t regSize1;
		common::uint8_t reg2;
		common::uint8_t regSize2;
		common::uint32_t imm;
		bool isConstant;
	
	} OperandData;

	
	
	class Compiler {

		public:
			filesystem::FileSystem* filesystem = nullptr;

			common::uint8_t code[16];
			common::uint8_t codeLength;

		public:
			Compiler(filesystem::FileSystem* filesystem);
			~Compiler();
	
			void Assemble(char* asmFile, char* outFile);
	
			common::uint32_t EncodeAsm(char* instruction);
	
			common::uint8_t GetRegisterBitsAndOffset(char* operand, common::uint8_t* regBits);
			
			common::int8_t GetOperandValues(char* operand1, char* operand2, 
							common::uint8_t* reg1, common::uint8_t* regSize1,
							common::uint8_t* reg2, common::uint8_t* regSize2,
							common::uint32_t* imm, bool* isConstant);
	
			//generate instructions
			void GenMov(OperandData* operands);
			
			
			void GenOperation(OperandData* operands, 
					common::uint8_t opcode, 
					common::uint8_t opcodeEax, 
					common::uint8_t regBaseOffset);
		
			//void GenAdd(char* operand1, char* operand2);
			//void GenSub(char* operand1, char* operand2);
			
			//void GenAnd(char* operand1, char* operand2);
			//void GenOr(char* operand1, char* operand2);
			//void GenXor(char* operand1, char* operand2);
	};
	
	
	
}

#endif
