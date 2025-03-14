#ifndef __OS__MEMORYMANAGEMENT_H
#define __OS__MEMORYMANAGEMENT_H

#include <common/types.h>



namespace os {

	struct MemoryChunk {

		MemoryChunk* next;
		MemoryChunk* prev;
		bool allocated;
		common::size_t size;
	};

	class MemoryManager {
	
		public:
		//protected:
			MemoryChunk* first;
			common::uint32_t size = 0;
		
		public:
			static MemoryManager* activeMemoryManager;

			MemoryManager(common::size_t start, common::size_t size);
			~MemoryManager();

			void* malloc(common::size_t size);
			void free(void* ptr);

			void BufferShift(common::uint8_t* buffer, common::uint32_t bufsize, 
					 common::uint32_t index, common::int32_t shift);
	};
}


void* operator new(unsigned size);
void* operator new[](unsigned size);

// placement new
void* operator new(unsigned size, void* ptr);
void* operator new[](unsigned size, void* ptr);


void operator delete(void* ptr);
void operator delete[](void* ptr);



#endif
