// Script flags and a rectangle test.

#include "game.h"

// Tests script flag `index` (4000 flags in m_3ee694, saved with the game). A negative index tests that the flag
// is clear. Flag 0 is always set; out-of-range indexes read as clear (or as set for the negated test).
int Game::M_100022b0(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > 3999)
            return 1;
        return m_3ee694[index] == 0;
    }
    if (index > 3999)
        return 0;
    if (index == 0)
        return 1;
    return m_3ee694[index];
}

// Sets script flag `index` to `value` (flag 0 and out-of-range indexes are ignored).
int Game::M_10002300(int index, int value)
{
    if (index > 3999)
        return 0;
    if (index != 0)
        m_3ee694[index] = value;
    return 1;
}

// 1 when the point (x, y) lies inside the rectangle (edges included), else 0.
extern "C" int __stdcall FUN_10008430(int x, int y, int left, int top, int right, int bottom)
{
    if (x < left || y < top || x > right || y > bottom)
        return 0;
    return 1;
}

