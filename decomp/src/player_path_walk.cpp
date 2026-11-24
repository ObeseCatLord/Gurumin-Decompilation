// Walking the player along the camera path of the map (run from 0x100ba2a0 while the stick or mouse is moved): the
// input becomes a step of up to 20 units, the step is bent towards the nearest path segment, and the player moves
// unless another character stands within 100 units of the new position.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "key_track.h"

#include "node_data.h"

#include "node.h"

#include "anim_model.h"

#include "ui_vars.h"
#include "map_vars.h"

#include "path_walk.h"

#define PI_UPPER 3.1415f
#define PI_LOWER (-3.1415f)
#define TWO_PI 6.283f
#define HEADING_TURN_DIVISOR 3.0f
#define HEADING_SCALE 1000.0f
#define STEP_SCALE 0.35f
#define STEP_MAX 20.0f
#define STEP_ROUNDING 0.5f
#define FAR_DISTANCE 1000000.0f
#define KEY_JUMP_LIMIT 100.0f
#define PATH_NEAR_SQUARED 4.0f
#define PATH_MIN_SEGMENT_SQUARED 1.0f
#define PATH_ANCHOR_REACH 2.0f
#define BLOCK_DISTANCE 100.0f
#define CHARACTER_GONE_X (-10000.0f)
#define FIRST_OTHER_CHARACTER 1
#define OTHER_CHARACTER_END 0x7c
#define STANDING_STEP 3.0f

#define PATH (DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION])

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x1001f4d0 (Player_WalkAlongPath). The stick (stickX, stickY) gives a step of 0.35 times its length, at most 20,
// turned by a third of the way from the last heading (DAT_1020c30c) to the new one. The step's end point is kept
// near the nearest segment of the camera path (the segment between two keys whose positions do not jump, and whose
// markers have ground): far from it, the step heads for the segment's closest point or end key. The player moves
// there unless a character is within 100 units. The path key nearest to the player is stored (DAT_1020c314 and
// DAT_10c64a58), then the step's length and heading. Returns 0.
int Player_WalkAlongPath(int stickX, int stickY)
{
    Vec4 target;
    Vec4 start;
    Vec4 keyA;
    Vec4 keyB;
    int nearest = -1;
    target.x = (float)stickX;
    target.y = -(float)stickY;
    {
        float turn = Atan2(target.x, target.y) + PI_UPPER;
        if (DAT_1020c310 != 0)
        {
            turn = (float)DAT_1020c30c / HEADING_SCALE - turn;
            while (turn > PI_UPPER)
                turn = turn - TWO_PI;
            while (turn < PI_LOWER)
                turn = turn + TWO_PI;
            turn = turn / HEADING_TURN_DIVISOR;
            target.RotateZ(-turn);
        }
    }
    float nearestDistance = FAR_DISTANCE;
    target.Copy(target * STEP_SCALE);
    float length = target.Length2D();
    if (length > STEP_MAX)
    {
        length = STEP_MAX;
        target.Normalized3();
        target.Copy(target * STEP_MAX);
    }
    target.Copy(target + DAT_1086df38.GetTranslation());
    start.Copy(DAT_1086df38.GetTranslation());
    DAT_1020c308 = (int)(length + STEP_ROUNDING);
    {
        for (int i = 0; i < PATH.count - 1; i++)
        {
            if (DAT_104c0394 == 0 || (DAT_104c0394[i] == 0 && DAT_104c0394[i + 1] == 0))
            {
                keyA.Copy(DAT_104c03ac->extra->AnimatedMatrix(PATH.frames[i]).GetTranslation());
                keyB.Copy(DAT_104c03ac->extra->AnimatedMatrix(PATH.frames[i] + 1).GetTranslation());
                if (!((keyA - keyB).Length() > KEY_JUMP_LIMIT))
                {
                    float distance = target.DistanceSquaredToSegment(PATH.values[i], PATH.values[i + 1]);
                    if (distance < nearestDistance)
                    {
                        nearest = i;
                        nearestDistance = distance;
                    }
                }
            }
        }
        if (nearest > -1)
        {
            Vec4 toTarget;
            toTarget.Copy(target - PATH.values[nearest]);
            Vec4 segment;
            segment.Copy(PATH.values[nearest + 1] - PATH.values[nearest]);
            float along = segment.x * toTarget.x + segment.y * toTarget.y + segment.z * toTarget.z;
            float lengthSquared = segment.LengthSquared3();
            if (along < 0.0f)
                along = 0.0f;
            if (along > lengthSquared)
                along = lengthSquared;
            int key;
            if (lengthSquared > PATH_MIN_SEGMENT_SQUARED)
            {
                along = along / lengthSquared;
                target.z = segment.z * along + PATH.values[nearest].z;
                if (along < 0.5f)
                    key = PATH.frames[nearest];
                else
                    key = PATH.frames[nearest + 1];
            }
            else
            {
                key = PATH.frames[nearest];
            }
            DAT_1020c314 = key;
            DAT_10c64a58 = key;
        }
        if (!(nearestDistance < PATH_NEAR_SQUARED))
        {
            if (nearest <= -1)
                goto done;
            Vec4 toTarget;
            Vec4 segment;
            Vec4 closest;
            toTarget.Copy(target - PATH.values[nearest]);
            segment.Copy(PATH.values[nearest + 1] - PATH.values[nearest]);
            float along = segment.x * toTarget.x + segment.y * toTarget.y + segment.z * toTarget.z;
            float segmentMeasure = segment.LengthSquared3();
            if (!(segmentMeasure > PATH_MIN_SEGMENT_SQUARED))
                goto anchorKey;
            along = along / segmentMeasure;
            if (along < 0.0f)
                goto anchorKey;
            if (along > 1.0f)
                goto anchorNextKey;
            closest.Copy(segment * along + PATH.values[nearest]);
            toTarget.Copy(DAT_1086df38.GetTranslation() - PATH.values[nearest]);
            {
                segmentMeasure = (segment.x * toTarget.x + segment.y * toTarget.y + segment.z * toTarget.z) / segmentMeasure;
                if (along <= segmentMeasure)
                {
                    closest.Copy(segment * segmentMeasure + PATH.values[nearest]);
                    target.Copy(PATH.values[nearest] - closest);
                    target.Normalized3();
                    target.Copy(target * length);
                    target.Copy(target + PATH.values[nearest]);
                }
                else
                {
                    closest.Copy(segment * segmentMeasure + PATH.values[nearest]);
                    target.Copy(PATH.values[nearest + 1] - closest);
                    target.Normalized3();
                    target.Copy(target * length);
                    target.Copy(target + PATH.values[nearest + 1]);
                }
            }
            target.Copy(target - DAT_1086df38.GetTranslation());
            target.Normalized3();
            target.Copy(target * length);
            target.Copy(target + DAT_1086df38.GetTranslation());
            goto pulled;
        anchorNextKey:
            closest.Copy(PATH.values[nearest + 1]);
            goto anchored;
        anchorKey:
            closest.Copy(PATH.values[nearest]);
        anchored:
            target.Copy(target - closest);
            target.Normalized3();
            target.Copy(target * PATH_ANCHOR_REACH);
            target.Copy(target + closest);
        pulled:;
        }
        for (int i = FIRST_OTHER_CHARACTER; i < OTHER_CHARACTER_END; i++)
        {
            if (DAT_10db6ee8[i].matrix._41 > CHARACTER_GONE_X)
            {
                if ((DAT_10db6ee8[i].matrix.GetTranslation() - target).Length2D() < BLOCK_DISTANCE)
                    goto done;
            }
        }
        DAT_1086df38.SetTranslation(target);
    }
done:
    start.Copy(DAT_1086df38.GetTranslation() - start);
    DAT_1020c310 = 1;
    DAT_10e3dec0 = Atan2(start.x, start.y) + PI_UPPER;
    DAT_1020c30c = (int)((Atan2(start.x, start.y) + PI_UPPER) * HEADING_SCALE);
    if (start.Length() < STANDING_STEP)
    {
        target.y = -(float)stickY;
        target.x = (float)stickX;
        DAT_1020c30c = (int)((Atan2(target.x, target.y) + PI_UPPER) * HEADING_SCALE);
        DAT_10e3dec0 = Atan2(target.x, target.y) + PI_UPPER;
    }
    return 0;
}
