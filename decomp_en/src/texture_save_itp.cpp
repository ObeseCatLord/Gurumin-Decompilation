// Saving the loaded textures as .itp files (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include "texture_mgr.h"

// Bytes of the path buffer of one saved file.
const int ITP_PATH_SIZE = 0x100;

// Textures_SaveAllItp: writes every texture slot that has a name to "<folder>\<name>.itp". Returns 1. No direct caller.
extern "C" int Textures_SaveAllItp(const char *folder)
{
    char path[ITP_PATH_SIZE];
    for (int slot = 0; slot < TEXTURE_SLOT_COUNT; slot++)
    {
        if (g_textureNames[slot][0] != 0)
        {
            sprintf(path, "%s\\%s.itp", folder, g_textureNames[slot]);
            g_textures[slot].SaveItp(path);
        }
    }
    return 1;
}
