#ifndef __OS__MATH_H
#define __OS__MATH_H


#include <common/types.h>


namespace os {

	namespace math {

		common::uint32_t abs(common::int32_t x);
		
		

		struct point {
		
			common::int32_t x;
			common::int32_t y;

		};



		//functions for making line
		void LineFillLow(common::int32_t x0, common::int32_t y0,
				   common::int32_t x1, common::int32_t y1,
				   struct point arr[], common::uint16_t &index);
		
		void LineFillHigh(common::int32_t x0, common::int32_t y0,
				   common::int32_t x1, common::int32_t y1,
				   struct point arr[], common::uint16_t &index);
	
		common::uint16_t LineFillArray(common::int32_t x0, common::int32_t y0,
						common::int32_t x1, common::int32_t y1,
						struct point arr[]);
	}
}

#endif
