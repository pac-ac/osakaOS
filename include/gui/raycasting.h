#ifndef __OS__GUI__RAYCASTING_H
#define __OS__GUI__RAYCASTING_H


#include <common/types.h>
#include <drivers/vga.h>
#include <gui/widget.h>
#include <string.h>
#include <math.h>


#define TEXTURE_WALL 0
#define TEXTURE_FLOOR 2
#define TEXTURE_CARPET 1
#define TEXTURE_CEILING 3


#define NUM_SPRITES 0


namespace os {

	namespace gui {
		
		struct Sprite {
		
			double x;
			double y;
			int texture;

			int uDiv;
			int vDiv;
			double vMove;
		};

		class RaycastSpace {
	
			public:
				const double w = 320.0;
				const double h = 200.0;
				int texW = 64;
				int texH = 64;
			
				//common::uint8_t textures[8][texW*texH];
				common::uint8_t textures[8][64*64];


				double ZBuffer[320];
				common::int32_t spriteOrder[NUM_SPRITES];
				double spriteDistance[NUM_SPRITES];
				Sprite sprite[NUM_SPRITES];

				/*
				Sprite sprite[NUM_SPRITES] = {
				
					{20.5, 11.5, 8},
					{18.5, 4.5,  8},
					{10.0, 4.5,  8},
					{10.0, 12.5, 8},
					{3.5,  6.5,  8},
					{3.5, 20.5,  8},
					{3.5, 14.5,  8},
					{14.5, 20.5, 8},
					
					{18.5, 10.5, 7},
					{18.5, 11.5, 7},
					{18.5, 12.5, 7},
					
					{21.5, 1.5, 6},
					{15.5, 1.5, 6},
					{16.2, 1.2, 6},
					{3.5,  2.5, 6},
					{9.5,  15.5,6},
					{10.0, 15.1,6},
					{10.5, 15.8,6}
				};
				*/

				common::uint8_t worldMap[24][24];
				common::uint16_t spaceW = 24;
				common::uint16_t spaceH = 24;
		
				double posX = 22.0;
				double posY = 11.5;
				double dirX = -1.0;
				double dirY = 0.0;
				double planeX = 0.0;
				double planeY = 0.66;


				bool keyDown = false;
				bool mouseDown = false;
			public:
				RaycastSpace();
				~RaycastSpace();

				void ComputeSpace(common::GraphicsContext* gc, char keylog[16], common::uint8_t logIndex, 
						common::int32_t mouseX);

				void SortSprites(common::int32_t* order, double* dist, common::int32_t amount);
		};

		//int worldMap[mapWidth][mapHeight]=
		static int spaceMap[24][24] = {
		  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		  {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,1,1,0,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1},
		  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
		  {1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1},
		  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
		  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,1},
		  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,1},
		  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
		};
	}
}

#endif
