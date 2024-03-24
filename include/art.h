#ifndef __OS__ART_H
#define __OS__ART_H

#include <common/types.h>

namespace os {

	class Funny {

		public:
			Funny();
			~Funny();

			void osakaFace();
			void osakaHead();
			void osakaKnife();
	
			void cat();
			void god();
			
			void osakaAscii();
			
			void cubeAscii(os::common::uint16_t cubeCount);
	};
}

#endif
