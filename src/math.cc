#include <math.h>

using namespace os;
using namespace os::common;
using namespace os::math;


uint32_t os::math::abs(int32_t x) {

	int32_t y = x >> 31;
	return ((x ^ y) - y);
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
