// Script flags of the Game object (EN build). Ported from the matched JP source decomp/src/app_0003.cpp.
#include "game.h"

// Tests script flag `index` (4000 flags saved with the game). A negative index tests that the flag is clear. Flag 0
// is always set; out-of-range indexes read as clear (or as set for the negated test). Called by every script condition.
int Game::TestScriptFlag(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > SCRIPT_FLAG_LAST)
            return 1;
        return scriptFlags[index] == 0;
    }
    if (index > SCRIPT_FLAG_LAST)
        return 0;
    if (index == 0)
        return 1;
    return scriptFlags[index];
}

// Sets script flag `index` to `value` (flag 0 and out-of-range indexes are ignored). Returns 1 when in range.
int Game::SetScriptFlag(int index, int value)
{
    if (index > SCRIPT_FLAG_LAST)
        return 0;
    if (index != 0)
        scriptFlags[index] = value;
    return 1;
}
