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
    DAT_10db6ed8[index].model.M_10162610(DAT_10db6ed8[index - 2].matrix._41, DAT_10db6ed8[index - 2].matrix._42,
                                         DAT_10db6ed8[index - 2].matrix._43);
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




// One int every 0x40 bytes (laid over the CharState table at 0x1020b430, see world.h); used for the per-kind
// cooldown counters below.
struct KindCounter
{
    int value;
    char pad_0004[0x3c];
};

extern "C" KindCounter DAT_1020b498[0x80];  // cooldown of kind 0x64
extern "C" KindCounter DAT_1020b49c[0x80];  // cooldown of kind 0x46
extern "C" KindCounter DAT_1020b410[0x80];  // cooldown of kind 0x69
extern "C" KindCounter DAT_1020b4b4[0x80];  // cooldown of kind 0x2d
extern "C" KindCounter DAT_1020b454[0x80];  // cooldown of kind 0x3f
extern "C" KindCounter DAT_1020b46c[0x80];  // cooldown of kind 0x43

// Counts down the cooldown counter that belongs to character `index`'s kind (six kinds have one; other kinds do
// nothing). Counters stop at 0. Each branch decrements its own counter; the compiler merges the identical tails.
extern "C" int FUN_1001d7d0(int index)
{
    int kind = DAT_10daab84[index].m_0000;
    if (kind == 0x64)
    {
        if (DAT_1020b498[index].value > 0)
            DAT_1020b498[index].value--;
    }
    else if (kind == 0x46)
    {
        if (DAT_1020b49c[index].value > 0)
            DAT_1020b49c[index].value--;
    }
    else if (kind == 0x69)
    {
        if (DAT_1020b410[index].value > 0)
            DAT_1020b410[index].value--;
    }
    else if (kind == 0x2d)
    {
        if (DAT_1020b4b4[index].value > 0)
            DAT_1020b4b4[index].value--;
    }
    else if (kind == 0x3f)
    {
        if (DAT_1020b454[index].value > 0)
            DAT_1020b454[index].value--;
    }
    else if (kind == 0x43)
    {
        if (DAT_1020b46c[index].value > 0)
            DAT_1020b46c[index].value--;
    }
    return 0;
}
