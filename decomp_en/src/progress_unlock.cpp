// Unlock progress test (EN build, EN-only: no JP twin found).
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
