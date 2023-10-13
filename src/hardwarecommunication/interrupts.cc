#include <hardwarecommunication/interrupts.h>


using namespace os;
using namespace os::common;
using namespace os::hardwarecommunication;


void printf(char* str);
void printfHex(uint8_t);

void sleep(uint32_t);
uint16_t prng();
void forget();



InterruptHandler::InterruptHandler(uint8_t interruptNumber, InterruptManager* interruptManager) {

	this->interruptNumber = interruptNumber;
	this->interruptManager = interruptManager;
	
	interruptManager->handlers[interruptNumber] = this;
}



InterruptHandler::~InterruptHandler() {

	if (interruptManager->handlers[interruptNumber] = this) {
	
		interruptManager->handlers[interruptNumber] = 0;
	}
}



uint32_t InterruptHandler::HandleInterrupt(uint32_t esp) {


	return esp;
}



InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];
InterruptManager* InterruptManager::ActiveInterruptManager = 0;



void InterruptManager::SetInterruptDescriptorTableEntry (

	uint8_t interruptNumber,
        uint16_t codeSegmentSelectorOffset,
        void (*handler)(),
        uint8_t DescriptorPrivilegeLevel,
        uint8_t DescriptorType) {

	const uint8_t IDT_DESC_PRESENT = 0x80;

	interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
	interruptDescriptorTable[interruptNumber].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
	interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = codeSegmentSelectorOffset;
	interruptDescriptorTable[interruptNumber].access = IDT_DESC_PRESENT | DescriptorType | ((DescriptorPrivilegeLevel&3) << 5);
	interruptDescriptorTable[interruptNumber].reserved = 0;

}



InterruptManager::InterruptManager(uint16_t hardwareInterruptOffset, GlobalDescriptorTable* gdt, TaskManager* taskManager) 
: picMasterCommand(0x20),
  picMasterData(0x21), 
  picSlaveCommand(0xA0), 
  picSlaveData(0xA1) {


	this->taskManager = taskManager;
	this->hardwareInterruptOffset = hardwareInterruptOffset;
	
	this->interruptCount = 1;
	

	uint32_t CodeSegment = gdt->CodeSegmentSelector();
	const uint8_t IDT_INTERRUPT_GATE = 0xE;

	for (uint8_t i = 255; i > 0; i--) {
		
		SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
		handlers[i] = 0;
	}
	
	SetInterruptDescriptorTableEntry(0, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
	handlers[0] = 0;

	/*
	SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);
	*/

	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x01, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x02, CodeSegment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x03, CodeSegment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x04, CodeSegment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x05, CodeSegment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x06, CodeSegment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x07, CodeSegment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x08, CodeSegment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x09, CodeSegment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x0A, CodeSegment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x0B, CodeSegment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x0C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x0D, CodeSegment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x0E, CodeSegment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x0F, CodeSegment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);
	
	SetInterruptDescriptorTableEntry(hardwareInterruptOffset+0x31, CodeSegment, &HandleInterruptRequest0x31, 0, IDT_INTERRUPT_GATE);
	

	SetInterruptDescriptorTableEntry(                        0x80, CodeSegment, &HandleInterruptRequest0x80, 0, IDT_INTERRUPT_GATE);
	

	picMasterCommand.Write(0x11);
	picSlaveCommand.Write(0x11);


	picMasterData.Write(hardwareInterruptOffset);
	picSlaveData.Write(hardwareInterruptOffset+8);
	
	picMasterData.Write(0x04);
	picSlaveData.Write(0x02);
	
	picMasterData.Write(0x01);
	picSlaveData.Write(0x01);
	
	picMasterData.Write(0x00);
	picSlaveData.Write(0x00);
	
	
	
	InterruptDescriptorTablePointer idt;
	idt.size = 256 * sizeof(GateDescriptor) - 1;
	idt.base = (uint32_t)interruptDescriptorTable;
	asm volatile("lidt %0" : : "m" (idt));
}

InterruptManager::~InterruptManager() {

	Deactivate();
}


uint16_t InterruptManager::HardwareInterruptOffset() {

	return hardwareInterruptOffset;
}



void InterruptManager::Activate() {

	if (ActiveInterruptManager != 0) {
	
		ActiveInterruptManager->Deactivate();
	}

	ActiveInterruptManager = this;
	asm ("sti");
}



void InterruptManager::Deactivate() {
	
	if (ActiveInterruptManager != this) {
	
		ActiveInterruptManager = 0;
		asm ("cli");
	}
}



uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp) {

	if (ActiveInterruptManager != 0) {

		return ActiveInterruptManager->DoHandleInterrupt(interruptNumber, esp);
	}
	return esp;
}
	


uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp) {
	
	
	//'forget' feature
	this->interruptCount++;

	if (this->interruptCount == 0 && (prng() % 15) == 0) {
	//if (this->interruptCount == 0) {
	
		printf("\nuhhhhh.........\n");
		sleep(5000);
		forget();
	}



	if (handlers[interruptNumber] != 0) {
	
		esp = handlers[interruptNumber]->HandleInterrupt(esp);
	
							         //??????? qemu pata driver problems
	} else if (interruptNumber != hardwareInterruptOffset && interruptNumber != 0x2e) {
	//} else if (interruptNumber != hardwareInterruptOffset) {
		
		printf("UNHANDLED INTERRUPT ");
		printfHex(interruptNumber);
		printf(" probably not important lol\n");
	}


	if (interruptNumber == hardwareInterruptOffset) {
		esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
	}


	if (hardwareInterruptOffset <= interruptNumber && interruptNumber < hardwareInterruptOffset+16) {
	
		picMasterCommand.Write(0x20);

		if (hardwareInterruptOffset+8 <= interruptNumber) {
		
			picSlaveCommand.Write(0x20);
		}
	}
	return esp;
}

