// Save files (EN build). Ported from the matched JP source decomp/src/save_slot.cpp.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "save_slot.h"

// Size of the path buffers, and the character that ends a folder name.
const int SAVE_PATH_SIZE = 0x100;
const char PATH_SEPARATOR = '\\';

// Save_LoadSlot: loads save file `name` from the SaveData folder next to the program: the program's folder is cut back
// to its last backslash, "SaveData\" and the name are appended and the file goes through Save_Load. Called by the
// load menu.
extern "C" int Save_LoadSlot(const char *name)
{
    char folder[SAVE_PATH_SIZE];
    char path[SAVE_PATH_SIZE];
    GetModuleFileNameA(0, folder, SAVE_PATH_SIZE);
    char *end = folder + strlen(folder) - 1;
    if (*end != PATH_SEPARATOR)
    {
        do
        {
            *end = 0;
            end--;
        } while (*end != PATH_SEPARATOR);
    }
    sprintf(path, "%sSaveData\\%s", folder, name);
    return Save_Load(path, name);
}
