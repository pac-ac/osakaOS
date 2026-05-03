#include <net/icmp.h>

using namespace os;
using namespace os::common;
using namespace os::net;


void printf(char*);
void printfHex(uint8_t);


InternetControlMessageProtocol::InternetControlMessageProtocol(InternetProtocolProvider* backend) 
: InternetProtocolHandler(backend, 0x01) {

	this->ipv4 = backend;
}

InternetControlMessageProtocol::~InternetControlMessageProtocol() {
}


bool InternetControlMessageProtocol::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE,
								uint8_t* internetprotocolPayload, uint32_t size) {

	if (size < sizeof(InternetControlMessageProtocolMessage)) { return false; }

	InternetControlMessageProtocolMessage* msg = (InternetControlMessageProtocolMessage*)internetprotocolPayload;
	dataICMP* dataMsg = (dataICMP*)internetprotocolPayload;

	switch (msg->type) {
	
		case 0:
			printf("Ping received from: ");
			printfHex(srcIP_BE & 0xff);
			printf("."); printfHex((srcIP_BE >> 8) & 0xff);
			printf("."); printfHex((srcIP_BE >> 16) & 0xff);
			printf("."); printfHex((srcIP_BE >> 24) & 0xff);
			printf("\n");
			
			if (size >= 8) {
				for (int i = 0; i < size-8; i++) {
			
					char* foo = " ";
					foo[0] = (char)dataMsg->data[i+4];
					printf(foo);
				}
				printf("\n");
			}
			break;
		case 8:
			msg->type = 0;
			msg->checksum = 0;
			msg->checksum = this->ipv4->Checksum((uint16_t*)msg, 
					sizeof(InternetControlMessageProtocolMessage));
			return true;
			break;
	}	
	return false;
}


void InternetControlMessageProtocol::RequestEchoReply(uint32_t ip_be, uint8_t* message, uint16_t messageLength) {

	if (message != nullptr) {

		dataICMP icmp;
		icmp.type = 8;
		icmp.code = 0;
		icmp.data[0] = 0x00;
		icmp.data[1] = 0x00;
		icmp.data[2] = 0x37;
		icmp.data[3] = 0x13;

		for (int i = 0; i < 512; i++) {
			
			if (i < messageLength) { icmp.data[i+4] = message[i];
			} else { 		 icmp.data[i+4] = 0x00; }
		}

		icmp.checksum = 0;
		icmp.checksum = this->ipv4->Checksum((uint16_t*)&icmp, sizeof(dataICMP));
		
		this->Send(ip_be, (uint8_t*)&icmp, sizeof(dataICMP));
	} else {
		InternetControlMessageProtocolMessage icmp;
		icmp.type = 8; //ping
		icmp.code = 0;
		icmp.data = 0x3713;
		icmp.checksum = 0;
		icmp.checksum = this->ipv4->Checksum((uint16_t*)&icmp, sizeof(InternetControlMessageProtocolMessage));
	
		this->Send(ip_be, (uint8_t*)&icmp, sizeof(InternetControlMessageProtocolMessage));
	}
}
