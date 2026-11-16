#include "game.h"

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

int Game::M_10002300(int index, int value)
{
    if (index > 3999)
        return 0;
    if (index != 0)
        m_3ee694[index] = value;
    return 1;
}

extern "C" int __stdcall FUN_10008430(int x, int y, int left, int top, int right, int bottom)
{
    if (x < left || y < top || x > right || y > bottom)
        return 0;
    return 1;
}

