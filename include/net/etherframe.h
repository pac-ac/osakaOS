#ifndef __OS__NET__ETHERFRAME_H
#define __OS__NET__ETHERFRAME_H

#include <common/types.h>
#include <drivers/amd_am79c973.h>
#include <memorymanagement.h>


namespace os {

	namespace net {



		struct EtherFrameHeader {

			common::uint64_t dstMAC_BE:48;
			common::uint64_t srcMAC_BE:48;
			
			common::uint16_t etherType_BE;

		} __attribute__((packed));

		typedef common::uint32_t EtherFrameFooter;




		class EtherFrameProvider;

		class EtherFrameHandler {
			
			//protected:
			public: 
				EtherFrameProvider* backend;
				common::uint16_t etherType_BE;
			public:
				EtherFrameHandler(EtherFrameProvider* backend, common::uint16_t etherType_BE);	
				~EtherFrameHandler();

				virtual bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);	
				void Send(common::uint64_t dstMAC_BE, common::uint8_t* etherframePayload, common::uint32_t size);	
				
				//common::uint32_t GetIPAddress();	
		};

		class EtherFrameProvider : public os::drivers::RawDataHandler {
	
			friend class EtherFrameHandler;

			//protected:
			public:
				EtherFrameHandler* handlers[65535];

			public:
				EtherFrameProvider(drivers::amd_am79c973* backend);	
				~EtherFrameProvider();	
			
				bool OnRawDataReceived(common::uint8_t* buffer, common::uint32_t size);
				void Send(common::uint64_t dstMAC_BE, common::uint16_t etherType_BE, common::uint8_t* buffer, common::uint32_t size);
	
				common::uint64_t GetMACAddress();	
				common::uint32_t GetIPAddress();	
		};
	}
}


#endif
