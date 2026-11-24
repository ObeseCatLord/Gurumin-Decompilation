// The route flier (behaviour 0x21 of the per-kind update 0x100f9670): a character with a flapping wing node ("@E_010:")
// that flies along the route of its map record. It waits until the route leads upwards, then flies, steering sideways
// by random drifts whenever it strays 40 units from the route line, and bumps the player when it touches him. When it
// reaches the end of the route, or has hardly moved for 30 frames, it bursts in an effect, drops a coin piece if the
// slot carries money, shrinks away for five frames and is parked far off the map.

#include <math.h>
#include <stdlib.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_slot.h"

#include "char_state.h"

#include "effect_manager.h"

#include "char_tables.h"
#include "char_vars.h"
#include "map_vars.h"
#include "world.h"

#include "char_ground.h"
#include "debris.h"
#include "enemy_chase_hop.h"
#include "mole_game.h"
#include "route_gates.h"
#include "route_flier.h"

extern "C" int FUN_1013f880(void);   // the game's random number

Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360

// Makes a velocity component point towards positive y (or negative y): the flier is steered back to its route line.
// (Macros: the original function has an exception frame and its helpers are not expanded inline.)
#define KEEP_POSITIVE(velocity)     if ((velocity) < 0.0f)         (velocity) *= -1.0f;
#define KEEP_NEGATIVE(velocity)     if ((velocity) > 0.0f)         (velocity) *= -1.0f;

// 0x1007c050 (Char_RouteFlier). One frame of the flier `slot`. A slot told it is gone (state 3) bursts at once and
// returns 1. First call: finds the wing node and takes its offset as the flap pivot. Every 15th frame (by slot) it
// probes the ground below itself. It counts the frames it moved less than 10 units and bursts after 30. Mode 0 waits
// for a route leading upwards; mode 1 flies: the route's direction steers it, the wing flaps, a random sideways drift
// is added (and the velocity of the model flipped) when it strays from the route line, a bump into the map or the
// player knocks the player back, and it bursts at the route's end; mode 2 slides on for five frames, fading, then
// removes the model. Returns 0 (1 for a removed slot).
int FUN_1007c050(int slot)
{

    if (DAT_10210d30[slot] == FLIER_STATE_VANISHED)
    {
        Vec4 offset;
        Matrix4 matrix;
        Vec4 origin;
        offset.Fill(0.0f);
        origin.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
        matrix.SetIdentity();
        matrix.SetTranslation(origin);
        matrix = DAT_10db6ee8[slot].matrix;
        DAT_10daaa80.Start(FLIER_BURST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, &DAT_10219770[slot]);
    }
    DAT_10255a88[slot] = 0;
    if (DAT_10210d30[slot] != 0)
        return 1;
    if (DAT_1020b430[slot].flierReady == 0)
    {
        DAT_1020b430[slot].flierReady = 1;
        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find(FLIER_WING_NODE, 0);
        DAT_10802bf8[slot].step.Copy(DAT_1021bdf8[slot][0]->matrix.GetTranslation());
    }
    if (DAT_10210b30[slot] != 0)
        DAT_10210b30[slot] = 0;
    DAT_10daaae8[slot].stats[STAT_BUSY] = 1;
    int probePhase = DAT_104c0380;
    DAT_10218fc0[slot] = 0;
    DAT_1021f6c0[slot] = 1;
    DAT_10db6ee8[slot].alpha = 1.0f;
    if (probePhase % FLIER_PROBE_PERIOD == slot % FLIER_PROBE_PERIOD)
    {
        Vec4 ground;
        DAT_11b002c0 = DAT_10db6ee8[slot].collisionFilter;
        Vec4 start;
        Vec4 down;
        Vec4 spare;
        Vec4 hit;
        hit.Fill(0.0f);
        start.x = DAT_10db6ee8[slot].matrix.translation.x;
        start.y = DAT_10db6ee8[slot].matrix.translation.y;
        start.z = DAT_10db6ee8[slot].matrix.translation.z;
        start.w = DAT_10db6ee8[slot].matrix.translationW;
        hit.x = FLIER_NO_HIT;
        down.Copy(start);
        down.z -= FLIER_PROBE_DEPTH;
        if (DAT_10255f08 != 0)
            DAT_10255f08->CastSegment(0, FLIER_PROBE_MASK, &down, &start, &hit, 0);
        ground.Copy(hit);
        if (hit.x == FLIER_NO_HIT)
            DAT_1020b430[slot].flierGround = FLIER_NO_GROUND;
        else
            DAT_1020b430[slot].flierGround = (int)ground.z;
        DAT_10db6ee8[slot].m_0012 = 2;
        FUN_1001c090(slot, 1);
        DAT_11b002c0 = 0;
    }
    DAT_10db6ee8[slot].m_238c = 0;
    Vec4 last;
    last.x = (float)DAT_1020b430[slot].flierLastX;
    last.y = (float)DAT_1020b430[slot].flierLastY;
    last.z = (float)DAT_1020b430[slot].flierLastZ;
    if ((last - DAT_10db6ee8[slot].matrix.GetTranslation()).Length() < FLIER_STUCK_DISTANCE)
        DAT_1020b430[slot].flierStillFrames++;
    else
        DAT_1020b430[slot].flierStillFrames = 0;
    last.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
    DAT_1020b430[slot].flierLastX = (int)last.x;
    DAT_1020b430[slot].flierLastY = (int)last.y;
    DAT_1020b430[slot].flierLastZ = (int)last.z;
    if (DAT_1020b430[slot].flierStillFrames > FLIER_STUCK_FRAMES && DAT_1020b430[slot].flierMode < 2)
    {
        DAT_1020b430[slot].flierFrames = FLIER_BURST_FRAMES;
        DAT_1020b430[slot].flierMode = 2;
        Vec4 offset;
        EffectOptions options;
        Matrix4 matrix;
        Vec4 origin;
        offset.Fill(0.0f);
        origin.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
        matrix.SetIdentity();
        matrix.SetTranslation(origin);
        matrix = DAT_10db6ee8[slot].matrix;
        options.m_0004 = 0;
        options.m_0000 = 0;
        FUN_1001d8b0(slot, FLIER_BURST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
        if (DAT_10daaae8[slot].money != 0)
            FUN_1000dd90(DAT_10db6ee8[slot].matrix.translation.x, DAT_10db6ee8[slot].matrix.translation.y,
                         DAT_10db6ee8[slot].matrix.translation.z + FLIER_DEBRIS_LIFT, FLIER_DEBRIS_TYPE);
    }
    if (DAT_1020b430[slot].behaviour == 0 && DAT_1020b430[slot].flierMode != 2)
    {
        DAT_10255160[slot][FLIER_SPEED] = 0.0f;
        DAT_1020b430[slot].flierMode = 0;
        DAT_1020b430[slot].flierPoint = 0;
        if (DAT_10daaae8[slot].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[slot].mode = 0;
        return 0;
    }
    DAT_10db6ee8[slot].mode = 1;
    Vec4 pos;
    Vec4 dir;
    int type;
    int next;
    int param;
    int value;
    pos.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
    FUN_1002f050(slot, DAT_1020b430[slot].flierRoute, DAT_1020b430[slot].flierPoint, &next, &pos.x, &type, &param,
                 &value);
    dir.Copy(pos - DAT_10db6ee8[slot].matrix.GetTranslation());
    dir.Length2D();
    if (dir.x != 0.0f || dir.y != 0.0f || dir.z != 0.0f)
        dir.Normalized3();
    if (DAT_1020b430[slot].flierMode == 0)
    {
        if (!(dir.z > 0.0f))
            return 0;
        DAT_1020b430[slot].flierPoint = 1;
        DAT_1020b430[slot].flierMode = 1;
        DAT_10255160[slot][FLIER_SPEED] = FLIER_START_SPEED;
        float spread = (float)(FUN_1013f880() % FLIER_DRIFT_START_RANGE);
        DAT_10255160[slot][FLIER_DRIFT] = spread * FLIER_DRIFT_STEP - FLIER_DRIFT_START_CENTRE;
        pos.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
        FUN_1002f050(slot, DAT_1020b430[slot].flierRoute, DAT_1020b430[slot].flierPoint, &next, &pos.x, &type,
                     &param, &value);
        dir.Fill(0.0f);
        dir.x = 1.0f;
        if (DAT_1020b430[slot].flierRoute != FLIER_GROUP_ROUTE)
            dir.x = -1.0f;
        DAT_10db6ee8[slot].heading = DAT_10db6ee8[slot].targetHeading = (float)atan2(dir.x, dir.y) + FLIER_HALF_TURN;
        return 0;
    }
    if (DAT_1020b430[slot].flierMode == 1)
    {
        DAT_1021bdf8[slot][0]->matrix = RotationX(-DAT_10255160[slot][FLIER_ANGLE]) *
            TranslationMatrix(DAT_10802bf8[slot].step.x, DAT_10802bf8[slot].step.y, DAT_10802bf8[slot].step.z);
        DAT_10255160[slot][FLIER_ANGLE] += FLIER_FLAP_STEP;
        float speed = DAT_10255160[slot][FLIER_SPEED] + FLIER_SPEED_GAIN;
        DAT_10255160[slot][FLIER_SPEED] = speed;
        if (speed > FLIER_TOP_SPEED)
            DAT_10255160[slot][FLIER_SPEED] = FLIER_TOP_SPEED;
        dir.x = 0.0f;
        dir.y = DAT_10255160[slot][FLIER_SPEED] * -1.0f;
        dir.RotateZ(-DAT_10db6ee8[slot].heading);
        float side = DAT_10db6ee8[slot].matrix.translation.y - pos.y;
        if (side < -FLIER_DRIFT_EDGE)
        {
            float roll = (float)(FUN_1013f880() % FLIER_DRIFT_RANGE);
            DAT_10255160[slot][FLIER_DRIFT] = roll * FLIER_DRIFT_STEP;
            KEEP_POSITIVE(DAT_10db6ee8[slot].velocity.y)
            KEEP_POSITIVE(DAT_10db6ee8[slot].m_23e8.y)
            KEEP_POSITIVE(DAT_10db6ee8[slot].m_23f8.y)
            KEEP_POSITIVE(DAT_10db6ee8[slot].m_2408.y)
        }
        else if (side > FLIER_DRIFT_EDGE)
        {
            float roll = (float)(FUN_1013f880() % FLIER_DRIFT_RANGE);
            DAT_10255160[slot][FLIER_DRIFT] = roll * -FLIER_DRIFT_STEP;
            KEEP_NEGATIVE(DAT_10db6ee8[slot].velocity.y)
            KEEP_NEGATIVE(DAT_10db6ee8[slot].m_23e8.y)
            KEEP_NEGATIVE(DAT_10db6ee8[slot].m_23f8.y)
            KEEP_NEGATIVE(DAT_10db6ee8[slot].m_2408.y)
        }
        Vec4 probe;
        probe.Copy(dir);
        probe.y = DAT_10255160[slot][FLIER_DRIFT] * FLIER_DRIFT_PROBE + probe.y;
        if (!DAT_10db6ee8[slot].IsClearAny(&probe) && DAT_104686b0[0] == 0 && DAT_10daaae8[0].helmet != FLIER_HELMET_SAFE)
            FUN_10055ee0();
        probe.z = FLIER_PROBE_HEIGHT;
        probe.Copy(dir);
        probe.y = DAT_10255160[slot][FLIER_DRIFT] * FLIER_DRIFT_PROBE_FAR + probe.y;
        if (!DAT_10db6ee8[slot].IsClearOfMapLimited(&probe))
        {
            if (DAT_10db6ee8[slot].matrix.translation.y - pos.y < 0.0f)
            {
                float roll = (float)(FUN_1013f880() % FLIER_DRIFT_RANGE);
                DAT_10255160[slot][FLIER_DRIFT] = roll * FLIER_DRIFT_STEP;
                KEEP_POSITIVE(DAT_10db6ee8[slot].velocity.y)
                KEEP_POSITIVE(DAT_10db6ee8[slot].m_23e8.y)
                KEEP_POSITIVE(DAT_10db6ee8[slot].m_23f8.y)
                KEEP_POSITIVE(DAT_10db6ee8[slot].m_2408.y)
            }
            else
            {
                float roll = (float)(FUN_1013f880() % FLIER_DRIFT_RANGE);
                DAT_10255160[slot][FLIER_DRIFT] = roll * -FLIER_DRIFT_STEP;
                KEEP_NEGATIVE(DAT_10db6ee8[slot].velocity.y)
                KEEP_NEGATIVE(DAT_10db6ee8[slot].m_23e8.y)
                KEEP_NEGATIVE(DAT_10db6ee8[slot].m_23f8.y)
                KEEP_NEGATIVE(DAT_10db6ee8[slot].m_2408.y)
            }
        }
        FUN_100241e0(slot, dir.x, DAT_10255160[slot][FLIER_DRIFT] * FLIER_DRIFT_PROBE + dir.y, dir.z);
        if ((dir.x < -FLIER_TURN_DISTANCE && DAT_10db6ee8[slot].matrix.translation.x < pos.x && type == FLIER_ROUTE_END_TYPE) ||
            (dir.x > FLIER_TURN_DISTANCE && DAT_10db6ee8[slot].matrix.translation.x > pos.x && type == FLIER_ROUTE_END_TYPE))
            next = 0;
        if (next != DAT_1020b430[slot].flierPoint && type == FLIER_ROUTE_END_TYPE)
        {
            DAT_1020b430[slot].flierMode = 2;
            DAT_1020b430[slot].flierFrames = FLIER_BURST_FRAMES;
            Vec4 offset;
            EffectOptions options;
            Matrix4 matrix;
            Vec4 origin;
            offset.Fill(0.0f);
            origin.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
            matrix.SetIdentity();
            matrix.SetTranslation(origin);
            matrix = DAT_10db6ee8[slot].matrix;
            options.m_0004 = 0;
            options.m_0000 = 0;
            FUN_1001d8b0(slot, FLIER_BURST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
            if (DAT_10daaae8[slot].money != 0)
                FUN_1000dd90(DAT_10db6ee8[slot].matrix.translation.x, DAT_10db6ee8[slot].matrix.translation.y,
                             DAT_10db6ee8[slot].matrix.translation.z + FLIER_DEBRIS_LIFT, FLIER_DEBRIS_TYPE);
        }
        DAT_10802bf8[slot].position.Copy(dir);
        DAT_1020b430[slot].flierPoint = next;
        return 0;
    }
    if (DAT_1020b430[slot].flierMode != 2)
        return 0;
    DAT_1020b430[slot].flierFrames--;
    DAT_10db6ee8[slot].matrix.translation.x = DAT_10802bf8[slot].position.x + DAT_10db6ee8[slot].matrix.translation.x;
    DAT_10db6ee8[slot].matrix.translation.y = DAT_10802bf8[slot].position.y + DAT_10db6ee8[slot].matrix.translation.y;
    DAT_10db6ee8[slot].matrix.translation.z = DAT_10802bf8[slot].position.z + DAT_10db6ee8[slot].matrix.translation.z;
    if (DAT_1020b430[slot].flierFrames == 0)
    {
        DAT_1020b430[slot].flierPoint = 0;
        DAT_1020b430[slot].flierMode = 0;
        DAT_1020b430[slot].flierRoute = 0;
        DAT_1020b430[slot].flierFrames = 0;
        DAT_1020b430[slot].behaviour = 0;
        DAT_10255160[slot][FLIER_SPEED] = 0.0f;
        DAT_10db6ee8[slot].SetPosition(FLIER_NO_HIT, 0, 0);
        if (DAT_10c655e4 < FLIER_COUNT_LIMIT)
            DAT_10c655e4++;
    }
    DAT_10db6ee8[slot].alpha = (float)DAT_1020b430[slot].flierFrames * FLIER_FADE_STEP;
    return 0;
}
