#include <net/arp.h>

using namespace os;
using namespace os::common;
using namespace os::net;
using namespace os::drivers;



AddressResolutionProtocol::AddressResolutionProtocol(EtherFrameProvider* backend) 
: EtherFrameHandler(backend, 0x806) {
	
	numCacheEntries = 0;
}


AddressResolutionProtocol::~AddressResolutionProtocol() {
}

void printf(char*);
void printfHex(uint8_t);

bool AddressResolutionProtocol::OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size) {
	
	if (size < sizeof(AddressResolutionProtocolMessage)) {
	
		return false;
	}

	AddressResolutionProtocolMessage* arp = (AddressResolutionProtocolMessage*)etherframePayload;

	if (arp->hardwareType == 0x0100) {
	
		if (arp->protocol == 0x0008
			&& arp->hardwareAddressSize == 6
			&& arp->protocolAddressSize == 4
			&& arp->dstIP == backend->GetIPAddress()) {
		
			switch (arp->command) {
			
				case 0x0100: // request
					
					arp->command = 0x0200;
					arp->dstIP = arp->srcIP;
					arp->dstMAC = arp->srcMAC;

					arp->srcIP = backend->GetIPAddress();
					arp->srcMAC = backend->GetMACAddress();
					
					return true;
					break;
				
				case 0x0200: // response

					if (numCacheEntries < 128) {
						
						IPcache[numCacheEntries] = arp->srcIP;
						MACcache[numCacheEntries] = arp->srcMAC;
						numCacheEntries++;
					}
					
					break;
			}
		}
		
	}

	return false;
}



void AddressResolutionProtocol::BroadcastMACAddress(uint32_t IP_BE) {
	
	AddressResolutionProtocolMessage* arp;
	arp->hardwareType = 0x0100; // ethernet
	arp->protocol = 0x0008; // ipv4
	arp->hardwareAddressSize = 6; //mac
	arp->protocolAddressSize = 4; //ipv4
	arp->command = 0x0200; // response
	
	arp->srcMAC = backend->GetMACAddress();
	arp->srcIP = backend->GetIPAddress();
	arp->dstMAC = Resolve(IP_BE);
	arp->dstIP = IP_BE;

	this->Send(arp->dstMAC, (uint8_t*)arp, sizeof(AddressResolutionProtocolMessage));
}


void AddressResolutionProtocol::RequestMACAddress(uint32_t IP_BE) {

	AddressResolutionProtocolMessage* arp;
	arp->hardwareType = 0x0100; // ethernet
	arp->protocol = 0x0008; // ipv4
	arp->hardwareAddressSize = 6; //mac
	arp->protocolAddressSize = 4; //ipv4
	arp->command = 0x0100; // request
	
	arp->srcMAC = backend->GetMACAddress();
	arp->srcIP = backend->GetIPAddress();
	arp->dstMAC = 0xffffffffffff;
	arp->dstIP = IP_BE;

	this->Send(arp->dstMAC, (uint8_t*)arp, sizeof(AddressResolutionProtocolMessage));

	/*	
	AddressResolutionProtocolMessage arp;
	arp.hardwareType = 0x0100; // ethernet
	arp.protocol = 0x0008; // ipv4
	arp.hardwareAddressSize = 6; //mac
	arp.protocolAddressSize = 4; //ipv4
	arp.command = 0x0100; // request
	
	arp.srcMAC = backend->GetMACAddress();
	arp.srcIP = backend->GetIPAddress();
	arp.dstMAC = 0xffffffffffff;
	arp.dstIP = IP_BE;

	this->Send(arp.dstMAC, (uint8_t*)&arp, sizeof(AddressResolutionProtocolMessage));
	*/
}


uint64_t AddressResolutionProtocol::GetMACFromCache(common::uint32_t IP_BE) {


	for (uint8_t i = 0; i < numCacheEntries; i++) {
	
		if (IPcache[i] == IP_BE) {
		
			return MACcache[i];
		}
	}
	
	//FF:FF:FF:FF:FF:FF broadcast
	return 0xffffffffffff;
}



void sleep(uint32_t);

uint64_t AddressResolutionProtocol::Resolve(common::uint32_t IP_BE) {


	uint64_t result = GetMACFromCache(IP_BE);

	if (result == 0xffffffffffff) {
	
		RequestMACAddress(IP_BE);
	}

	
	uint8_t attempts = 0;

	while (result == 0xffffffffffff && attempts < 128) { //possible infinite loop
	
		result = GetMACFromCache(IP_BE);
		attempts++;

	}

	if (attempts >= 128) {
	
		printf("\nARP Resolve has timed out.\n");
	} else {
	
		printf("\nARP Resolve has NOT timed out.\n");
	}	

	
	return result;
}
