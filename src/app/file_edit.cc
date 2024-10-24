#include <app/file_edit.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::filesystem;


Journal::Journal() {

	this->appType = 3;

	//init block
	for (uint16_t i = 0; i < 1920; i++) {
	
		this->LBA[i] = 0x00;
		this->LBA2[i] = 0x00;
	}
}

Journal::~Journal() {
}


void Journal::ComputeAppState(GraphicsContext* gc, CompositeWidget* widget) {

	if (!init) {
	
		this->DrawTheme(widget);
		init = true;
	}
	App::ComputeAppState(gc, widget);
}


void Journal::DrawTheme(CompositeWidget* widget) {

	//blue lines
	for (uint8_t y = font_height-1; y < 200; y += font_height) {
		
		widget->DrawLine(widget->x+0, widget->y+y, widget->x+320, widget->y+y, 0x2b);
	}
	
	//red line
	widget->DrawLine(widget->x+0, widget->y+0, widget->x+0, widget->y+200, 0x3c);
}



void Journal::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {
}




void Journal::SaveOutput(char* fileName, CompositeWidget* widget, FileSystem* filesystem) {

	if (filesystem->FileIf(fnv1a(fileName))) {

		//write to first block for now
		filesystem->WriteLBA(fileName, LBA, 0);
	} else {
		filesystem->NewFile(fileName, LBA, fnv1a(fileName), 1920);
	}
}


void Journal::ReadInput(char* fileName, CompositeWidget* widget, FileSystem* filesystem) {

	if (filesystem->FileIf(fnv1a(fileName))) {
	
		//numOfBlocks = GetSize(fileName) / 1920;
		filesystem->ReadLBA(fileName, LBA, 0);
	} else {
		//fill in with empty zeros
		for (uint16_t i = 0; i < 1920; i++) { LBA[i] = 0x00; }
		
		//write little error message
		const char* error = "file not found";
		for (uint8_t i = 0; error[i] != '\0'; i++) { LBA[i] = error[i]; }
	}
	widget->Print("\v");
	this->DrawTheme(widget);
	widget->textColor = 0x40;
	for (index = 0; LBA[index] != 0x00; index++) {
		
		widget->PutChar((char)(LBA[index]));
	}
	widget->PutChar('\v');
	widget->textColor = 0x3f;
}



void Journal::OnKeyDown(char ch, CompositeWidget* widget) {
	
	widget->Print("\v");
	this->DrawTheme(widget);
	widget->textColor = 0x40;


	//change to next block
	//looks like thats not finished lol
	if (index >= 1920) {
	
		return;
	}


	int i = 0;

	//key input
	switch (ch) {
	
		case '\b':
			if (index > 0 && cursor > 0) {
			
				if (cursor < index) {

					for (i = cursor-1; i < index; i++) {
				
						LBA[i] = LBA[i+1];
					}
				}
				index--;
				cursor--;
				LBA[index] = 0x00;
			}
			break;
		//left
		case '\xfc':
			cursor -= 1 * (cursor > 0);
			break;
		//up
		case '\xfd':
			while(cursor > 0 && LBA[cursor] != '\n') { cursor--; }
			cursor -= 1 * (cursor > 0);
			break;
		//down
		case '\xfe':
			while(cursor < index && LBA[cursor] != '\n') { cursor++; }
			cursor += 1 * (cursor < index);
			break;
		//right
		case '\xff':
			cursor += 1 * (cursor < index);
			break;
		default:
			if (cursor < index) {

				LBA[index+1] = LBA[index];
				
				for (i = index; i > cursor; i--) {
				
					LBA[i] = LBA[i-1];
				}
			}
			LBA[cursor] = (uint8_t)(ch);
			index++;
			cursor++;
			break;
	}


	//determine where text ends
	uint16_t prevNewLine = 0;
	uint16_t j = 0;
	bool cursorPassed = false;

	for (j = 0; j < index; j++) {
	
		if (LBA[j] == '\n') {
		
			prevNewLine = j;
			if (cursorPassed) { break; }
		}
		cursorPassed = (j >= cursor);
	}


	//limit how much text gets printed
	//(smoother scrolling)
	uint16_t printLimit = index;
	

	//print text to screen	
	for (i = 0; i < printLimit; i++) {
	
		if (i == cursor) {
	
			widget->PutChar('\v');
			if (LBA[i] == '\n') { widget->PutChar('\n'); }
		} else {
			if (LBA[i] == '\v') {
			
				widget->PutChar(' ');
				widget->PutChar(' ');
				widget->PutChar(' ');
				widget->PutChar(' ');
			} else {
				widget->PutChar((char)(LBA[i]));
			}
		}

		//dont scroll screen if cursor aint there
		if (widget->textScroll) { printLimit = j; }
	}
	if (cursor == index) { widget->PutChar('\v'); }

	
	widget->textColor = 0x3f;
}

void Journal::OnKeyUp(char ch, CompositeWidget* widget) {
}



void Journal::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {


	widget->Dragging = true;
}
void Journal::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
}
void Journal::OnMouseMove(int32_t oldx, int32_t oldy, 
			int32_t newx, int32_t newy, 
			CompositeWidget* widget) {
}
