#ifndef GURUMIN_SCRIPT_FLAG_H
#define GURUMIN_SCRIPT_FLAG_H

#include "game_vars.h"

// Script flag `index` (as Game::TestScriptFlag does it): a negative index tests that the flag is clear, flag 0 is
// always set and indexes past 3999 read as clear (or as set for the negated test).
__forceinline int ScriptFlag(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > 3999)
            return 1;
        return DAT_10c62edc[index] == 0;
    }
    if (index > 3999)
        return 0;
    if (index == 0)
        return 1;
    return DAT_10c62edc[index];
}

#endif
