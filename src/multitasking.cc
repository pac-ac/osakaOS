#include <multitasking.h>

using namespace os;
using namespace os::common;



Task::Task(GlobalDescriptorTable *gdt, void entrypoint()) {

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

	//cpustate -> error = 0;

	//cpustate -> esp = 0;
	cpustate -> eip = (uint32_t)entrypoint;
	//cpustate -> cs = 0x10;
	cpustate -> cs = gdt->CodeSegmentSelector();
	//cpustate -> ss = 0;
	cpustate -> eflags = 0x202;
}


Task::~Task() {
}




TaskManager::TaskManager() {

	numTasks = 0;
	currentTask = -1;
}



TaskManager::~TaskManager() {
}



bool TaskManager::AddTask(Task* task) {

	if (numTasks >= 256) {
		return false;
	} 
	
	tasks[numTasks++] = task;
	return true;	
}



void TaskManager::ReplaceTask(Task* task, uint8_t index) {
	
	tasks[index] = task;	
}



CPUState* TaskManager::Schedule(CPUState* cpustate) {

	if (numTasks <= 0) {
		return cpustate;
	}

	if (currentTask >= 0) {
		tasks[currentTask]->cpustate = cpustate;
	}

	if (++currentTask >= numTasks) {
		
		currentTask = 0;
	}

	return tasks[currentTask]->cpustate;
}


	
	
