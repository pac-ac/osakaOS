#ifndef __OS__MODE__FILE_EDIT_H
#define __OS__MODE__FILE_EDIT_H

#include <common/types.h>
#include <filesys/ofs.h>

void fileTUI();

void file(bool pressed, char key, bool ctrl, bool reset, os::filesystem::FileSystem* file);

void fileMain(bool pressed, char key, bool ctrl, os::filesystem::FileSystem* file);

#endif
