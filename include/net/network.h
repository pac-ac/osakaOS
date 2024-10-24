#ifndef __OS__NET__NETWORK_H
#define __OS__NET__NETWORK_H

#include <drivers/amd_am79c973.h>
#include <common/types.h>
#include <net/arp.h>
#include <net/etherframe.h>
#include <net/icmp.h>
#include <net/ipv4.h>


namespace os {

	namespace net {
	
		class Network {
	
			public:
				drivers::amd_am79c973* eth0;
				AddressResolutionProtocol* arp;
				InternetProtocolProvider* ipv4;
				InternetControlMessageProtocol* icmp;
		
				common::uint32_t ip;
				common::uint32_t mac;
				common::uint32_t gateway;
				common::uint32_t subnet;
			public:
				Network(drivers::amd_am79c973* eth0,
					AddressResolutionProtocol* arp,
					InternetProtocolProvider* ipv4,
					InternetControlMessageProtocol* icmp,
					common::uint32_t gip, common::uint32_t subnet);
				~Network();
		};
	}
}

#endif
