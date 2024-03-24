#include <gui/button.h>

using namespace os;
using namespace os::common;
using namespace os::gui;


Button::Button(Desktop* desktop, int32_t x, int32_t y, uint8_t appType, uint8_t* buf) 
: CompositeWidget(desktop, x, y, 20, 20, "fgsfds", 0, false) {
}


Button::~Button() {
}


void Button::Draw(GraphicsContext* gc) {
}




void Button::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
}


void Button::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
}


void Button::OnMouseMove(int32_t oldx, int32_t oldy, 
			int32_t newx, int32_t newy) {
}
