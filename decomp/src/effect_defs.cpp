// Effect definitions: 0x47e records of 0x3d8 bytes at 0x1101dfdc, one per effect type the scripts can create
// (camera.cpp keeps the matching node pool of the same size).

#include <string.h>

#include "effect_manager.h"
extern "C" int DAT_110186a8[32];
extern "C" char DAT_1101de64;

// Takes the first free effect definition, names it "未設定" ("not set"), gives it its defaults and clears the
// lookup caches (-1). Returns its index, or -1 when all are in use.
extern "C" int FUN_1016f900(void)
{
    int i;
    for (i = 0; i < 0x47e; i++)
    {
        if (DAT_1101dfdc[i].id == -1)
            break;
    }
    if (i == 0x47e)
        return -1;
    DAT_1101dfdc[i].id = -1;
    strcpy(DAT_1101dfdc[i].name, "\x96\xa2\x90\xdd\x92\xe8");
    DAT_1101dfdc[i].m_0154 = 0;
    DAT_1101dfdc[i].m_0164 = 50;
    int j;
    for (j = 0; j < 32; j++)
        DAT_110186a8[j] = -1;
    // The two five-entry lookup caches.
    DAT_1101d820[0] = -1;
    DAT_1101d820[1] = -1;
    DAT_1101d820[2] = -1;
    DAT_1101d820[3] = -1;
    DAT_1101d820[4] = -1;
    DAT_11018604[0] = -1;
    DAT_11018604[1] = -1;
    DAT_11018604[2] = -1;
    DAT_11018604[3] = -1;
    DAT_11018604[4] = -1;
    DAT_1101de64 = 0;
    return i;
}
