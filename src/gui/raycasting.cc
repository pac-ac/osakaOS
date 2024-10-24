#include <gui/raycasting.h>

using namespace os;
using namespace os::gui;
using namespace os::math;
using namespace os::common;
using namespace os::drivers;


double getTicks();
uint8_t Web2EGA(uint32_t color);


RaycastSpace::RaycastSpace() {

	for (int h = 0; h < spaceH; h++) {
		for (int w = 0; w < spaceW; w++) {
		
			this->worldMap[w][h] = spaceMap[w][h];
		}
	}


	//generate textures
	for (int i = 0; i < 8; i++) {
		for (int x = 0; x < texW; x++) {
			for (int y = 0; y < texH; y++) {
		
				uint16_t texCoord = texW*y+x;

				if (i == 2) {
				
					switch (texCoord % 8) {
					
						case 0:textures[i][texCoord] = 0x31;break;
						default:textures[i][texCoord] = 0x0f;break;
					}

				} else if (i == 3) {
					
					switch (texCoord % 8) {
					
						case 0:textures[i][texCoord] = 0x38;break;
						default:textures[i][texCoord] = 0x07;break;
					}
				} else {
					switch (y) {

						case 0:case 1:case 56:case 57:case 62:case 63:
							textures[i][texCoord] = 0x39;break;
						
						case 3:textures[i][texCoord] = 0x0f;break;
						
						case 58:case 59:case 60:case 61:
							textures[i][texCoord] = 0x03;break;
						
						default:textures[i][texCoord] = 0x1f;break;
					}
				}
			}
		}
	}
}

RaycastSpace::~RaycastSpace() {
}



void RaycastSpace::ComputeSpace(GraphicsContext* gc, char keylog[16], uint8_t logIndex, int32_t mouseX) {

	
	double time = 0;
	double oldTime = 0;

	const uint16_t w = 320;
	const uint8_t h = 200;

	gc->FillRectangle(0, 0, 320, 200, 0x40);


	//floor casting
	for (int y = 0; y < h; y++) {
	
		float rayDirX0 = dirX - planeX;
		float rayDirY0 = dirY - planeY;
		float rayDirX1 = dirX + planeX;
		float rayDirY1 = dirY + planeY;

		//horizon
		int p = y - h / 2;

		//vertical pos of cam
		float posZ = 0.5 * h;

		//horizontal distance from cam to floor
		float rowDistance = posZ / p;

		//real world step vector to add for each x
		float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / w;
		float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / w;

		//coords of leftmost column
		float floorX = posX + rowDistance * rayDirX0;
		float floorY = posY + rowDistance * rayDirY0;

		for (int x = 0; x < w; ++x) {
		
			int cellX = (int)(floorX);
			int cellY = (int)(floorY);

			//tex coords from fractional part
			int tx = (int)(texW * (floorX - cellX)) & (texW - 1);
			int ty = (int)(texH * (floorY - cellY)) & (texH - 1);

			floorX += floorStepX;
			floorY += floorStepY;

			//choose textures and color
			int floorTexture = 2;
			int ceilingTexture = 3;
			uint8_t color;

			//draw floor
			color = textures[floorTexture][texW*ty+tx];
			gc->PutPixel(x, y, color);

			//draw ceiling
			color = textures[ceilingTexture][texW*ty+tx];
			gc->PutPixel(x, h-y-1, color);
		}
	}


	//draw frame and walls
	for (double x = 0; x < w; x++) {
	
		//calculate ray position and direction
		double cameraX = 2.0 * x / w - 1.0;
		double rayDirX = dirX + planeX * cameraX;
		double rayDirY = dirY + planeY * cameraX;
	
		//box of map player is in
		int mapX = int(posX);
		int mapY = int(posY);

		//length of ray from current position to next x or y side
		double sideDistX = 0;
		double sideDistY = 0;
		

		double deltaDistX = (rayDirX == 0) ? 1e30 : absD(1.0/rayDirX);
		double deltaDistY = (rayDirY == 0) ? 1e30 : absD(1.0/rayDirY);
		double perpWallDist = 0;

		int stepX, stepY = 0;

		int hit = 0; //wall hit
		int side = 0; //ns or ew wall hit

		//calculate step and initial sideDist
		if (rayDirX < 0) {
		
			stepX = -1;
			sideDistX = (posX - mapX) * deltaDistX;
		} else {
			stepX = 1;
			sideDistX = (mapX + 1.0 - posX) * deltaDistX;
		}

		if (rayDirY < 0) {

			stepY = -1;
			sideDistY = (posY - mapY) * deltaDistY;
		} else {
			stepY = 1;
			sideDistY = (mapY + 1.0 - posY) * deltaDistY;
		}

		//perform DDA
		while (hit == 0) {

			//jump to next map square
			if (sideDistX < sideDistY) {

				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			} else {
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}
			//check if ray has hit a wall
			if (worldMap[mapX][mapY] > 0) { hit = 1; }
		}

		//calculate distance projected on camera direction
		//otherwise fisheye effect
		if (side == 0) { perpWallDist = (sideDistX - deltaDistX); }
		else           { perpWallDist = (sideDistY - deltaDistY); }


		//calculate height of line to draw on screen
		int lineHeight = (int)(h / perpWallDist);

		//height of camera
		int pitch = 0;

		//calculate lowest and highest pixel to fill current stripe
		int drawStart = -lineHeight / 2 + h / 2 + pitch;
		if (drawStart < 0) { drawStart = 0; }
		int drawEnd = lineHeight / 2 + h / 2 + pitch;
		if (drawEnd >= h) { drawEnd = h - 1; }


		//texture calculations
		int texNum = worldMap[mapX][mapY] - 1;

		//calculate value of wallx
		double wallX; //where wall was hit
		if (side == 0) { wallX = posY + perpWallDist * rayDirY;
		} else {         wallX = posX + perpWallDist * rayDirX; }
		wallX -= (double)((int)wallX);

		//x coordinate on texture
		int texX = int(wallX * double(texW));
		if (side == 0 && rayDirX > 0) { texX = texW - texX - 1; }
		if (side == 1 && rayDirX < 0) { texX = texW - texX - 1; }
		double step = 1.0 * texH / lineHeight;
		

		//draw walls and their textures.
		//starting texture coordinate
		double texPos = (drawStart - h / 2 + lineHeight / 2) * step;
		for (int y = drawStart; y < drawEnd; y++) {
		
			int texY = (int)texPos & (texH - 1);
			texPos += step;
			uint8_t color = textures[texNum][texH*texY+texX];
			
			//make color darker for y-sides
			if (side == 1) { color = light2dark[color]; }
			gc->PutPixel(x, y, color);
		}
	}
	oldTime = time;
	time = getTicks() / 40.0;
	double frameTime = (time - oldTime) / 1000.0;

	//speed modifiers
	double moveSpeed = frameTime * 3.0;
	double rotSpeed = frameTime * (double)(mouseX/4);



	//input
	if (this->keyDown) {
	
		for (int i = 0; i < logIndex; i++) {
	
			switch (keylog[i]) {
	
				//walking
				case 'w':
					if (worldMap[int(posX+dirX*moveSpeed)][int(posY)] == false) { posX += dirX * moveSpeed; }
					if (worldMap[int(posX)][int(posY+dirY*moveSpeed)] == false) { posY += dirY * moveSpeed; }
					break;
				case 'a':
					if (worldMap[int(posX-planeX*moveSpeed)][int(posY)] == false) { posX -= planeX * moveSpeed; }
					if (worldMap[int(posX)][int(posY-planeY*moveSpeed)] == false) { posY -= planeY * moveSpeed; }
					break;
				case 's':
					if (worldMap[int(posX-dirX*moveSpeed)][int(posY)] == false) { posX -= dirX * moveSpeed; }
					if (worldMap[int(posX)][int(posY-dirY*moveSpeed)] == false) { posY -= dirY * moveSpeed; }
					break;
				case 'd':
					if (worldMap[int(posX+planeX*moveSpeed)][int(posY)] == false) { posX += planeX * moveSpeed; }
					if (worldMap[int(posX)][int(posY+planeY*moveSpeed)] == false) { posY += planeY * moveSpeed; }
					break;
			}
		}
	}

	//rotate right
	if (mouseX < w/2) {
		
		double oldDirX = dirX;
		dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
		dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
		double oldPlaneX = planeX;
		planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
		planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
	}
	
	//rotate left
	if (mouseX > w/2) {
		
		double oldDirX = dirX;
		dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
		dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
		double oldPlaneX = planeX;
		planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
		planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
	}
}
