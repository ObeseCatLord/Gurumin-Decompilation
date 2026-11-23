// Camera mode 0x11 (run by the scene frame, FUN_100f5a80): the camera stands behind the player and swings round
// behind it when the player turns away from the character it follows.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "map_vars.h"
#include "render_vars.h"
#include "char_state.h"
#include "anim_model.h"
#include "last_position.h"

#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "camera_modes.h"

extern "C" void FUN_1016a8d0(void);

#define PI_UPPER 3.1415f
#define PI_LOWER (-3.1415f)
#define TWO_PI 6.28319f
#define CAMERA_BEHIND_DISTANCE 900.0f
#define CAMERA_FAR_DISTANCE 1100.0f
#define CAMERA_MIN_HEIGHT 120.0f
#define CAMERA_MIN_EYE_HEIGHT 50.0f
#define TURN_NOTICE_ANGLE 0.2618f
#define TURN_SHARP_ANGLE 0.787982f
#define TARGET_FAR_RANGE 250.0f
#define MAX_STEP 100.0f
#define BLEND_SLOT 1
#define BLEND_LIMIT 0.2f
#define BLEND_GROW 0.005f
#define BLEND_GROW_SHARP 0.01f
#define BLEND_SHRINK 0.05f
#define BLEND_MIN 0.05f
#define HEIGHT_KEEP 0.7f
#define HEIGHT_FROM_TARGET 0.3f

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10035050 (Camera_ChaseTarget). `player` is character 0's position and `target` the position of character
// `index` (the character the camera follows). The camera position goal starts 900 units behind the player's
// heading; while the player faces away from the target (more than 0.2618 rad) and the target is over 250 units
// away, the goal is turned about the player to look at the target, and the per-character blend weight
// (DAT_10255160[index][1]) grows (by 0.01 more on a sharp turn, up to 0.2). Otherwise the weight shrinks (not
// below 0.05) and the goal is pulled towards the position kept in DAT_10802bf8[index].step. The goal is blended
// with the saved position DAT_10da9150 by the weight, limited to 100 units a step, and the camera position and its
// copies are set to it. The camera target is the player's position lifted by 0.7 of its height plus 120 and 0.3 of
// the target's height (at least 120), eased the same way from the saved target DAT_10868818. Returns 0.
extern "C" int FUN_10035050(Vec4 *player, Vec4 *target, int index)
{
    Vec4 goal;
    Vec4 spare1;    // the original declares two more vectors that it never uses
    Vec4 spare2;
    Vec4 step;
    float blend;
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
    }
    goal.Fill(0.0f);
    goal.y = CAMERA_BEHIND_DISTANCE;
    goal.z = CAMERA_MIN_HEIGHT;
    float behind = PI_UPPER - Atan2(target->x - player->x, target->y - player->y);
    float turn = PI_UPPER - Atan2(player->x - DAT_10da9150.x, player->y - DAT_10da9150.y);
    turn = turn - behind;
    while (turn > PI_UPPER)
        turn -= TWO_PI;
    while (turn < PI_LOWER)
        turn += TWO_PI;
    float turnSize = (float)fabs(turn);
    if (turnSize > TURN_NOTICE_ANGLE && (*player - *target).Length2D() > TARGET_FAR_RANGE)
    {
        blend = DAT_10255160[index][BLEND_SLOT];
        if (DAT_10255160[index][BLEND_SLOT] < BLEND_LIMIT)
            DAT_10255160[index][BLEND_SLOT] += BLEND_GROW;
        float sharpness = (float)fabs(turn);
        if (sharpness > TURN_SHARP_ANGLE)
        {
            if (turn > 0.0f)
                behind -= turn - TURN_SHARP_ANGLE;
            else
                behind += turn + TURN_SHARP_ANGLE;
            if (DAT_10255160[index][BLEND_SLOT] < BLEND_LIMIT)
                DAT_10255160[index][BLEND_SLOT] += BLEND_GROW_SHARP;
        }
        goal.RotateZ(behind);
        goal.x = DAT_10db6ee8[0].matrix.m[12] + goal.x;
        goal.y = DAT_10db6ee8[0].matrix.m[13] + goal.y;
        DAT_10802bf8[index].step.Copy(goal);
        goal.Copy(goal * blend + DAT_10da9150 * (1.0f - blend));
        step.Copy(goal - DAT_10da9150);
        if (step.Length() > MAX_STEP)
        {
            step.Normalized3();
            step.Copy(step * MAX_STEP);
        }
        goal.Copy(DAT_10da9150 + step);
        behind = PI_UPPER - Atan2(player->x - goal.x, player->y - goal.y);
        goal.x = 0.0f;
        goal.y = CAMERA_FAR_DISTANCE;
        goal.RotateZ(behind);
        goal.x = DAT_10db6ee8[0].matrix.m[12] + goal.x;
        goal.y = DAT_10db6ee8[0].matrix.m[13] + goal.y;
    }
    else
    {
        goal.RotateZ(behind);
        goal.x = DAT_10db6ee8[0].matrix.m[12] + goal.x;
        goal.y = DAT_10db6ee8[0].matrix.m[13] + goal.y;
        DAT_10802bf8[index].step.Copy(goal);
        goal.x = 0.0f;
        goal.y = CAMERA_FAR_DISTANCE;
        if (DAT_10255160[index][BLEND_SLOT] > 0.0f)
            DAT_10255160[index][BLEND_SLOT] -= BLEND_SHRINK;
        if (DAT_10255160[index][BLEND_SLOT] < BLEND_MIN)
            DAT_10255160[index][BLEND_SLOT] = BLEND_MIN;
        blend = DAT_10255160[index][BLEND_SLOT];
        goal.RotateZ(turn + behind);
        goal.x = DAT_10db6ee8[0].matrix.m[12] + goal.x;
        goal.y = DAT_10db6ee8[0].matrix.m[13] + goal.y;
        goal.Copy(goal * (1.0f - blend) + DAT_10802bf8[index].step * blend);
        step.Copy(goal - DAT_10da9150);
        if (step.Length() > MAX_STEP)
        {
            step.Normalized3();
            step.Copy(step * MAX_STEP);
        }
        goal.Copy(DAT_10da9150 + step);
    }
    DAT_118e69e8.Copy(goal);
    DAT_11af3e28.Copy(goal);
    DAT_11af3f48.Copy(goal);
    if (!(CAMERA_MIN_EYE_HEIGHT < DAT_118e69e8.z))
        DAT_118e69e8.z = CAMERA_MIN_EYE_HEIGHT;
    goal.x = player->x;
    goal.y = player->y;
    float height = target->z * HEIGHT_FROM_TARGET + (player->z + CAMERA_MIN_HEIGHT) * HEIGHT_KEEP;
    goal.z = CAMERA_MIN_HEIGHT < height ? height : CAMERA_MIN_HEIGHT;
    step.Copy(goal - DAT_10868818);
    if (step.Length() > MAX_STEP)
    {
        step.Normalized3();
        step.Copy(step * MAX_STEP);
    }
    DAT_118e6cb8.Copy(DAT_10868818 + step);
    DAT_118e6c78.Copy(DAT_10868818 + step);
    DAT_10da9150.Copy(DAT_118e69e8);
    DAT_10868818.Copy(DAT_118e6cb8);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    return 0;
}
