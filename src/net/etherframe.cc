#include <net/etherframe.h>

using namespace os;
using namespace os::common;
using namespace os::net;
using namespace os::drivers;



EtherFrameHandler::EtherFrameHandler(EtherFrameProvider* backend, uint16_t etherType) { 

	this->backend = backend;
	this->etherType_BE = ((etherType & 0x00ff) << 8) |
			     ((etherType & 0xff00) >> 8);

	backend->handlers[etherType_BE] = this;
}



EtherFrameHandler::~EtherFrameHandler() {

	if (backend->handlers[etherType_BE] == this) {
	
		backend->handlers[etherType_BE] = 0;
	}
}	



bool EtherFrameHandler::OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size) {

	//for now
	return false;
}

void EtherFrameHandler::Send(uint64_t dstMAC_BE, uint8_t* data, uint32_t size) {

	backend->Send(dstMAC_BE, this->etherType_BE, data, size);
}


                
EtherFrameProvider::EtherFrameProvider(amd_am79c973* backend) 
: RawDataHandler(backend) {

	for (uint32_t i = 0; i < 65535; i++) {
	
		handlers[i] = 0;
	}
}



EtherFrameProvider::~EtherFrameProvider() {
}	



bool EtherFrameProvider::OnRawDataReceived(uint8_t* buffer, uint32_t size) {

	if (size < sizeof(EtherFrameHeader)) {
	
		return false;
	}

	EtherFrameHeader* frame = (EtherFrameHeader*)buffer;
	bool sendBack = false;

	if (frame->dstMAC_BE == 0xffffffffffff
	    || frame->dstMAC_BE == backend->GetMACAddress()) {
	
		if (handlers[frame->etherType_BE] != 0) {
		
			sendBack = handlers[frame->etherType_BE]->
			OnEtherFrameReceived(buffer + sizeof(EtherFrameHeader), size - sizeof(EtherFrameHeader));
		}
	}

	if (sendBack) {
	
		frame->dstMAC_BE = frame->srcMAC_BE;
		frame->srcMAC_BE = backend->GetMACAddress();
	}


	return sendBack;
}



void EtherFrameProvider::Send(uint64_t dstMAC_BE, uint16_t etherType_BE, uint8_t* buffer, uint32_t size) {

	uint8_t* buffer2 = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(EtherFrameHeader) + size);
	EtherFrameHeader* frame = (EtherFrameHeader*)buffer2;

	frame->dstMAC_BE = dstMAC_BE;
	frame->srcMAC_BE = backend->GetMACAddress();
	frame->etherType_BE = etherType_BE;

	uint8_t* src = buffer;
	uint8_t* dst = buffer2 + sizeof(EtherFrameHeader);

	for (uint32_t i = 0; i < size; i++) {
	
		dst[i] = src[i];
	}

	backend->Send(buffer2, size + sizeof(EtherFrameHeader));



	MemoryManager::activeMemoryManager->free(buffer2);
}

uint64_t EtherFrameProvider::GetMACAddress() {

	return backend->GetMACAddress();
}

uint32_t EtherFrameProvider::GetIPAddress() {

	return backend->GetIPAddress();
}
