#ifndef __OS__GDT_H
#define __OS__GDT_H

#include <common/types.h>


namespace os {


	class GlobalDescriptorTable {

		public:
			class SegmentDescriptor {
			
				private:
					os::common::uint16_t limit_lo; 
					os::common::uint16_t base_lo;
					os::common::uint8_t base_hi; 
					os::common::uint8_t type; 
					os::common::uint8_t flags_limit_hi; 
					os::common::uint8_t base_vhi;

				public:
					SegmentDescriptor(os::common::uint32_t base, os::common::uint32_t limit, os::common::uint8_t type);
					
					os::common::uint32_t Base();
					os::common::uint32_t Limit();
			
			} __attribute__((packed));
	
		SegmentDescriptor nullSegmentSelector;
		SegmentDescriptor unusedSegmentSelector;
		SegmentDescriptor codeSegmentSelector;
		SegmentDescriptor dataSegmentSelector;

	
	public:
		GlobalDescriptorTable();
		~GlobalDescriptorTable();
	
		os::common::uint16_t CodeSegmentSelector();
		os::common::uint16_t DataSegmentSelector();
	
	};
}



#endif
