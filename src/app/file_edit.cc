#include <app/file_edit.h>


using namespace os;
using namespace os::gui;
using namespace os::common;
using namespace os::filesystem;


uint8_t* memset(uint8_t*, int, size_t);
uint16_t hash(char* str);

Journal::Journal(MemoryManager* mm) {

	this->appType = APP_TYPE_JOURNAL;
	this->mm = mm;

	this->degreePoints = (List*)this->mm->malloc(sizeof(List));

	//init block
	memset(this->LBA, 0x00, OFS_BLOCK_SIZE);
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


void Journal::DrawAppMenu(GraphicsContext* gc, CompositeWidget* widget) {

	gc->DrawRectangle(widget->x+widget->w-51, widget->y+24, 48, 17, W202041);
	gc->DrawRectangle(widget->x+widget->w-50, widget->y+25, 46, 15, W394571);
	gc->FillRectangle(widget->x+widget->w-49, widget->y+26, 44, 13, W515171);
	gc->PutText("Analyze", widget->x+widget->w-48, widget->y+31, W000000);
	gc->PutText("Analyze", widget->x+widget->w-49, widget->y+30, WFFFFFF);
	
	if (this->degreePoints->numOfNodes > 0) {
	
		gc->MakeDark(2, widget->x+5, widget->y+24, widget->w-55, widget->h-29);
		uint16_t pixelsBetweenPoints = ((widget->w-50) / this->degreePoints->numOfNodes);
		uint32_t lastNum = *((uint32_t*)(this->degreePoints->Read(0)));

		if (pixelsBetweenPoints > 0) {
		
			for (int i = 1; i < this->degreePoints->numOfNodes; i++) {
		
				uint32_t num = *((uint32_t*)(this->degreePoints->Read(i)));
				gc->DrawLine(widget->x+((i-1)*pixelsBetweenPoints)+5, widget->y+widget->h-(5+lastNum), 
					     widget->x+(i*pixelsBetweenPoints)+5,     widget->y+widget->h-(5+num), W00FF41);
				lastNum = num;
			}
		}
		
		gc->PutText("CMD CNT: ", widget->x+8, widget->y+30, W000000);
		gc->PutText("CMD CNT: ", widget->x+7, widget->y+29, WFFFFFF);
		gc->PutText(int2str(this->degreePoints->numOfNodes), widget->x+61, widget->y+30, W000000);
		gc->PutText(int2str(this->degreePoints->numOfNodes), widget->x+60, widget->y+29, WFFFFFF);
	}
}


void Journal::DrawTheme(CompositeWidget* widget) {

	//blue lines
	for (uint16_t y = FONT_HEIGHT-1; y < widget->gc->gfxHeight; y += FONT_HEIGHT) {
		
		widget->DrawLine(widget->x+0, widget->y+y, widget->x+widget->gc->gfxWidth, widget->y+y, W829EFF);
	}
	//red line
	widget->DrawLine(widget->x+0, widget->y+0, widget->x+0, widget->y+widget->gc->gfxHeight, WFF5555);
}


void Journal::SaveOutput(char* fileName, CompositeWidget* widget, FileSystem* filesystem) {

	if (filesystem->FileIf(filesystem->GetFileSector(fileName))) {

		for (int i = 0; i < this->numOfBlocks; i++) {
		
			filesystem->WriteLBA(fileName, this->fileBuffer+(i*OFS_BLOCK_SIZE), i);
		}
	} else {
		filesystem->NewFile(fileName, this->fileBuffer, OFS_BLOCK_SIZE);
		
		for (int i = 1; i < this->numOfBlocks; i++) {
		
			filesystem->WriteLBA(fileName, this->fileBuffer+(i*OFS_BLOCK_SIZE), i);
		}
	}
}


void Journal::ReadInput(char* fileName, CompositeWidget* widget, FileSystem* filesystem) {
	
	//free current memory	
	if (this->fileBuffer != nullptr) {
		
		filesystem->memoryManager->free(this->fileBuffer);
	}

	if (filesystem->FileIf(filesystem->GetFileSector(fileName))) {
	
		uint32_t size = filesystem->GetFileSize(fileName);
		this->numOfBlocks = size/OFS_BLOCK_SIZE;
		
		this->fileBuffer = (uint8_t*)filesystem->memoryManager->malloc(sizeof(uint8_t)*size);
		
		//save data to buffer
		for (int i = 0; i < this->numOfBlocks; i++) {
		
			filesystem->ReadLBA(fileName, this->LBA, i);

			for (int j = 0; j < OFS_BLOCK_SIZE; j++) {
			
				this->fileBuffer[j+(i*OFS_BLOCK_SIZE)] = this->LBA[j];
			}
		}
	} else {
		this->fileBuffer = (uint8_t*)filesystem->memoryManager->malloc(sizeof(uint8_t)*OFS_BLOCK_SIZE);
		memset(this->fileBuffer, 0x00, OFS_BLOCK_SIZE);

		//write little error message
		const char* error = "file not found";
		for (uint8_t i = 0; error[i] != '\0'; i++) { this->fileBuffer[i] = error[i]; }
	}
	widget->Print("\v");
	this->DrawTheme(widget);
	widget->textColor = W000000;
	
	for (index = 0; this->fileBuffer[index] != 0x00 && index < this->numOfBlocks*OFS_BLOCK_SIZE; index++) {
		
		widget->PutChar((char)(this->fileBuffer[index]));
	}
	widget->PutChar('\v');
	widget->textColor = WFFFFFF;
}



void Journal::OnKeyDown(char ch, CompositeWidget* widget) {
	
	widget->Print("\v");
	widget->textColor = W000000;

	//increase buffer size
	if (index >= this->numOfBlocks*OFS_BLOCK_SIZE) {

		//increase size by 1 block and switch pointers
		
		uint8_t* oldBuf = this->fileBuffer;
		this->fileBuffer = (uint8_t*)this->mm->malloc((this->numOfBlocks+1)*OFS_BLOCK_SIZE);

		for (int i = 0; i < this->numOfBlocks*OFS_BLOCK_SIZE; i++) {
		
			this->fileBuffer[i] = oldBuf[i];
		}
		
		for (int i = 0; i < OFS_BLOCK_SIZE; i++) {
		
			this->fileBuffer[(this->numOfBlocks*OFS_BLOCK_SIZE)+i] = 0x00;
		}
		
		
		this->numOfBlocks++;
		
		if (oldBuf != nullptr) {
		
			this->mm->free(oldBuf);
		}
		//return;
	}


	int i = 0;

	//key input
	switch (ch) {
	
		case '\b':
			if (index > 0 && cursor > 0) {
			
				if (cursor < index) {

					for (i = cursor-1; i < index; i++) {
				
						fileBuffer[i] = fileBuffer[i+1];
					}
				}
				index--;
				cursor--;
				fileBuffer[index] = 0x00;
			}
			break;
		//left
		case '\xfc':
			cursor -= 1 * (cursor > 0);
			break;
		//up
		case '\xfd':
			while(cursor > 0 && fileBuffer[cursor] != '\n') { cursor--; }
			cursor -= 1 * (cursor > 0);
			break;
		//down
		case '\xfe':
			while(cursor < index && fileBuffer[cursor] != '\n') { cursor++; }
			cursor += 1 * (cursor < index);
			break;
		//right
		case '\xff':
			cursor += 1 * (cursor < index);
			break;
		default:
			if (cursor < index) {

				fileBuffer[index+1] = fileBuffer[index];
				
				for (i = index; i > cursor; i--) {
				
					fileBuffer[i] = fileBuffer[i-1];
				}
			}
			fileBuffer[cursor] = (uint8_t)(ch);
			index++;
			cursor++;
			break;
	}


	//determine where text ends
	uint16_t prevNewLine = 0;
	uint16_t j = 0;
	bool cursorPassed = false;

	for (j = 0; j < index; j++) {
	
		if (fileBuffer[j] == '\n') {
		
			prevNewLine = j;
			if (cursorPassed) { break; }
		}
		cursorPassed = (j >= cursor);
	}


	//limit how much text gets printed
	//(smoother scrolling)
	uint16_t printLimit = index;
	

	char word[256];
	uint16_t wordIndex = 0;
	bool quotes = false;
	memset((uint8_t*)word, '\0', 256);

	//print text to screen	
	for (i = 0; i < printLimit; i++) {
	
		if (i == cursor) {
	
			widget->PutChar('\v');
			if (fileBuffer[i] == '\n') { widget->PutChar('\n'); }
		} else {
			if (fileBuffer[i] == '\v') {
			
				widget->PutChar(' ');
				widget->PutChar(' ');
				widget->PutChar(' ');
				widget->PutChar(' ');
			} else {
				if (wordIndex < 256) {

					if ((char)fileBuffer[i] == '\"') {
					
						quotes ^= 1;
					}
					word[wordIndex] = (char)fileBuffer[i];
					wordIndex++;
				}

				if ((((char)fileBuffer[i] == ' ' && !quotes) || (char)fileBuffer[i] == '\n') && wordIndex > 0) {

					word[wordIndex-1] = '\0';
						
					for (int j = 0; j < strlen(word); j++) { widget->PutChar('\b'); }
						
					if (CommandLine::cmdTable[hash(word)] != nullptr) { 	   widget->textColor = W0000FF;
												   widget->Print(word, TEXT_BOLD);
					
					} else if (word[0] == '\"' && word[wordIndex-2] == '\"') { widget->textColor = WFF00BE;
												   widget->Print(word, TEXT_ITALIC);
					
					} else if (word[0] >= '0' && word[0] <= '9') {		   widget->textColor = W00FF00;
												   widget->Print(word, TEXT_BOLD);
					
					} else if (word[0] == '$') {				   widget->textColor = W00FFFF;
												   widget->Print(word);
					
					} else {						   widget->Print(word);
					}
					widget->textColor = W000000;

					quotes = false;
					wordIndex = 0;
					memset((uint8_t*)word, '\0', 256);
				}

				widget->PutChar((char)(fileBuffer[i]));
			}
		}

		//dont scroll screen if cursor aint there
		if (widget->textScroll) { printLimit = j; }
	}
	if (cursor == index) { widget->PutChar('\v'); }

	this->DrawTheme(widget);
	widget->textColor = WFFFFFF;
}

void Journal::OnKeyUp(char ch, CompositeWidget* widget) {
}



void Journal::OnMouseDown(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
	
	//gc->DrawRectangle(widget->x+widget->w-51, widget->y+24, 48, 17, W202041);

	if (x <= widget->x+widget->w-2 && x >= widget->x+widget->w-51 
		&& y >= widget->y+24 && y <= widget->y+41) 
	{
		this->degreePoints->DestroyList();
		AyumuScriptFindDegree(this->fileBuffer, this->index, this->degreePoints);
	} else {
		widget->Dragging = true;
	}
}
void Journal::OnMouseUp(int32_t x, int32_t y, uint8_t button, CompositeWidget* widget) {
}
void Journal::OnMouseMove(int32_t oldx, int32_t oldy, 
			int32_t newx, int32_t newy, 
			CompositeWidget* widget) {
}
