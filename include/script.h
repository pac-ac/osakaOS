#ifndef __OS__SCRIPT_H
#define __OS__SCRIPT_H

#include <common/types.h>
#include <filesys/ofs.h>
#include <gui/widget.h>
#include <functiontypes.h>
#include <string.h>
#include <math.h>
#include <list.h>
#include <tree.h>
#include <cli.h>
#include <app.h>


#define OPERATOR_ADD 1
#define OPERATOR_SUB 2
#define OPERATOR_MUL 3
#define OPERATOR_DIV 4
#define OPERATOR_MOD 5

#define OPERATOR_EQUAL 6
#define OPERATOR_OR 7
#define OPERATOR_AND 8
#define OPERATOR_LESS 9
#define OPERATOR_LESS_EQUAL 10
#define OPERATOR_MORE 11
#define OPERATOR_MORE_EQUAL 12


namespace os {

	class Script : public App {
	
		public:
			bool init = false;
			CommandLine* parentCli;
		public:
			
			Script(CommandLine* cli);
			~Script();
	
			virtual void ComputeAppState(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			virtual void DrawAppMenu(common::GraphicsContext* gc, gui::CompositeWidget* widget);
			
			virtual void Close();
			
			virtual void SaveOutput(char* fileName, gui::CompositeWidget* widget);
			virtual void ReadInput(char* fileName, gui::CompositeWidget* widget);
			
			virtual void OnKeyDown(char ch, gui::CompositeWidget* widget);
			virtual void OnKeyUp(char ch, gui::CompositeWidget* widget);
			
			virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button, gui::CompositeWidget* widget);
			virtual void OnMouseMove(common::int32_t oldx, common::int32_t oldy, 
						 common::int32_t newx, common::int32_t newy, 
						 gui::CompositeWidget* widget);
	};
}


os::CallStackUnit* createCall(os::FunctionType* func, os::CommandLine* cli, os::common::uint32_t returnIndex);
void function(char* name, os::CommandLine* cli);
void setReturn(char* name, os::CommandLine* cli);
void setFunctionArgs(char* args, os::CommandLine* cli, os::FunctionType* func);

os::common::int32_t getVarNum(char* args, os::CommandLine* cli, os::common::uint8_t argNum);
os::FunctionType* getVarFunction(char* args, os::CommandLine* cli, os::common::uint8_t argNum);

os::common::uint8_t determineType(char* args, os::CommandLine* cli);

os::Type* evalVar(char* args, os::CommandLine* cli, os::common::uint8_t argNum);
os::common::int32_t numOrVar(char* args, os::CommandLine* cli, os::common::uint8_t argNum);
float floatOrVar(char* args, os::CommandLine* cli, os::common::uint8_t argNum);
char* strOrVar(char* args, os::CommandLine* cli, os::common::int8_t argNum);


void var(char* name, os::CommandLine* cli);
void rvar(char* name, os::CommandLine* cli);
void wvar(char* name, os::CommandLine* cli);
void dvar(char* name, os::CommandLine* cli);

void varList(char* name, os::CommandLine* cli);
void printList(char* name, os::CommandLine* cli);
void insertList(char* name, os::CommandLine* cli);
void removeList(char* name, os::CommandLine* cli);
void readList(char* name, os::CommandLine* cli);
void writeList(char* name, os::CommandLine* cli);
void destroyList(char* name, os::CommandLine* cli);

void comment(char* name, os::CommandLine* cli);

void If(char* name, os::CommandLine* cli);
void Else(char* name, os::CommandLine* cli);
void Fi(char* name, os::CommandLine* cli);

void loop(char* name, os::CommandLine* cli);
void pool(char* name, os::CommandLine* cli);

void evalMath(char* name, os::CommandLine* cli);

void inputStr(char* name, os::CommandLine* cli);
void ex(char* name, os::CommandLine* cli);
void ext(char* name, os::CommandLine* cli);
void exb(char* name, os::CommandLine* cli);

void CleanUpScript(os::CommandLine* cli);

void UserScript();
void AyumuScriptCli(char* name, os::CommandLine* cli, os::common::uint8_t* scriptBuf = nullptr, os::common::uint32_t scriptSize = 0);
void AyumuScriptFindDegree(os::common::uint8_t* file, os::common::uint32_t size, os::List* degreePoints);



#endif
