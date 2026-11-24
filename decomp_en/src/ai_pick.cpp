// Enemy behaviour pickers (EN build). Ported from the matched JP source decomp/src/ai_pick.cpp and app_0012.cpp.
#include "char.h"
#include "engine.h"

// EnemyAI_PickSingle: picker with a single behaviour: always behaviour 0 (the random roll is made but unused).
extern "C" int EnemyAI_PickSingle(int index)
{
    Random();
    g_charState[index].state = 0;
    g_charState[index].values[0] = 0;
    g_charState[index].values[1] = 0;
    return 0;
}

// Enemy_PickAction_65_35: picker with two behaviours only: behaviour 0 65 %, behaviour 1 35 %.
extern "C" int Enemy_PickAction_65_35(int index)
{
    int roll = Random() % 100;
    if (roll < 65)
    {
        g_charState[index].state = 0;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        g_charState[index].state = 1;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
    }
    return 0;
}
