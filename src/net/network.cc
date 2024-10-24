#include <net/network.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::drivers;
using namespace os::net;


Network::Network(amd_am79c973* eth0, AddressResolutionProtocol* arp,
		InternetProtocolProvider* ipv4, 
		InternetControlMessageProtocol* icmp,
		uint32_t gip, uint32_t subnet) {

	this->eth0 = eth0;
	this->arp = arp;
	this->ipv4 = ipv4;
	this->icmp = icmp;

	this->ip = eth0->GetIPAddress();
	this->mac = eth0->GetMACAddress();
	this->gateway = gip;
	this->subnet = subnet;
}

Network::~Network() {
}
