#ifndef __OS__DRIVERS__SOUND16_H
#define __OS__DRIVERS__SOUND16_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>


namespace os {

	namespace drivers {

		class SoundBlaster16 : public Driver, public hardwarecommunication::InterruptHandler {
		//class SoundBlaster16 {
		
			//private:
			public:
				hardwarecommunication::Port16Bit IO_Port;
				
				hardwarecommunication::Port16Bit mixerPort;
				hardwarecommunication::Port16Bit mixerDataPort;
				hardwarecommunication::Port16Bit resetPort;
				hardwarecommunication::Port16Bit readPort;
				hardwarecommunication::Port16Bit writePort;
				
				hardwarecommunication::Port16Bit readStatusPort;
				hardwarecommunication::Port16Bit interruptAckPort;
			
			public:
				//SoundBlaster16();
				SoundBlaster16(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor *dev, 
						hardwarecommunication::InterruptManager* interrupts);
				
				~SoundBlaster16();
			
				common::uint32_t HandleInterrupt(common::uint32_t esp);
				void reset();
				
				/*
				common::uint8_t ReadSB();
				void WriteSB(common::uint8_t data);

				void PlaySound(common::uint8_t *data, common::uint32_t size, common::uint16_t rate);
				*/
		};
	}
}




#endif
