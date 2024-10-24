#include <app.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::drivers;
using namespace os::filesystem;


App::App() {

	this->PID = 0;
}

App::~App() {
}

void App::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {
}

void App::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {
}

void App::Close() {
}

void App::SaveOutput(char* file, CompositeWidget* widget, FileSystem* filesystem) {
}
void App::ReadInput(char* file, CompositeWidget* widget, FileSystem* filesystem) {
}


void App::OnKeyDown(char ch, CompositeWidget* widget) {
}
void App::OnKeyUp(char ch, CompositeWidget* widget) {
}


void App::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
}
void App::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
}
void App::OnMouseMove(int32_t oldx, int32_t oldy,
			int32_t newx, int32_t newy,
			CompositeWidget* widget) {
}
