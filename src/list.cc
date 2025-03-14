#include <list.h>

using namespace os;
using namespace os::common;


void printf(char*);
uint16_t strlen(char*);
bool strcmp(char* one, char* two);



Node::Node(void* value) {

	this->value = value;
	this->next = nullptr;
}

Node::~Node() {
}


List::List(MemoryManager* memoryManager) {
	
	this->memoryManager = memoryManager;
	this->indexNode = nullptr;
}


List::~List() {
}


void List::DestroyList() {

	Node* indexNode = this->entryNode;
	Node* node;

	for (int i = 0; i < this->numOfNodes; i++) {
	
		node = indexNode;
		indexNode = indexNode->next;
		this->DestroyNode(node);
	}

	this->numOfNodes = 0;
}



Node* List::CreateNode(void* value) {
	
	Node* newNode = (Node*)(this->memoryManager->malloc(sizeof(Node)));
	new (newNode) Node(value);
	
	return newNode;
}


void List::DestroyNode(Node* node) {

	this->memoryManager->free(node->value);
	this->memoryManager->free(node);
}


void* List::Read(uint32_t index) {
	
	//return if index is out of bounds
	if (this->numOfNodes <= index) { return nullptr; }

	
	Node* indexNode = this->entryNode;
	
	for (int i = 0; i < index; i++) {
	
		indexNode = indexNode->next;
	}
	return indexNode->value;
}


void List::Write(void* value, uint32_t index) {
	
	//return if index is out of bounds
	if (this->numOfNodes <= index) { return; }

	
	Node* indexNode = this->entryNode;
	
	for (int i = 0; i < index; i++) {
	
		indexNode = indexNode->next;
	}
	
	//write value
	if (indexNode->value != nullptr) {
	
		this->memoryManager->free(indexNode->value);
	}
	indexNode->value = value;
}



void List::Push(void* value) {
	
	Node* node = this->CreateNode(value);

	if (this->numOfNodes == 0) {
	
		//first node in list
		this->entryNode = node;
		this->lastNode = node;
	} else {
		//point to new node as last
		this->lastNode->next = node;
		this->lastNode = node;
	}
	
	this->lastNode->next = nullptr;
	this->numOfNodes++;
}


void List::Pop() {

	if (this->numOfNodes <= 0) { return; }

	Node* indexNode = this->entryNode;
	
	for (int i = 0; i < this->numOfNodes-1; i++) {
	
		indexNode = indexNode->next;
	}

	//delete and update last node
	indexNode->next = nullptr;
	this->DestroyNode(this->lastNode);
	this->lastNode = indexNode;
	
	this->numOfNodes--;
}


void List::Insert(void* value, uint32_t index) {
	
	//push and return if index is out of bounds
	if (this->numOfNodes <= index) { 
		
		this->Push(value);
		return; 
	}

	
	Node* node = this->CreateNode(value);
	Node* indexNode = this->entryNode;

	if (index > 0) {
		
		for (int i = 0; i < index-1; i++) {
	
			//get index in list
			indexNode = indexNode->next;
		}

		//insert in list
		node->next = indexNode->next;
		indexNode->next = node;
	} else {
		node->next = indexNode;
		this->entryNode = node;
	}
	this->numOfNodes++;
}


void List::Remove(uint32_t index) {

	//pop and return if index is out of bounds
	if (this->numOfNodes <= index) { 
		
		this->Pop();
		return; 
	}

	Node* indexNode = this->entryNode;

	if (index > 0) {
	
		for (int i = 0; i < index-1; i++) {
	
			//get index in list
			indexNode = indexNode->next;
		}

		//remove from list
		Node* removeIndex = indexNode->next;
		indexNode->next = indexNode->next->next;
		this->DestroyNode(removeIndex);
	} else {
		//remove from first index
		this->entryNode = indexNode->next;
		Node* removeIndex = indexNode;
		this->DestroyNode(indexNode);
	}
	this->numOfNodes--;
}


void List::AddByte(uint8_t value, int32_t index) {

	uint8_t* newInt = (uint8_t*)(this->memoryManager->malloc(sizeof(uint8_t)));
	new (newInt) uint8_t;
	*newInt = value;

	if (index >= 0) { this->Insert(newInt, index);
	} else { 	  this->Push(newInt); }
}

void List::RemoveByte(uint8_t value) {
	
	Node* indexNode = this->entryNode;

	for (int i = 0; i < this->numOfNodes; i++) {

		if (*((uint8_t*)(indexNode->value)) == value) {
		
			this->Remove(i);
		}
		indexNode = indexNode->next;
	}
}


void List::AddInt(uint32_t value) {

	uint32_t* newInt = (uint32_t*)(this->memoryManager->malloc(sizeof(uint32_t)));
	new (newInt) uint32_t;
	*newInt = value;
	this->Push(newInt);
}

void List::RemoveInt(uint32_t value) {
	
	Node* indexNode = this->entryNode;

	for (int i = 0; i < this->numOfNodes; i++) {

		if (*((uint32_t*)(indexNode->value)) == value) {
		
			this->Remove(i);
		}
		indexNode = indexNode->next;
	}
}



void List::AddString(char* str) {

	int strLen = strlen(str);

	char* newStr = (char*)(this->memoryManager->malloc((sizeof(char)*(strLen+1))));
	new (newStr) char;

	for (int i = 0; i < strLen+1; i++) { newStr[i] = str[i]; }

	this->Push(newStr);
}


void List::RemoveString(char* str) {

}
