#include <gui/button.h>

using namespace os;
using namespace os::common;
using namespace os::gui;



Button::Button() {

	for (int i = 0; i < 96; i++) { 
		
		this->buttonTypes[i] = 0xff; 
		this->buttonFiles[i] = nullptr;
	}

	buttonTypes[0] = 1;
	buttonTypes[1] = 2;
	buttonTypes[2] = 3;

	buttonTypes[3] = 16;
	buttonFiles[3] = "test";
	//this->numOfButtons = 3;
	this->numOfButtons = 4;
}


Button::~Button() {
}


void Button::Draw(GraphicsContext* gc) {

	//init buffer 
	//with pretty buttons
	uint8_t buttonx = 0;
	uint8_t buttony = 0;

	//96 different possible desktop shortcuts
	for (int i = 0; i < this->numOfButtons; i++) {

		switch (this->buttonTypes[i]) {

			//ayumuscript programs
			case 0:	
				//gc->PutText();
				break;
			//kernel programs
			case 1:
				gc->FillBuffer(buttonx, buttony+3, 20, 15, cliButton, false);
				gc->PutText("CLI", buttonx+1, buttony+21, 0x40);
				gc->PutText("CLI", buttonx, buttony+20, 0x3f);
				break;
			case 2:
				gc->FillBuffer(buttonx, buttony+3, 20, 15, paintButton, false);
				gc->PutText("PNT", buttonx+1, buttony+21, 0x40);
				gc->PutText("PNT", buttonx, buttony+20, 0x3f);
				break;
			case 3:
				gc->FillBuffer(buttonx, buttony+3, 20, 15, journalButton, false);
				gc->PutText("JRN", buttonx+1, buttony+21, 0x40);
				gc->PutText("JRN", buttonx, buttony+20, 0x3f);
				break;
			//files
			case 16:
				gc->FillBuffer(buttonx+2, buttony+2, 16, 16, fileButton, false);
				gc->PutText("FIL", buttonx+1, buttony+21, 0x40);
				gc->PutText("FIL", buttonx, buttony+20, 0x3f);
				break;
			default:
				break;
		}
		buttonx += 20;

		if (buttonx >= 320) {
		
			buttonx = 0;
			buttony += 30;
		}
	}
}


void Button::OnMouseDown(int32_t x, int32_t y, uint8_t button, Desktop* desktop) {
		
	for (int i = 0; i < desktop->numChildren; i++) {
	
		if (desktop->children[i]->ContainsCoordinate(x, y)) { return; }
	}


	uint8_t buttonNum = (16*(y/30)+(x/20));
	uint8_t appType = buttonTypes[buttonNum];
	CompositeWidget* widget = nullptr;
	Window* win = nullptr;

	if (button == 1) {

		switch (appType) {
	
			case 0:
				break;
			case 1:
				desktop->CreateChild(1, "Osaka's Terminal", nullptr);
				break;
			case 2:
				desktop->CreateChild(2, "KasugaPaint", nullptr);
				break;
			case 3:
				desktop->CreateChild(3, "Journal", nullptr);
				break;
			case 16:
				widget = desktop->CreateChild(3, "Journal", nullptr);
				win = (Window*)widget;
				win->app->ReadInput(buttonFiles[buttonNum], widget, win->filesystem);
				break;
			default:
				break;
		}
	} else {
	}
}


void Button::AddFile(char* fileName) {

	this->buttonTypes[numOfButtons] = 16;
	this->buttonFiles[numOfButtons] = fileName;
	numOfButtons++;
}
/*
void Button::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
}


void Button::OnMouseMove(int32_t oldx, int32_t oldy, 
			int32_t newx, int32_t newy) {
}
*/
