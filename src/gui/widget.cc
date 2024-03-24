#include <gui/widget.h>


using namespace os::common;
using namespace os::gui;


uint16_t strlen(char* str);
void sleep(uint32_t);


Widget::Widget(Widget* parent,  int32_t x, int32_t y, 
				int32_t w, int32_t h, 
				char* name,
				uint8_t color, 
				bool window) 
: KeyboardEventHandler() {

	this->parent = parent;
	
	this->x = x;
	this->y = y;
	
	this->w = w;
	this->h = h;

	
	this->windowOffset = 0;
	
	//orignial resolution	
	this->wo = w;
	this->ho = h;

	this->name = name;
	this->color = color;
	
	this->Focussable = true;
	this->Buttons = window;
	this->Resizable = window;
}


Widget::~Widget() {
}


void Widget::GetFocus(Widget* widget) {

	if (parent != 0) {
		
		parent->GetFocus(widget);
	}
}                                



void Widget::ModelToScreen(int32_t &x, int32_t &y) {

	if (parent != 0) {
	
		parent->ModelToScreen(x, y);
	}

	x += this->x;
	y += this->y;
}
                                
void Widget::Draw(GraphicsContext* gc) {

	int X = 0;
	int Y = 0;
	this->ModelToScreen(X, Y);
}                                





void Widget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {

	if (Focussable) {
		
		GetFocus(this);
	}
}	

bool Widget::ContainsCoordinate(int32_t x, int32_t y) {

	return this->x <= x && x < this->x + this->w
	    && this->y <= y && y < this->y + this->h;
}



uint8_t Widget::ContainsCoordinateButton(int32_t x, int32_t y) {

	if (this->x <= x && x < this->x + this->w
	    && this->y <= y && y < this->y + 10) {
	
		uint8_t buttonDistance = this->w - (x - this->x);

		//close
		if (buttonDistance <= 10) {
	
			return 1;
		//maximize
		} else if (buttonDistance <= 20) {
		
			return 2;
		//minimize
		} else if (buttonDistance <= 30) {
		
			return 3;
		} else {
		
			return 0;
		}
	}
			
	return 0;
}


bool Widget::AddChild(Widget* child) {
}

void Widget::Print(char* str) {
}
void Widget::PutChar(char ch) {
}

void Widget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
}                                

void Widget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {
}
     

void Widget::OnKeyDown(char str) {
}

void Widget::OnKeyUp(char str) {
}








CompositeWidget::CompositeWidget(Widget* parent,  int32_t x, int32_t y, 
						int32_t w, int32_t h, 
						char* name,
						uint8_t color,
						bool window) 
: Widget(parent, x, y, w, h, name, color, window) {

	focussedChild = 0;
	numChildren = 0;
}

CompositeWidget::~CompositeWidget() {
}

                                
void CompositeWidget::GetFocus(Widget* widget) {

	this->focussedChild = widget;
	int i = windowOffset;

	//find widget index
	for (i; i < numChildren; i++) {
		if (children[i] == focussedChild) { break; }
	}
	
	//push children back by 1
	for (i; i > 0; i--) {
	
		children[i] = children[i-1];
	}
	children[0] = focussedChild;


	if (parent != 0) {
	
		parent->GetFocus(this);
	}
}                                


bool CompositeWidget::AddChild(Widget* child) {

	if (numChildren >= 100) {
		return false;
	}
	children[numChildren++] = child;

	return true;
}


bool CompositeWidget::DeleteChild() {

	if (numChildren <= 0) {
		
		return false;

	} else if (numChildren > 1) {
	
		for (int i = 0; i < numChildren-1; i++) {
		
			children[i] = children[i+1];
		}
	} else {
		children[0] = 0; 
	}

	numChildren--;

	return true;
}


bool CompositeWidget::Maximize() {

	if (	children[0]->w >= 320 && 
		children[0]->h >= 200 &&
		children[0]->x > 0    &&
		children[0]->y > 0) {
	
		return false;
	}
	
	children[0]->w = 320;
	children[0]->h = 200;
	children[0]->x = 0;
	children[0]->y = 0;

	return true;
}

bool CompositeWidget::Minimize() {
	
	children[0]->w = children[0]->wo;
	children[0]->h = children[0]->ho;

	return true;
}

void CompositeWidget::Resize(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {

	children[0]->w += (oldx - newx);
}

                                
void CompositeWidget::Draw(GraphicsContext* gc) {

	Widget::Draw(gc);
	
	for (int i = numChildren - 1; i >= windowOffset; i--) {
		
		children[i]->Draw(gc);
	}
}       



void CompositeWidget::ButtonAction(uint8_t button) {

	switch (button) {
	
		case 1:
			DeleteChild();
			break;
		case 2:
			Maximize();
			break;
		case 3:
			Minimize();
			break;
		default:
			break;
	}
}


void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
		
	for (int i = windowOffset; i < numChildren; i++) {
		
		if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
		

			if (children[i] != focussedChild) {
				
				GetFocus(children[i]);
			}


			//left click
			if (button == 1) {
			
				uint8_t buttonAction = focussedChild->ContainsCoordinateButton(x, y);

				switch (buttonAction) {
			
					//delete window
					case 1:
						DeleteChild();
						break;
					case 2:
						Maximize();
						break;
					case 3:
						Minimize();
						break;
					default:
						break;
				}
			}


			focussedChild->OnMouseDown(x - this->x, y - this->y, button);
			break;
		}
	}
}	

void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {

	for (int i = windowOffset; i < numChildren; i++) {
	//for (int i = 0; i < numChildren; i++) {
			
		children[i]->OnMouseUp(x - this->x, y - this->y, button);
		
		/*
		if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
			
			children[i]->OnMouseUp(x - this->x, y - this->y, button);
			break;
		}
		*/
	}
}                                

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) {


	int firstchild = -1;
	for (int i = windowOffset; i < numChildren; i++) {
		
		if (children[i]->ContainsCoordinate(oldx - this->x, oldy - this->y)) {
			
			children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x, newy - this->y);
			firstchild = i;
			break;
		}
	}

	for (int i = windowOffset; i < numChildren; i++) {
		
		if (children[i]->ContainsCoordinate(newx - this->x, newy - this->y)) {
	
			if (firstchild != i) {	
			
				children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x, newy - this->y);
			}
				
			break;
		}
	}
}
                                
void CompositeWidget::Print(char* str) {
}
void CompositeWidget::PutChar(char ch) {
}



void CompositeWidget::OnKeyDown(char str) {
					
	if (focussedChild != 0 && windowOffset == 0) {
	
		focussedChild->OnKeyDown(str);
	}
}
                                
void CompositeWidget::OnKeyUp(char str) {
	
	if (focussedChild != 0 && windowOffset == 0) {
	
		focussedChild->OnKeyDown(str);
	}
}


