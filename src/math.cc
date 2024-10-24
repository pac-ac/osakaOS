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
