#ifndef __OS__DRIVERS__AC97_H
#define __OS__DRIVERS__AC97_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>


namespace os {

	namespace drivers {

		struct AC97Buffer {
			
			common::uint32_t sampleMemory;
			common::uint16_t sampleNum;
			common::uint16_t reserved: 14;
			common::uint8_t previousBuffer: 1;
			common::uint8_t interruptDone: 1;

		} __attribute__((packed));

		class AC97 : public Driver {
	
			//private:
			public:
				struct AC97Buffer* bufferPtr;
				
				hardwarecommunication::Port8Bit NAM;
				hardwarecommunication::Port8Bit NAM;
				hardwarecommunication::Port8Bit NABM;
				hardwarecommunication::Port8Bit NABM;

			public:
				AC97();
				AC97();

				void InitSoundCard(common::uint8_t soundCardNum);
				
		};
	}
}


#endif
