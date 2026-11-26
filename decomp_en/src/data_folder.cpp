// Setting the data folder (EN build, EN-only: no JP twin found).
#include <string.h>
#include "engine.h"

// The path separator a folder name may end with.
#define FOLDER_SEPARATOR '\\'

// DataFolder_Set: copies `folder` into g_dataFolder and drops one trailing backslash so later paths can add their own.
// Called by the start-up code at EN 0x609f70.
extern "C" void DataFolder_Set(const char *folder)
{
    strcpy(g_dataFolder, folder);
    size_t length = strlen(g_dataFolder);
    if (g_dataFolder[length - 1] == FOLDER_SEPARATOR)
        g_dataFolder[length - 1] = '\0';
}
