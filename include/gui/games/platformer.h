#ifndef __OS__GUI__GAMES__PLATFORMER_H
#define __OS__GUI__GAMES__PLATFORMER_H

#include <common/types.h>
#include <common/graphicscontext.h>
#include <drivers/vga.h>
#include <gui/pixelart.h>


namespace os {
	
	namespace gui {

		struct PlatformerData {

			common::uint16_t levelLength;
			common::uint16_t x;
			common::uint8_t y;
			common::uint8_t yJump;

			bool scroll;
			bool run;
			bool jump;
			bool falling;
			bool facingLeft;

		} __attribute__((packed));


		class Platformer {

			public:
				///game data
				PlatformerData data;
				common::uint16_t playerX = 0;
				common::uint16_t playerY = 0;
			public:
				Platformer();
				~Platformer();

				void Draw(common::GraphicsContext* gc, common::uint16_t ticks);
				void ComputePlatformer(char keylog[16], common::uint8_t logIndex, bool keyPress);

				bool LoadData();
		};
	}
}

#endif
