// Naming a data folder (EN build, EN-only: no JP twin found).
#include "engine.h"

// Copies `name` into the table of data folder names as entry `folder`; Path_Data prefixes it to file names. Called by
// the start-up code at EN 0x609f70.
extern "C" void DataFolder_SetFolderName(const char *name, int folder)
{
    const char *source = name;
    char *destination = g_folderNames[folder];
    while ((*destination++ = *source++) != '\0')
    {
    }
}
