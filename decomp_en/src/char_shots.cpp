// Shots fired by characters and the ground probe (EN build). JP twin: char_shots.cpp.
#include "char_shots.h"
#include "collide_regions.h"
#include "engine.h"

// Starts a shot of character `owner` from `position` in the first free helper slot: state 1, 300 frames to live,
// `spread` stored in the slot, the angle of the owner and its target heading, speed 5. With `spread` set the heading is
// turned by a random amount of up to +-0.025 radians. Returns 0, also when all six helper slots are in use. Called by
// shooting enemies.
extern "C" int Char_FireShot(int owner, const Vec4 *position, int spread)
{
    for (int slot = owner + SHOT_HELPER_FIRST; slot < owner + SHOT_HELPER_END; slot++)
    {
        if (g_charState[slot].state != 0)
            continue;
        g_charState[slot].state = 1;
        g_shotSlots[slot].position.Copy(*position);
        g_charState[slot].values[SHOT_VALUE_LIFE] = SHOT_LIFE_FRAMES;
        g_charState[slot].values[SHOT_VALUE_SPREAD] = spread;
        g_shotParams[slot][SHOT_ANGLE] = g_shotParams[owner][SHOT_ANGLE];
        g_charState[slot].values[SHOT_VALUE_COUNTER] = 0;
        g_shotParams[slot][SHOT_HEADING] = g_characterModels[owner].targetHeading;
        if (spread != 0)
            g_shotParams[slot][SHOT_HEADING] =
                (float)(Random() % SHOT_SPREAD_RANGE - SHOT_SPREAD_CENTRE) / SHOT_SPREAD_DIVISOR + g_shotParams[slot][SHOT_HEADING];
        g_shotParams[slot][SHOT_SPEED] = SHOT_SPEED_START;
        return 0;
    }
    return 0;
}

// Ground probe for slot `slot` (third argument: distance; second unused): casts a ray from `distance` units along the
// angle of the slot, from 50 units above that point to 300 units below it, against the world collision, filtered by
// the collision filter of the slot's model. The ray is not offset by the position of the slot. Returns 1 on a hit, 0
// when there is none or the slot skips the probe.
extern "C" int Char_ProbeGround(int slot, const Vec4 *unused, float distance)
{
    if (g_charState[slot].values[CHARSTATE_SKIP_GROUND_PROBE] == 0)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.y = -distance;
        from.RotateZ(-g_shotParams[slot][SHOT_ANGLE]);
        from.z += GROUND_PROBE_UP;
        to.Copy(from);
        to.z -= GROUND_PROBE_DOWN;
        g_collisionFilter = g_characterModels[slot].collisionFilter;
        if (g_mapRootNode != 0 && g_mapRootNode->Collide(0, GROUND_PROBE_MASK, &from, &to, &hit) != 0)
        {
            g_collisionFilter = 0;
            return 1;
        }
        g_collisionFilter = 0;
    }
    return 0;
}
