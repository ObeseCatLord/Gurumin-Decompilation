// Effect slots and enemy helpers.

#include <stdlib.h>
#include <string.h>
#include "world.h"
#include "anim_model.h"
#include "char_state.h"

extern "C" int DAT_10255f10[];
extern "C" int FUN_1013f880(void);
extern "C" void FUN_10043cc0(int);

// Returns the first free effect slot (of 0x40), or -1.
inline int FindFreeEffectSlot()
{
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10860df0[i].m_00ec == 0 && DAT_10860df0[i].active == 0)
            return i;
    }
    return -1;
}

// Out-of-line FindFreeEffectSlot.
extern "C" int FUN_10017b80(void)
{
    return FindFreeEffectSlot();
}

// Starts a type-14 effect `name` with `id` and `value` at (x, y, z) in a free effect slot. Returns 1. The original
// does not check for a full table (slot -1 would be written).
extern "C" int FUN_1003e650(int value, int x, int y, int z, const char *name, int id)
{
    int slot = FindFreeEffectSlot();
    DAT_10860df0[slot].m_0010 = value;
    DAT_10860df0[slot].active = 1;
    DAT_10860df0[slot].m_00ec = 0;
    DAT_10860df0[slot].type = 14;
    DAT_10860df0[slot].id = id;
    strcpy(DAT_10860df0[slot].name, name);
    DAT_10860df0[slot].position.Set((float)x, (float)y, (float)z, 0);
    return 1;
}

// Returns the value of the active type-10 effect with `id`, or -1.
extern "C" int FUN_10017bb0(int id)
{
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10860df0[i].active != 0 && DAT_10860df0[i].type == 10 && DAT_10860df0[i].id == id)
            return DAT_10860df0[i].value;
    }
    return -1;
}

// Random idle animation: a 1 % chance per call to play the idle motion (speed 30) on character `index`
// when it has one (m_239c).
extern "C" int FUN_1001b840(int index)
{
    if (DAT_10db6ee8[index].m_238c != 0)
    {
        if (FUN_1013f880() % 10000 < 100)
            DAT_10db6ee8[index].AddMotionSpeed(30.0f);
    }
    return 0;
}

// Starts character `index`'s defeat sequence unless it is already busy (DAT_10210d30); returns 1 when busy.
extern "C" int FUN_10034c80(int index)
{
    if (DAT_10210d30[index] != 0)
        return 1;
    DAT_10daab84[index].m_00a5 = 1;
    DAT_1020b430[index].behaviour = 1;
    return 0;
}

// Stores the character's formation number as its group (100 means "no group" = -1) and refreshes the group
// links (FUN_10043cc0).
extern "C" int FUN_10047400(int index)
{
    int value = DAT_1020b430[index].values[1];
    if (value == 100)
        DAT_10255f10[index] = -1;
    else
        DAT_10255f10[index] = value;
    FUN_10043cc0(index);
    return 0;
}
