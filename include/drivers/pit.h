#ifndef __OS__DRIVERS__PIT_H
#define __OS__DRIVERS__PIT_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>


namespace os {

	namespace drivers {

		class PIT : public os::hardwarecommunication::InterruptHandler, public Driver {
	
			//private:
			public:
				hardwarecommunication::Port8Bit channel0;
				hardwarecommunication::Port8Bit channel1;
				hardwarecommunication::Port8Bit channel2;
				hardwarecommunication::Port8Bit commandPort;
				
				hardwarecommunication::Port8BitSlow PIC;

			public:
				PIT(os::hardwarecommunication::InterruptManager* manager);
				~PIT();

				common::uint32_t readCount();
				void setCount(common::uint32_t count);
				
				void sleep(common::uint32_t ms);

				virtual os::common::uint32_t HandleInterrupt(os::common::uint32_t esp);
		};
	}
}


#endif
