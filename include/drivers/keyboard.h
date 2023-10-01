#ifndef __OS__DRIVERS__KEYBOARD_H
#define __OS__DRIVERS__KEYBOARD_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>


namespace os {

	namespace drivers {

		class KeyboardEventHandler {
		
			//protected:
			public:
				bool ctrl;
				bool alt;
				bool shift;
				bool caps;
				
				bool cli;

			public:
				KeyboardEventHandler();

				virtual void OnKeyDown(char);
				virtual void OnKeyUp();
				
				virtual void resetMode();
				
				//modes
				virtual void modeSet(os::common::uint8_t);
		};


		class KeyboardDriver : public os::hardwarecommunication::InterruptHandler, public Driver {

				
			//private:
			public:
				os::hardwarecommunication::Port8Bit dataport;
				os::hardwarecommunication::Port8Bit commandport;

				KeyboardEventHandler* handler;

			public:
			
				os::common::uint8_t keyHex = 0xff;

				KeyboardDriver(os::hardwarecommunication::InterruptManager* manager, KeyboardEventHandler *handler);
				~KeyboardDriver();
		
				virtual os::common::uint32_t HandleInterrupt(os::common::uint32_t esp);
				virtual void Activate();	
		};

	}
}


#endif
