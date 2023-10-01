#include <gui/desktop.h>

using namespace os;
using namespace common;
using namespace gui;



Desktop::Desktop(common::int32_t w, common::int32_t h,
		 common::uint8_t r, common::uint8_t g, common::uint8_t b) 
: CompositeWidget(0, 0, 0, w, h, r, g, b), 
  MouseEventHandler() {

	MouseX = w / 2;
	MouseY = h / 2;
	
}


Desktop::~Desktop() {
}



void Desktop::Draw(common::GraphicsContext* gc, bool mode) {

	CompositeWidget::Draw(gc, mode);

	for (int i = 0; i < 6; i++) {
		gc -> PutPixel(MouseX, MouseY-i, 0xff, 0xff, 0xff);
		
		for (int j = 0; j < 6; j++) {
			gc -> PutPixel(MouseX+j, MouseY-i, 0xff, 0xff, 0xff);
		}
	}
}

void Desktop::DrawNoMouse(common::GraphicsContext* gc, bool mode) {

	CompositeWidget::Draw(gc, mode);
}



void Desktop::MouseDraw(common::GraphicsContext* gc, bool mode) {

	for (int i = 0; i < 6; i++) {
		gc -> PutPixel(MouseX, MouseY-i, 0xff, 0xff, 0xff);
		
		for (int j = 0; j < 6; j++) {
			gc -> PutPixel(MouseX+j, MouseY-i, 0xff, 0xff, 0xff);
		}
	}

	if (mode) {
	
		for (int i = 0; i < 6; i++) {
			gc -> PutPixel(oldMouseX, oldMouseY-i, 0x00, 0x00, 0x00);
		
			for (int j = 0; j < 6; j++) {
				gc -> PutPixel(oldMouseX+j, oldMouseY-i, 0x00, 0x00, 0x00);
			}
		}
	}
}



void Desktop::OnMouseDown(common::uint8_t button) {

	CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(common::uint8_t button) {
	
	CompositeWidget::OnMouseUp(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int x, int y) {


	
	//x /= 2;
	//y /= 2;


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

