#ifndef __OS__NET__UDP_H
#define __OS__NET__UDP_H

#include <common/types.h>
#include <net/ipv4.h>
#include <memorymanagement.h>

#define HANDLE_BUF_SIZE_UDP 0xffff

namespace os {

	namespace net {

		struct UserDatagramProtocolHeader {
		
			common::uint16_t srcPort;
			common::uint16_t dstPort;
			common::uint16_t length;
			common::uint16_t checksum;
		
		} __attribute__((packed));
		
		
		class UserDatagramProtocolProvider;
		class UserDatagramProtocolHandler;

		class UserDatagramProtocolSocket {
		
			friend class UserDatagramProtocolProvider;

			public:
				common::uint32_t remoteIP;
				common::uint32_t localIP;
				common::uint16_t remotePort;
				common::uint16_t localPort;
				UserDatagramProtocolProvider* backend;
				UserDatagramProtocolHandler* handler;

				bool listening;

				common::uint8_t handleBuffer[HANDLE_BUF_SIZE_UDP];
				common::uint32_t bufferIndex = 0;
				common::uint8_t handleType = 0;
			public:
				UserDatagramProtocolSocket(UserDatagramProtocolProvider* backend);
				~UserDatagramProtocolSocket();
				virtual void HandleUserDatagramProtocolMessage(common::uint8_t* data, common::uint16_t size);
				virtual void Send(common::uint8_t* data, common::uint16_t size);
				virtual void Disconnect();
		};
		
		class UserDatagramProtocolHandler {
		
			public:
				UserDatagramProtocolHandler();
				~UserDatagramProtocolHandler();
				
				virtual void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, 
										common::uint8_t* data, common::uint16_t size);
		};


		class UserDatagramProtocolProvider : InternetProtocolHandler {
		
			public:
				UserDatagramProtocolSocket* sockets[65535];
				common::uint16_t numSockets;
				common::uint16_t freePort;
			public:
				UserDatagramProtocolProvider(InternetProtocolProvider* backend);
				~UserDatagramProtocolProvider();

				virtual bool OnInternetProtocolRecieved(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE,
									common::uint8_t* internetProtocolPayload, common::uint32_t size);
		
				virtual UserDatagramProtocolSocket* Connect(common::uint32_t ip, common::uint16_t port);	
				virtual UserDatagramProtocolSocket* Listen(common::uint16_t port);	
				virtual void Disconnect(UserDatagramProtocolSocket* socket);

				virtual void Send(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
				virtual void Bind(UserDatagramProtocolSocket* socket, UserDatagramProtocolHandler* handler);
		};		
	}
}

#endif
