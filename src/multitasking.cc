#include <multitasking.h>

using namespace os;
using namespace os::common;

void printf(char*);


Task::Task(GlobalDescriptorTable *gdt, void(*entrypoint)(), char name[33], uint32_t intCount) {

	//assign task name
	for (int i = 0; i < 33; i++) {
		
		taskname[i] = name[i];
	}

	this->instructionCount = intCount;

	cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));

	cpustate -> eax = 0;
	cpustate -> ebx = 0;
	cpustate -> ecx = 0;
	cpustate -> edx = 0;

	cpustate -> esi = 0;
	cpustate -> edi = 0;
	cpustate -> ebp = 0;

	/*	
	cpustate -> gs = 0;
	cpustate -> fs = 0;
	cpustate -> es = 0;
	cpustate -> ds = 0;
	*/

	cpustate -> error = 0;

	cpustate -> esp = 0;
	cpustate -> eip = (uint32_t)entrypoint;
	this->intPtr = cpustate->eip;;
	
	//cpustate -> cs = 0x10;
	cpustate -> cs = gdt->CodeSegmentSelector();
	cpustate -> ss = 0;
	cpustate -> eflags = 0x202;
}

Task::~Task() {}


TaskManager::TaskManager(GlobalDescriptorTable* gdt) {

	this->gdt = gdt;

	numTasks = 0;
	currentTask = -1;

	for (int i = 0; i < 256; i++) {
	
		tasks[i] = 0;
		taskPriority[i] = 0;
	}
}


TaskManager::~TaskManager() {}


bool TaskManager::AddTask(Task* task) {

	if (numTasks >= 256) { return false; }

	tasks[numTasks++] = task;
	return true;
}



bool TaskManager::DeleteTask(uint32_t taskNum) {

	if (numTasks <= 0) { return false; }
	
	currentTask = -1;
	numTasks--;
	
	tasks[taskNum]->kill = true;
	//tasks[taskNum] = 0;

	return true;
}



CPUState* TaskManager::Schedule(CPUState* cpustate) {

	if (numTasks <= 0) { return cpustate; }
	
	if (currentTask >= 0) {

		tasks[currentTask]->kill = 
			((cpustate->eip - tasks[currentTask]->intPtr) >= tasks[currentTask]->instructionCount 
				&& tasks[currentTask]->instructionCount > 0);

		//check if task is to be ended
		if (tasks[currentTask]->kill == true) {
	
			//save final state of registers for debugging
			this->eaxPrint = cpustate->eax;
			this->ebxPrint = cpustate->ebx;
			this->ecxPrint = cpustate->ecx;
			this->edxPrint = cpustate->edx;
			
			this->espPrint = cpustate->esp;
			this->ebpPrint = cpustate->ebp;

			
			
			//remove task from manager
			this->DeleteTask(currentTask);
			if (++currentTask >= numTasks) { currentTask = 0; }
			return tasks[currentTask]->cpustate;
		}
		tasks[currentTask]->cpustate = cpustate;

		
		/*	
		//compute task according to assigned priority
		for (uint16_t i = 0; i < taskPriority[currentTask]+1; i++) {
		
			tasks[currentTask]->cpustate = cpustate;
		}
		*/

		//if task is binary
		if (tasks[currentTask]->instructionCount > 0) {
	
			tasks[currentTask]->currentCount++;
		}
	}
	
	//go through tasks again
	if (++currentTask >= numTasks) { currentTask = 0; }
	

	//dont crash lol
	return tasks[currentTask]->cpustate;
}
