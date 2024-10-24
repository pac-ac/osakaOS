#include <gui/games/shooter.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::drivers;


Shooter::Shooter() {
}
Shooter::~Shooter() {
}

void Shooter::Draw(GraphicsContext* gc, uint16_t ticks, 
		   char keylog[16], uint8_t logIndex, bool keyPress, 
		   int32_t mouseX, int32_t mouseY) {

	this->raycaster.keyDown = keyPress;
	this->raycaster.ComputeSpace(gc, keylog, logIndex, mouseX);
}

void Shooter::ComputeShooter(char keylog[16], uint8_t logIndex, bool keyPress, 
				int32_t mouseX, int32_t mouseY) {
}

bool Shooter::LoadData() {

	this->raycaster.posX = 22.0;
	this->raycaster.posY = 11.5;
	this->raycaster.dirX = -1.0;
	this->raycaster.dirY = 0.0;
	this->raycaster.planeX = 0.0;
	this->raycaster.planeY = 0.66;
	this->raycaster.keyDown = false;
	
	return true;
}
