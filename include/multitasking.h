#ifndef __OS__MULTITASKING_H
#define __OS__MULTITASKING_H

#include <gdt.h>
#include <memorymanagement.h>
#include <common/types.h>


namespace os {

	struct CPUState {

		common::uint32_t eax;
		common::uint32_t ebx;
		common::uint32_t ecx;
		common::uint32_t edx;

		common::uint32_t esi;
		common::uint32_t edi;
		common::uint32_t ebp;


		/*
		common::uint32_t gs;
		common::uint32_t fs;
		common::uint32_t es;
		common::uint32_t ds;
		*/


		common::uint32_t error;
		
		common::uint32_t eip;
		common::uint32_t cs;
		common::uint32_t eflags;	
		common::uint32_t esp;
		common::uint32_t ss;

	} __attribute__((packed)); 


	class Task {

		friend class TaskManager;
		
		//private:
		public:
			common::uint8_t stack[4096]; // 4 KB
			CPUState* cpustate;
		
			char* taskname;
		public:
			//Task(GlobalDescriptorTable *gdt, void (*entrypoint)(TaskManager*));
			Task(GlobalDescriptorTable *gdt, void entrypoint(), char* name);
			~Task();

	};

	class TaskManager {
		
		//private:
		public:
			GlobalDescriptorTable* gdt;

			Task* tasks[256];
			int numTasks;
			int currentTask;

		public:
			TaskManager(GlobalDescriptorTable* gdt);
			~TaskManager();

			bool AddTask(Task* task);
			bool DeleteTask(common::uint32_t taskNum);

			void EndTask();

			CPUState* Schedule(CPUState* cpustate);
	};
}




#endif
