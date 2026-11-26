// Keeping textures across a map change (EN build, EN-only: no JP twin found).
#include "texture_mgr.h"

// Marks every texture slot that has a name (is loaded) as kept, so the next map change does not unload it. Returns 0.
// Called when the game start-up has loaded its textures (EN 0x5f2cd0).
extern "C" int Textures_KeepAllLoaded(void)
{
    for (int slot = 0; slot < TEXTURE_SLOT_COUNT; slot++)
    {
        if (g_textureNames[slot][0] != 0)
            g_textureKept[slot] = 1;
    }
    return 0;
}
