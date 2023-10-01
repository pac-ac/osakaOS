#ifndef __OS__DRIVERS__ATA_H
#define __OS__DRIVERS__ATA_H


#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>
#include <common/types.h>


namespace os {

	namespace drivers {

		class AdvancedTechnologyAttachment {
		
			//protected:
			public:
				hardwarecommunication::Port16Bit dataPort;
				hardwarecommunication::Port8Bit errorPort;
				hardwarecommunication::Port8Bit sectorCountPort;
				hardwarecommunication::Port8Bit lbaLowPort;
				hardwarecommunication::Port8Bit lbaMidPort;
				hardwarecommunication::Port8Bit lbaHiPort;
				hardwarecommunication::Port8Bit devicePort;
				hardwarecommunication::Port8Bit commandPort;
				hardwarecommunication::Port8Bit controlPort;

				bool master;
				common::uint16_t bytesPerSector;

			public:
				AdvancedTechnologyAttachment(common::uint16_t portBase, bool master);
				~AdvancedTechnologyAttachment();

				bool Identify();
				void Read28(common::uint32_t sector, common::uint8_t* data, int count, int offset);
				void Write28(common::uint32_t sector, common::uint8_t* data, int count, int offset);
				void Flush();

				
				
				
				void ReadPrintSector(common::uint32_t sector, int count);
				void WritePrintSector(common::uint32_t sector, common::uint8_t* data, int count);
		};
	}
}

#endif
