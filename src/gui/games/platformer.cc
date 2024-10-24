#include <gui/games/platformer.h>

using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::drivers;


Platformer::Platformer() {

	this->LoadData();
}

Platformer::~Platformer() {
}


void Platformer::Draw(GraphicsContext* gc, uint16_t ticks) {

	gc->FillRectangle(0, 0, 320, 200, 0x0b);
	gc->FillRectangle(0, 160, 320, 200, 0x10);

	if (data.run) {
	
		switch ((ticks/5)%2) {
		
			case 1:
				gc->FillBuffer(playerX, playerY, 13, 20, osakaPlatSprite1, data.facingLeft);
				break;
			default:
				gc->FillBuffer(playerX, playerY, 13, 20, osakaPlatSprite2, data.facingLeft);
				break;
		}
	} else { gc->FillBuffer(playerX, playerY, 13, 20, osakaPlatSprite1, data.facingLeft); }
}



void Platformer::ComputePlatformer(char keylog[16], uint8_t logIndex, bool keyPress) {

	if (keyPress) {

		for (uint8_t i = 0; i < logIndex; i++) {
		
			switch (keylog[i]) {
			
				case 'd':
					if (data.x + 20 < data.levelLength) {
					
						data.x += 2;
						data.scroll = (data.levelLength - data.x < 320);
						playerX += (2 * data.scroll);
					}
					data.facingLeft = false;
					data.run = true;
					break;
				case 'a':
					if (data.x > 0) {
					
						data.x -= 2;
						data.scroll = data.x < 320;
						playerX -= (2 * data.scroll);
					}
					data.facingLeft = true;
					data.run = true;
					break;
				case ' ':
					if (data.jump == false) {
					
						data.yJump = data.y;
						data.jump = true;
					}
					break;
				default: break;
			}
		}
	} else { data.run = false; }

	//jumping
	if (data.yJump > 0) {
	
		data.y -= 4;
		playerY -= 4;
		if (data.y + 60 < data.yJump) { data.yJump = 0; }
	} else {
		//falling
		if (data.y < 140) {
		
			data.y += 4;
			playerY += 4;
		} else {
			data.jump = false;
		}
	}
}

bool Platformer::LoadData() {

	//default
	playerX = 160;
	playerY = 140;
	data.x = 160;
	data.y = 140;
	data.yJump = 0;

	data.levelLength = 320;
	data.scroll = false;
	data.jump = false;
	data.falling = false;
	data.facingLeft = false;

	return true;
}
