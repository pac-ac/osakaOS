#ifndef __OS__DRIVERS__AC97_H
#define __OS__DRIVERS__AC97_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <string.h>


namespace os {

	namespace drivers {

		struct AC97Buffer {
			
			common::uint32_t sampleAddress;
			common::uint16_t sampleNum;
			common::uint16_t reserved: 14;
			common::uint8_t previousBuffer: 1;
			common::uint8_t interruptDone: 1;

		} __attribute__((packed));

		class AC97 : public Driver, public hardwarecommunication::InterruptHandler {
	
			//private:
			public:
				struct AC97Buffer* bufferPtr = nullptr;
				
				AC97Buffer bufferDescriptorList[32];
				common::uint8_t bufferEntryNum = 0;


				hardwarecommunication::Port16Bit resetPort;
				hardwarecommunication::Port16Bit masterVolumePort;
				hardwarecommunication::Port16Bit auxVolumePort;
				hardwarecommunication::Port16Bit micVolumePort;
				hardwarecommunication::Port16Bit pcmVolumePort;
				hardwarecommunication::Port16Bit inputDevicePort;
				hardwarecommunication::Port16Bit inputGainPort;
				hardwarecommunication::Port16Bit micGainPort;
				hardwarecommunication::Port16Bit extCapabilitiesPort;
				hardwarecommunication::Port16Bit controlExtCapPort;
				hardwarecommunication::Port16Bit ratePcmFrontDacPort;
				hardwarecommunication::Port16Bit ratePcmSurrDacPort;
				hardwarecommunication::Port16Bit ratePcmLfeDacPort;
				hardwarecommunication::Port16Bit ratePcmLeftRightPort;
				
				hardwarecommunication::Port32Bit buffDescrAddressIN;
				hardwarecommunication::Port8Bit procDescrEntryNumIN;
				hardwarecommunication::Port8Bit allDescrEntryNumIN;
				hardwarecommunication::Port16Bit dataTransferStatusIN;
				hardwarecommunication::Port16Bit sampleTransferNumIN;
				hardwarecommunication::Port8Bit buffNextEntryNumIN;
				hardwarecommunication::Port8Bit controlTransferIN;
				
				hardwarecommunication::Port32Bit buffDescrAddressOUT;
				hardwarecommunication::Port8Bit procDescrEntryNumOUT;
				hardwarecommunication::Port8Bit allDescrEntryNumOUT;
				hardwarecommunication::Port16Bit dataTransferStatusOUT;
				hardwarecommunication::Port16Bit sampleTransferNumOUT;
				hardwarecommunication::Port8Bit buffNextEntryNumOUT;
				hardwarecommunication::Port8Bit controlTransferOUT;

				hardwarecommunication::Port32Bit globalControlRegister;
				hardwarecommunication::Port32Bit globalStatusRegister;

			public:
				AC97(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* dev,
						hardwarecommunication::InterruptManager* interrupts, common::uint32_t nabm_offset);
				~AC97();

				void PlaySound(AC97Buffer* buffer);

				void Activate();

				common::uint32_t HandleInterrupt(common::uint32_t esp);
		};
	}
}


#endif
