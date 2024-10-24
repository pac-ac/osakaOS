#include <drivers/cmos.h>

using namespace os;
using namespace os::drivers;
using namespace os::common;
using namespace os::hardwarecommunication;

void sleep(uint32_t);
char* int2str(uint32_t);
void printf(char*);

CMOS::CMOS()
: WriteCMOS(0x70), ReadCMOS(0x71) {
}

CMOS::~CMOS() {
}


void CMOS::CMOS_OUT(uint8_t val, uint8_t reg) {
	
	asm volatile ("cli");
	WriteCMOS.Write(reg);
	sleep(10);
	ReadCMOS.Write(val);
	asm volatile ("sti");
}

uint8_t CMOS::CMOS_IN(uint8_t reg) {
	
	uint8_t val = 0;

	asm volatile ("cli");
	WriteCMOS.Write(reg);
	sleep(10);
	val = ReadCMOS.Read();
	asm volatile ("sti");

	return val;
}



int32_t CMOS::GetUpdate() {

	
	int32_t val = 0;

	
	//asm volatile ("cli");
	WriteCMOS.Write(0x0a);
	//sleep(10);
	val = (ReadCMOS.Read() & 0x80);
	//asm volatile ("sti");
	return val;
	

	//val = (CMOS_IN(0x0a) & 0x80);
	//return val;
}

uint8_t CMOS::GetRegisterRTC(int32_t reg) {

	
	uint8_t val = 0;

	//asm volatile ("cli");
	WriteCMOS.Write(reg);
	//sleep(10);
	val = ReadCMOS.Read();
	//asm volatile ("sti");

	return val;
	

	//return CMOS_IN(reg);
}


void CMOS::DumpRTC() {

	printf("seconds: ");
	printf(int2str(timeData.second));
	printf("\n");
	
	printf("minutes: ");
	printf(int2str(timeData.minute));
	printf("\n");
	
	printf("hours: ");
	printf(int2str(timeData.hour));
	printf("\n");
}


void CMOS::ReadRTC() {

	uint8_t century;
	uint8_t last_second;
	uint8_t last_minute;
	uint8_t last_hour;
	uint8_t last_day;
	uint8_t last_month;
	uint8_t last_year;
	uint8_t last_century;
	uint8_t registerB;


	while (GetUpdate());
	
	timeData.second = GetRegisterRTC(0x00);
	timeData.minute = GetRegisterRTC(0x02);
	timeData.hour = GetRegisterRTC(0x04);
	timeData.day = GetRegisterRTC(0x07);
	timeData.month = GetRegisterRTC(0x08);
	timeData.year = GetRegisterRTC(0x09);
	
	//century = GetRegisterRTC(0x00);

	do {
		last_second = timeData.second;
		last_minute = timeData.minute;
		last_hour = timeData.hour;
		last_day = timeData.day;
		last_month = timeData.month;
		last_year = timeData.year;
		last_century = century;
	
		while (GetUpdate()) {
	
			timeData.second = GetRegisterRTC(0x00);
			timeData.minute = GetRegisterRTC(0x02);
			timeData.hour = GetRegisterRTC(0x04);
			timeData.day = GetRegisterRTC(0x07);
			timeData.month = GetRegisterRTC(0x08);
			timeData.year = GetRegisterRTC(0x09);
	
				//century = GetRegisterRTC(0x00);
		}
	
	} while((last_second != timeData.second) || (last_minute != timeData.minute) || 
		(last_hour != timeData.hour) || (last_day != timeData.day) || 
		(last_month != timeData.month) || (last_year != timeData.year) || (last_century != century));

	registerB = GetRegisterRTC(0x08);

	if (!(registerB & 0x04)) {
	
		timeData.second = (timeData.second & 0x0f) + ((timeData.second / 16) * 10);
		timeData.minute = (timeData.minute & 0x0f) + ((timeData.minute / 16) * 10);
		timeData.hour = ((timeData.hour & 0x0f) + (((timeData.hour & 0x70) / 16) * 10)) | (timeData.hour & 0x80);
		timeData.day = (timeData.day & 0x0f) + ((timeData.day / 16) * 10);
		timeData.month = (timeData.month & 0x0f) + ((timeData.month / 16) * 10);
		timeData.year = (timeData.year & 0x0f) + ((timeData.year / 16) * 10);
	}
	
}
