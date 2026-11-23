// Map icons: each entry of the map table at 0x10210fc0 names an icon picture ("maruf11.bmp", ...) in the
// name table at 0x101d65b0.

#include "texture_manager.h"
#include <string.h>

// windows.h-style max: evaluates the larger argument twice.
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#include "map_records.h"
#include "ui_vars.h"

extern "C" int FUN_10161920(const char *name);                    // Texture_FindByName (mmv_texreg.cpp)
extern "C" int FUN_1001e1a0(const char *name, const char *fallback);  // loads a texture, `fallback` if missing

// 0x1001e240 (Map_IconTexture). Returns the texture of the icon of the current map: the icon of the first table
// entry with the current entry's map id, found by its name without the extension or loaded (with "maruf00a.bmp" as
// the fallback; the frame timer is reset afterwards because loading takes long). Returns 0 when no entry matches.
extern "C" int FUN_1001e240(void)
{
    char name[16];
    int i;
    for (i = 0; i < DAT_10216dc4; i++)
    {
        if (DAT_10210fc0[DAT_1020c2c0].entry.id == DAT_10210fc0[i].entry.id)
            break;
    }
    if (i == DAT_10216dc4)
        return 0;
    strcpy(name, DAT_101d65b0[DAT_10210fc0[i].entry.icon]);
    name[MAX(strlen(name) - 4, 0)] = 0;
    int texture = FUN_10161920(name);
    if (texture < 0)
    {
        texture = FUN_1001e1a0(DAT_101d65b0[DAT_10210fc0[i].entry.icon], "maruf00a.bmp");
        DAT_10836c70.ResetFrameTimer();
    }
    return texture;
}
