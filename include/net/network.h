#ifndef __OS__NET__NETWORK_H
#define __OS__NET__NETWORK_H

#include <drivers/amd_am79c973.h>
#include <common/types.h>
#include <filesys/ofs.h>
#include <drivers/vga.h>
#include <net/arp.h>
#include <net/etherframe.h>
#include <net/icmp.h>
#include <net/ipv4.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <string.h>
#include <list.h>


#define HANDLE_FLAG_EMPTY 0
#define HANDLE_FLAG_TCP 1
#define HANDLE_FLAG_UDP 2

#define RESPONSE_HTTP "HTTP/1.1 200 OK\r\nServer: osakaOS\r\nContent-Type: text/html\r\n\r\n"
#define HNM_PORT 54740

namespace os {

	namespace net {
	
		class Network :  public UserDatagramProtocolHandler, 
				 public TransmissionControlProtocolHandler {
	
			public:
				drivers::amd_am79c973* eth0;
				AddressResolutionProtocol* arp;
				InternetProtocolProvider* ipv4;
				InternetControlMessageProtocol* icmp;
				UserDatagramProtocolProvider* udp;
				TransmissionControlProtocolProvider* tcp;

				filesystem::FileSystem* filesystem;
				common::int8_t requestID;

				common::uint32_t ip;
				common::uint32_t mac;
				common::uint32_t gateway;
				common::uint32_t subnet;
				
				common::uint8_t* HNM_Buffer = nullptr;
				//common::uint8_t HNM_Buffer[BUFFER_SIZE_13H];

				common::uint8_t* fileData = nullptr;
				common::uint32_t fileDataSize = 0;
				bool activateServerHTTP = false;
				bool activateServerHNM = false;
				bool activateServerCMD = false;
			public:
				Network(drivers::amd_am79c973* eth0,
					AddressResolutionProtocol* arp,
					InternetProtocolProvider* ipv4,
					InternetControlMessageProtocol* icmp,
					UserDatagramProtocolProvider* udp,
					TransmissionControlProtocolProvider* tcp,
					filesystem::FileSystem* filesystem,
					common::uint32_t gip, common::uint32_t subnet);
				~Network();
				
				
				void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, 
									common::uint8_t* data, common::uint16_t size);

				bool HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, 
									common::uint8_t* data, common::uint16_t size);
				
				void EmptyHandleBufferTCP(TransmissionControlProtocolSocket* socket);
				void EmptyHandleBufferUDP(UserDatagramProtocolSocket* socket);
		};
	}
}

#endif
