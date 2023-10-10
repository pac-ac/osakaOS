#include <drivers/sound16.h>


using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;


void printf(char*);
void printfHex(uint8_t);
void sleep(uint32_t);


SoundBlaster16::SoundBlaster16(PeripheralComponentInterconnectDeviceDescriptor 
		*dev, InterruptManager* interrupts) 
: Driver(),
  InterruptHandler(dev->interrupt + interrupts->HardwareInterruptOffset(), interrupts),	

//SoundBlaster16::SoundBlaster16():
  IO_Port(0x220),
  
  mixerPort(0x224),
  mixerDataPort(0x225),
  
  resetPort(0x226),
  readPort(0x22a),
  writePort(0x22c),
  
  readStatusPort(0x22e),
  interruptAckPort(0x22f)
{
	mixerPort.Write(0x80);
	mixerDataPort.Write(dev->interrupt);
}


SoundBlaster16::~SoundBlaster16() {
}




void SoundBlaster16::reset() {

	resetPort.Write(0x01);
	for (int i = 0; i < 1000; i++);
	
	resetPort.Write(0x00);
	//for (int i = 0; i < 1000 && !(mixerDataPort.Read() & 0x80); i++);


	if (resetPort.Read() != 0xaa) {
	
		printf("SoundBlaster16 reset failed.\n");
	} else {
		printf("It works!!!!!!!.\n");
	}	
}


uint32_t SoundBlaster16::HandleInterrupt(uint32_t esp) {

	//mixerPort.Write(0x80);
	//mixerDataPort.Write(esp);
	printf("Recieved interrupt from SoundBlaster 16.\n");

	readStatusPort.Read();
	interruptAckPort.Read();


	return esp;
}
