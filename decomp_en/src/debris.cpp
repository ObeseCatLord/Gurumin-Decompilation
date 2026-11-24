// Debris pieces (EN build). Ported from the matched JP source decomp/src/debris.cpp.
#include "debris.h"
#include "engine.h"

// Debris_Spawn: starts a simple piece of type `type` at (x, y, z) in the first free slot of the simple slots: random
// spin, a random sideways velocity (-5..5 each way) with 20 upwards, full scale. Returns 1, or 0 when the slots are
// all taken. Called by the map scripts and the break effects.
extern "C" int Debris_Spawn(float x, float y, float z, int type)
{
    int i;
    for (i = 0; i < DEBRIS_SIMPLE_SLOTS; i++)
    {
        if (g_debrisType[i] == 0)
            break;
    }
    if (i == DEBRIS_SIMPLE_SLOTS)
        return 0;
    g_debrisType[i] = type;
    g_debrisPosition[i].x = x;
    g_debrisPosition[i].y = y;
    g_debrisPosition[i].z = z;
    g_debrisSettled[i] = 0;
    g_debrisBounces[i] = 0;
    int spin = Random() % DEBRIS_SPIN_RANGE;
    g_debrisLife[i] = 0;
    g_debrisSpin[i] = spin;
    g_debrisHeld[i] = 0;
    g_debrisVelocity[i].Set((float)(Random() % DEBRIS_SIDEWAYS_RANGE - Random() % DEBRIS_SIDEWAYS_RANGE),
                            (float)(Random() % DEBRIS_SIDEWAYS_RANGE - Random() % DEBRIS_SIDEWAYS_RANGE),
                            DEBRIS_UPWARD_SPEED, 0);
    g_debrisScale[i].x = 1.0f;
    g_debrisScale[i].y = 1.0f;
    g_debrisScale[i].z = 1.0f;
    return 1;
}
