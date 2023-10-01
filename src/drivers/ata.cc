#include <drivers/ata.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;


void printf(char*);
void printfHex(uint8_t);
void sleep(uint32_t);


AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(common::uint16_t portBase, bool master)
	
	: dataPort(portBase),
	  errorPort(portBase + 0x01),
	  sectorCountPort(portBase + 0x02),
	  lbaLowPort(portBase + 0x03),
	  lbaMidPort(portBase + 0x04),
	  lbaHiPort(portBase + 0x05),
	  devicePort(portBase + 0x06),
	  commandPort(portBase + 0x07),
	  controlPort(portBase + 0x206)

	{

	bytesPerSector = 512;
	this->master = master;
}



AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment() {
}



bool AdvancedTechnologyAttachment::Identify() {

	devicePort.Write(master ? 0xa0 : 0xb0);
	controlPort.Write(0);

	devicePort.Write(0xa0);
	uint8_t status = commandPort.Read();
	
	if (status == 0xff) {
	
		return false;
	}

	devicePort.Write(master ? 0xa0 : 0xb0);
	sectorCountPort.Write(0);
	lbaLowPort.Write(0);
	lbaMidPort.Write(0);
	lbaHiPort.Write(0);
	commandPort.Write(0xec);

	status = commandPort.Read();

	if (status == 0x00) {
	
		printf("NO DEVICE FOUND\n");
		return false;
	}

	//while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
	while (((status & 0x80) == 0x80) && ((status & 0x08) != 0x08)) {
	
		status = commandPort.Read();
	}

	if (status & 0x01) {
	
		printf("ATA ERROR\n");
		return false;
	}

	/*
	for (uint16_t i = 0; i < 256; i++) {
	
		
		uint16_t data = dataPort.Read();
		
		char* foo = "  ";
		foo[1] = (data >> 8) & 0x00ff;
		foo[0] = data & 0x00ff;
		printf(foo);
	}
	*/

	return true;
}



void AdvancedTechnologyAttachment::Read28(common::uint32_t sector, common::uint8_t* data, int count, int offset) {	

	if ((sector & 0xf0000000) || count > bytesPerSector) {
	
		printf("STORAGE UNAVAILABLE\n");
		return;
	}

	devicePort.Write((master ? 0xe0 : 0xf0) | ((sector & 0x0f000000) >> 24));
	errorPort.Write(0x00);
	sectorCountPort.Write(0x01);

	lbaLowPort.Write( sector & 0x000000ff);
	lbaMidPort.Write((sector & 0x0000ff00) >> 8);
	lbaHiPort.Write(( sector & 0x00ff0000) >> 16);
/*	^
	|
	|

	I accidentally mixed up these ports troubleshooting these drivers (again)
	and it made for some really crazy file corruption in case you wanted 
	to see that :^)
*/

	commandPort.Write(0x20);
	

	uint8_t status = commandPort.Read();
	while (((status & 0x80) == 0x80) || ((status & 0x08) != 0x08)) {

		status = commandPort.Read();
	}




	if (status & 0x01) {
	
		printf("ATA ERROR\n");
		return;
	}


	//
	if (offset) {
		for (uint16_t i = 0; i < offset; i += 2) {
	
			dataPort.Read();
		}
	}
	//

	
	for (uint16_t i = offset; i < count; i+= 2) {
	
		uint16_t rdata = dataPort.Read();
		data[i] = rdata & 0x00ff;

		if (i+1 < count) {
		
			data[i+1] = (rdata >> 8) & 0x00ff;
		}
	}

	for (uint16_t i = count + (count % 2); i < bytesPerSector; i += 2) {
	
		dataPort.Read();
	}
}



void AdvancedTechnologyAttachment::Write28(common::uint32_t sector, common::uint8_t* data, int count, int offset) {
	
	if ((sector & 0xf0000000) || count > bytesPerSector) {
	
		printf("STORAGE UNAVAILABLE\n");
		return;
	}

	devicePort.Write((master ? 0xe0 : 0xf0) | ((sector & 0x0f000000) >> 24));
	errorPort.Write(0x00);
	sectorCountPort.Write(0x01);

	lbaLowPort.Write( sector & 0x000000ff);
	lbaMidPort.Write((sector & 0x0000ff00) >> 8);
	lbaHiPort.Write( (sector & 0x00ff0000) >> 16);
	
	commandPort.Write(0x30);
	
	
	uint8_t status = commandPort.Read();
	while (((status & 0x80) == 0x80) || ((status & 0x08) != 0x08)) {

		status = commandPort.Read();
	}
	
	
	
	//
	if (offset) {

		uint8_t fillData[offset];
		this->Read28(sector, fillData, offset, 0);	
		
		for (uint16_t i = 0; i < offset; i += 2) {

			uint16_t wdata = fillData[i];

			if ((i + 1) < count) {
		
				wdata |= ((uint16_t)fillData[i+1]) << 8;
			}
		
			dataPort.Write(wdata);
		}
	}
	//
	
	
	for (uint16_t i = offset; i < count; i += 2) {
	
		uint16_t wdata = data[i];

		if ((i + 1) < count) {
		
			wdata |= ((uint16_t)data[i+1]) << 8;
		}
		
		dataPort.Write(wdata);
	}
	
	
	for (uint16_t j = count + (count % 2); j < bytesPerSector; j += 2) {
	
		dataPort.Write(0x0000);
	}
}




void AdvancedTechnologyAttachment::Flush() {
	
	devicePort.Write(master ? 0xe0 : 0xf0);
	commandPort.Write(0xe7);


	uint8_t status = commandPort.Read();

	if (status == 0x00) {

		return;
	}


	while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
	
		status = commandPort.Read();
	}

	if (status & 0x01) {
	
		printf("ATA ERROR");
		return;
	}
}

