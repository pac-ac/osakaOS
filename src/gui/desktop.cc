#include <gui/desktop.h>

using namespace os;
using namespace common;
using namespace gui;


void sleep(uint32_t);


Desktop::Desktop(common::int32_t w, common::int32_t h,
		 common::uint8_t color) 
: CompositeWidget(0, 0, 0, w, h, color), 
  MouseEventHandler() {

	MouseX = w / 2;
	MouseY = h / 2;
	
	this->color = color;
}


Desktop::~Desktop() {
}



void Desktop::Draw(common::GraphicsContext* gc, bool mode) {

	//desktop background, one big color for now
	gc->FillRectangle(0, 0, 200, 320, this->color);
	
	//windows on top
	CompositeWidget::Draw(gc, mode);
	
	//cursor
	this->MouseDraw(gc, mode);
}

void Desktop::DrawNoMouse(common::GraphicsContext* gc, bool mode) {

	CompositeWidget::Draw(gc, mode);
}



void Desktop::MouseDraw(common::GraphicsContext* gc, bool mode) {

	//mouse icon
	uint16_t cursorIndex = 0;
	uint16_t asdf = 0;
	
	for (uint8_t y = 0; y < 20; y++) {
		for (uint8_t x = 0; x < 13; x++) {
			
			if (this->click) {
				if (cursorClick[cursorIndex]) {
					gc->PutPixel(MouseX+x, MouseY+y, cursorClick[cursorIndex]);
				}
			} else {
				if (cursorNormal[cursorIndex]) {
					gc->PutPixel(MouseX+x, MouseY+y, cursorNormal[cursorIndex]);
				}
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

