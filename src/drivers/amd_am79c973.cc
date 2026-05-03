#include <drivers/amd_am79c973.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;

void printf(char*);
void printfHex(uint8_t);

RawDataHandler::RawDataHandler(amd_am79c973* backend) {

	this->backend = backend;
	backend->SetHandler(this);
}

RawDataHandler::~RawDataHandler() {
	
	backend->SetHandler(0);
}


bool RawDataHandler::OnRawDataReceived(uint8_t* buffer, uint32_t size) {

	return false;
}

void RawDataHandler::Send(uint8_t* buffer, uint32_t size) {

	backend->Send(buffer, size);
}



amd_am79c973::amd_am79c973(
		PeripheralComponentInterconnectDeviceDescriptor* dev, 
		InterruptManager* interrupts) 
: Driver(),
  InterruptHandler(dev->interrupt + interrupts->HardwareInterruptOffset(), interrupts),
  //InterruptHandler(dev->interrupt + interrupts->HardwareInterruptOffset(), interrupts),
  
  MACAddress0Port(dev->portBase),
  MACAddress2Port(dev->portBase + 0x02),
  MACAddress4Port(dev->portBase + 0x04),
  registerDataPort(dev->portBase + 0x10),
  registerAddressPort(dev->portBase + 0x12),
  resetPort16(dev->portBase + 0x14),
  resetPort32(dev->portBase + 0x18),
  busControlRegisterDataPort(dev->portBase + 0x16)
{
	this->handler = 0;
	this->currentSendBuffer = 0;
	this->currentRecvBuffer = 0;
	this->verbose = false;
	

	uint64_t MAC0 = MACAddress0Port.Read() % 256;
	uint64_t MAC1 = MACAddress0Port.Read() / 256;
	uint64_t MAC2 = MACAddress2Port.Read() % 256;
	uint64_t MAC3 = MACAddress2Port.Read() / 256;
	uint64_t MAC4 = MACAddress4Port.Read() % 256;
	uint64_t MAC5 = MACAddress4Port.Read() / 256;

	uint64_t MAC = MAC5 << 40
		     | MAC4 << 32
		     | MAC3 << 24
		     | MAC2 << 16
		     | MAC1 << 8
		     | MAC0;


	// 32 bit mode
	registerAddressPort.Write(20);
	busControlRegisterDataPort.Write(0x102);
  
	/*
	//resetPort16.Write(58);
  	registerAddressPort.Write(58);
  	uint32_t csr58 = registerDataPort.Read();
	csr58 |= 2;
  	registerAddressPort.Write(58);
  	registerDataPort.Write(csr58);
	*/

	// STOP reset
	registerAddressPort.Write(0);
	registerDataPort.Write(0x04);

	// initBlock
	initBlock.mode = 0x0000; // promiscuous mode = false
	initBlock.reserved1 = 0;
	initBlock.numSendBuffers = 3;
	initBlock.reserved2 = 0;
	initBlock.numRecvBuffers = 3;
	initBlock.physicalAddress = MAC;
	initBlock.reserved3 = 0;
	initBlock.logicalAddress = 0;

	sendBufferDescr = (BufferDescriptor*)((((uint32_t)&sendBufferDescrMemory[0]) + 15) & ~((uint32_t)0xf));
	initBlock.sendBufferDescrAddress = (uint32_t)sendBufferDescr;
	//initBlock.sendBufferDescrAddress = *((uint32_t*)sendBufferDescr);
	
	recvBufferDescr = (BufferDescriptor*)((((uint32_t)&recvBufferDescrMemory[0]) + 15) & ~((uint32_t)0xf));
	initBlock.recvBufferDescrAddress = (uint32_t)recvBufferDescr;
	//initBlock.recvBufferDescrAddress = *((uint32_t*)recvBufferDescr);
	

	for (uint8_t i = 0; i < 8; i++) {
	
		sendBufferDescr[i].address = (((uint32_t)&sendBuffers[i]) + 15) & ~(uint32_t)0xf;
		sendBufferDescr[i].flags = 0x7ff | 0xf000;
		//sendBufferDescr[i].flags |= 800000;
		//sendBufferDescr[i].flags = 0xf7ff | 0x800000;
		
		sendBufferDescr[i].flags2 = 0;
		sendBufferDescr[i].avail = 0;
				
		recvBufferDescr[i].address = (((uint32_t)&recvBuffers[i]) + 15) & ~(uint32_t)0xf;
		recvBufferDescr[i].flags = 0xf7ff | 0x80000000;
		recvBufferDescr[i].flags2 = 0;
		sendBufferDescr[i].avail = 0;
	}

	registerAddressPort.Write(1);
	registerDataPort.Write(((uint32_t)(&initBlock)) & 0xffff);
	registerAddressPort.Write(2);
	registerDataPort.Write((((uint32_t)(&initBlock)) >> 16) & 0xffff);

	/*
	registerAddressPort.Write(3);
	uint16_t interruptMask = registerDataPort.Read();
	
	interruptMask &= ~(1 << 10);
	interruptMask &= ~(1 << 9);
	interruptMask &= ~(1 << 8);
	interruptMask &= ~(1 << 2);
	
	registerAddressPort.Write(3);
	registerDataPort.Write(interruptMask);
	*/
}



amd_am79c973::~amd_am79c973() {
}



void amd_am79c973::Activate() {

	registerAddressPort.Write(0);
	registerDataPort.Write(0x41);

	registerAddressPort.Write(4);
	uint32_t temp = registerDataPort.Read();
	registerAddressPort.Write(4);
	registerDataPort.Write(temp | 0xc00);
	
	registerAddressPort.Write(0);
	registerDataPort.Write(0x42);
}


void sleep(uint32_t);

int amd_am79c973::Reset() {

	resetPort32.Read();
	resetPort16.Read();
	sleep(1);

	//resetPort16.Write(0);
	//registerDataPort.Write(0);
	return 10;
}


uint32_t amd_am79c973::HandleInterrupt(uint32_t esp) {

	//printf("\nINTERRUPT FROM AMD am79c973\n");

	registerAddressPort.Write(0);
	uint32_t temp = registerDataPort.Read();

	if ((temp & 0x0400) == 0x0400) { this->Receive(); }

	/*	
	if ((temp & 0x8000) == 0x8000) { printf(" AMD am79c973 ERROR\n"); } 
	if ((temp & 0x2000) == 0x2000) { printf(" AMD am79c973 COLLISION ERROR\n"); } 
	if ((temp & 0x1000) == 0x1000) { printf(" AMD am79c973 MISSED FRAME\n"); }
	if ((temp & 0x0800) == 0x0800) { printf(" AMD am79c973 MEMORY ERROR\n"); }
	if ((temp & 0x0400) == 0x0400) { this->Receive();  printf("AMD am79c973 DATA RECEIVED\n"); }
	if ((temp & 0x0200) == 0x0200) { printf(" AMD am79c973 DATA SENT\n"); } 
	if ((temp & 0x0100) == 0x0100) { printf("AMD am79c973 INIT DONE\n"); }
	*/

	//printf(int2str(temp));
	//printf("\n");
	
	
	//acknowledge
	registerAddressPort.Write(0);
	registerDataPort.Write(temp);

	/*
	registerAddressPort.Write(0);
	temp = registerDataPort.Read();
	printf(int2str(temp));
	printf("\n");
	*/

	return esp;
}




void amd_am79c973::Send(uint8_t* buffer, int size) {
	
	int sendDescriptor = currentSendBuffer;
	currentSendBuffer = (currentSendBuffer + 1) % 8;

	//ethernet max size
	if (size > 1518) { size = 1518; }

	for (uint8_t* src = buffer + size - 1, 
		*dst = (uint8_t*)(sendBufferDescr[sendDescriptor].address + size - 1);
			src >= buffer; src--, dst--) {
		*dst = *src;
	}

	
	//print send data	
	if (this->verbose) {
	
		printf("Sending Data: ");
		for (int i = 14+20; i < (size>64?64:size); i++) {
	
			//printfHex(*((uint32_t*)(sendBufferDescr[sendDescriptor].address+i)));
			printfHex(buffer[i]);
			printf(" ");
		}
		printf("\n");
	}


	sendBufferDescr[sendDescriptor].avail = 0;
	sendBufferDescr[sendDescriptor].flags2 = 0;
	sendBufferDescr[sendDescriptor].flags = 0x8300f000 | ((uint16_t)((-size) & 0xfff));
	//recvBufferDescr[sendDescriptor].flags = 0xf7ff | 0x80000000;
	
	registerAddressPort.Write(0);
	registerDataPort.Write(0x48);
	
	/*	
	registerAddressPort.Write(0);
	uint32_t temp = registerDataPort.Read();
	registerAddressPort.Write(0);
	registerDataPort.Write(temp | 0x0200);
	*/
}



void amd_am79c973::Receive() {

	for (; (recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0; 
		currentRecvBuffer = (currentRecvBuffer + 1) % 8) {
	
		if (!(recvBufferDescr[currentRecvBuffer].flags & 0x40000000) &&
		     (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000) {
		
			uint32_t size = recvBufferDescr[currentRecvBuffer].flags & 0xfff;
			
			//remove checksum
			if (size > 64) { size -= 4; }

			uint8_t* buffer = (uint8_t*)(recvBufferDescr[currentRecvBuffer].address);
			

			//print receive data
			if (this->verbose) {
				
				printf("Receiving Data: ");
				for (int i = 14+20; i < (size>64?64:size); i++) {
			
					printfHex(buffer[i]);
					printf(" ");
				}
				printf("\n");
			}
			
			/*
			printf("Receiving Data: ");
			for (int i = 14+20; i < size; i++) {
			
				char* foo = " ";
				foo[0] = (char)(buffer[i]);
				printf(foo);
			}
			*/



			if (handler != 0) {
				if (handler->OnRawDataReceived(buffer, size)) {
				
					Send(buffer, size);
				}
			}
		}

		recvBufferDescr[currentRecvBuffer].flags2 = 0;
		recvBufferDescr[currentRecvBuffer].flags = 0x8000f7ff;
	}
	
	/*
	registerAddressPort.Write(0);
	uint32_t temp = registerDataPort.Read();
	registerAddressPort.Write(0);
	registerDataPort.Write(temp | 0x0400);
	*/
}


void amd_am79c973::SetHandler(RawDataHandler* handler) {

	this->handler = handler;
}


uint64_t amd_am79c973::GetMACAddress() {

	return initBlock.physicalAddress;
}


void amd_am79c973::SetIPAddress(uint32_t ip) {

	initBlock.logicalAddress = ip;
}

uint32_t amd_am79c973::GetIPAddress() {

	return initBlock.logicalAddress;
}
