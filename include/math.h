#ifndef __OS__MATH_H
#define __OS__MATH_H


#include <common/types.h>


namespace os {

	namespace math {

		//math library
		common::uint32_t abs(common::int32_t x);
		double absD(double x);
		
		double power(double x, double power);
		
		common::uint8_t log2(common::int32_t x);
		
		double factorial(double x);



		struct point {
		
			common::int32_t x;
			common::int32_t y;
		};


		//trig
		const double pi = 3.14159265358979323846;
		
		double fmod(double a, double b);
		
		double sin(double x);
		double cos(double x);


		//functions for making shapes
		void LineFillLow(common::int32_t x0, common::int32_t y0,
				   common::int32_t x1, common::int32_t y1,
				   struct point arr[], common::uint16_t &index);
		
		void LineFillHigh(common::int32_t x0, common::int32_t y0,
				   common::int32_t x1, common::int32_t y1,
				   struct point arr[], common::uint16_t &index);
	
		common::uint16_t LineFillArray(common::int32_t x0, common::int32_t y0,
						common::int32_t x1, common::int32_t y1,
						struct point arr[]);
		
		typedef struct {
		
			float A;
			float B;
			float C;
			
			float cubeWidth;
			int width;
			int height;
			
			float zBuffer[160*44];
			char buffer[160*44];

			int distanceFromCam;
			float horizontalOffset;
			float K1;

			float x;
			float y;
			float z;
			float ooz;
			int xp;
			int yp;
			int idx;

		} Cube;


		float calculateX(int i, int j, int k, Cube* data);
		float calculateY(int i, int j, int k, Cube* data);
		float calculateZ(int i, int j, int k, Cube* data);
		void calculateForSurface(float cubeX, float cubeY, float cubeZ, 
				int ch,  Cube* data);
		void calculateCube(float incrementSpeed, Cube* data);
	}
}

#endif
