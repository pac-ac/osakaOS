#include <math.h>

using namespace os;
using namespace os::common;
using namespace os::math;



uint32_t os::math::abs(int32_t x) {

	int32_t y = x >> 31;
	return ((x ^ y) - y);
}


double os::math::absD(double x) {

	if (x < 0.0) { return -x; }
	return x;
}


double os::math::power(double x, double p) {

	double n = x;
	for (double i = 1.0; i < p; i += 1.0) { n *= x; }
	return n;
}


uint8_t os::math::log2(int32_t x) {

	uint8_t highestBitSet = 0;

	while (x >>= 1) { highestBitSet++; }
	return highestBitSet;
}


double os::math::factorial(double x) {

	if (x == 0) { return 1.0; }
	return x * (factorial(x - 1.0));
}



double os::math::fmod(double a, double b) {

	double frac = a / b;
	int floor = frac > 0 ? (int)frac : (int)(frac - 0.999999999999999);
	//int floor = frac > 0 ? (int)frac : (int)(frac - 0.9999999999999999);
	return (a - b * (double)floor);
}



double os::math::sin(double x) {

	x = fmod(x, 2*pi);
	if (x < 0) { x = (2 * pi) - x; }
	
	int8_t sign = 1;

	if (x > pi) {
	
		x -= pi;
		sign = -1;
	}
	double result = x;
	double coefficient = 3.0;

	for (int i = 0; i < 10; i++) {
	
		double pow = power(x, coefficient);
		double frac = factorial(coefficient);
	
		if (i % 2 == 0) { result -= (pow/frac); }
		else { result += (pow/frac); }

		coefficient += 2.0;
	}
	return ((double)sign)*result;
}

double os::math::cos(double x) { 
	
	return sin((pi / 2.0) - x); 
}



void os::math::LineFillLow(int32_t x0, int32_t y0,
				int32_t x1, int32_t y1,
				struct point arr[], uint16_t &index) {

	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int16_t yi = 1;

	if (dy < 0) {
	
		yi = -1;
		dy = -dy;
	}
	int16_t D = (2*dy) - dx;
	int16_t y = y0;

	for (int x = x0; x < x1; x++) {
	
		arr[index].x = x;
		arr[index].y = y;
		index++;
	
		if (D > 0) {
		
			y += yi;
			D += (2*(dy-dx));
		} else {
		
			D += 2*dy;
		}
	}
}


void os::math::LineFillHigh(int32_t x0, int32_t y0,
				int32_t x1, int32_t y1,
				struct point arr[], uint16_t &index) {

	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int16_t xi = 1;

	if (dx < 0) {
	
		xi = -1;
		dx = -dx;
	}
	int16_t D = (2*dx) - dy;
	int16_t x = x0;

	for (int y = y0; y < y1; y++) {
	
		arr[index].x = x;
		arr[index].y = y;
		index++;

		if (D > 0) {
		
			x += xi;
			D += (2*(dx-dy));
		} else {
		
			D += 2*dx;
		}
	}
}


uint16_t os::math::LineFillArray(int32_t x0, int32_t y0,
				int32_t x1, int32_t y1,
				struct point arr[]) {

	uint16_t index = 0;

	if (abs(y1 - y0) < abs(x1 - x0)) {
	
		if (x0 > x1) { LineFillLow(x1, y1, x0, y0, arr, index);
		} else {       LineFillLow(x0, y0, x1, y1, arr, index); }
	} else {
	
		if (y0 > y1) { LineFillHigh(x1, y1, x0, y0, arr, index);
		} else {       LineFillHigh(x0, y0, x1, y1, arr, index); }
	}

	return index;
}


float os::math::calculateX(int i, int j, int k, Cube* data) {

	return j * sin(data->A) * sin(data->B) * cos(data->C) - k * cos(data->A) * sin(data->B) * cos(data->C) + 
		j * cos(data->A) * sin(data->C) + k * sin(data->A) * sin(data->C) + i * cos(data->B) * cos(data->C);
}
float os::math::calculateY(int i, int j, int k, Cube* data) {

	return j * cos(data->A) * cos(data->C) + k * sin(data->A) * cos(data->C) - 
		j * sin(data->A) * sin(data->B) * sin(data->C) + k * cos(data->A) * sin(data->B) * sin(data->C) - 
		i * cos(data->B) * sin(data->C);
}
float os::math::calculateZ(int i, int j, int k, Cube* data) {
	
	return k * cos(data->A) * cos(data->B) - j * sin(data->A) * cos(data->B) + i * sin(data->B);
}

void os::math::calculateForSurface(float cubeX, float cubeY, float cubeZ, int ch, Cube* data) {

	data->x = calculateX(cubeX, cubeY, cubeZ, data);
	data->y = calculateY(cubeX, cubeY, cubeZ, data);
	data->z = calculateZ(cubeX, cubeY, cubeZ, data) + data->distanceFromCam;

	data->ooz = 1.0 / data->z;

	data->xp = (int)(data->width / 2 + data->horizontalOffset + data->K1 * data->ooz * data->x * 2);
	data->yp = (int)(data->height / 2 + data->K1 * data->ooz * data->y);

	data->idx = data->xp + data->yp * data->width;

	if (data->idx >= 0 && data->idx < data->width*data->height) {
	
		if (data->ooz > data->zBuffer[data->idx]) {

			data->zBuffer[data->idx] = data->ooz;
			data->buffer[data->idx] = ch;
		}
	}
}

void os::math::calculateCube(float incrementSpeed, Cube* data) {

	for (float cubeX = -data->cubeWidth; cubeX < data->cubeWidth; cubeX += incrementSpeed) {
		for (float cubeY = -data->cubeWidth; cubeY < data->cubeWidth; cubeY += incrementSpeed) {

			calculateForSurface(cubeX, cubeY, -data->cubeWidth, '@', data);
			calculateForSurface(data->cubeWidth, cubeY, cubeX, '$', data);
			calculateForSurface(-data->cubeWidth, cubeY, -cubeX, '~', data);
			calculateForSurface(-cubeX, cubeY, data->cubeWidth, '#', data);
			calculateForSurface(cubeX, -data->cubeWidth, -cubeY, ';', data);
			calculateForSurface(cubeX, data->cubeWidth, cubeY, '+', data);
		}
	}
}
