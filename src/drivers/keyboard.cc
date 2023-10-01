#include <drivers/keyboard.h>



using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;


KeyboardEventHandler::KeyboardEventHandler() {
}

void KeyboardEventHandler::OnKeyDown(char) {
}

void KeyboardEventHandler::OnKeyUp() {
}

void KeyboardEventHandler::resetMode() {
}


void KeyboardEventHandler::modeSet(uint8_t) {
}




KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler *handler)
: InterruptHandler(0x21, manager),
dataport(0x60),
commandport(0x64) {

	this->handler = handler;
}



KeyboardDriver::~KeyboardDriver() {
}



void KeyboardDriver::Activate() {
	
	while (commandport.Read() & 0x1) {
	
		dataport.Read();

		commandport.Write(0xAE); // activate interrupts
		commandport.Write(0x20); // get current state
		
		uint8_t status = (dataport.Read() | 1) & ~0x10;
		
		commandport.Write(0x60); // set state
		dataport.Write(status);

		dataport.Write(0xF4);
	}
}




void printf(char*);
void printfHex(uint8_t);



uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp) {

	uint8_t key = dataport.Read();
	this->keyHex = key;

	if (handler == 0) {

		return esp;
	}

	//modes				
	static uint8_t mode = 0;
	
		
	if (handler->cli) {
		
		printf("\t");
	}

	

	switch (key) {

		//KEY DOWN

		//special chars 1
		case 0x29:
			if (handler->shift) { handler->OnKeyDown('~'); } else { handler->OnKeyDown('`'); }
			break;
		//numbers
		case 0x02:
			if (handler->shift) { handler->OnKeyDown('!'); } else { handler->OnKeyDown('1'); }
			break;
		case 0x03:
			if (handler->shift) { handler->OnKeyDown('@'); } else { handler->OnKeyDown('2'); }
			break;
		case 0x04:
			if (handler->shift) { handler->OnKeyDown('#'); } else { handler->OnKeyDown('3'); }
			break;
		case 0x05:
			if (handler->shift) { handler->OnKeyDown('$'); } else { handler->OnKeyDown('4'); }
			break;
		case 0x06:
			if (handler->shift) { handler->OnKeyDown('%'); } else { handler->OnKeyDown('5'); }
			break;
		case 0x07:
			if (handler->shift) { handler->OnKeyDown('^'); } else { handler->OnKeyDown('6'); }
			break;
		case 0x08:
			if (handler->shift) { handler->OnKeyDown('&'); } else { handler->OnKeyDown('7'); }
			break;
		case 0x09:
			if (handler->shift) { handler->OnKeyDown('*'); } else { handler->OnKeyDown('8'); }
			break;
		case 0x0a:
			if (handler->shift) { handler->OnKeyDown('('); } else { handler->OnKeyDown('9'); }
			break;
		case 0x0b:
			if (handler->shift) { handler->OnKeyDown(')'); } else { handler->OnKeyDown('0'); }
			break;
		//special chars 2
		case 0x0c:
			if (handler->shift) { handler->OnKeyDown('_'); } else { handler->OnKeyDown('-'); }
			break;
		case 0x0d:
			if (handler->shift) { handler->OnKeyDown('+'); } else { handler->OnKeyDown('='); }
			break;
		//first row of letters
		case 0x10:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('Q'); } else { handler->OnKeyDown('q'); }
			break;
		case 0x11:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('W'); } else { handler->OnKeyDown('w'); }
			break;
		case 0x12:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('E'); } else { handler->OnKeyDown('e'); }
			break;
		case 0x13:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('R'); } else { handler->OnKeyDown('r'); } 
			break;
		case 0x14:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('T'); } else { handler->OnKeyDown('t'); }
			break;
		case 0x15:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('Y'); } else { handler->OnKeyDown('y'); }
			break;
		case 0x16:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('U'); } else { handler->OnKeyDown('u'); }
			break;
		case 0x17:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('I'); } else { handler->OnKeyDown('i'); }
			break;
		case 0x18:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('O'); } else { handler->OnKeyDown('o'); }
			break;
		case 0x19:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('P'); } else { handler->OnKeyDown('p'); }
			break;
		//special chars 3
		case 0x1a:
			if (handler->shift) { handler->OnKeyDown('{'); } else { handler->OnKeyDown('['); }
			break;
		case 0x1b:
			if (handler->shift) { handler->OnKeyDown('}'); } else { handler->OnKeyDown(']'); }
			break;
		case 0x2b:
			if (handler->shift) { handler->OnKeyDown('|'); } else { handler->OnKeyDown('\\'); }
			break;
		//second row of letters		
		case 0x1e:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('A'); } else { handler->OnKeyDown('a'); }
			break;
		case 0x1f:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('S'); } else { handler->OnKeyDown('s'); }
			break;
		case 0x20:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('D'); } else { handler->OnKeyDown('d'); }
			break;
		case 0x21:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('F'); } else { handler->OnKeyDown('f'); }
			break;
		case 0x22:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('G'); } else { handler->OnKeyDown('g'); }
			break;
		case 0x23:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('H'); } else { handler->OnKeyDown('h'); }
			break;
		case 0x24:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('J'); } else { handler->OnKeyDown('j'); }
			break;
		case 0x25:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('K'); } else { handler->OnKeyDown('k'); }
			break;
		case 0x26:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('L'); } else { handler->OnKeyDown('l'); }
			break;
		//special chars 4
		case 0x27:
			if (handler->shift) { handler->OnKeyDown(':'); } else { handler->OnKeyDown(';'); }
			break;
		case 0x28:
			if (handler->shift) { handler->OnKeyDown('\"'); } else { handler->OnKeyDown('\''); }
			break;
		//third row of letters
		case 0x2c:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('Z'); } else { handler->OnKeyDown('z'); }
			break;
		case 0x2d:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('X'); } else { handler->OnKeyDown('x'); }
			break;
		case 0x2e:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('C'); } else { handler->OnKeyDown('c'); }
			break;
		case 0x2f:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('V'); } else { handler->OnKeyDown('v'); }
			break;
		case 0x30:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('B'); } else { handler->OnKeyDown('b'); }
			break;
		case 0x31:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('N'); } else { handler->OnKeyDown('n'); }
			break;
		case 0x32:
			if (handler->shift ^ handler->caps) { handler->OnKeyDown('M'); } else { handler->OnKeyDown('m'); }
			break;
		//special chars 5
		case 0x33:
			if (handler->shift) { handler->OnKeyDown('<'); } else { handler->OnKeyDown(','); }
			break;
		case 0x34:
			if (handler->shift) { handler->OnKeyDown('>'); } else { handler->OnKeyDown('.'); }
			break;
		case 0x35:
			if (handler->shift) { handler->OnKeyDown('?'); } else { handler->OnKeyDown('/'); }
			break;
		
		//space
		case 0x39:
			handler->OnKeyDown(' ');
			break;
		//enter
		case 0x1c:
			handler->OnKeyDown('\n');
			break;
		//handler->caps lock
		case 0x3a:
			//handler->caps ^= 1;
			handler->caps = handler->caps ^ 1;
			break;
		//windows key
		case 0x5b:
			break;
		
		//backspace
		case 0x0e:
			handler->OnKeyDown('\b');
			break;
		//escape
		case 0x01:
			handler->OnKeyDown('\x1b');
			break;

		//right control
		case 0x1d:
			//handler->ctrl = true;
			handler->ctrl = true;
			break;
		case 0x9d:
			//handler->ctrl = false;
			handler->ctrl = false;
			break;
		
		//tab
		case 0x0f:
			handler->OnKeyDown('\v');
			break;
		
		//alt keys
		case 0x38:
			//alt = true;
			handler->alt = true;
			break;
		case 0xb8:
			//alt = true;
			handler->alt = true;
			break;
		case 0xe0:
			//alt = false;
			handler->alt = false;
			break;

		//left arrow
		case 0x4b:
			handler->OnKeyDown('\xfc');
			break;
		//up arrow
		case 0x48:
			handler->OnKeyDown('\xfd');
			break;
		//down arrow
		case 0x50:
			handler->OnKeyDown('\xfe');
			break;
		//right arrow
		case 0x4d:
			handler->OnKeyDown('\xff');
			break;
		
		//handler->shift keys
		case 0x2a:
			handler->shift = true;
			break;
		case 0x36:
			handler->shift = true;
			break;
		case 0xaa: 
			handler->shift = false;
			break;
		case 0xb6: 
			handler->shift = false;
			break;
		default:
			/*		
			printf("KEYBOARD ");
			printfHex(key);
			*/
			handler->OnKeyUp();
			break;
	}

	return esp;
}
