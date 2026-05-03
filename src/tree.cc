#include <tree.h>

using namespace os;
using namespace os::common;


void printf(char*);


Child::Child(void* value, Child* parent) {

	this->value = value;
	this->parent = parent;
	this->children = nullptr;
}

Child::~Child() {
}


Tree::Tree(MemoryManager* memoryManager, void* value) 
: Child(value, nullptr) {
	
	this->memoryManager = memoryManager;
	this->numOfNodes = 0;
	this->depth = 0;
}


Tree::~Tree() {
}



Child* Tree::AddChild(void* value, Child* parent) {

	if (parent->children == nullptr) {
	
		parent->children = (List*)this->memoryManager->malloc(sizeof(List));
		new (parent->children) List(memoryManager);
	}

	Child* newChild = (Child*)this->memoryManager->malloc(sizeof(Child));
	new (newChild) Child(value, parent);
	parent->children->Push(newChild);

	this->numOfNodes++;

	return newChild;
}


Child* Tree::AddChildInt(uint32_t num, Child* parent) {

	if (parent->children == nullptr) {
	
		parent->children = (List*)this->memoryManager->malloc(sizeof(List));
		new (parent->children) List(memoryManager);
	}

	uint32_t* value = (uint32_t*)this->memoryManager->malloc(sizeof(uint32_t));
	*value = num;

	Child* newChild = (Child*)this->memoryManager->malloc(sizeof(Child));
	new (newChild) Child(value, parent);
	parent->children->Push(newChild);

	this->numOfNodes++;
	
	return newChild;
}


bool Tree::DestroyChildrenOfNode(Child* child) {

	if (child->children == nullptr) {

		return false;
	} else {
		for (int i = 0; i < child->children->numOfNodes; i++) {
	
			if (this->DestroyChildrenOfNode((Child*)child->children->Read(i)) == false) {
			
				child->children->Remove(i);
				
				if (child->children->numOfNodes == 0) {
				
					this->memoryManager->free(child->children);
					child->children = nullptr;
				}
				i--;
			}
		}
	}
	return true;
}
