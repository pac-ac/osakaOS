#ifndef __OS__DRIVERS__CMOS_H
#define __OS__DRIVERS__CMOS_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/pit.h>

namespace os {

	namespace drivers {

		struct RTCData {

			common::uint8_t second;
			common::uint8_t minute;
			common::uint8_t hour;
			common::uint8_t day;
			common::uint8_t month;
			common::int32_t year;

		} __attribute__((packed));


		class CMOS {

			public:
				hardwarecommunication::Port8Bit WriteCMOS;
				hardwarecommunication::Port8Bit ReadCMOS;
		
				RTCData timeData;

				//here purely for conveniency in sum else
				PIT* pit;
			public:
				CMOS();
				~CMOS();

				void CMOS_OUT(common::uint8_t val, common::uint8_t reg);
				common::uint8_t CMOS_IN(common::uint8_t reg);

				//rtc date and time stuff
				common::int32_t GetUpdate();
				common::uint8_t GetRegisterRTC(common::int32_t reg);
				void ReadRTC();
			
				void DumpRTC();
		};	

	}
}


#endif
