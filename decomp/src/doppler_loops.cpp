// The player's two positional sound loops (effect ids 0x23c and 0x23b): each frame the player's positions of the last
// three frames give the loops a position and a velocity, so the sound process can apply its Doppler effect.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "anim_model.h"
#include "map_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "camera_vars.h"
#include "doppler_loops.h"

#define UNSET_POSITION (-100000.0f)
#define LOOP_ID_FIRST 0x23c
#define LOOP_ID_SECOND 0x23b
#define LOOP_VOLUME 1.0f
#define PI_HALF 1.5708f
#define PI_FULL 3.14159f
#define PI_UPPER 3.1415f
#define PI_LOWER (-3.1415f)
#define TWO_PI 6.283f
#define SLOT_GAP 0
#define SLOT_YAW 1
#define SLOT_PITCH 2
#define GAP_BASE 20.0f
#define GAP_NEW_WEIGHT 0.25f
#define GAP_OLD_WEIGHT 0.75f
#define GAP_TO_STRENGTH 75.0f
#define LEAD_FRACTION 0.25f
#define ANGLE_NEW_WEIGHT 0.15f
#define ANGLE_OLD_WEIGHT 0.85f
#define SWING_DEAD_ZONE 0.08727f
#define SWING_PULL 0.75f
#define SPEED_SCALE 4e-06f
#define VELOCITY_SCALE 0.15f
#define POSITION_SCALE 0.15f
#define FAR_DISTANCE (-200000.0f)
#define SWING_DEPTH 100.0f
#define SWING_WIDTH 7.0f
#define MIN_SPEED 1.0f

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x1004fbe0 (Sound_UpdateDopplerLoops). DAT_10edbd58 keeps the player's position of the last three frames (the oldest
// is dropped, the player's current position is added). While both older positions are unset the loops are started
// (both at the player) and the yaw state is set from the first step. Each frame the gap between the player's position
// and the older ones is smoothed (DAT_10255160[0][0], 3/4 old + 1/4 new, minus 20) and becomes the strength (x 75, at
// least 75); the yaw (DAT_10255160[0][1]) and pitch (DAT_10255160[0][2]) of the player's movement are eased towards
// their goals (wrapped to a half turn, 0.15 new + 0.85 old, swung by 0.75 outside a 0.08727 dead zone). The first
// loop sits at a point behind the player, pushed away by the yaw and the strength, with a velocity from the speed;
// the second sits at the player and has none. Both are sent to the sound process. Returns 0.
extern "C" int FUN_1004fbe0(void)
{
    if (DAT_10edbd58[2].x < UNSET_POSITION && DAT_10edbd58[1].x > UNSET_POSITION)
    {
        int first = FUN_1003ad20(LOOP_ID_FIRST, &DAT_10edbd58[0], LOOP_VOLUME);
        int second = FUN_1003ad20(LOOP_ID_SECOND, &DAT_10edbd58[0], LOOP_VOLUME);
        DAT_10c635d4 = first;
        DAT_10c635d8 = second;
        DAT_10c635dc = 1;
        DAT_10255160[0][SLOT_GAP] = 0.0f;
        float startYaw = Atan2(DAT_10edbd58[0].y - DAT_10edbd58[1].y, DAT_10edbd58[0].x - DAT_10edbd58[1].x);
        DAT_10255160[0][SLOT_PITCH] = 0.0f;
        DAT_10255160[0][SLOT_YAW] = startYaw + PI_FULL;
    }
    DAT_10edbd58[2].Copy(DAT_10edbd58[1]);
    DAT_10edbd58[1].Copy(DAT_10edbd58[0]);
    DAT_10edbd58[0].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    if (DAT_10edbd58[2].x > UNSET_POSITION)
    {
        Vec4 step;
        Vec4 lead;
        step.Copy(DAT_10edbd58[0] - DAT_10edbd58[1]);
        lead.Copy(DAT_10edbd58[1] + step * LEAD_FRACTION - DAT_10edbd58[2]);
        float gap = lead.Length() - GAP_BASE;
        gap = DAT_10255160[0][SLOT_GAP] * GAP_OLD_WEIGHT + gap * GAP_NEW_WEIGHT;
        DAT_10255160[0][SLOT_GAP] = gap;
        if (gap < 1.0f)
            gap = 1.0f;
        gap = gap * GAP_TO_STRENGTH;
        float yawGoal = Atan2(lead.x, lead.y) + PI_FULL;
        float yawDelta = yawGoal - DAT_10255160[0][SLOT_YAW];
        while ((float)fabs(yawDelta) > PI_UPPER)
        {
            if (yawGoal - DAT_10255160[0][SLOT_YAW] > 0.0f)
                DAT_10255160[0][SLOT_YAW] += TWO_PI;
            else
                DAT_10255160[0][SLOT_YAW] -= TWO_PI;
            yawDelta = yawGoal - DAT_10255160[0][SLOT_YAW];
        }
        float swing = yawGoal - (Atan2(step.x, step.y) + PI_FULL);
        while (swing > PI_UPPER)
            swing -= TWO_PI;
        while (swing < PI_LOWER)
            swing += TWO_PI;
        if (swing < -SWING_DEAD_ZONE)
        {
            swing = (swing + PI_HALF) * -SWING_PULL;
            yawGoal = swing + yawGoal;
        }
        if (swing > SWING_DEAD_ZONE)
        {
            swing = (PI_HALF - swing) * SWING_PULL;
            yawGoal = swing + yawGoal;
        }
        yawGoal = DAT_10255160[0][SLOT_YAW] * ANGLE_OLD_WEIGHT + yawGoal * ANGLE_NEW_WEIGHT;
        DAT_10255160[0][SLOT_YAW] = yawGoal;
        swing = yawGoal - (Atan2(step.x, step.y) + PI_FULL);
        while (swing > PI_UPPER)
            swing -= TWO_PI;
        while (swing < PI_LOWER)
            swing += TWO_PI;
        swing = swing + swing;
        float pitchGoal;
        if (lead.Length() < MIN_SPEED)
            pitchGoal = 0.0f;
        else
        {
            pitchGoal = acosf(lead.Length2D() / lead.Length());
            if (lead.z < 0.0f)
                pitchGoal = pitchGoal * -1.0f;
        }
        float pitchDelta = pitchGoal - DAT_10255160[0][SLOT_PITCH];
        while ((float)fabs(pitchDelta) > PI_UPPER)
        {
            if (pitchGoal - DAT_10255160[0][SLOT_PITCH] > 0.0f)
                DAT_10255160[0][SLOT_PITCH] += TWO_PI;
            else
                DAT_10255160[0][SLOT_PITCH] -= TWO_PI;
            pitchDelta = pitchGoal - DAT_10255160[0][SLOT_PITCH];
        }
        pitchGoal = DAT_10255160[0][SLOT_PITCH] * ANGLE_OLD_WEIGHT + pitchGoal * ANGLE_NEW_WEIGHT;
        DAT_10255160[0][SLOT_PITCH] = pitchGoal;
        step.Fill(0.0f);
        if (gap < MIN_SPEED)
            step.y = FAR_DISTANCE;
        else
            step.y = -SWING_DEPTH - swing * swing * SWING_WIDTH - 200000.0f / gap;
        step.RotateX(pitchGoal);
        step.RotateZ(-yawGoal);
        lead.Copy(step + (DAT_10f65688 + DAT_10f668e8) * 0.5f);
        step.Copy(DAT_10edbd58[1] - DAT_10edbd58[0]);
        step.Copy(step * SPEED_SCALE * gap * gap);
        lead.Copy(lead * POSITION_SCALE);
        step.Copy(step * VELOCITY_SCALE);
        FUN_10001890(DAT_10c635d4, &lead.x, &lead.y, &lead.z, &step.x, &step.y, &step.z);
        lead.Fill(0.0f);
        if (gap < MIN_SPEED)
            lead.y = FAR_DISTANCE;
        else
            lead.y = FAR_DISTANCE / gap;
        lead.RotateX(pitchGoal);
        lead.RotateZ(PI_FULL - Atan2(DAT_10edbd58[0].y - DAT_10edbd58[1].y, DAT_10edbd58[0].x - DAT_10edbd58[1].x));
        lead.Copy(lead + DAT_10edbd58[0]);
        step.Fill(0.0f);
        lead.Copy(lead * POSITION_SCALE);
        FUN_10001890(DAT_10c635d8, &lead.x, &lead.y, &lead.z, &step.x, &step.y, &step.z);
    }
    return 0;
}
