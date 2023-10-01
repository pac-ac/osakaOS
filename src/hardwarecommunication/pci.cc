#include <hardwarecommunication/pci.h>
#include <drivers/amd_am79c973.h>

using namespace os::common;
using namespace os::hardwarecommunication;
using namespace os::drivers;




PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {
}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {
}


PeripheralComponentInterconnectController::PeripheralComponentInterconnectController() 
: dataport(0xcfc),
  commandport(0xcf8) {

}



PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController() {
}



uint32_t PeripheralComponentInterconnectController::Read(uint16_t bus, uint16_t device, 
		uint16_t function, uint32_t registeroffset) {

	uint32_t id = 
		
		0x1 << 31
		| ((bus & 0xff) << 16)
		| ((device & 0x1f) << 11)
		| ((function & 0x07) << 8)
		| (registeroffset & 0xfc);

	commandport.Write(id);
	uint32_t result = dataport.Read();
	
	return result >> (8* (registeroffset % 4));


}


void PeripheralComponentInterconnectController::Write(uint16_t bus, uint16_t device, uint16_t function, 
		uint32_t registeroffset, uint32_t value) {

	uint32_t id = 
		
		0x1 << 31
		| ((bus & 0xff) << 16)
		| ((device & 0x1f) << 11)
		| ((function & 0x07) << 8)
		| (registeroffset & 0xfc);

	commandport.Write(id);
	dataport.Write(value);

}


bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device) {

	return Read(bus, device, 0, 0x0e) & (1<<7);
}


void printf(char* str);
void printfHex(uint8_t);


void PeripheralComponentInterconnectController::SelectDrivers(DriverManager* driverManager, InterruptManager* interrupts) {

	for (int bus = 0; bus < 8; bus++) {
	
		for (int device = 0; device < 32; device++) {
		
			int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
			
			for (int function = 0; function < numFunctions; function++) {
		
				PeripheralComponentInterconnectDeviceDescriptor *dev =
				GetDeviceDescriptor(bus, device, function);

				if (dev->vendor_id == 0x0000 || dev->vendor_id == 0xffff) {
					
					continue;
				}	

				for (int barNum = 0; barNum < 6; barNum++) {
				
					BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);

					if (bar.address && (bar.type == InputOutput)) {
					
						dev->portBase = (uint32_t)bar.address;
					}
				}
				

				Driver* driver = GetDriver(dev, interrupts);
					
				if (driver != 0) {
					
					driverManager->AddDriver(driver);
				}

				
					
				printf("PCI BUS ");		
				printfHex(bus & 0xff);		
				
				printf(", DEVICE ");		
				printfHex(device & 0xff);		
				
				printf(", FUNCTION ");		
				printfHex(function & 0xff);		
				
				
				
				printf(" = VENDOR ");		
				printfHex((dev->vendor_id & 0xff00) >> 8);		
				printfHex(dev->vendor_id & 0xff);		
				
				printf(", DEVICE");		
				printfHex((dev->device_id & 0xff00) >> 8);		
				printfHex(dev->device_id & 0xff);		
				printf("\n");
				
			}
		}
	}
}



BaseAddressRegister PeripheralComponentInterconnectController::GetBaseAddressRegister(
		uint16_t bus, uint16_t device, uint16_t function, uint16_t bar) {

	BaseAddressRegister result;
	
	uint32_t headertype = Read(bus, device, function, 0x0e) & 0x7f;
	int maxBARS = 6 - (4 * headertype);
	
	if (bar >= maxBARS) {
		
		return result;
	}
	
	uint32_t bar_value = Read(bus, device, function, 0x10 + 4*bar);
	result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;


	uint32_t temp;
	
	if (result.type == MemoryMapping) {
	
		switch ((bar_value >> 1) & 0x3) {
		
			//finish later

			case 0x00: //32 bit mode
				break;		
			case 0x01: //20 bit mode
				break;
			case 0x10: //64 bit mode
				break;
		}
		result.prefetchable = ((bar_value >> 3) & 0x1) == 0x1;
		
	} else {
	
		result.address = (uint8_t*)(bar_value & ~0x3);
		result.prefetchable = false;
	}

	return result;
}



Driver* PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor* dev, InterruptManager* interrupts) {

	Driver *driver = 0;


	switch (dev->vendor_id) {
	
		case 0x1022: //AMD
			
			switch (dev->device_id) {
				
				case 0x2000: //am79c973
					printf("AMD am79c973 ");
					driver = (amd_am79c973*)MemoryManager::activeMemoryManager->malloc(sizeof(amd_am79c973));
					
					if (driver != 0) {
					
						new (driver) amd_am79c973(dev, interrupts);
					} else {
						printf("Init amd_am79c973 failed.");
					}
					return driver;	
					
					break;
			}
			break;
		
		case 0x1274:
			printf("hilolololol\n");
			break;
		
		case 0x8086: //Intel
			printf("Intel ");
			break;
	}

	switch (dev->class_id) {

		case 0x03: //graphics
			switch (dev->subclass_id) {
			
				case 0x00: //VGA
					printf("VGA ");
					break;
			}
			break;
		case 0x04: //audio
			switch (dev->subclass_id) {
			
				case 0x01:
					printf("AC97 ");
					break;
			}
			break;

		case 0x08: //base system peripheral
			switch (dev->subclass_id) {
			
				case 0x00:
					printf("PIC ");
					break;
				case 0x01:
					printf("DMA ");
					break;
				case 0x03:
					printf("Timer ");
					break;
			}
			break;

		case 0x0b: //cpu
			switch (dev->subclass_id) {
			
				case 0x00:
					printf("386 ");
					break;
				case 0x02:
					printf("Pentium ");
					break;
				case 0x03:
					printf("Pentium Pro ");
					break;
				case 0x10:
					printf("Alpha ");
					break;
				case 0x20:
					printf("PowerPC ");
					break;
				case 0x80:
					printf("Other (CPU) ");
					break;
			}
			break;
	}

	//return driver;
	return 0;
}





PeripheralComponentInterconnectDeviceDescriptor* PeripheralComponentInterconnectController::
GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function) {

	PeripheralComponentInterconnectDeviceDescriptor *result;
	

	result->bus = bus;
	result->device = device;
	result->function = function;

	result->vendor_id = Read(bus, device, function, 0x00);
	result->device_id = Read(bus, device, function, 0x02);

	result->class_id = Read(bus, device, function, 0x0b);
	result->subclass_id = Read(bus, device, function, 0x0a);
	result->interface_id = Read(bus, device, function, 0x09);
	
	result->revision = Read(bus, device, function, 0x08);
	result->interrupt = Read(bus, device, function, 0x3c);


	return result;
}

