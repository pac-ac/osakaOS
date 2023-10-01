#ifndef __OS__DRIVERS__MOUSE_H
#define __OS__DRIVERS__MOUSE_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>

namespace os {

	namespace drivers {

		class MouseEventHandler {

			public:
				MouseEventHandler();

				virtual void OnActivate();
				virtual void OnMouseDown(os::common::uint8_t button);
				virtual void OnMouseUp(os::common::uint8_t button);
				virtual void OnMouseMove(int x, int y);
		};


		class MouseDriver : public os::hardwarecommunication::InterruptHandler, public Driver {

			//private:
			public:
				os::hardwarecommunication::Port8Bit dataport;
				os::hardwarecommunication::Port8Bit commandport;

				os::common::uint8_t buffer[3];
				os::common::int8_t offset;
				os::common::uint8_t buttons;

				MouseEventHandler* handler;

        		public:
				bool pressed;
                		
				MouseDriver(os::hardwarecommunication::InterruptManager* manager, MouseEventHandler* handler);
                		~MouseDriver();

                		virtual os::common::uint32_t HandleInterrupt(os::common::uint32_t esp);
				virtual void Activate();
		};
	}
}


#endif

