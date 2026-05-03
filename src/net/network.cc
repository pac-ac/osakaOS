#include <net/network.h>

using namespace os;
using namespace os::common;
using namespace os::filesystem;
using namespace os::drivers;
using namespace os::net;


void printf(char*);
void sleep(uint32_t);
uint8_t* memset(uint8_t*, int, size_t);


Network::Network(amd_am79c973* eth0, 
		AddressResolutionProtocol* arp,
		InternetProtocolProvider* ipv4, 
		InternetControlMessageProtocol* icmp,
		UserDatagramProtocolProvider* udp,
		TransmissionControlProtocolProvider* tcp,
		FileSystem* filesystem,
		uint32_t gip, uint32_t subnet)

: TransmissionControlProtocolHandler(), 
	UserDatagramProtocolHandler() {

	this->eth0 = eth0;
	this->arp = arp;
	this->ipv4 = ipv4;
	this->icmp = icmp;
	this->udp = udp;
	this->tcp = tcp;

	this->filesystem = filesystem;
	this->requestID = -1;

	this->ip = eth0->GetIPAddress();
	this->mac = eth0->GetMACAddress();
	this->gateway = gip;
	this->subnet = subnet;

	
	if (this->HNM_Buffer == nullptr) {
	
		this->HNM_Buffer = (uint8_t*)filesystem->memoryManager->malloc(sizeof(uint8_t)*filesystem->gc->gfxBufferSize);
	}
	memset(this->HNM_Buffer, 0x00, filesystem->gc->gfxBufferSize);
}

Network::~Network() {
}




bool Network::HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size) {	

	//if (size == 0) { return true; }	
	if (socket->bufferIndex+size > HANDLE_BUF_SIZE_TCP) { this->EmptyHandleBufferTCP(socket); }

	socket->handleType = HANDLE_FLAG_TCP;

	for (int i = socket->bufferIndex; i < socket->bufferIndex+size; i++) { 
		
		socket->handleBuffer[i] = data[i]; 
	}
	socket->bufferIndex += size;

	//TransmissionControlProtocolHandler::HandleTransmissionControlProtocolMessage(socket, data, size);
	
	if (this->activateServerHTTP == true && size > 4 
			&& data[0] == 'G'
			&& data[1] == 'E'
			&& data[2] == 'T'
			&& data[3] == ' '
			&& data[4] == '/'
			) {
		
		//get requested file name
		if (data[5] != ' ') {
		
			char fileRequested[256];
			memset((uint8_t*)fileRequested, (uint8_t)'\0', 256);
		
			for (int i = 0; data[i+5] != ' ' && data[i+5] != '\0' && i < 256; i++) {

				fileRequested[i] = data[i+5];
			}

			//if file found
			if (this->filesystem->FileIf(this->filesystem->GetFileSector(fileRequested))) {
			
				MemoryManager* mm = this->filesystem->memoryManager;

				uint16_t messageLength = 0;
				uint16_t responseLength = strlen(RESPONSE_HTTP);
				uint32_t fileSize = this->filesystem->GetFileSize(fileRequested);
				uint8_t* customFileData = (uint8_t*)mm->malloc(sizeof(uint8_t)*(fileSize+responseLength+2));

				//prepend http header to response
				for (int i = 0; i < responseLength; i++) {
				
					customFileData[i] = RESPONSE_HTTP[i];
				}
				uint8_t blockData[OFS_BLOCK_SIZE];
				
				//read and load file data into response
				for (int i = 0; i < fileSize/OFS_BLOCK_SIZE; i++) {
				
					this->filesystem->ReadLBA(fileRequested, blockData, i);

					for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
					
						if (blockData[j] != 0x00) {
						
							customFileData[(i*OFS_BLOCK_SIZE)+j+responseLength] = blockData[j];
						} else {
							messageLength = i*OFS_BLOCK_SIZE+j;
							break;
						}
					}
				}

				customFileData[messageLength+responseLength] = '\r';
				customFileData[messageLength+responseLength+1] = '\n';

				//send file data
				socket->Send(customFileData, messageLength+responseLength+2);

				mm->free(customFileData);
			
			//404 error
			} else {
				socket->Send(this->fileData, this->fileDataSize);
			}
		} else {
			//no file specified, just give mounted default file
			if (this->fileData != nullptr) { socket->Send(this->fileData, 	       this->fileDataSize);
			} else {			 socket->Send((uint8_t*)RESPONSE_HTTP, strlen(RESPONSE_HTTP)); }	
			socket->Disconnect();
		}
			
		if (this->fileData != nullptr) { socket->Send(this->fileData, 	       this->fileDataSize);
		} else {			 socket->Send((uint8_t*)RESPONSE_HTTP, strlen(RESPONSE_HTTP)); }	
		socket->Disconnect();
		
		
		return true;
	}


	//if (this->activateServerHTTP == true && size > 9 && strcmp("GET / HTTP", (char*)data)) {

	/*
	for (int i = 0; i < size; i++) {
	
		char* foo = " ";
		foo[0] = data[i];
		printf(foo);
	}
	*/
	
	return true;
}

void Network::HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size) {	

	if (size == 0) { return; }	
	if (socket->bufferIndex+size > HANDLE_BUF_SIZE_UDP) { this->EmptyHandleBufferUDP(socket); }

	socket->handleType = HANDLE_FLAG_UDP;
	
	for (int i = socket->bufferIndex; i < socket->bufferIndex+size; i++) { 
		
		socket->handleBuffer[i] = data[i]; 
	}
	socket->bufferIndex += size;
	
	
	//UserDatagramProtocolHandler::HandleUserDatagramProtocolMessage(socket, data, size);


	if (this->activateServerHNM == true && size > 8
			&& data[0] == 'G'
			&& data[1] == 'E'
			&& data[2] == 'T'
			&& data[3] == '-'
			&& data[4] == '-'
			&& data[5] == '-'
			&& data[6] == 'H'
			&& data[7] == 'N'
			&& data[8] == 'M'
			) {
		
		//socket->Send(this->HNM_Buffer, filesystem->gc->gfxBufferSize);
		socket->Send(this->HNM_Buffer, BUFFER_SIZE_13H);
	}
	
	
	/*
	for (int i = 0; i < size; i++) {
	
		char* foo = " ";
		foo[0] = data[i];
		printf(foo);
	}
	*/
	
	/*
	if (size >= filesystem->gc->gfxBufferSize) {
	
		for (int i = 0; i < filesystem->gc->gfxBufferSize; i++) {
		
			this->HNM_Buffer[i] = data[i];
		}
	}
	*/
}


void Network::EmptyHandleBufferTCP(TransmissionControlProtocolSocket* socket) {

	memset(socket->handleBuffer, 0x00, HANDLE_BUF_SIZE_TCP);
	socket->bufferIndex = 0;
}

void Network::EmptyHandleBufferUDP(UserDatagramProtocolSocket* socket) {

	memset(socket->handleBuffer, 0x00, HANDLE_BUF_SIZE_UDP);
	socket->bufferIndex = 0;
}
