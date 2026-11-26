// Unlock progress test (EN build, EN-only: no JP twin found).
#include "game.h"
#include "progress_unlock.h"

// Unlock_IsBonusAvailable: true when all unlock bits are set and the code has a non-zero high part (its digits above
// the lowest six) or a non-zero middle part (digits 5 and 6 from the right, taken modulo 100). Called by the scene
// setup (EN 0x5fd1b0).
extern "C" bool Unlock_IsBonusAvailable(void)
{
    bool allUnlocked = g_unlockBits == UNLOCK_BITS_ALL;
    int high = g_unlockCode / UNLOCK_CODE_HIGH_DIVISOR;
    if (allUnlocked && (high != 0 || (g_unlockCode / UNLOCK_CODE_MID_DIVISOR) % UNLOCK_CODE_MID_RANGE != 0))
        return true;
    return false;
}

// Unlock_AllFlagsClear: true when every unlock bit is set and no counted flag of g_unlockFlagIds holds a value whose high
// digits (above the lowest six) or middle digit group (digits 5 and 6 from the right) are non-zero. A flag counts when its
// value modulo 100 is below 4 or its id is UNLOCK_FLAG_ALWAYS_COUNTED. Called by the scene setup (EN 0x5fd1b0).
extern "C" bool Unlock_AllFlagsClear(void)
{
    bool allUnlocked = g_unlockBits == UNLOCK_BITS_ALL;
    bool clear = true;
    for (int i = 0; i < UNLOCK_FLAG_COUNT; i++)
    {
        short id = g_unlockFlagIds[i];
        int level = g_game.TestScriptFlag(id + UNLOCK_FLAG_OFFSET) % UNLOCK_FLAG_LEVEL_DIVISOR;
        int high = g_game.TestScriptFlag(id + UNLOCK_FLAG_OFFSET) / UNLOCK_CODE_HIGH_DIVISOR;
        int middle = (g_game.TestScriptFlag(id + UNLOCK_FLAG_OFFSET) / UNLOCK_CODE_MID_DIVISOR) % UNLOCK_CODE_MID_RANGE;
        if ((level < UNLOCK_FLAG_LEVEL_LIMIT || id == UNLOCK_FLAG_ALWAYS_COUNTED) && (high != 0 || middle != 0))
        {
            clear = false;
            break;
        }
    }
    return allUnlocked ? clear : false;
}
