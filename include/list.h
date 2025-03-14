#ifndef __OS__LIST_H
#define __OS__LIST_H


#include <common/types.h>
#include <memorymanagement.h>


namespace os {


	class Node {

		public:
			void* value;
			Node* next;
	
		public:
			Node(void* value);
			~Node();
	};

	class List {

		public:
			MemoryManager* memoryManager;

			common::uint32_t numOfNodes;
			Node* entryNode;
			Node* lastNode;
			
			Node* indexNode;
		public:
			List(MemoryManager* memoryManager);
			~List();
			
			void DestroyList();


			Node* CreateNode(void* value);
			void DestroyNode(Node* node);

			void* Read(common::uint32_t index);
			void Write(void* value ,common::uint32_t index);

			void Push(void* value);
			void Pop();
			
			void Insert(void* value, common::uint32_t index);
			void Remove(common::uint32_t index);

			void AddByte(common::uint8_t value, common::int32_t index = -1);
			void RemoveByte(common::uint8_t value);
			
			void AddInt(common::uint32_t value);
			void RemoveInt(common::uint32_t value);
			
			void AddString(char* str);
			void RemoveString(char* str);
	};
}


#endif
