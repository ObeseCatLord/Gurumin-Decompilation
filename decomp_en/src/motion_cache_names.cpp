// Names of the shared motion cache entries (EN build, EN-only: no JP twin found).
#include <string.h>
#include "mesh_cache.h"

// Characters of the combined name that are kept (the name table entries are FILE_NAME_SIZE bytes with the terminator).
const int COMBINED_NAME_LAST = FILE_NAME_SIZE - 1;
// Bytes of the work buffer the two names are joined in.
const int NAME_WORK_SIZE = 0x100;

// MotionCache_SetNames: joins the file name and the node name, cuts the result to the table entry size and stores it as
// the name of motion cache entry `slot` (the table shares its storage with the texture file names). Returns 0. Called
// by the node animation loader after it picked the free entry.
extern "C" int MotionCache_SetNames(int slot, const char *file, const char *nodeName)
{
    char combined[NAME_WORK_SIZE];
    strcpy(combined, file);
    strcat(combined, nodeName);
    combined[COMBINED_NAME_LAST] = '\0';
    strcpy(g_textureFileNames[slot], combined);
    return 0;
}
