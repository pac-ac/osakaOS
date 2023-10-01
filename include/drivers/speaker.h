#ifndef __OS__DRIVERS__SPEAKER_H
#define __OS__DRIVERS__SPEAKER_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>


namespace os {

	namespace drivers {

		//class Speaker : public Drivers {
		class Speaker {
	
			//private:
			public:
				hardwarecommunication::Port8Bit PIT2;
				hardwarecommunication::Port8Bit PITcommand;
			
				hardwarecommunication::Port8Bit speakerPort;

			public:
				Speaker();
				~Speaker();

				void PlaySound(common::uint32_t freq);
				void NoSound();
				void Speak(common::uint32_t freq);
		};
	}
}


#endif
