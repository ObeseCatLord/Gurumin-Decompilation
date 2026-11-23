// Resetting the per-map state when a map is entered (run by the field and event modes 0x100fc4a0/0x100fda40): the
// map timer and visit counter, the enemy states of the map's character slots, sounds and HUD fields are cleared,
// the item boxes of the treasure hunt are shuffled into script flags 0x93f.. on the first visit, and the seasonal
// flags (Christmas, New Year) are set from the local date.

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "char_slot.h"

#include "char_state.h"

#include "node.h"

#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"

extern "C" void (*DAT_11b1ba40)(void *block);        // the engine's free
extern "C" char DAT_103150c0[0x80];
extern "C" int DAT_10446e00[0x28];

extern "C" int FUN_1013fb10(int active, int target, int frames);
extern "C" int FUN_1000e810(void);
extern "C" int FUN_101011b0(int id, int value);

// 0x10023900 (Map_ResetOnEnter). Moves the map timer (flag 0x44c) on by the time since flag 0x44a, counts a new map
// in flag 0x448 (0-11), clears the state of character slots 58-123 (and their slot field 0xbc to -1), the held
// model, the sound and HUD tables and flags 0xf7e-0xf86, sets the costume flags from the bought items, shuffles the
// twenty treasure boxes into flags 0x93f-0x952 (box 0x11 never among the first five) when they are unset, and sets
// flag 0x828 on Christmas (24th from 19:00 and 25th December), 0x829 on New Year (1st-3rd January). Returns 0.
extern "C" int FUN_10023900(void)
{
    int timer = DAT_10c62edc[0x44c];
    int paused = DAT_10c62edc[0x44a];
    DAT_10c62edc[0x44c] = GetTickCount() + (timer - paused);
    if (DAT_10c62edc[0x446] != DAT_10c62ed0)
    {
        int visits = DAT_10c62edc[0x448] + 1;
        if (visits < 0)
            visits = 0;
        else if (visits > 11)
            visits = 11;
        DAT_10c62edc[0x448] = visits;
    }
    DAT_10c62edc[0x446] = DAT_10c62ed0;
    memset(DAT_1021f3fa, 0, sizeof(DAT_1021f3fa));
    for (int i = 0; i < 66; i++)
    {
        DAT_102549a8[i] = 0;
        DAT_1020b430[58 + i].state = 0;
        DAT_1020b430[58 + i].wait = 0;
        DAT_1020b430[58 + i].talk = 0;
        DAT_1020b430[58 + i].range = 0;
        DAT_1020b430[58 + i].action = 0;
        DAT_1020b430[58 + i].alarmFlashes = 0;
        DAT_1020b430[58 + i].fall = 0;
        DAT_1020b430[58 + i].rider = 0;
        DAT_1020b430[58 + i].pointPause = 0;
        DAT_1020b430[58 + i].m_0024 = 0;
        DAT_1020b430[58 + i].fadeActions = 0;
        DAT_1020b430[58 + i].m_002c = 0;
        DAT_1020b430[58 + i].m_0030 = 0;
        DAT_1020b430[58 + i].m_0034 = 0;
        DAT_1020b430[58 + i].sawPlayer = 0;
        DAT_1020b430[58 + i].behaviour = 0;
        DAT_10daaae8[58 + i].money = -1;
    }
    DAT_10c62edc[0x44e] = DAT_10daaae8[0].stats[STAT_HEALTH];
    DAT_104c0354 = 0;
    if (DAT_104c0390 != 0)
    {
        DAT_104c0390->Release();
        DAT_11b1ba40(DAT_104c0390);
        DAT_104c0390 = 0;
    }
    FUN_1013fb10(2, 0, 0);
    memset(DAT_1021a370, 0, sizeof(DAT_1021a370));
    memset(DAT_1020dcd0, 0, sizeof(DAT_1020dcd0));
    DAT_104c03b4 = 0;
    DAT_104c0364 = 0;
    DAT_104c0368 = 0;
    DAT_104c036c = 0;
    DAT_104c0370 = 0;
    if (DAT_102028c8 > -1)
        DAT_102028c8 = -1;
    if (DAT_102028d0 > -1)
        DAT_102028d0 = -1;
    memset(DAT_10c62250, 0, sizeof(DAT_10c62250));
    memset(DAT_10c62650, 0, sizeof(DAT_10c62650));
    if (DAT_104c0398 > 0 && DAT_104c00b8 != 0)
    {
        DAT_104c00b8 = 0;
        DAT_104c0398 = 0;
    }
    DAT_10c62edc[0xf86] = 0;
    DAT_10c62edc[0xf85] = 0;
    DAT_10c62edc[0xf84] = 0;
    DAT_10c62edc[0xf83] = 0;
    DAT_10c62edc[0xf7f] = 0;
    DAT_10c62edc[0xf7e] = 0;
    FUN_1000e810();
    if (DAT_10c62edc[0x8d7] != 0)
    {
        DAT_10c62edc[0x6db] = 1;
        DAT_10c62edc[0x6d6] = 2;
    }
    if (DAT_10c62edc[0x9db] != 0)
        DAT_10c62edc[0x6db] = 2;
    if (DAT_10c62edc[0x8d9] != 0)
        DAT_10c62edc[0x6d7] = 2;
    if (DAT_10c62edc[0x8db] != 0)
        DAT_10c62edc[0x6d9] = 2;
    if (DAT_10c62edc[0x8dc] != 0)
    {
        DAT_10c62edc[0x6db] = 3;
        DAT_10c62edc[0x6d8] = 2;
        DAT_10c62edc[0x85e] = 1;
    }
    FUN_101011b0(0x27da, 0);
    DAT_10c62edc[0x9ed] = DAT_103150c0[0x17] != 0;
    DAT_10c62edc[0x9f0] = DAT_103150c0[0x18] != 0;
    DAT_10c62edc[0x9ef] = DAT_103150c0[0x1a] != 0;
    DAT_10c62edc[0x9ee] = DAT_103150c0[0x1c] != 0;
    DAT_10c62edc[0x9f1] = DAT_103150c0[0x37] != 0;
    if (DAT_10c62edc[0x93f] == 0)
    {
        int boxes[20] = {4, 4, 0x2b, 5, 5, 5, 0x14, 0x13, 0x13, 0x16, 0x16, 0x34, 0x27, 0x2c, 0x2f, 0x14, 0x14, 0x11,
                         0x16, 0x13};
        int i = 0;
        int last = 19;
        for (; i < 20; i++)
        {

            int pick = rand() % (last + 1);
            if (i < 5 && boxes[pick] == 0x11)
            {
                i--;
                continue;
            }
            int flag = 0x93f + i;
            if (flag <= 3999 && flag != 0)
                DAT_10c62edc[flag] = boxes[pick];
            for (int j = pick; j < last; j++)
                boxes[j] = boxes[j + 1];
            last--;
        }
    }
    memset(DAT_1021d018, 0, sizeof(DAT_1021d018));
    memset(DAT_10446e00, 0, sizeof(DAT_10446e00));
    SYSTEMTIME now;
    GetLocalTime(&now);
    if (now.wMonth == 12)
    {
        if (now.wDay == 25 || (now.wDay == 24 && now.wHour > 18))
            DAT_10c62edc[0x828] = 1;
        else
            DAT_10c62edc[0x828] = 0;
        DAT_10c62edc[0x829] = 0;
    }
    else if (now.wMonth == 1 && now.wDay < 4)
    {
        DAT_10c62edc[0x828] = 0;
        DAT_10c62edc[0x829] = 1;
    }
    else
    {
        DAT_10c62edc[0x828] = 0;
        DAT_10c62edc[0x829] = 0;
    }
    if (DAT_10c62edc[0x602] != 0)
        DAT_10c62edc[0x82a] = DAT_10c62edc[0x700] != 0x1f1f1f1f;
    return 0;
}
