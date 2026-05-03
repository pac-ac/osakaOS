#include <string.h>

using namespace os;
using namespace os::common;


uint8_t* memset(uint8_t* buf, int value, size_t n);


uint16_t os::strlen(char* args) {

	uint16_t length = 0;
	for (length = 0; args[length] != '\0'; length++) {}
	return length;
}


bool os::strcmp(char* one, char* two) {

	uint16_t i = 0;

	for (i; one[i] != '\0'; i++) {
	
		if (one[i] != two[i]) { return false; }
	}
	return true;
}


char* os::strcpy(char* one, char* two) {

	uint16_t length = strlen(two);
	if (length < strlen(one)) { length = strlen(one); }

	for (int i = 0; i < length+1; i++) {
	
		one[i] = two[i];
	}
	return one;
}


bool os::findCharInStr(char ch, char* str) {

	for (int i = 0; str[i] != '\0'; i++) {
	
		if (str[i] == ch) { return true; }
	}
	return false;
}


int32_t os::str2int(char* args) {

	int64_t number = 0;
	uint16_t i = 0;
	bool gotNum = false;
	bool neg = args[0] == '-';

	for (uint16_t i = 0; args[i] != '\0'; i++) {
		
		if ((args[i] >= 58 || args[i] <= 47) && args[i] != ' ' && i > 0) {

			return 0;
		}

		if (args[i] != ' ') {

			number *= 10;
			number += ((int32_t)args[i] - 48);
			gotNum = true;
			args[i] = ' ';
                } else {
                        if (gotNum) { 
				if (neg) { return -number; } 
				return number; 
			}
		}
	}
	if (neg) { return -number; } 
	return number;
}


char* os::int2str(int32_t num) {

	bool neg = (num < 0);
	if (neg) { num *= -1; }

	uint32_t numChar = 1;
	uint8_t i = 1;

	if (num % 10 != num) {

		while ((num / (numChar)) >= 10) {

			numChar *= 10;
			i++;
		}
		uint8_t strIndex = 1 * neg;
		char* str = "-4294967296";
		//char* str = "9223372036854775807";
		//uint8_t strIndex = 0;

		while (i) {
			str[strIndex] = (char)(((num / (numChar)) % 10) + 48);

			if (numChar >= 10) { numChar /= 10; }
			strIndex++;
			i--;
		}
		str[strIndex] = '\0';
		if (neg) { str[0] = '-'; }
		return str;
	}
	
	char* str = "- ";
	
	if (neg) { 
		str[1] = (num + 48);
	} else {
		str[0] = (num + 48);
		str[1] = '\0';
	}
	return str;
}

float os::str2float(char* str) {

	if (strlen(str) > 32) { return 0.0; }

	char beforeDecimal[16];
	char afterDecimal[16];

	int i = 0;
	for (i; str[i] != '.'; i++) { beforeDecimal[i] = str[i]; }
	beforeDecimal[i] = '\0';
	
	int j = 0;
	for (j = i; str[j] != '\0'; j++) { afterDecimal[j-i] = str[j]; }
	afterDecimal[j-i] = '\0';

	float val = 0.0;
	val += (float)(str2int(beforeDecimal));
	val += ((float)(str2int(beforeDecimal)))/((j-i)*10);

	return val;
}


char* os::float2str(float val) {
		
	char str[40];
	memset((uint8_t*)str, 0x00, 40);
	uint8_t i = 0;

	int32_t integer = (int32_t)val;
	char* intStr = int2str(integer);
	float decimals = val/((float)integer);

	for (i = 0; intStr[i] != '\0'; i++) {
	
		str[i] = intStr[i];
	}
	str[i] = '.';
	i++;

	for (i; i < 4 && decimals > 0.0; i++) {
	
		decimals *= 10.0;
		str[i] = ((int)decimals)+'\0';
		decimals -= (float)((int)decimals);
	}
	return str;
}



uint32_t os::str2ip(char* str) {

	uint32_t ip = 0;
	
	char* valStr = "000";
	uint8_t valStrIndex = 0;
	uint8_t byteNum = 0;

	for (int i = 0; i < strlen(str)+1; i++) {
	
		if ((str[i] == '.' || str[i] == '\0') || valStrIndex >= 3) {
		
			valStr[valStrIndex] = '\0';
			ip |= (str2int(valStr) << byteNum*8);
			byteNum++;
			valStr[0] = '0';
			valStr[1] = '0';
			valStr[2] = '0';
			valStrIndex = 0;
		} else {
			valStr[valStrIndex] = str[i];
			valStrIndex++;
		}
	}

	return ip;
}


char* os::ip2str(uint32_t ip) {

	char* str = "255.255.255.255";
	uint8_t strIndex = 0;

	for (int i = 0; i < 4; i++) {
	
		uint8_t val = ((ip >> (i*8)) & 0xff);
		uint8_t valStrLen = 0;

		valStrLen += ((val / 10) >= 1);
		valStrLen += ((val / 10) >= 10);
	
		for (int j = valStrLen; j >= 0; j--) {
		
			switch (j) {
			
				case 2:str[strIndex] = (val/100)+'0';	 break;
				case 1:str[strIndex] = ((val/10)%10)+'0';break;
				case 0:str[strIndex] = (val%10)+'0';	 break;
				
				default:str[strIndex] = '?';break;
			}
			strIndex++;
		}
		str[strIndex] = '.';
		strIndex++;
	}
	strIndex--;
	str[strIndex] = '\0';

	return str;
}



char* os::argparse(char* args, int8_t num) {

	if (num < 0) { return args; }
	char buffer[256];

	bool valid = false;
	uint8_t argIndex = 0;
	uint8_t bufferIndex = 0;


	for (int i = 0; i < (strlen(args) + 1); i++) {
	
		if (args[i] == ' ' || args[i] == '\0') {
		
			if (valid) {
				if (argIndex == num) {
				
					buffer[bufferIndex] = '\0';
					char* arg = buffer;
					return arg;
				}
				argIndex++;
			}
			valid = false;
		} else {
			if (argIndex == num) {
				
				buffer[bufferIndex] = args[i];
				bufferIndex++;
			}
			valid = true;
		}
	}
	//       |
	//this   v
	return "wtf";
}

uint8_t os::argcount(char* args) {

	uint8_t i = 0;
	char* foo = argparse(args, i);
	
	//and this gotta be the same
	while (foo != "wtf") {
	
		foo = argparse(args, i);
		i++;
	};
	return i-1;
}


uint8_t os::arg1len(char* args) {

	for (int i = 0; i < strlen(args); i++) {
	
		if (args[i] == ' ') {
		
			return i;
		}
	}
	return 0;
}


char* os::argshift(char* args, uint16_t size) {

	uint16_t spaceIndex = 0;
	bool spaceFound = false;

	for (int i = 0; i < size; i++) {
	
		if (args[i] == ' ') {
		
			spaceIndex = i+1;
			spaceFound = true;
		} else {
			if (spaceFound) { break; }
		}
	}
	
	for (int i = 0; i < size; i++) {
	
		args[i] = args[i+spaceIndex];
	}
	args[size-spaceIndex] = '\0';
	
	return args;
}
