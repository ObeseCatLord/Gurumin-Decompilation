// The player's two positional sound loops (effect ids 0x23c and 0x23b): each frame the player's positions of the last
// three frames give the loops a position and a velocity, so the sound process can apply its Doppler effect. Ported from
// the matched JP source decomp/src/doppler_loops.cpp.
#include <math.h>

#include "anim_model.h"
#include "camera.h"
#include "char.h"
#include "char_shots.h"
#include "doppler_loops.h"
#include "sound_effects.h"
#include "vec4.h"

// Player trail x value that marks "no position yet".
#define DOPPLER_UNSET_POSITION (-100000.0f)
// Effect ids of the two loops, and their volume.
const int DOPPLER_LOOP_ID_FIRST = 0x23c;
const int DOPPLER_LOOP_ID_SECOND = 0x23b;
#define DOPPLER_LOOP_VOLUME 1.0f
// Angles in radians (the game's own roundings of pi): a quarter and half turn, the wrap limits and a full turn.
#define PI_HALF 1.5708f
#define PI_FULL 3.14159f
#define PI_UPPER 3.1415f
#define PI_LOWER (-3.1415f)
#define TWO_PI 6.283f
// Entries of the row of g_shotParams of slot 0 these loops keep their state in.
const int DOPPLER_GAP_SLOT = 0;     // smoothed gap between the player and the older positions
const int DOPPLER_YAW_SLOT = 1;     // smoothed heading of the player's movement
const int DOPPLER_PITCH_SLOT = 2;   // smoothed pitch of the player's movement
// Smoothing and scaling of the gap, the angles and the positions sent to the sound process.
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
// The swing is doubled before it is squared into the depth.
#define SWING_DOUBLE 2.0f
// Distance behind the player (divided by the strength) the first loop sits at, scaled with the swing.
#define BEHIND_DISTANCE 200000.0f

// Heading of the vector (x, y) from the y axis, as the game's own float wrapper of atan2.
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Sound_UpdateDopplerLoops: g_playerTrail keeps the player's position of the last three frames (the oldest is dropped,
// the player's current position is added). While both older positions are unset the loops are started (both at the
// player) and the yaw state is set from the first step. Each frame the gap between the player's position and the older
// ones is smoothed (3/4 old + 1/4 new, minus 20) and becomes the strength (x 75, at least 75); the yaw and pitch of the
// player's movement are eased towards their goals (wrapped to a half turn, 0.15 new + 0.85 old, swung by 0.75 outside a
// 0.08727 dead zone). The first loop sits at a point behind the player, pushed away by the yaw and the strength, with a
// velocity from the speed; the second sits at the player and has none. Both are sent to the sound process. Returns 0.
// Called every frame by the scene frame.
extern "C" int Sound_UpdateDopplerLoops(void)
{
    if (g_playerTrail[2].x < DOPPLER_UNSET_POSITION && g_playerTrail[1].x > DOPPLER_UNSET_POSITION)
    {
        int first = Sound_PlayEffect(DOPPLER_LOOP_ID_FIRST, &g_playerTrail[0], DOPPLER_LOOP_VOLUME);
        int second = Sound_PlayEffect(DOPPLER_LOOP_ID_SECOND, &g_playerTrail[0], DOPPLER_LOOP_VOLUME);
        g_dopplerLoopFirst = first;
        g_dopplerLoopSecond = second;
        g_dopplerLoopsRunning = 1;
        g_shotParams[0][DOPPLER_GAP_SLOT] = 0.0f;
        g_shotParams[0][DOPPLER_YAW_SLOT] =
            Atan2(g_playerTrail[0].y - g_playerTrail[1].y, g_playerTrail[0].x - g_playerTrail[1].x) + PI_FULL;
        g_shotParams[0][DOPPLER_PITCH_SLOT] = 0.0f;
    }
    g_playerTrail[2].Copy(g_playerTrail[1]);
    g_playerTrail[1].Copy(g_playerTrail[0]);
    g_playerTrail[0].Copy(g_characterModels[0].matrix.GetTranslation());
    if (g_playerTrail[2].x > DOPPLER_UNSET_POSITION)
    {
        Vec4 step;
        Vec4 lead;
        step.Copy(g_playerTrail[0] - g_playerTrail[1]);
        lead.Copy(g_playerTrail[1] + step * LEAD_FRACTION - g_playerTrail[2]);
        float gap = lead.Length() - GAP_BASE;
        gap = g_shotParams[0][DOPPLER_GAP_SLOT] * GAP_OLD_WEIGHT + gap * GAP_NEW_WEIGHT;
        g_shotParams[0][DOPPLER_GAP_SLOT] = gap;
        if (gap < 1.0f)
            gap = 1.0f;
        gap = gap * GAP_TO_STRENGTH;
        float yawGoal = Atan2(lead.x, lead.y) + PI_FULL;
        float yawDelta = yawGoal - g_shotParams[0][DOPPLER_YAW_SLOT];
        while ((float)fabs(yawDelta) > PI_UPPER)
        {
            if (yawGoal - g_shotParams[0][DOPPLER_YAW_SLOT] > 0.0f)
                g_shotParams[0][DOPPLER_YAW_SLOT] += TWO_PI;
            else
                g_shotParams[0][DOPPLER_YAW_SLOT] -= TWO_PI;
            yawDelta = yawGoal - g_shotParams[0][DOPPLER_YAW_SLOT];
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
        yawGoal = g_shotParams[0][DOPPLER_YAW_SLOT] * ANGLE_OLD_WEIGHT + yawGoal * ANGLE_NEW_WEIGHT;
        g_shotParams[0][DOPPLER_YAW_SLOT] = yawGoal;
        swing = yawGoal - (Atan2(step.x, step.y) + PI_FULL);
        while (swing > PI_UPPER)
            swing -= TWO_PI;
        while (swing < PI_LOWER)
            swing += TWO_PI;
        swing = swing * SWING_DOUBLE;
        float pitchGoal;
        if (lead.Length() < MIN_SPEED)
            pitchGoal = 0.0f;
        else
        {
            pitchGoal = acosf(lead.Length2D() / lead.Length());
            if (lead.z < 0.0f)
                pitchGoal = pitchGoal * -1.0f;
        }
        float pitchDelta = pitchGoal - g_shotParams[0][DOPPLER_PITCH_SLOT];
        while ((float)fabs(pitchDelta) > PI_UPPER)
        {
            if (pitchGoal - g_shotParams[0][DOPPLER_PITCH_SLOT] > 0.0f)
                g_shotParams[0][DOPPLER_PITCH_SLOT] += TWO_PI;
            else
                g_shotParams[0][DOPPLER_PITCH_SLOT] -= TWO_PI;
            pitchDelta = pitchGoal - g_shotParams[0][DOPPLER_PITCH_SLOT];
        }
        pitchGoal = g_shotParams[0][DOPPLER_PITCH_SLOT] * ANGLE_OLD_WEIGHT + pitchGoal * ANGLE_NEW_WEIGHT;
        g_shotParams[0][DOPPLER_PITCH_SLOT] = pitchGoal;
        step.Fill(0.0f);
        if (gap < MIN_SPEED)
            step.y = FAR_DISTANCE;
        else
            step.y = -SWING_DEPTH - swing * swing * SWING_WIDTH - BEHIND_DISTANCE / gap;
        step.RotateX(pitchGoal);
        step.RotateZ(-yawGoal);
        lead.Copy(step + (g_cameraEye + g_cameraTarget) * 0.5f);
        step.Copy(g_playerTrail[1] - g_playerTrail[0]);
        step.Copy(step * SPEED_SCALE * gap * gap);
        lead.Copy(lead * POSITION_SCALE);
        step.Copy(step * VELOCITY_SCALE);
        g_soundMove(g_dopplerLoopFirst, &lead.x, &lead.y, &lead.z, &step.x, &step.y, &step.z);
        lead.Fill(0.0f);
        if (gap < MIN_SPEED)
            lead.y = FAR_DISTANCE;
        else
            lead.y = FAR_DISTANCE / gap;
        lead.RotateX(pitchGoal);
        lead.RotateZ(PI_FULL - Atan2(g_playerTrail[0].y - g_playerTrail[1].y, g_playerTrail[0].x - g_playerTrail[1].x));
        lead.Copy(lead + g_playerTrail[0]);
        step.Fill(0.0f);
        lead.Copy(lead * POSITION_SCALE);
        g_soundMove(g_dopplerLoopSecond, &lead.x, &lead.y, &lead.z, &step.x, &step.y, &step.z);
    }
    return 0;
}
