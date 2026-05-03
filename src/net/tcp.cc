#include <net/tcp.h>

using namespace os;
using namespace os::common;
using namespace os::net;

void printf(char*);
void sleep(uint32_t);
uint16_t prng();



uint32_t bigEndian32(uint32_t x) {

	return ((x & 0xff000000) >> 24)  | 
		((x & 0x00ff0000) >> 8)  |
		((x & 0x0000ff00) << 8)  |
		((x & 0x000000ff) << 24);
}

TransmissionControlProtocolSocket::TransmissionControlProtocolSocket(TransmissionControlProtocolProvider* backend) {

	this->backend = backend;
	this->handler = 0;
	this->state = CLOSED;
}
TransmissionControlProtocolSocket::~TransmissionControlProtocolSocket() {
}


//this socket fucker is not recieving data from tcp provider
bool TransmissionControlProtocolSocket::HandleTransmissionControlProtocolMessage(uint8_t* data, uint16_t size) {
	

	if (handler != nullptr) {
	
		return handler->HandleTransmissionControlProtocolMessage(this, data, size);
	}
	return false;
}


void TransmissionControlProtocolSocket::Send(uint8_t* data, uint16_t size) {

	//possible infinite loop
	uint32_t waitForConnection = 0;
	
	//while (state != ESTABLISHED && waitForConnection < 0xFFFFFF) {
	while (state != ESTABLISHED && waitForConnection < 128) {
	
		if (this->poll) {
		
			sleep(1);
			waitForConnection++;
		}
	}
	this->poll = true;

	if (state == ESTABLISHED) { this->backend->Send(this, data, size, PSH | ACK);
	} else {
		//connection failed
		this->connectionFail = true;
		this->backend->Send(this, 0, 0, RST);
	}
}

void TransmissionControlProtocolSocket::Disconnect() {

	this->backend->Disconnect(this);
}




TransmissionControlProtocolHandler::TransmissionControlProtocolHandler() {
}
TransmissionControlProtocolHandler::~TransmissionControlProtocolHandler() {
}


bool TransmissionControlProtocolHandler::HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size) {

	return true;
}



TransmissionControlProtocolProvider::TransmissionControlProtocolProvider(InternetProtocolProvider* backend)
: InternetProtocolHandler(backend, 0x06)
{
	for (int i = 0; i < 65535; i++) {
	
		this->sockets[i] = 0;
	}
	this->numSockets = 0;
	this->freePort = 1024;
}

TransmissionControlProtocolProvider::~TransmissionControlProtocolProvider() {
}


bool TransmissionControlProtocolProvider::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* internetProtocolPayload, uint32_t size) {

	/*
	char* foo = " ";
	for (int i = 0; i < size; i++) {
	
		foo[0] = internetProtocolPayload[i];
		printf(foo);
	}
	printf("\n");
	*/

	if (size < 20) { return false; }

	TransmissionControlProtocolHeader* msg = (TransmissionControlProtocolHeader*)internetProtocolPayload;
	
	uint16_t localPort = msg->dstPort;
	uint16_t remotePort = msg->srcPort;
	TransmissionControlProtocolSocket* socket = 0;


	for (uint16_t i = 0; i < numSockets && socket == 0; i++) {
	
		if (sockets[i]->localPort == msg->dstPort
		 && sockets[i]->localIP == dstIP_BE
		 && sockets[i]->state == LISTEN
		 && (((msg->flags) & (SYN | ACK)) == SYN)) {
		
			socket = sockets[i];
		
		} else if (sockets[i]->localPort == msg->dstPort 
		 && sockets[i]->localIP == dstIP_BE
		 && sockets[i]->remotePort == msg->srcPort
		 && sockets[i]->remoteIP == srcIP_BE) {
		
			socket = sockets[i];
		}
	}
	
	bool reset = false;
	if (socket != 0 && msg->flags & RST) { socket->state = CLOSED; }


	if (socket != 0 && socket->state != CLOSED) {

		switch ((msg->flags) & (SYN | ACK | FIN)) {

			case SYN:
				if (socket->state == LISTEN) {

					socket->state = SYN_RECEIVED;
					socket->remotePort = msg->srcPort;
					socket->remoteIP = srcIP_BE;
					socket->acknowledgementNumber = bigEndian32(msg->sequenceNumber) + 1;
					socket->sequenceNumber = (prng() << 16) | prng();
					Send(socket, 0, 0, SYN | ACK);
					socket->sequenceNumber++;
				} else {
					reset = true;
				}
				break;
			
			case SYN | ACK:
				if (socket->state == SYN_SENT) {
				
					socket->state = ESTABLISHED;
					socket->acknowledgementNumber = bigEndian32(msg->sequenceNumber) + 1;
					socket->sequenceNumber++;
					Send(socket, 0, 0, ACK);
				} else {
					reset = true;
				}
				break;

			case SYN | FIN:
			case SYN | FIN | ACK:
				reset = true;
				break;
			
			case FIN:
			case FIN | ACK:
				if (socket->state == ESTABLISHED) {
				
					socket->state = CLOSE_WAIT;
					socket->acknowledgementNumber++;
					Send(socket, 0, 0, ACK);
					Send(socket, 0, 0, FIN|ACK);
				
				} else if (socket->state == CLOSE_WAIT) {
				
					socket->state = CLOSED;
				
				} else if (socket->state == FIN_WAIT1
					|| socket->state == FIN_WAIT2) {

					socket->state = CLOSED;
					socket->acknowledgementNumber++;
					Send(socket, 0, 0, ACK);
				} else {
					reset = true;
				}
				break;

			case ACK:
				if (socket->state == SYN_RECEIVED) {
				
					socket->state = ESTABLISHED;
					return false;

				} else if (socket->state == FIN_WAIT1) {

					socket->state = FIN_WAIT2;
					return false;
				
				} else if (socket->state == CLOSE_WAIT) {

					socket->state = CLOSED;
					break;
				}
				if (msg->flags == ACK) { break; }
				//no break because of piggybacking
			default:
				/*
				printf("sequence num: ");
				printf(int2str(bigEndian32(msg->sequenceNumber)));
				printf("\n");
				
				printf("acknowle num: ");
				printf(int2str(socket->acknowledgementNumber));
				printf("\n");
				*/

				//if (bigEndian32(msg->sequenceNumber) == socket->acknowledgementNumber) {
	
					//handle tcp message	
					reset = !(socket->HandleTransmissionControlProtocolMessage(
							internetProtocolPayload + (msg->headerSize32 * 4), size - (msg->headerSize32 * 4)));
				
					if (!reset) {
				
						int x = 0;

						for (int i = msg->headerSize32*4; i < size; i++) {
						
							if (internetProtocolPayload[i] != 0) { x = i; }
						}
						socket->acknowledgementNumber += (x - msg->headerSize32*4 + 1);
						Send(socket, 0, 0, ACK);
					}
						/*	
						//check for packets out of order
						uint32_t compareNumber = bigEndian32(msg->sequenceNumber)+1;
						uint8_t packetsHandled = 0;
						
						for (int i = 0; i < this->wrongPacketIndex; i++) {
					
							TransmissionControlProtocolHeader* head = (TransmissionControlProtocolHeader*)(this->wrongPackets[i]);
							
							if (bigEndian32(head->sequenceNumber) == compareNumber) {
					
								//handle packet
								reset = !(socket->HandleTransmissionControlProtocolMessage(
									this->wrongPackets[i]->data + (head->headerSize32 * 4), 
									this->wrongPackets[i]->size - (head->headerSize32 * 4)));
							
								int y = 0;

								for (int j = head->headerSize32*4; j < size; j++) {
						
									if (this->wrongPackets[i]->data[j] != 0) { y = j; }
								}
								socket->acknowledgementNumber += (y - msg->headerSize32*4 + 1);
								Send(socket, 0, 0, ACK);

								compareNumber = bigEndian32(head->sequenceNumber)+1;

								//free and move up
								this->backend->backend->memoryManager->free(this->wrongPackets[i]->data);
								this->backend->backend->memoryManager->free(this->wrongPackets[i]);
								this->wrongPackets[i] = nullptr;
								packetsHandled++;
							}
						}

						for (int i = 0; i < packetsHandled; i++) {
						
							this->wrongPackets[i] = this->wrongPackets[i+packetsHandled];
						}
					}
						*/
				/*
				} else {
					//packets have arrived in incorrect order
					//TransmissionControlProtocolHeader* msg = (TransmissionControlProtocolHeader*)internetProtocolPayload;
					//must sort packets or youre fucked
					
					Packet* packet  = (Packet*)this->backend->backend->memoryManager->malloc(sizeof(Packet));
					packet->data 	= (uint8_t*)this->backend->backend->memoryManager->malloc(sizeof(uint8_t)*size);
					packet->size = size;

					for (int i = 0; i < size; i++) {
					
						packet->data[i] = internetProtocolPayload[i];
					}

					
					if (this->wrongPacketIndex < 16) {
						
						this->wrongPackets[wrongPacketIndex] = packet;
						this->wrongPacketIndex++;

						//insertion sort packets
						for (int i = 1; i < this->wrongPacketIndex; i++) {
						
							Packet* key = this->wrongPackets[i];
							int j = i - 1;

							while (j >= 0 && 
								bigEndian32(((TransmissionControlProtocolHeader*)this->wrongPackets[j])->sequenceNumber) 
								> bigEndian32(((TransmissionControlProtocolHeader*)key)->sequenceNumber)) {
							
								this->wrongPackets[j+1] = this->wrongPackets[j];
								j = j - 1;
							}
							this->wrongPackets[j+1] = key;
						}
					} else {
						for (int i = 0; i < 16; i++) {
						
							this->backend->backend->memoryManager->free(this->wrongPackets[i]);
							this->wrongPackets[i] = nullptr;
						}
						reset = true;
					}
				}
				*/
		}
	}

	if (reset) {
	
		if (socket != 0) {
		
			Send(socket, 0, 0, RST);
		} else {
			TransmissionControlProtocolSocket socket(this);
			socket.remotePort = msg->srcPort;
			socket.remoteIP = srcIP_BE;
			socket.localPort = msg->dstPort;
			socket.localIP = dstIP_BE;
			socket.sequenceNumber = bigEndian32(msg->acknowledgementNumber);
			socket.acknowledgementNumber = bigEndian32(msg->sequenceNumber) + 1;
			Send(&socket, 0, 0, RST);
		}
	}
	
	if (socket != 0 && socket->state == CLOSED) {	
	
		for (int i = 0; i < numSockets; i++) {
	
			if (sockets[i] == socket) {
		
				sockets[i] = sockets[--numSockets];
				this->backend->backend->memoryManager->free(socket);
				break;
			}
		}
	}

	return false;
}

TransmissionControlProtocolSocket* TransmissionControlProtocolProvider::Connect(uint32_t ip, uint16_t port) {

	TransmissionControlProtocolSocket* socket = (TransmissionControlProtocolSocket*)this->backend->backend->memoryManager->malloc(sizeof(TransmissionControlProtocolSocket));
	
	if (socket != 0) {
	
		new (socket) TransmissionControlProtocolSocket(this);
		socket->remotePort = port;
		socket->remoteIP = ip;
		socket->localPort = freePort++;
		//you can set this to 127.0.0.1 
		//so socket only accepts local connections
		socket->localIP = backend->GetIPAddress();
	
		socket->remotePort = ((socket->remotePort & 0xff00) >> 8) | ((socket->remotePort & 0x00ff) << 8);
		socket->localPort = ((socket->localPort & 0xff00) >> 8) | ((socket->localPort & 0x00ff) << 8);

		sockets[this->numSockets++] = socket;
		socket->state = SYN_SENT;
		
		//randomize sequence numbers
		socket->sequenceNumber = (prng() << 16) | prng();
		
		Send(socket, 0, 0, SYN);
	}
	return socket;
}


TransmissionControlProtocolSocket* TransmissionControlProtocolProvider::Listen(uint16_t port) {

	TransmissionControlProtocolSocket* socket = (TransmissionControlProtocolSocket*)this->backend->backend->memoryManager->malloc(sizeof(TransmissionControlProtocolSocket));

	if (socket != 0) {
	
		new (socket) TransmissionControlProtocolSocket(this);
	
		socket->state = LISTEN;	
		socket->localIP = backend->GetIPAddress();
		socket->localPort = ((port & 0xff00) >> 8) | ((port & 0x00ff) << 8);
	
		sockets[this->numSockets++] = socket;
	}
	return socket;
}


void TransmissionControlProtocolProvider::Disconnect(TransmissionControlProtocolSocket* socket) {

	socket->state = FIN_WAIT1;
	Send(socket, 0, 0, FIN + ACK);
	socket->sequenceNumber++;
}



void TransmissionControlProtocolProvider::Send(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size, uint16_t flags) {

	uint16_t totalLength = size + sizeof(TransmissionControlProtocolHeader);
	uint16_t lengthInclPHdr = totalLength + sizeof(TransmissionControlProtocolPseudoHeader);
	
	uint8_t* buffer = (uint8_t*)this->backend->backend->memoryManager->malloc(lengthInclPHdr);

	TransmissionControlProtocolPseudoHeader* phdr = (TransmissionControlProtocolPseudoHeader*)buffer;
	TransmissionControlProtocolHeader* msg = (TransmissionControlProtocolHeader*)(buffer + sizeof(TransmissionControlProtocolPseudoHeader));
	
	uint8_t* buffer2 = buffer + sizeof(TransmissionControlProtocolHeader) 
				  + sizeof(TransmissionControlProtocolPseudoHeader);

	msg->headerSize32 = sizeof(TransmissionControlProtocolHeader)/4;
	msg->srcPort = socket->localPort;
	msg->dstPort = socket->remotePort;
	
	msg->acknowledgementNumber = bigEndian32(socket->acknowledgementNumber);
	msg->sequenceNumber = bigEndian32(socket->sequenceNumber);
	msg->reserved = 0;
	msg->flags = flags;
	msg->windowSize = 0xffff;
	msg->urgentPtr = 0;
	
	msg->options = ((flags & SYN) != 0) ? 0xb4050402 : 0;

	socket->sequenceNumber += size;

	//printf("making request: ");
	//char* foo = " ";
	for (int i = 0; i < size; i++) {

		buffer2[i] = data[i];
		
		//foo[0] = (char)(buffer2[i]);
		//printf(foo);
	}
	//printf("\n");
	
	phdr->srcIP = socket->localIP;
	phdr->dstIP = socket->remoteIP;
	phdr->protocol = 0x0600;
	phdr->totalLength = ((totalLength & 0x00ff) << 8) | ((totalLength & 0xff00) >> 8);
	
	
	msg->checksum = 0;
	msg->checksum = InternetProtocolProvider::Checksum((uint16_t*)buffer, lengthInclPHdr);


	InternetProtocolHandler::Send(socket->remoteIP, (uint8_t*)msg, totalLength);
	this->backend->backend->memoryManager->free(buffer);
}

void TransmissionControlProtocolProvider::Bind(TransmissionControlProtocolSocket* socket, TransmissionControlProtocolHandler* handler) {

	socket->handler = handler;
}
