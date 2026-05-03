#include <drivers/ac97.h>

using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;


void sleep(uint32_t);
void printf(char*);

AC97::AC97(PeripheralComponentInterconnectDeviceDescriptor* dev,
		InterruptManager* interrupts, uint32_t nabm_offset) 
	: Driver(),
	  InterruptHandler(dev->interrupt + interrupts->HardwareInterruptOffset(), interrupts),	

	  //nam
	  resetPort(dev->portBase),
	  masterVolumePort(dev->portBase + 0x02),
	  auxVolumePort(dev->portBase + 0x04),
	  micVolumePort(dev->portBase + 0x0e),
	  pcmVolumePort(dev->portBase + 0x18),
	  inputDevicePort(dev->portBase + 0x1a),
	  inputGainPort(dev->portBase + 0x1c),
	  micGainPort(dev->portBase + 0x1e),
	  extCapabilitiesPort(dev->portBase + 0x28),
	  controlExtCapPort(dev->portBase + 0x2a),
	  ratePcmFrontDacPort(dev->portBase + 0x2c),
	  ratePcmSurrDacPort(dev->portBase + 0x2e),
	  ratePcmLfeDacPort(dev->portBase + 0x30),
	  ratePcmLeftRightPort(dev->portBase + 0x32),

	  //nabm in
	  buffDescrAddressIN(nabm_offset),
	  procDescrEntryNumIN(nabm_offset + 0x04),
	  allDescrEntryNumIN(nabm_offset + 0x05),
	  dataTransferStatusIN(nabm_offset + 0x06),
	  sampleTransferNumIN(nabm_offset + 0x08),
	  buffNextEntryNumIN(nabm_offset + 0x0a),
	  controlTransferIN(nabm_offset + 0x0b),
	  
	  //nabm out
	  buffDescrAddressOUT(nabm_offset + 0x10),
	  procDescrEntryNumOUT(nabm_offset + 0x14),
	  allDescrEntryNumOUT(nabm_offset + 0x15),
	  dataTransferStatusOUT(nabm_offset + 0x16),
	  sampleTransferNumOUT(nabm_offset + 0x18),
	  buffNextEntryNumOUT(nabm_offset + 0x1a),
	  controlTransferOUT(nabm_offset + 0x1b),

	  globalControlRegister(nabm_offset + 0x2c),
	  globalStatusRegister(nabm_offset + 0x30)
{
	//resume card from cold reset and enable interrupts
	globalControlRegister.Write(0x3);

	//reset all NAM registers
	resetPort.Write(0x00);

	uint32_t cardCapabilities = globalStatusRegister.Read();

	//set max volume
	pcmVolumePort.Write(0x00);
	
	//set max volume for left and right channels
	masterVolumePort.Write(0x3030);
	auxVolumePort.Write(0x3030);

	//set default sample rate of 44.1k
	ratePcmFrontDacPort.Write(44100);
	ratePcmSurrDacPort.Write(44100);
	ratePcmLfeDacPort.Write(44100);
	ratePcmLeftRightPort.Write(44100);
}


AC97::~AC97() {
}


void AC97::PlaySound(AC97Buffer* buffer) {

	printf("Playing test sound...\n");
	//this->bufferDescriptorList[bufferEntryNum] = buffer;
	this->bufferPtr = buffer;
	
	//stop sound
	controlTransferOUT.Write(0x00);

	//min volume
	//masterVolumePort.Write(0x1010);
	//auxVolumePort.Write(0x1010);

	controlTransferOUT.Write(0x2);

	//wait for card to clear register
	//sleep(10);
	uint8_t ticks = 0;
	while ((controlTransferOUT.Read() & 0x2) == 0x02) {
	
		if (ticks>100) { 
			printf("AC97 status timed out.\n");
			return; 
		}
		ticks++;
	}
	controlTransferOUT.Write(0x00);

	//write buffer address and num in list
	buffDescrAddressOUT.Write((uint32_t)(this->bufferDescriptorList));
	
	for (uint32_t i = 0; i < 32; i++) {
	
		this->bufferDescriptorList[i].sampleAddress = buffer->sampleAddress;
		this->bufferDescriptorList[i].sampleNum = buffer->sampleNum;
		this->bufferEntryNum = i;
	}
	allDescrEntryNumOUT.Write(this->bufferEntryNum);


	//clear status register
	globalStatusRegister.Write(0x1c);
	
	//allDescrEntryNumOUT.Write(this->bufferEntryNum);
	//this->bufferEntryNum++;
	
	//send out
	controlTransferOUT.Write(0x1);
}


void AC97::Activate() {
	
	//resume card from cold reset and enable interrupts
	globalControlRegister.Write(0x3);

	//reset all NAM registers
	resetPort.Write(0x00);

	uint32_t cardCapabilities = globalStatusRegister.Read();

	//set max volume
	pcmVolumePort.Write(0x00);
	
	//set max volume for left and right channels
	//masterVolumePort.Write(0x3030);
	//auxVolumePort.Write(0x3030);

	//set default sample rate of 44.1k
	ratePcmFrontDacPort.Write(44100);
	ratePcmSurrDacPort.Write(44100);
	ratePcmLfeDacPort.Write(44100);
	ratePcmLeftRightPort.Write(44100);
}


uint32_t AC97::HandleInterrupt(uint32_t esp) {
	
	printf("interrupt received by ac97 sound!\n");

	//clear status register
	globalStatusRegister.Write(0x1c);

	return esp;
}
