#ifndef __OS__DRIVERS__DRIVER_H
#define __OS__DRIVERS__DRIVER_H

namespace os {

	namespace drivers {

		class Driver {
	
			public:
				Driver();
				~Driver();

				virtual void Activate();
				virtual int Reset();
				virtual void Deactive();
	
		};

		class DriverManager {
	
			//private:
			public:
				Driver* drivers[256];
				int numDrivers = 0;

			public:
				DriverManager();
				void AddDriver(Driver*);
				void ActivateAll();
				void Replace(Driver* drv, int numDrv);
	
		};
	}
}









#endif
