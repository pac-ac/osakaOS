#include <gui/desktop.h>

using namespace os;
using namespace common;
using namespace gui;


void sleep(uint32_t);


Desktop::Desktop(common::int32_t w, common::int32_t h,
		 common::uint8_t color,
		 common::GraphicsContext* gc,
		 GlobalDescriptorTable* gdt,
		 TaskManager* taskManager,
		 Simulator* osaka)
: CompositeWidget(0, 0, 0, w, h, "desktop", color, false), 
  MouseEventHandler() {

	MouseX = w / 2;
	MouseY = h / 2;
	
	this->color = color;

	this->gc = gc;
	this->taskManager = taskManager;
	this->gdt = gdt;
	this->osaka = osaka;
}


Desktop::~Desktop() {
}



bool Desktop::AddTaskGUI(Task* task) {
	
	return this->taskManager->AddTask(task);
}



void Desktop::Draw(common::GraphicsContext* gc) {
	
	if (this->keyValue == 0x5b) {

		this->sim ^= 1;
		this->windowOffset ^= 100;
		this->OnMouseUp(0);
		this->keyValue = 0;
	}

	uint8_t index = 0;
	
	if (this->sim == false) {

	
		for (int y = 0; y < 200; y++) {
			for (int x = 0; x < 320; x++) {
			
				index = defaultbg[320*y+x];
				gc->PutPixel(x, y, rrrgggbb_to_ega[index]);
			}
		}

		//windows on top
		CompositeWidget::Draw(gc);
	} else {
		this->osaka->DrawRoom(gc);
	}
		
	//cursor
	this->MouseDraw(gc);
		
	//write to video memory
	gc->DrawToScreen();
}


void Desktop::DrawNoMouse(common::GraphicsContext* gc) {

	CompositeWidget::Draw(gc);
}



void Desktop::MouseDraw(common::GraphicsContext* gc) {
	

	//mouse icon
	uint16_t cursorIndex = 0;
	uint8_t* cursorArt = nullptr;


	uint8_t mouseW = 13;
	uint8_t mouseH = 20;


	if (this->sim == false) {

		if (click) { cursorArt = cursorClickLeft; } 
		else { cursorArt = cursorNormal; }
	} else {
	
		cursorArt = cursorClassic;
		mouseW = 7;
		mouseH = 7;
	}
	

	
	for (uint8_t y = 0; y < mouseH; y++) {
		for (uint8_t x = 0; x < mouseW; x++) {
			
			if (cursorArt[cursorIndex]) {
			
				gc->PutPixel(MouseX+x, MouseY+y, cursorArt[cursorIndex]);
			}
			cursorIndex++;
		}
	}
}



void Desktop::OnMouseDown(common::uint8_t button) {

	this->click = true;
	CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(common::uint8_t button) {
	
	this->click = false;
	CompositeWidget::OnMouseUp(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int x, int y) {


	int32_t newMouseX = MouseX + x;
	this->oldMouseX = MouseX;

	if (newMouseX < 0) {
	
		newMouseX = 0;
	}	
	if (newMouseX >= w) {
	
		newMouseX = w - 1;
	}



	int32_t newMouseY = MouseY + y;
	this->oldMouseY = MouseY;
	
	if (newMouseY < 0) {
	
		newMouseY = 0;
	}	
	if (newMouseY >= h) {
	
		newMouseY = h - 1;
	}

	CompositeWidget::OnMouseMove(MouseX, MouseY, newMouseX, newMouseY);
	MouseX = newMouseX;
	MouseY = newMouseY;
	
}


void Desktop::OnKeyDown(char str) {

	CompositeWidget::OnKeyDown(str);
}


void Desktop::OnKeyUp(char str) {
		
	CompositeWidget::OnKeyUp(str);
}
