// Camera mode 0x11: the chase camera (EN build). Ported from the matched JP source decomp/src/camera_chase.cpp.
#include <math.h>

#include "camera.h"
#include "char.h"
#include "char_shots.h"
#include "colour_fades.h"
#include "scene_start.h"
#include "vec4.h"

#define PI_UPPER 3.1415f
#define PI_LOWER (-3.1415f)
#define TWO_PI 6.28319f
// Distance of the position goal behind the player's heading.
#define CAMERA_BEHIND_DISTANCE 900.0f
// Distance of the position goal once it is turned to look at the target.
#define CAMERA_FAR_DISTANCE 1100.0f
// Lowest height of the camera target and of the position goal.
#define CAMERA_MIN_HEIGHT 120.0f
// The camera position is never higher than this.
#define CAMERA_MIN_EYE_HEIGHT 50.0f
// Turn (radians) from which the camera starts to swing round, and the sharper turn that swings it faster.
#define TURN_NOTICE_ANGLE 0.2618f
#define TURN_SHARP_ANGLE 0.787982f
// The target must be further than this (x/y) for the swing.
#define TARGET_FAR_RANGE 250.0f
// Longest move per frame of the camera position and target.
#define MAX_STEP 100.0f
// Blend weight limits and rates (the weight lives in the heading slot of g_shotParams).
#define BLEND_LIMIT 0.2f
#define BLEND_GROW 0.005f
#define BLEND_GROW_SHARP 0.01f
#define BLEND_SHRINK 0.05f
#define BLEND_MIN 0.05f
// Share of the player's height and of the target's height in the camera target height.
#define HEIGHT_KEEP 0.7f
#define HEIGHT_FROM_TARGET 0.3f

// The chase camera: `player` is character 0's position and `target` the position of character `index` (the character
// the camera follows). The position goal starts 900 units behind the player's heading; while the player faces away
// from the target (more than 0.2618 rad) and the target is over 250 units away, the goal is turned about the player
// to look at the target and the per-character blend weight grows (more on a sharp turn, up to 0.2). Otherwise the
// weight shrinks (not below 0.05) and the goal is pulled towards the position kept in the character's shot slot. The
// goal is blended with the saved player position by the weight, limited to 100 units a step, and the camera position
// and its copies are set to it. The camera target is the player's position lifted by 0.7 of its height plus 120 and
// 0.3 of the target's height (at least 120), eased the same way from the saved target. Returns 0. Called every frame
// while the camera runs this mode.
extern "C" int Camera_ChaseTarget(Vec4 *player, Vec4 *target, int index)
{
    Vec4 goal;
    Vec4 spare1;    // the original declares two more vectors that it never uses
    Vec4 spare2;
    Vec4 step;
    float blend;
    if (g_cameraShotStarted == 0)
    {
        Camera_SetMode2();
        g_cameraShotStarted = 1;
    }
    goal.Fill(0.0f);
    goal.y = CAMERA_BEHIND_DISTANCE;
    goal.z = CAMERA_MIN_HEIGHT;
    float behind = PI_UPPER - (float)atan2(target->x - player->x, target->y - player->y);
    float turn = PI_UPPER - (float)atan2(player->x - g_savedPlayerPosition.x, player->y - g_savedPlayerPosition.y);
    turn = turn - behind;
    while (turn > PI_UPPER)
        turn -= TWO_PI;
    while (turn < PI_LOWER)
        turn += TWO_PI;
    float turnSize = (float)fabs(turn);
    if (turnSize > TURN_NOTICE_ANGLE && (*player - *target).Length2D() > TARGET_FAR_RANGE)
    {
        blend = g_shotParams[index][SHOT_HEADING];
        if (g_shotParams[index][SHOT_HEADING] < BLEND_LIMIT)
            g_shotParams[index][SHOT_HEADING] += BLEND_GROW;
        if (turnSize > TURN_SHARP_ANGLE)
        {
            if (turn > 0.0f)
                behind -= turn - TURN_SHARP_ANGLE;
            else
                behind += turn + TURN_SHARP_ANGLE;
            if (g_shotParams[index][SHOT_HEADING] < BLEND_LIMIT)
                g_shotParams[index][SHOT_HEADING] += BLEND_GROW_SHARP;
        }
        goal.RotateZ(behind);
        goal.x = g_characterModels[0].matrix.m[12] + goal.x;
        goal.y = g_characterModels[0].matrix.m[13] + goal.y;
        g_shotSlots[index].step.Copy(goal);
        goal.Copy(goal * blend + g_savedPlayerPosition * (1.0f - blend));
        step.Copy(goal - g_savedPlayerPosition);
        if (step.Length() > MAX_STEP)
        {
            step.Normalized3();
            step.Copy(step * MAX_STEP);
        }
        goal.Copy(g_savedPlayerPosition + step);
        behind = PI_UPPER - (float)atan2(player->x - goal.x, player->y - goal.y);
        goal.x = 0.0f;
        goal.y = CAMERA_FAR_DISTANCE;
        goal.RotateZ(behind);
        goal.x = g_characterModels[0].matrix.m[12] + goal.x;
        goal.y = g_characterModels[0].matrix.m[13] + goal.y;
    }
    else
    {
        goal.RotateZ(behind);
        goal.x = g_characterModels[0].matrix.m[12] + goal.x;
        goal.y = g_characterModels[0].matrix.m[13] + goal.y;
        g_shotSlots[index].step.Copy(goal);
        goal.x = 0.0f;
        goal.y = CAMERA_FAR_DISTANCE;
        if (g_shotParams[index][SHOT_HEADING] > 0.0f)
            g_shotParams[index][SHOT_HEADING] -= BLEND_SHRINK;
        if (g_shotParams[index][SHOT_HEADING] < BLEND_MIN)
            g_shotParams[index][SHOT_HEADING] = BLEND_MIN;
        blend = g_shotParams[index][SHOT_HEADING];
        goal.RotateZ(turn + behind);
        goal.x = g_characterModels[0].matrix.m[12] + goal.x;
        goal.y = g_characterModels[0].matrix.m[13] + goal.y;
        goal.Copy(goal * (1.0f - blend) + g_shotSlots[index].step * blend);
        step.Copy(goal - g_savedPlayerPosition);
        if (step.Length() > MAX_STEP)
        {
            step.Normalized3();
            step.Copy(step * MAX_STEP);
        }
        goal.Copy(g_savedPlayerPosition + step);
    }
    g_cameraPositionNow.Copy(goal);
    g_cameraPositionCopy.Copy(goal);
    g_colourEffectColour.Copy(goal);
    if (!(CAMERA_MIN_EYE_HEIGHT < g_cameraPositionNow.z))
        g_cameraPositionNow.z = CAMERA_MIN_EYE_HEIGHT;
    goal.x = player->x;
    goal.y = player->y;
    float height = target->z * HEIGHT_FROM_TARGET + (player->z + CAMERA_MIN_HEIGHT) * HEIGHT_KEEP;
    goal.z = CAMERA_MIN_HEIGHT < height ? height : CAMERA_MIN_HEIGHT;
    step.Copy(goal - g_savedPlayerVelocity);
    if (step.Length() > MAX_STEP)
    {
        step.Normalized3();
        step.Copy(step * MAX_STEP);
    }
    g_cameraTargetNow.Copy(g_savedPlayerVelocity + step);
    g_cameraTargetCopy.Copy(g_savedPlayerVelocity + step);
    g_savedPlayerPosition.Copy(g_cameraPositionNow);
    g_savedPlayerVelocity.Copy(g_cameraTargetNow);
    g_cameraTargetCopy.Copy(g_cameraTargetNow);
    g_cameraPositionCopy.Copy(g_cameraPositionNow);
    g_cameraTarget.Copy(g_cameraTargetNow);
    g_cameraEye.Copy(g_cameraPositionNow);
    return 0;
}
