// An NPC walking up to the player: on the first frame it starts walking and picks how near it stops; then it keeps
// turning towards the player until it is near enough (or released by its code), when its state is reset.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"


#include "char_state.h"

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

extern "C" void FUN_10043cc0(int slot);         // Char_ResetState

// 0x10045ed0 (Npc_ApproachPlayer). Runs one frame of the approach of character `slot`: the first frame unlocks it,
// sets the stop distance to 10 * (code % 100) and starts the walk motion. Later frames keep walking, turn it
// towards the player and reset its state once it is nearer than the stop distance (heights counting double) or,
// when releasable, the code's release flag is set. Returns 0.
extern "C" int FUN_10045ed0(int slot)
{
    Vec4 offset;
    offset = DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[slot].matrix.GetTranslation();
    offset.z *= 2.0f;
    float distance = offset.Length();
    if (DAT_1020b430[slot].approachStarted == 0)
    {
        DAT_1020b430[slot].approachStarted = 1;
        DAT_10db6ee8[slot].mode = 0;
        DAT_1020b430[slot].stopDistance = DAT_1020b430[slot].approachCode % 100 * 10;
        DAT_10db6ee8[slot].PlayMotion(2, 0, 5, 1.0f);
        return 0;
    }
    float turn = DAT_10db6ee8[slot].heading - DAT_10db6ee8[slot].targetHeading;
    while (turn > 3.1415f)
        turn -= 6.283f;
    while (turn < -3.1415f)
        turn += 6.283f;
    DAT_10db6ee8[slot].PlayMotion(2, 0, 5, 1.0f);
    DAT_10db6ee8[slot].targetHeading =
        Atan2(DAT_10db6ee8[0].matrix.m[12] - DAT_10db6ee8[slot].matrix.m[12],
              DAT_10db6ee8[0].matrix.m[13] - DAT_10db6ee8[slot].matrix.m[13]) +
        3.1415f;
    if (distance < (float)DAT_1020b430[slot].stopDistance ||
        DAT_1020b430[slot].releasable != 0 && DAT_1020b430[slot].approachCode / 100 != 0)
        FUN_10043cc0(slot);
    return 0;
}
