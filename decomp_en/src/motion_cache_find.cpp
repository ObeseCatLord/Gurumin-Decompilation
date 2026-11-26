// Motion cache lookup (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include <string.h>
#include "mesh_cache.h"

// Bytes of the work buffer the two names are joined in.
const int FIND_WORK_SIZE = 0x100;

// MotionCache_Find: looks up file + nodeName (cut to FILE_NAME_SIZE - 1 characters) in the shared name table. Returns
// its index when it is there. Otherwise the name is stored in the first free entry (its reference count reset), that
// index goes to *freeSlot and -1 is returned. A full table is fatal: the original calls fread with null arguments on
// purpose to crash. Called by the node animation loader before it shares a motion.
extern "C" int MotionCache_Find(const char *file, const char *nodeName, int *freeSlot)
{
    char joined[FIND_WORK_SIZE];
    strcpy(joined, file);
    strcat(joined, nodeName);
    joined[FILE_NAME_SIZE - 1] = 0;
    *freeSlot = -1;
    for (int i = 0; i < TEXTURE_FILE_COUNT; i++)
    {
        if (g_textureFileNames[i][0] != 0)
        {
            if (strcmp(joined, g_textureFileNames[i]) == 0)
                return i;
        }
        else if (*freeSlot == -1)
        {
            g_textureFileRefCount[i] = 0;
            *freeSlot = i;
        }
    }
    if (*freeSlot == -1)
        fread(0, 1, 1, 0);
    strcpy(g_textureFileNames[*freeSlot], joined);
    return -1;
}
