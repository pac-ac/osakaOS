#include <net/ipv4.h>


using namespace os;
using namespace os::common;
using namespace os::net;




InternetProtocolHandler::InternetProtocolHandler(InternetProtocolProvider* backend, uint8_t protocol) {

	this->backend = backend;
	this->ip_protocol = protocol;
	backend->handlers[protocol] = this;
}



InternetProtocolHandler::~InternetProtocolHandler() {

	if (backend->handlers[ip_protocol] == this) {
	
		backend->handlers[ip_protocol] = 0;
	}
}





bool InternetProtocolHandler::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE,
                                                uint8_t* internetprotocolPayload, uint32_t size) {

	return false;
}


void InternetProtocolHandler::Send(uint32_t dstIP_BE, uint8_t* internetprotocolPayload, uint32_t size) {

	backend->Send(dstIP_BE, ip_protocol, internetprotocolPayload, size);

}               









InternetProtocolProvider::InternetProtocolProvider(EtherFrameProvider* backend, 
                                                   AddressResolutionProtocol* arp, 
                                                   uint32_t gatewayIP, 
                                                   uint32_t subnetMask) 
						   : EtherFrameHandler(backend, 0x800) {

	for (int i = 0; i < 255; i++) {
	
		handlers[i] = 0;
	}

	this->arp = arp;
	this->gatewayIP = gatewayIP;
	this->subnetMask = subnetMask;
}


InternetProtocolProvider::~InternetProtocolProvider() {
}




bool InternetProtocolProvider::OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size) {

	if (size < sizeof(InternetProtocolV4Message)) {
	
		return false;
	}

	InternetProtocolV4Message* ipmessage = (InternetProtocolV4Message*)etherframePayload;
	bool sendBack = false;

	if (ipmessage->dstIP == backend->GetIPAddress()) {
	
		int length = ipmessage->totalLength;
		if (length > size) {
		
			length = size;
		}

		if (handlers[ipmessage->protocol] != 0) {
		
			sendBack = handlers[ipmessage->protocol]->
				OnInternetProtocolReceived(ipmessage->srcIP, 
							   ipmessage->dstIP, 
							   etherframePayload + 4*ipmessage->headerLength,
							   length - 4*ipmessage->headerLength);
		}
	}


	if (sendBack) {
	
		uint32_t temp = ipmessage->dstIP;
		ipmessage->dstIP = ipmessage->srcIP;
		ipmessage->srcIP = temp;
	
		ipmessage->timeToLive = 64;
		ipmessage->checksum = 0;
		ipmessage->checksum = Checksum((uint16_t*)ipmessage, 4*ipmessage->headerLength);
	}


	return sendBack;
}



void InternetProtocolProvider::Send(uint32_t dstIP_BE, uint8_t protocol, uint8_t* data, uint32_t size) {

	uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(InternetProtocolV4Message) + size);
	InternetProtocolV4Message* message = (InternetProtocolV4Message*)buffer;

	message->version = 4;
	message->headerLength = sizeof(InternetProtocolV4Message) / 4;
	message->tos = 0;
	message->totalLength = size + sizeof(InternetProtocolV4Message);
	message->totalLength = ((message->totalLength & 0xff00) >> 8) |
			       ((message->totalLength & 0x00ff) << 8);
	
	message->ident = 0x0100;
	message->flagsAndOffset = 0x0040;
	message->timeToLive = 0x40;
	message->protocol = protocol;

	message->dstIP = dstIP_BE;
	message->srcIP = backend->GetIPAddress();

	message->checksum = 0;
	message->checksum = Checksum((uint16_t*)message, sizeof(InternetProtocolV4Message));


	uint8_t* databuffer = buffer + sizeof(InternetProtocolV4Message);
	
	for (int i = 0; i < size; i++) {
	
		databuffer[i] = data[i];
	}

	uint32_t route = dstIP_BE;


	//if dst and src are not in same network
	if ((dstIP_BE & subnetMask) != (message->srcIP & subnetMask)) {
	
		route = gatewayIP;
	}

	backend->Send(arp->Resolve(route), this->etherType_BE, buffer, sizeof(InternetProtocolV4Message) + size);
	
	MemoryManager::activeMemoryManager->free(buffer);
}



uint16_t InternetProtocolProvider::Checksum(uint16_t* data, uint32_t lengthInBytes) {

	uint32_t temp = 0;
	
	for (int i = 0; i < lengthInBytes / 2; i++) {
	
		temp += ((data[i] & 0xff00) >> 8) |
			((data[i] & 0x00ff) << 8);

		
	}

	if (lengthInBytes % 2) {
	
		temp += ((uint16_t)((char*)data)[lengthInBytes-1]) << 8;
	}

	while (temp & 0xffff0000) {
	
		temp = (temp & 0xffff) + (temp >> 16);
	}

	return ((~temp & 0xff00) >> 8) | ((~temp & 0x00ff) << 8);
}

