#ifndef __OS__GUI__GAMES__SHOOTER_H
#define __OS__GUI__GAMES__SHOOTER_H

#include <common/types.h>
#include <common/graphicscontext.h>
#include <drivers/vga.h>
#include <gui/raycasting.h>
#include <gui/pixelart.h>


namespace os {

	namespace gui {
	
		class Shooter {
		
			public:
				RaycastSpace raycaster;
			public:
				Shooter();
				~Shooter();

				void Draw(common::GraphicsContext* gc, common::uint16_t ticks, 
						char keylog[16], common::uint8_t logIndex, bool keyPress,
						common::int32_t mouseX, common::int32_t mouseY);
				
				void ComputeShooter(char keylog[16], common::uint8_t logIndex, bool keyPress,
							common::int32_t mouseX, common::int32_t mouseY);
				bool LoadData();
		};
	};
}

#endif
