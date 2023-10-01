#ifndef __OS__NET__IPV4_H
#define __OS__NET__IPV4_H

#include <common/types.h>
#include <net/etherframe.h>
#include <net/arp.h>


namespace os {

	namespace net {


		struct InternetProtocolV4Message {
		
			common::uint8_t headerLength:4;
			common::uint8_t version:4;
			common::uint8_t tos;
			common::uint16_t totalLength;
			
			common::uint16_t ident;
			//common::uint8_t flags:3;
			//common::uint16_t fragmentOffset:13;
			common::uint16_t flagsAndOffset;
			
			common::uint8_t timeToLive;
			common::uint8_t protocol;
			common::uint16_t checksum;
			
			common::uint32_t srcIP;
			common::uint32_t dstIP;

		} __attribute((packed));	


		class InternetProtocolProvider;

		class InternetProtocolHandler {
		
			//protected:
			public:
				InternetProtocolProvider* backend;
				common::uint8_t ip_protocol;

			public:
				InternetProtocolHandler(InternetProtocolProvider* backend, common::uint8_t protocol);
				~InternetProtocolHandler();

				virtual bool OnInternetProtocolReceived(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, 
						common::uint8_t* internetprotocolPayload, common::uint32_t size);
				
				
				void Send(common::uint32_t dstIP_BE, common::uint8_t* internetprotocolPayload, common::uint32_t size);
		};



		class InternetProtocolProvider : public EtherFrameHandler {
		
			friend class InternetProtocolHandler;

			//protected:
			public:
				InternetProtocolHandler* handlers[255];
				AddressResolutionProtocol* arp;
				common::uint32_t gatewayIP;
				common::uint32_t subnetMask;

			public:
				InternetProtocolProvider(EtherFrameProvider* backend, 
						         AddressResolutionProtocol* arp, 
							 common::uint32_t gatewayIP, 
							 common::uint32_t subnetMask);
				
				~InternetProtocolProvider();



				bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);


				void Send(common::uint32_t dstIP_BE, common::uint8_t protocol, common::uint8_t* buffer, common::uint32_t size);


				static common::uint16_t Checksum(common::uint16_t* data, common::uint32_t lengthInBytes);
		};

	}
}


#endif
