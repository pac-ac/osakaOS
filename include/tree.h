#ifndef __OS__TREE_H
#define __OS__TREE_H


#include <common/types.h>
#include <memorymanagement.h>
#include <string.h>
#include <list.h>


namespace os {


	class Child {

		public:
			void* value;
			Child* parent;
			List* children;
		public:
			Child(void* value, Child* parent);
			~Child();
	};

	class Tree : public Child {

		public:
			MemoryManager* memoryManager;

			common::uint32_t numOfNodes;
			common::uint32_t depth;
		public:
			Tree(MemoryManager* memoryManager, void* value);
			~Tree();
			
			
			Child* AddChild(void* value, Child* parent);
			Child* AddChildInt(common::uint32_t num, Child* parent);
			bool DestroyChildrenOfNode(Child* child);
	};
}


#endif
