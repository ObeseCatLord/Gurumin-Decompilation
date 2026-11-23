// Movies, packed script variables and the seasonal check.

#include "mmv_player.h"
#include <windows.h>

#include "texture_object.h"

extern "C" const char *FUN_1013f8a0(const char *format, ...);
#include "game_vars.h"
#include "ui_vars.h"

// Movie_PlayToTexture: opens movie `name` (folder 0) in the movie player DAT_10f08ab0, decodes the first frame
// into the texture slot DAT_10202478 and records the movie's frame count. Returns 1.
extern "C" int FUN_10016580(const char *name)
{
    DAT_10f08ab0.Open(FUN_1013f8a0(name, 0));
    DAT_10f08ab0.M_10161050();
    DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
    DAT_104bfe74 = DAT_10f08ab0.size;
    DAT_104bfe70 = 1;
    return 1;
}

// Script_UnpackVar: when script global `index` holds a packed pair (low nibble = 1), returns 1 and unpacks
// bits 5..12 into *low (0..0x80) and bits 13..20 into *high (0..0xff); returns 0 otherwise.
extern "C" int FUN_10017b10(int index, int *high, int *low)
{
    int value = DAT_10c62edc[index];
    if ((value & 0xf) == 1)
    {
        *low = (value >> 5) & 0xff;
        *high = (DAT_10c62edc[index] >> 13) & 0xff;
        if (*low < 0 || *low > 0x80)
            *low = 0;
        if (*high < 0 || *high > 0xff)
            *high = 0;
        return 1;
    }
    return 0;
}

// 0x1001d850: reads the PC clock. Returns 1 on Christmas (Dec 25, or Dec 24 after 18:00), 2 on January 1-3,
// else 0. Seasonal content (and costume 7, see MODDING_MAP) depends on it; hook it to force an event.
int SeasonalEvent()
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    if (now.wMonth == 12)
    {
        if (now.wDay == 25 || (now.wDay == 24 && now.wHour > 18))
            return 1;
        return 0;
    }
    if (now.wMonth == 1 && now.wDay < 4)
        return 2;
    return 0;
}

