// Character slot helpers. Slots 0x5c..0x7b hold the map's NPCs and objects.

#include <stdlib.h>
#include "world.h"

extern "C" int DAT_104c0370;
extern "C" int DAT_10c62ed0;
extern "C" void FUN_100308e0(void);
extern "C" void FUN_100301a0(void);
extern "C" void FUN_1002f520(void);
extern "C" void FUN_100311b0(void);
extern "C" int FUN_1013f880(void);

// Clears the timers of every kind-0x58 object whose timer value lies in [low, high].
extern "C" int FUN_1001c4c0(int low, int high)
{
    for (int i = 0x5c; i < 0x7c; i++)
    {
        if (DAT_10daab84[i].m_0000 == 0x58)
        {
            for (int j = 0; j < 6; j++)
            {
                if (DAT_1020b430[i].m_0004[j] >= low && DAT_1020b430[i].m_0004[j] <= high)
                    DAT_1020b430[i].m_0004[j] = 0;
            }
        }
    }
    return 0;
}

// Sets parameter 5 of every NPC with `id` to `value` and restarts its behaviour; also sets DAT_104c0370 = 2.
extern "C" int FUN_1001dbd0(int id, int value)
{
    for (int i = 0x5c; i < 0x7c; i++)
    {
        if (DAT_10db6ed8[i].id == id)
        {
            DAT_1020b430[i].m_0004[5] = value;
            DAT_1020b430[i].m_0004[0] = 0;
            DAT_1020b430[i].m_0004[1] = 0;
        }
    }
    DAT_104c0370 = 2;
    return 0;
}

// Picker with a single behaviour: always behaviour 0 (the random roll is made but unused; see ai_pick.cpp).
extern "C" int FUN_10026c50(int index)
{
    FUN_1013f880();
    DAT_1020b430[index].m_0000 = 0;
    DAT_1020b430[index].m_0004[0] = 0;
    DAT_1020b430[index].m_0004[1] = 0;
    return 0;
}

// Wraps an angle in units of 1/10000 turn into the range -5000..5000.
extern "C" int FUN_100286b0(int angle)
{
    if (angle > 5000)
        angle -= 10000;
    return angle;
}

// Moves character `index` to the position of character `index - 2` (a follower placed on its leader).
extern "C" int FUN_100347d0(int index)
{
    DAT_10db6ed8[index].model.M_10162610(DAT_10db6ed8[index - 2].m_2368.x, DAT_10db6ed8[index - 2].m_2368.y,
                                         DAT_10db6ed8[index - 2].m_2368.z);
    return 0;
}

// Runs the per-frame map script of maps 0x15..0x18 (DAT_10c62ed0 = current map).
extern "C" int FUN_10052480(void)
{
    switch (DAT_10c62ed0)
    {
    case 0x15:
        FUN_100308e0();
        break;
    case 0x16:
        FUN_100301a0();
        break;
    case 0x17:
        FUN_1002f520();
        break;
    case 0x18:
        FUN_100311b0();
        break;
    }
    return 0;
}



