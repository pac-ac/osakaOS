#include <drivers/driver.h>

using namespace os::drivers;



Driver::Driver() {
}

Driver::~Driver() {
}

void Driver::Activate() {
}

int Driver::Reset() {
}

void Driver::Deactive() {
}



DriverManager::DriverManager() {
	numDrivers = 0;
}


void DriverManager::AddDriver(Driver* drv) {

	drivers[numDrivers] = drv;
	numDrivers++;
}


void DriverManager::ActivateAll() {

	for (int i = 0; i < numDrivers; i++) {
		
		drivers[i]->Activate();
	}
}


void DriverManager::Replace(Driver* drv, int drvNum) {

	drivers[drvNum] = drv;
        drivers[drvNum]->Activate();	
}
