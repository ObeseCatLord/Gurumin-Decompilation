// Turning characters towards a target heading, and pushing them.
#include "world.h"

#include "vec4.h"
#include "anim_model.h"

extern "C" int DAT_104c00d0;  // when non-zero, the player (character 0) cannot be pushed

// 0x1001bfb0 (Char_LimitTurn). Limits how far character `index` turns this step: the difference between its target
// heading and its heading, wrapped into -pi..pi (pi = 3.1415, a full turn = 6.283 here), is clamped to
// -maxTurn..maxTurn and the target heading is set to heading + that difference. Returns 0.
extern "C" int FUN_1001bfb0(int index, float maxTurn)
{
    float turn = DAT_10db6ee8[index].targetHeading - DAT_10db6ee8[index].heading;
    while (turn > 3.1415f)
        turn -= 6.283f;
    while (turn < -3.1415f)
        turn += 6.283f;
    if (turn > maxTurn)
        turn = maxTurn;
    else if (turn < -maxTurn)
        turn = -maxTurn;
    DAT_10db6ee8[index].targetHeading = turn + DAT_10db6ee8[index].heading;
    return 0;
}

// 0x10011b70 (Char_PushAwayFrom). Pushes character `index` away from `point` with `strength` (in the x/y plane of
// the direction, z = 0), passing `flag` on with the push. Character 0 is not pushed while DAT_104c00d0 is set, and
// nothing happens when the character stands exactly on the point. Returns 0.
extern "C" int FUN_10011b70(int strength, char flag, int index, const Vec4 *point)
{
    Vec4 direction = DAT_10db6ee8[index].matrix.GetTranslation();
    Vec4 from;
    from.Copy(*point);
    direction.Copy(direction - from);
    direction.Normalized3();
    if ((index != 0 || DAT_104c00d0 == 0) && (direction.x != 0.0f || direction.y != 0.0f || direction.z != 0.0f))
    {
        direction.Copy(direction * (float)strength);
        DAT_10db6ee8[index].SetPendingPush(direction.x, direction.y, 0, flag);
    }
    return 0;
}
