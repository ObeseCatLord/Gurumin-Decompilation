// Enemy AI updaters, one per enemy kind, called once per frame per enemy.
// Pattern: skip while the enemy is busy (Enemy_IsBusy 0x100e1c40, e.g. stunned or hit), then run the current
// behaviour (CharState::m_0000: 0, 1, 2) and mark it finished (-1) when the behaviour function reports done.
// In the finished/idle state a short delay counter (m_0004[2]) runs and a picker (ai_pick.cpp) chooses the next
// behaviour. Each updater combines different move functions (walk towards the player, jump, charge, ...).
// Each case ends with its own copy of the "finished" handling (it matches the original, which keeps 0 in ebx).

#include "world.h"
#include "char_state.h"

#include "enemy_queue.h"
#include "enemy_flee.h"
extern "C" int FUN_10026af0(int index);
extern "C" int FUN_10026b70(int index);
extern "C" int FUN_10026c50(int index);
extern "C" int FUN_10026c80(int index);
extern "C" int FUN_10026d80(int index, float range);
extern "C" int FUN_10027400(int index);
extern "C" int FUN_10027560(int index);
extern "C" int FUN_10027630(int index, int flag);
extern "C" int FUN_10027da0(int index, int a);
extern "C" int FUN_10028360(int index, int a);

// Kind A: 0 = approach within 300 units (FUN_10026d80), 1 = FUN_10027560, 2 = FUN_10028360(2); picker A.
extern "C" int FUN_100e2080(int index)
{
    if (Enemy_IsBusy(index) == 0)
    {
        switch (DAT_1020b430[index].state)
        {
        case 0:
            if (FUN_10026d80(index, 300.0f) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 1:
            if (FUN_10027560(index) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 2:
            if (FUN_10028360(index, 2) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        default:
            if (++DAT_1020b430[index].values[2] > 0)
            {
                DAT_1020b430[index].values[2] = 0;
                FUN_10026af0(index);
            }
            break;
        }
    }
    return 0;
}

// Kind B: 0 = FUN_10028360(2), 1 = FUN_100279b0(4, 0), 2 = FUN_10027630(0); picker B.
extern "C" int FUN_100e2110(int index)
{
    if (Enemy_IsBusy(index) == 0)
    {
        switch (DAT_1020b430[index].state)
        {
        case 0:
            if (FUN_10028360(index, 2) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 1:
            if (FUN_100279b0(index, 4, 0) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 2:
            if (FUN_10027630(index, 0) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        default:
            if (++DAT_1020b430[index].values[2] > 0)
            {
                DAT_1020b430[index].values[2] = 0;
                FUN_10026b70(index);
            }
            break;
        }
    }
    return 0;
}

// Kind C (see source for its move set).
extern "C" int FUN_100e2440(int index)
{
    if (Enemy_IsBusy(index) == 0)
    {
        switch (DAT_1020b430[index].state)
        {
        case 0:
            if (FUN_10027400(index) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 1:
            if (FUN_10027560(index) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 2:
            if (FUN_10028360(index, 2) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        default:
            if (++DAT_1020b430[index].values[2] > 0)
            {
                DAT_1020b430[index].values[2] = 0;
                FUN_10026c50(index);
            }
            break;
        }
    }
    return 0;
}

// Kind D (see source for its move set).
extern "C" int FUN_100e24d0(int index)
{
    if (Enemy_IsBusy(index) == 0)
    {
        switch (DAT_1020b430[index].state)
        {
        case 0:
            if (FUN_10028360(index, 2) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 1:
            if (FUN_10027da0(index, 2) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 2:
            if (FUN_10027560(index) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        default:
            if (++DAT_1020b430[index].values[2] > 0)
            {
                DAT_1020b430[index].values[2] = 0;
                FUN_10026c80(index);
            }
            break;
        }
    }
    return 0;
}

extern "C" int FUN_10026d00(int index);

// Kind E: 0 = FUN_10027630(0), 1 = FUN_10028360(2), 2 = FUN_100279b0(0, 1); picker E.
extern "C" int FUN_100e2560(int index)
{
    if (Enemy_IsBusy(index) == 0)
    {
        switch (DAT_1020b430[index].state)
        {
        case 0:
            if (FUN_10027630(index, 0) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 1:
            if (FUN_10028360(index, 2) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        case 2:
            if (FUN_100279b0(index, 0, 1) != 0)
            {
                DAT_1020b430[index].values[2] = 0;
                DAT_1020b430[index].state = -1;
            }
            break;
        default:
            if (++DAT_1020b430[index].values[2] > 0)
            {
                DAT_1020b430[index].values[2] = 0;
                FUN_10026d00(index);
            }
            break;
        }
    }
    return 0;
}
