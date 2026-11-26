// Text listing of the loaded textures (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include "texture_mgr.h"
#include "render.h"

// File the developers' texture list is written to.
const char TEXTURE_DUMP_PATH[] = "d:\texdump.txt";

// Textures_DumpList: writes one line per texture slot to TEXTURE_DUMP_PATH: the slot number, then (for a named slot) both
// names, the source size and the buffer pool index. Does nothing when the file cannot be opened. Returns 0. No direct
// caller.
extern "C" int Textures_DumpList(void)
{
    FILE *file = fopen(TEXTURE_DUMP_PATH, "wt");
    if (file != 0)
    {
        for (int slot = 0; slot < TEXTURE_SLOT_COUNT; slot++)
        {
            fprintf(file, "%04d:", slot);
            if (g_textureNames[slot][0] != 0)
            {
                fprintf(file, "%s", g_textureNames[slot]);
                if (g_textureNamesAux[slot][0] != 0)
                    fprintf(file, "%s", g_textureNamesAux[slot]);
                fprintf(file, " %dx%d  ID=%d", g_textures[slot].sourceWidth, g_textures[slot].sourceHeight,
                        g_textures[slot].poolIndex);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }
    return 0;
}
