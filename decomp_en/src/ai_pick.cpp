// Enemy behaviour pickers (EN build). Ported from the matched JP source decomp/src/ai_pick.cpp and app_0012.cpp.
#include "char.h"
#include "effect_model.h"
#include "engine.h"
#include "map_view.h"

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

// Enemy_PickAction_40_25_35: picker with three behaviours: behaviour 0 40 %, behaviour 1 25 %, behaviour 2 35 %.
extern "C" int Enemy_PickAction_40_25_35(int index)
{
    int roll = Random() % 100;
    if (roll < 40)
    {
        g_charState[index].state = 0;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
        return 0;
    }
    if (roll < 65)
    {
        g_charState[index].state = 1;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        g_charState[index].state = 2;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
    }
    return 0;
}

// Enemy_PickAction_60_10_30: picker with three behaviours: behaviour 0 60 %, behaviour 1 10 %, behaviour 2 30 %.
extern "C" int Enemy_PickAction_60_10_30(int index)
{
    int roll = Random() % 100;
    if (roll < 60)
    {
        g_charState[index].state = 0;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
        return 0;
    }
    if (roll < 70)
    {
        g_charState[index].state = 1;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        g_charState[index].state = 2;
        g_charState[index].values[0] = 0;
        g_charState[index].values[1] = 0;
    }
    return 0;
}

// UpdateMeshSlots: draws every used effect model slot with its key frames and the current view matrices. The identity
// matrix built at the end is unused (left over from older code). Called once per frame by the world renderer.
int UpdateMeshSlots()
{
    for (int i = 0; i < EFFECT_MODEL_COUNT; i++)
    {
        if (g_effectModels[i].node != 0)
            g_effectModels[i].DrawKeyed(g_cameraViewProjection, g_cameraView);
    }
    Matrix4 identity;
    identity.SetIdentity();
    return 0;
}
