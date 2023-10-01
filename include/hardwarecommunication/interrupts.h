#ifndef __OS__HARDWARECOMMUNICATION__INTERRUPTS_H
#define __OS__HARDWARECOMMUNICATION__INTERRUPTS_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <gdt.h>
#include <multitasking.h>


namespace os {

	namespace hardwarecommunication {


		class InterruptManager;


		class InterruptHandler {

			//protected:
			public:

				os::common::uint8_t interruptNumber;
				InterruptManager* interruptManager;
				

				InterruptHandler(os::common::uint8_t interruptNumber, InterruptManager* interruptManager);
				~InterruptHandler();

			public:

				virtual os::common::uint32_t HandleInterrupt(os::common::uint32_t esp);

		};



		class InterruptManager {

			friend class InterruptHandler;
				
			//protected:
			public:
				static InterruptManager* ActiveInterruptManager;


				InterruptHandler* handlers[256];	
				TaskManager *taskManager;

				struct GateDescriptor {
		
					os::common::uint16_t handlerAddressLowBits;
					os::common::uint16_t gdt_codeSegmentSelector;
					os::common::uint8_t reserved;
					os::common::uint8_t access;
					os::common::uint16_t handlerAddressHighBits; 

				} __attribute__((packed));

				static GateDescriptor interruptDescriptorTable[256];

				struct InterruptDescriptorTablePointer {
		
					os::common::uint16_t size;
					os::common::uint32_t base;

				} __attribute__((packed));
	

				os::common::uint16_t hardwareInterruptOffset;


				static void SetInterruptDescriptorTableEntry (

					os::common::uint8_t interruptNumber,
					os::common::uint16_t codeSegmentSelectorOffset,
					void (*handler)(),
					os::common::uint8_t DescriptorPrivilegeLevel,
					os::common::uint8_t DescriptorType
				);

				static void IgnoreInterruptRequest();
				
				//requests
				static void HandleInterruptRequest0x00();
				static void HandleInterruptRequest0x01();	
				static void HandleInterruptRequest0x02();
				static void HandleInterruptRequest0x03();
				static void HandleInterruptRequest0x04();
				static void HandleInterruptRequest0x05();
				static void HandleInterruptRequest0x06();
				static void HandleInterruptRequest0x07();
				static void HandleInterruptRequest0x08();
				static void HandleInterruptRequest0x09();
				static void HandleInterruptRequest0x0A();
				static void HandleInterruptRequest0x0B();
				static void HandleInterruptRequest0x0C();
				static void HandleInterruptRequest0x0D();
				static void HandleInterruptRequest0x0E();
				static void HandleInterruptRequest0x0F();

				
				static void HandleInterruptRequest0x31();
				static void HandleInterruptRequest0x80();




				//exceptions
				static void HandleException0x00();
				static void HandleException0x01();
				static void HandleException0x02();
				static void HandleException0x03();
				static void HandleException0x04();
				static void HandleException0x05();
				static void HandleException0x06();
				static void HandleException0x07();
				static void HandleException0x08();
				static void HandleException0x09();
				static void HandleException0x0A();
				static void HandleException0x0B();
				static void HandleException0x0C();
				static void HandleException0x0D();
				static void HandleException0x0E();
				static void HandleException0x0F();
				static void HandleException0x10();
				static void HandleException0x11();
				static void HandleException0x12();
				static void HandleException0x13();


				static os::common::uint32_t handleInterrupt(os::common::uint8_t interruptNumber, os::common::uint32_t esp);
			
				os::common::uint16_t interruptCount;
				//os::common::uint8_t countCheck;
				os::common::uint32_t DoHandleInterrupt(os::common::uint8_t interruptNumber, os::common::uint32_t esp);

				Port8BitSlow picMasterCommand;
				Port8BitSlow picMasterData;
				Port8BitSlow picSlaveCommand;
				Port8BitSlow picSlaveData;


			public:

				InterruptManager(os::common::uint16_t hardwareInterruptOffset, 
						GlobalDescriptorTable* gdt, os::TaskManager* taskManager);
				~InterruptManager();

				os::common::uint16_t HardwareInterruptOffset();

				void Activate();
				void Deactivate();
		};
	}
}



#endif
