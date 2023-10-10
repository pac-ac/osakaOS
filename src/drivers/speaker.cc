#include <drivers/speaker.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;


void sleep(uint32_t ms);


Speaker::Speaker() 
: PIT2(0x42),
  PITcommand(0x43),
  speakerPort(0x61) {

}


Speaker::~Speaker() {
}


void Speaker::PlaySound(uint32_t freq) {


	uint32_t Div = 1193180 / freq;
	
	PITcommand.Write(0xb6);
	PIT2.Write((uint8_t)(Div));
	PIT2.Write((uint8_t)(Div >> 8));


	uint8_t tmp = speakerPort.Read();
	
	if (tmp != (tmp | 0x03)) {
	
		speakerPort.Write(tmp | 0x03);
	}
}



void Speaker::NoSound() {

	uint8_t tmp = speakerPort.Read();
	speakerPort.Write(tmp & 0xfc);
}



void Speaker::Speak(uint32_t freq) {
	
	uint32_t reset = PIT2.Read();

	this->PlaySound(freq);
	sleep(40);
	this->NoSound();

	PIT2.Write(reset);
}
