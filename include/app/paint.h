#ifndef __OS__APP__PAINT_H
#define __OS__APP__PAINT_H

#include <common/types.h>
#include <gui/window.h>
#include <gui/sim.h>
#include <filesys/ofs.h>



#define DRAW_OPTION_DEFAULT 0
#define DRAW_OPTION_CIRCLE 1

#define DRAW_OPTION_BRUSH 2
#define DRAW_OPTION_BUCKET 3

#define DRAW_OPTION_TEXT 4
#define DRAW_OPTION_TEXT_STYLE 5



namespace os {

	class KasugaPaint : public App {

		public:
		common::uint8_t* backup;
		bool initBuffer = false;
		//common::uint8_t backup[64000];

		bool drawing = false;
		common::uint8_t paintColor = 0x10;
		common::uint8_t replaceColor = 0xff;
		common::uint8_t colorOffset = 0;
		common::uint8_t size = 0;
		common::uint8_t zoomSize = 1;
	
		common::uint16_t width = WIDTH_13H;
		common::uint16_t height = HEIGHT_13H;

		common::uint16_t menuX = 0;
		common::uint16_t menuY = 0;
		common::uint8_t menuTarget = 0;

		char textDrawStr[64];
		common::uint16_t textDrawIndex = 0;
		common::uint16_t textDrawX = 0;
		common::uint16_t textDrawY = 0;
		common::uint8_t textDrawFlags = TEXT_BASIC;

		bool fileMenu = false;
		bool searchFile = false;
		bool paintMenu = false;
		bool compress = false;

		char fileName[33];
		common::uint8_t drawOption = 0;
		common::uint8_t menuOption = 0;

		public:
			KasugaPaint();
			~KasugaPaint();

			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			virtual void DrawAppMenu(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			
			virtual void SaveOutput(char* file, gui::CompositeWidget* widget, filesystem::FileSystem* filesystem);
			virtual void ReadInput(char* file, gui::CompositeWidget* widget, filesystem::FileSystem* filesystem);	
			
			//perspective
			void Zoom(gui::CompositeWidget* widget, bool increase);
			void Dimensions(bool width, bool increase, gui::CompositeWidget* widget);

			//draw tools
			void DrawSize(bool increase);
			
			void Fill(gui::CompositeWidget* widget);
			
			void Rectangle(common::int32_t x0, common::int32_t y0, 
					common::int32_t x1, common::int32_t y1,
					gui::CompositeWidget* widget);
			
			void Circle(common::int32_t x, common::int32_t y, 
					common::int32_t r, gui::CompositeWidget* widget);

			void LightBrush(common::int32_t oldx, common::int32_t oldy, 
					common::int32_t newx, common::int32_t newy, gui::CompositeWidget* widget);
			
			void Bucket(common::int32_t x, common::int32_t y, gui::CompositeWidget* widget);

			virtual void OnKeyDown(char ch, gui::CompositeWidget* widget);
			virtual void OnKeyUp(char ch, gui::CompositeWidget* widget);
			
			virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						common::int32_t newx, common::int32_t newy,
						gui::CompositeWidget* widget);
	};
}

#endif
