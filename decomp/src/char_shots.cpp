// Shots fired by characters. Each character slot `owner` has six helper slots owner+2 .. owner+7 that its shots
// use; a helper slot is free while its state (CharState::m_0000) is 0.
#include "char_tables.h"
#include "node.h"
#include "map_vars.h"
#include "world.h"

#include "vec4.h"
#include "anim_model.h"
#include "char_state.h"

#include "last_position.h"

extern "C" int FUN_1013f880(void);  // the game's random number
#include "char_vars.h"

// The shot's values in DAT_10255160 (char_tables.h).
enum ShotMotionValue
{
    SHOT_ANGLE = 0,      // rotation about the vertical axis, copied from the owner (see FUN_1001a590)
    SHOT_HEADING = 1,    // radians
    SHOT_SPEED = 2
};

// 0x1001a680 (Char_FireShot). Starts a shot of character `owner` from `position` in the first free helper slot:
// state 1, 300 frames to live, `spread` stored in the slot, the owner's pitch and target heading, speed 5. With
// `spread` set the heading is turned by a random amount of up to +-0.025 radians. Returns 0, also when all six
// helper slots are in use.
extern "C" int FUN_1001a680(int owner, const Vec4 *position, int spread)
{
    for (int slot = owner + 2; slot < owner + 8; slot++)
    {
        if (DAT_1020b430[slot].state != 0)
            continue;
        DAT_1020b430[slot].state = 1;
        DAT_10802bf8[slot].position.Copy(*position);
        DAT_1020b430[slot].values[0] = 300;
        DAT_1020b430[slot].values[1] = spread;
        DAT_1020b430[slot].values[2] = 0;
        DAT_10255160[slot][SHOT_ANGLE] = DAT_10255160[owner][SHOT_ANGLE];
        DAT_10255160[slot][SHOT_HEADING] = DAT_10db6ee8[owner].targetHeading;
        if (spread != 0)
            DAT_10255160[slot][SHOT_HEADING] = (float)(FUN_1013f880() % 250 - 125) / 5000.0f + DAT_10255160[slot][SHOT_HEADING];
        DAT_10255160[slot][SHOT_SPEED] = 5.0f;
        return 0;
    }
    return 0;
}

// 0x1001a590. Ground probe for slot `slot` (third argument: distance; second unused): casts a ray from `distance`
// units along the slot's angle, from 50 units above that point to 300 units below it, against the world collision (mask
// 0x4000000), filtered by the slot's nearCells. The ray is not offset by the slot's position. Returns 1 on a hit,
// 0 when there is none or the slot's m_0028 is set.
extern "C" int FUN_1001a590(int slot, int unused, float distance)
{
    if (DAT_1020b430[slot].skipGroundProbe == 0)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.y = -distance;
        from.RotateZ(-DAT_10255160[slot][SHOT_ANGLE]);
        from.z += 50.0f;
        to.Copy(from);
        to.z -= 350.0f;
        DAT_11b002c0 = (int)DAT_10db6ee8[slot].nearCells;
        if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, 0x4000000, &from, &to, &hit) != 0)
        {
            DAT_11b002c0 = 0;
            return 1;
        }
        DAT_11b002c0 = 0;
    }
    return 0;
}
