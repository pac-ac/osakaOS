#include <drivers/pit.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;


void printf(char*);


PIT::PIT(InterruptManager* manager) 

: InterruptHandler(0x00, manager),
  channel0(0x40),
  channel1(0x41),
  channel2(0x42),
  commandPort(0x43),
  PIC(0x20) {

}


PIT::~PIT() {
}


void PIT::sleep(uint32_t ms) {

	for (uint32_t i = 0; i < ms; i++) {
	
		this->setCount(1193182/1000);
		uint32_t start = this->readCount();

		while ((start - this->readCount()) < 1000) {}
	}
}


uint32_t PIT::readCount() {

	uint32_t count = 0;

	asm("cli");

	commandPort.Write(0b0000000);

	count = channel0.Read();
	count |= channel0.Read() << 8;

	asm("sti");
	return count;
}

void PIT::setCount(uint32_t count) {

	asm("cli");

	channel0.Write(count);
	channel0.Write(count >> 8);
	
	asm("sti");
}

uint32_t PIT::HandleInterrupt(uint32_t esp) {



	//send EOI to the PIC
	PIC.Write(0x20);

	return esp;
}
