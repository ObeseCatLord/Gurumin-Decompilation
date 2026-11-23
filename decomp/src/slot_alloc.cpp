// Small allocators over fixed tables.

#include "vec4.h"

#include "effect_manager.h"
#include "char_vars.h"
#include "game_vars.h"


// Takes the next free entry of the 0x400-entry position ring (DAT_10254c40 = in use) after the last one used,
// stores `position` there and returns 1; gives up after 100 tries and returns 0.
extern "C" int FUN_1000c5c0(const Vec4 &position)
{
    unsigned int i = DAT_104c013c;
    for (int tries = 0; tries < 100; tries++)
    {
        if (++i >= 0x400)
            i = 0;
        if (DAT_10254c40[i] == 0)
        {
            DAT_104c013c = i;
            DAT_10254c40[i] = 1;
            DAT_104c0438[i].Copy(position);
            return 1;
        }
    }
    DAT_104c013c = i;
    return 0;
}


// 0x1016fb10 (marking the three tables free) is EffectManager::ClearTables in char_effects.cpp: the effect
// manager's Init relies on knowing its registers.
