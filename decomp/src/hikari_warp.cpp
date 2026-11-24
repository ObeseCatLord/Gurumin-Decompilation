// The warp light (behaviours 0x1f, 0x22 and 0x30 of the per-kind update 0x100f9670): a light that follows the route
// points of its map record. The player standing in it is taken along (hidden, held on the light) and put down again
// where the route ends, with dust effects; a character found at the light's start is carried with it.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "last_position.h"

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "map_records.h"
#include "game.h"

#include "hop_spit_enemy.h"
#include "hikari_warp.h"

// Puts the light `slot` (route record `record`) on its start point: the first route point of kind HIKARI_START_KIND
// when its record's script flag is set, else point 0; the position is unpacked from the point (kinds 6, 7 and 9 are
// relative: negative values are taken modulo HIKARI_RELATIVE_WRAP), the first target is looked up and the light's step
// is cleared. The original expands no inline function in this unit (Game::TestScriptFlag stays a call) and writes
// this out twice.
#pragma inline_depth(0)
#define PLACE_ON_ROUTE(slot, record) \
    { \
    int row = HIKARI_ROUTE_ROWS; \
    if (DAT_10210fc0[record].points[0].code != 0 && DAT_10874848.TestScriptFlag(DAT_10210fc0[record].points[0].code)) \
    { \
        row = 0; \
        while (row < HIKARI_ROUTE_ROWS && DAT_10210fc0[record].points[row].z != HIKARI_START_KIND) \
            row++; \
    } \
    if (row != HIKARI_ROUTE_ROWS) \
    { \
        DAT_1020b430[slot].hikariPoint = row; \
        DAT_1020b430[slot].targetPoint = row; \
    } \
    else \
    { \
        DAT_1020b430[slot].hikariPoint = 0; \
        DAT_1020b430[slot].targetPoint = 0; \
    } \
    const RoutePoint &start = DAT_10210fc0[0].points[record * MAP_RECORD_ROWS + DAT_1020b430[slot].targetPoint]; \
    int packed = start.x; \
    int part = packed / HIKARI_COORD_UNIT; \
    int rest = packed % HIKARI_COORD_UNIT; \
    if (part > HIKARI_COORD_LIMIT) \
        part -= HIKARI_COORD_UNIT; \
    float startX = (float)part; \
    DAT_10db6ee8[slot].matrix.m[12] = startX; \
    if (rest > HIKARI_COORD_LIMIT) \
        rest -= HIKARI_COORD_UNIT; \
    float startY = (float)rest; \
    DAT_10db6ee8[slot].matrix.m[13] = startY; \
    part = start.y / HIKARI_COORD_UNIT; \
    if (part > HIKARI_COORD_LIMIT) \
        part -= HIKARI_COORD_UNIT; \
    float startZ = (float)part; \
    int kind = start.z; \
    DAT_10db6ee8[slot].matrix.m[14] = startZ; \
    if (kind == 6 || kind == 7 || kind == 9) \
    { \
        if (kind == 9) \
        { \
            if (startX < 0.0f) \
                DAT_10db6ee8[slot].matrix.m[12] = startX + HIKARI_RELATIVE_WRAP; \
            if (startZ < 0.0f) \
                DAT_10db6ee8[slot].matrix.m[14] = startZ + HIKARI_RELATIVE_WRAP; \
        } \
        else \
        { \
            if (startX < 0.0f && kind == 6) \
                DAT_10db6ee8[slot].matrix.m[12] = startX + HIKARI_RELATIVE_WRAP; \
            if (startY < 0.0f) \
                DAT_10db6ee8[slot].matrix.m[13] = startY + HIKARI_RELATIVE_WRAP; \
            if (startZ < 0.0f && kind == 6) \
                DAT_10db6ee8[slot].matrix.m[14] = startZ + HIKARI_RELATIVE_WRAP; \
        } \
    } \
    target.Copy(matrix->GetTranslation()); \
    FUN_100286d0(slot, &DAT_1020b430[slot].hikariPoint, DAT_1020b430[slot].targetPoint, &next, &target, &group, \
                 &value, &action); \
    DAT_1020b430[slot].targetPoint = next; \
    target.Copy(matrix->GetTranslation()); \
    zero = 0.0f; \
    DAT_10802bf8[slot].step.Fill(zero); \
    DAT_1020b430[slot].hikariSound = 0; \
    }

// 0x1002ae70 (Char_HikariWarp). One frame of the warp light `slot`. Like the light of Char_HikariRide it sets itself up
// on its route (behaviour WARP_MODE_RESTART does it again), waits while its wait counter runs and otherwise follows its
// route: the speed counter rises to HIKARI_TOP_SPEED (and falls towards corners), the step is scaled by the variant
// (`variant` 0 and 1 by the speed counter, 2 at a fixed speed, WARP_MODE_STEADY by the counter again), a character
// found at the start is carried and obstacles stop it (WARP_MODE_STEADY) or turn it round on map WARP_MAP_SWAP.
// The player within WARP_NEAR_XY / WARP_NEAR_Z of it at the start counts as inside (variant 2 hides him and holds
// him on the light, with trails of dust behind); at the end of the route he is put down again with the same dust.
// Returns 0.
int Char_HikariWarp(int slot, int variant)
{
    Vec4 target;
    Vec4 move;
    Vec4 rider;
    Vec4 player;
    Matrix4 *matrix = &DAT_10db6ee8[slot].matrix;
    int value;
    int action;
    float gap;
    int next;
    int group;
    int lift;
    int inside;
    float zero;
    target.Copy(matrix->GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    next = 0;
    lift = 0;
    inside = 0;
    DAT_10db6ee8[slot].mode = 0;
    DAT_10db6ee8[slot].m_0294 = 1;
    gap = player.z - target.z;
    gap = (float)fabs((double)gap);
    if (gap < WARP_NEAR_Z && ((gap = player.x - target.x), (gap = (float)fabs((double)gap)), gap < WARP_NEAR_XY) &&
        ((gap = player.y - target.y), (gap = (float)fabs((double)gap)), gap < WARP_NEAR_XY))
    {
        inside = 1;
        DAT_10255160[slot][1] = WARP_INSIDE_FRAMES;
    }
    else if (DAT_10255160[slot][1] > 1.0f)
    {
        DAT_10255160[slot][1] = 0.0f;
        if (DAT_104c00c8 != 0)
            DAT_104c00c8 = 0;
    }
    DAT_10802bf8[slot].position.Copy(matrix->GetTranslation());
    if (DAT_10255160[slot][0] > 0.0f)
    {
        DAT_10255160[slot][0] -= 1.0f;
        DAT_10db6ee8[0].matrix.SetTranslation(matrix->GetTranslation());
    }
    if (DAT_1020b430[slot].hikariReady != HIKARI_READY)
    {
        int record = DAT_10db6ee8[slot].bankIndex;
        DAT_1020b430[slot].hikariReady = HIKARI_READY;
        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find(HIKARI_NODE, 0);
        PLACE_ON_ROUTE(slot, record)
    }
    if (DAT_1020b430[slot].behaviour == WARP_MODE_RESTART)
    {
        int record = DAT_10db6ee8[slot].bankIndex;
        DAT_1020b430[slot].behaviour = HIKARI_MODE_ENDED;
        DAT_1020b430[slot].hikariReady = HIKARI_READY;
        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find(HIKARI_NODE, 0);
        PLACE_ON_ROUTE(slot, record)
    }
    if ((DAT_10c635ec != 0 && DAT_10c62ed0 == HIKARI_MAP_A) ||
        (DAT_10c65600 != 0 && DAT_10c62ed0 == HIKARI_MAP_B && DAT_10c635bc == 0))
    {
        DAT_1020b430[slot].behaviour = HIKARI_MODE_RESET;
        DAT_10c635ec = 0;
        DAT_10c635bc = 1;
    }
    if (DAT_1020b430[slot].wait == 0 && DAT_1020b430[slot].behaviour != 0)
    {
        if (DAT_1020b430[slot].hikariSound == 0)
        {
            DAT_1020b430[slot].hikariSound = 1;
            if (DAT_1021bdf8[slot][0] != 0)
                DAT_1021bdf8[slot][0]->RestartColourPlugs("anime", DAT_104bf8a4);
        }
        DAT_1020b430[slot].hikariStopped = 0;
        if (DAT_1020b430[slot].behaviour == WARP_MODE_BOARDING)
            DAT_104c00c8 = 1;
        if (FUN_100286d0(slot, &DAT_1020b430[slot].hikariPoint, DAT_1020b430[slot].targetPoint, &next, &target, &group,
                         &value, &action) == 1 &&
            DAT_1020b430[slot].behaviour != 0)
        {
            if (DAT_1020b430[slot].behaviour == WARP_MODE_BOARDING)
                DAT_104c00c8 = 0;
            DAT_1020b430[slot].targetPoint = next;
            DAT_1020b430[slot].behaviour = HIKARI_MODE_ENDED;
            DAT_1020b430[slot].hikariHold = HIKARI_END_HOLD;
            DAT_1020b430[slot].hikariScanned = 0;
            DAT_10db6ee8[slot].alpha = 1.0f;
            if (variant != WARP_VARIANT_PLAYER)
                return 0;
            if (inside != 0)
            {
                DAT_10db6ee8[0].SetPosition(DAT_10db6ee8[slot].matrix.m[12], DAT_10db6ee8[slot].matrix.m[13],
                                            DAT_10db6ee8[slot].matrix.m[14]);
                DAT_10db6ee8[0].matrix.m[12] = DAT_10db6ee8[slot].matrix.m[12];
                DAT_10db6ee8[0].matrix.m[13] = DAT_10db6ee8[slot].matrix.m[13];
                DAT_10db6ee8[0].matrix.m[14] = DAT_10db6ee8[slot].matrix.m[14];
                DAT_10daaad4.Fill(0.0f);
                DAT_10db6ee8[0].alpha = 1.0f;
                DAT_104c00c8 = 0;
                DAT_10255160[slot][0] = WARP_DROP_FRAMES;
            }
            {
                Vec4 offset;
                Matrix4 trail;
                Matrix4 light;
                Matrix4 carried;
                light = RotationZ(DAT_10db6ee8[slot].heading);
                carried = light;
                Vec4 dir;
                offset.Fill(0.0f);
                EffectOptions options;
                int level = DAT_10c64ad4;
                options.flags = 1;
                dir.Copy(DAT_10802bf8[slot].position - matrix->GetTranslation());
                dir.Copy(dir * WARP_HALF_STEP);
                if (DAT_104c00fc != 0 && inside != 0)
                {
                    int count;
                    trail = DAT_104c00fc->worldMatrix;
                    for (count = WARP_TRAIL_COUNT; count != 0; count--)
                    {
                        if (DAT_10c655f0 != WARP_SPECIAL_COSTUME)
                        {
                            if (level == 0)
                                FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_0, offset, -1, 0, 0, 0, 0, &trail, &options);
                            else if (level == 1)
                                FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_1, offset, -1, 0, 0, 0, 0, &trail, &options);
                            else if (level == 2)
                                FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_2, offset, -1, 0, 0, 0, 0, &trail, &options);
                            else if (level == 3)
                                FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_3, offset, -1, 0, 0, 0, 0, &trail, &options);
                            FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_ALL, offset, -1, 0, 0, 0, 0, &trail, &options);
                        }
                        trail.m[12] = trail.m[12] + dir.x;
                        trail.m[13] = trail.m[13] + dir.y;
                        trail.m[14] = trail.m[14] + dir.z;
                    }
                }
                float angle = (float)atan2(dir.x, dir.y);
                trail = RotationZ(angle - WARP_TURN_AROUND);
                trail.SetTranslation(DAT_10db6ee8[0].matrix.GetTranslation());
                trail.m[12] = DAT_10db6ee8[0].bipNode->worldMatrix.m[12] - (dir.x + dir.x);
                trail.m[13] = DAT_10db6ee8[0].bipNode->worldMatrix.m[13] - (dir.y + dir.y);
                trail.m[14] = DAT_10db6ee8[0].bipNode->worldMatrix.m[14] - (dir.z + dir.z);
                light.SetTranslation(matrix->GetTranslation());
                if (DAT_1020b430[slot].hikariFollower != 0)
                {
                    carried.SetTranslation(DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].matrix.GetTranslation());
                    DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].alpha = 1.0f;
                }
                for (int count = WARP_TRAIL_COUNT; count != 0; count--)
                {
                    if (inside != 0)
                    {
                        if (DAT_10c655f0 == WARP_SPECIAL_COSTUME)
                            FUN_1001d8b0(slot, WARP_EFFECT_DUST_SPECIAL, offset, -1, -1, 0, 0, 0, &trail, &options);
                        else
                            FUN_1001d8b0(slot, WARP_EFFECT_DUST, offset, -1, -1, 0, 0, 0, &trail, &options);
                    }
                    trail.m[12] = trail.m[12] + dir.x;
                    trail.m[13] = trail.m[13] + dir.y;
                    trail.m[14] = trail.m[14] + dir.z;
                    DAT_10daaa80.Start(WARP_EFFECT_LIGHT, offset, -1, -1, 0, 0, 0, &light, &options,
                                       &DAT_10219770[slot]);
                    light.m[12] = light.m[12] + dir.x;
                    light.m[13] = light.m[13] + dir.y;
                    light.m[14] = light.m[14] + dir.z;
                    if (DAT_1020b430[slot].hikariFollower != 0)
                    {
                        if (DAT_1021f3c0[DAT_1020b430[slot].hikariFollower] == WARP_SPECIAL_KIND)
                            FUN_1001d8b0(slot, WARP_EFFECT_CARRIED_SPECIAL, offset, -1, -1, 0, 0, 0, &carried,
                                         &options);
                        else
                            FUN_1001d8b0(slot, WARP_EFFECT_CARRIED, offset, -1, -1, 0, 0, 0, &carried, &options);
                        carried.m[12] = carried.m[12] + dir.x;
                        carried.m[13] = carried.m[13] + dir.y;
                        carried.m[14] = carried.m[14] + dir.z;
                    }
                }
            }
            return 0;
        }
        if (DAT_1020b430[slot].hikariScanned == 0)
        {
            int carried;
            DAT_1020b430[slot].hikariScanned = 1;
            for (carried = HIKARI_FIRST_CARRIED; carried < HIKARI_END_CARRIED; carried++)
            {
                if (slot != carried &&
                    DAT_10db6ee8[slot].matrix.m[12] + HIKARI_REACH_XY > DAT_10db6ee8[carried].matrix.m[12] - HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[12] - HIKARI_REACH_XY < DAT_10db6ee8[carried].matrix.m[12] + HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[13] + HIKARI_REACH_XY > DAT_10db6ee8[carried].matrix.m[13] - HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[13] - HIKARI_REACH_XY < DAT_10db6ee8[carried].matrix.m[13] + HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[14] + HIKARI_REACH_Z > DAT_10db6ee8[carried].matrix.m[14] - HIKARI_REACH_Z &&
                    DAT_10db6ee8[slot].matrix.m[14] - HIKARI_REACH_Z < DAT_10db6ee8[carried].matrix.m[14] + HIKARI_REACH_Z)
                {
                    DAT_1020b430[slot].hikariFollower = carried;
                    DAT_104bfdd8 = carried;
                    break;
                }
            }
            if (carried == HIKARI_END_CARRIED)
                DAT_1020b430[slot].hikariFollower = 0;
        }
        DAT_10db6ee8[slot].motionFrame = DAT_104bf8a4;
        DAT_1020b430[slot].targetPoint = next;
        move.Copy(target - matrix->GetTranslation());
        if (DAT_1020b430[slot].hikariSpeed < HIKARI_TOP_SPEED)
            DAT_1020b430[slot].hikariSpeed++;
        if (group == 2 || group == 4)
        {
            if (move.Length() < (float)DAT_1020b430[slot].hikariSpeed * HIKARI_SLOW_SCALE)
                DAT_1020b430[slot].hikariSpeed =
                    (int)(HIKARI_SLOW_BASE - (HIKARI_SLOW_RANGE - move.Length()) * HIKARI_SLOW_FACTOR);
        }
        if (move.x != 0.0f || move.y != 0.0f || move.z != 0.0f)
            move.Normalized3();
        float scale;
        if (variant == WARP_VARIANT_HALF && DAT_1020b430[slot].behaviour != WARP_MODE_BOARDING &&
            DAT_1020b430[slot].behaviour != WARP_MODE_STEADY)
            scale = (float)DAT_1020b430[slot].hikariSpeed / HIKARI_HALF_DIVISOR;
        else if (variant == WARP_VARIANT_SLOW)
            scale = (float)DAT_1020b430[slot].hikariSpeed / WARP_SLOW_DIVISOR;
        else if (DAT_1020b430[slot].behaviour == WARP_MODE_STEADY)
            scale = (float)DAT_1020b430[slot].hikariSpeed / WARP_STEADY_DIVISOR;
        else
            scale = WARP_FIXED_SPEED;
        move.Copy(move * scale);
        if (DAT_1020b430[slot].behaviour == HIKARI_MODE_SHAKY)
        {
            if (FUN_1013f880() % 2 == 0 && variant == WARP_VARIANT_PLAYER)
            {
                move.x = (float)(FUN_1013f880() % WARP_WOBBLE_BIG_RANGE - WARP_WOBBLE_BIG_CENTRE) *
                             WARP_WOBBLE_BIG_SCALE + move.x;
                move.y = move.y + (float)(FUN_1013f880() % WARP_WOBBLE_BIG_RANGE - WARP_WOBBLE_BIG_CENTRE) *
                             WARP_WOBBLE_BIG_SCALE;
            }
            else if (FUN_1013f880() % WARP_WOBBLE_CALM_PERIOD == 0)
            {
                move.x = (float)(FUN_1013f880() % WARP_WOBBLE_CALM_RANGE - WARP_WOBBLE_CALM_CENTRE) *
                             WARP_WOBBLE_CALM_SCALE + move.x;
                move.y = move.y + (float)(FUN_1013f880() % WARP_WOBBLE_CALM_RANGE - WARP_WOBBLE_CALM_CENTRE) *
                             WARP_WOBBLE_CALM_SCALE;
            }
        }
        if (DAT_1020b430[slot].behaviour < HIKARI_MODE_RESUMED)
            rider.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        int rising = move.z > 0.0f ? WARP_LIFT_RISING : lift;
        if (DAT_1020b430[slot].hikariFollower != 0)
        {
            DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].SetPosition(
                move.x + DAT_10db6ee8[slot].matrix.m[12], move.y + DAT_10db6ee8[slot].matrix.m[13],
                move.z + DAT_10db6ee8[slot].matrix.m[14] + HIKARI_FOLLOWER_HEIGHT);
            if (DAT_104bfdd0 == 0)
                DAT_104bfdd8 = DAT_1020b430[slot].hikariFollower;
        }
        if (DAT_1020b430[slot].behaviour == HIKARI_MODE_RESET)
        {
            DAT_10db6ee8[slot].matrix.m[12] = target.x;
            DAT_10db6ee8[slot].matrix.m[13] = target.y;
            DAT_10db6ee8[slot].matrix.m[14] = target.z;
        }
        else if (DAT_1020b430[slot].behaviour == WARP_MODE_STEADY && rising == 0 &&
                 DAT_10db6ee8[slot].IsClearAny(&move) == 0)
        {
            DAT_1020b430[slot].behaviour = HIKARI_MODE_ENDED;
            DAT_1020b430[slot].hikariStopped = 1;
        }
        else
        {
            DAT_10db6ee8[slot].root->M_10194730(WARP_ANY_NODE_VALUE, WARP_NODE_MASK_A);
            DAT_10db6ee8[slot].root->M_10194730(WARP_ANY_NODE_VALUE, WARP_NODE_MASK_B);
            if (DAT_10c62ed0 == WARP_MAP_SWAP && rising == 0 && DAT_10db6ee8[slot].IsClearAny(&move) == 0)
            {
                int swapped = DAT_1020b430[slot].hikariPoint;
                DAT_1020b430[slot].state = DAT_1020b430[slot].state != 1;
                DAT_1020b430[slot].hikariPoint = DAT_1020b430[slot].targetPoint;
                DAT_1020b430[slot].targetPoint = swapped;
            }
            else
            {
                DAT_10db6ee8[slot].matrix.m[12] = move.x + DAT_10db6ee8[slot].matrix.m[12];
                DAT_10db6ee8[slot].matrix.m[13] = move.y + DAT_10db6ee8[slot].matrix.m[13];
                DAT_10db6ee8[slot].matrix.m[14] = move.z + DAT_10db6ee8[slot].matrix.m[14];
                if (variant == WARP_VARIANT_PLAYER && DAT_1020b430[slot].behaviour != WARP_MODE_STEADY)
                {
                    if (inside != 0)
                    {
                        DAT_10db6ee8[0].SetPosition(DAT_10db6ee8[slot].matrix.m[12], DAT_10db6ee8[slot].matrix.m[13],
                                                    DAT_10db6ee8[slot].matrix.m[14]);
                        DAT_10db6ee8[0].matrix.m[12] = DAT_10db6ee8[slot].matrix.m[12];
                        DAT_10db6ee8[0].matrix.m[13] = DAT_10db6ee8[slot].matrix.m[13];
                        DAT_10db6ee8[0].matrix.m[14] = DAT_10db6ee8[slot].matrix.m[14];
                        DAT_10daaad4.Fill(0.0f);
                        DAT_10db6ee8[0].alpha = 0.0f;
                        DAT_104c00c8 = 1;
                    }
                    DAT_10db6ee8[slot].alpha = 0.0f;
                    {
                        Vec4 offset;
                        Matrix4 trail;
                        Matrix4 light;
                        Matrix4 carried;
                        light = RotationZ(DAT_10db6ee8[slot].heading);
                        carried = light;
                        Vec4 dir;
                        offset.Fill(0.0f);
                        EffectOptions options;
                        int level = DAT_10c64ad4;
                        options.flags = 1;
                        dir.Copy(DAT_10802bf8[slot].position - matrix->GetTranslation());
                        dir.Copy(dir * WARP_HALF_STEP);
                        if (DAT_104c00fc != 0 && inside != 0)
                        {
                            int count;
                            trail = DAT_104c00fc->worldMatrix;
                            for (count = WARP_TRAIL_COUNT; count != 0; count--)
                            {
                                if (DAT_10c655f0 != WARP_SPECIAL_COSTUME)
                                {
                                    if (level == 0)
                                        FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_0, offset, -1, 0, 0, 0, 0, &trail,
                                                     &options);
                                    else if (level == 1)
                                        FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_1, offset, -1, 0, 0, 0, 0, &trail,
                                                     &options);
                                    else if (level == 2)
                                        FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_2, offset, -1, 0, 0, 0, 0, &trail,
                                                     &options);
                                    else if (level == 3)
                                        FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_3, offset, -1, 0, 0, 0, 0, &trail,
                                                     &options);
                                    FUN_1001d8b0(slot, WARP_EFFECT_TRAIL_ALL, offset, -1, 0, 0, 0, 0, &trail,
                                                 &options);
                                }
                                trail.m[12] = trail.m[12] + dir.x;
                                trail.m[13] = trail.m[13] + dir.y;
                                trail.m[14] = trail.m[14] + dir.z;
                            }
                        }
                        float angle = (float)atan2(dir.x, dir.y);
                        trail = RotationZ(angle - WARP_TURN_AROUND);
                        trail.SetTranslation(DAT_10db6ee8[0].matrix.GetTranslation());
                        trail.m[12] = DAT_10db6ee8[0].bipNode->worldMatrix.m[12] - (dir.x + dir.x);
                        trail.m[13] = DAT_10db6ee8[0].bipNode->worldMatrix.m[13] - (dir.y + dir.y);
                        trail.m[14] = DAT_10db6ee8[0].bipNode->worldMatrix.m[14] - (dir.z + dir.z);
                        light.SetTranslation(matrix->GetTranslation());
                        if (DAT_1020b430[slot].hikariFollower != 0)
                        {
                            carried.SetTranslation(
                                DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].matrix.GetTranslation());
                            DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].alpha = 0.0f;
                        }
                        for (int count = WARP_TRAIL_COUNT; count != 0; count--)
                        {
                            if (inside != 0)
                            {
                                if (DAT_10c655f0 == WARP_SPECIAL_COSTUME)
                                    DAT_10daaa80.Start(WARP_EFFECT_DUST_SPECIAL, offset, -1, -1, 0, 0, 0, &trail,
                                                       &options, &DAT_10219770[slot]);
                                else
                                    DAT_10daaa80.Start(WARP_EFFECT_DUST, offset, -1, -1, 0, 0, 0, &trail, &options,
                                                       &DAT_10219770[slot]);
                            }
                            trail.m[12] = trail.m[12] + dir.x;
                            trail.m[13] = trail.m[13] + dir.y;
                            trail.m[14] = trail.m[14] + dir.z;
                            DAT_10daaa80.Start(WARP_EFFECT_LIGHT, offset, -1, -1, 0, 0, 0, &light, &options,
                                               &DAT_10219770[slot]);
                            light.m[12] = light.m[12] + dir.x;
                            light.m[13] = light.m[13] + dir.y;
                            light.m[14] = light.m[14] + dir.z;
                            if (DAT_1020b430[slot].hikariFollower != 0)
                            {
                                DAT_10daaa80.Start(WARP_EFFECT_CARRIED, offset, -1, -1, 0, 0, 0, &carried, &options,
                                                   &DAT_10219770[slot]);
                                carried.m[12] = carried.m[12] + dir.x;
                                carried.m[13] = carried.m[13] + dir.y;
                                carried.m[14] = carried.m[14] + dir.z;
                            }
                        }
                    }
                }
            }
            DAT_10db6ee8[slot].root->M_10194730(WARP_END_NODE_VALUE, WARP_NODE_MASK_A);
            DAT_10db6ee8[slot].root->M_10194730(WARP_END_NODE_VALUE, WARP_NODE_MASK_B);
        }
        zero = 0.0f;
        DAT_10db6ee8[slot].velocity.Fill(zero);
        DAT_10802bf8[slot].step.Copy(move);
        return 0;
    }
    if (DAT_1020b430[slot].hikariStopped != 0)
    {
        if (DAT_10db6ee8[slot].IsClearAny(&DAT_10802bf8[slot].step) == 0)
            return 0;
        DAT_1020b430[slot].hikariStopped = 0;
        DAT_1020b430[slot].behaviour = (DAT_1020b430[slot].hikariFollower != 0) * 2 + HIKARI_MODE_RESUMED;
        return 0;
    }
    DAT_1020b430[slot].hikariSpeed = HIKARI_IDLE_SPEED;
    if (DAT_1020b430[slot].hikariSound == 0)
        return 0;
    DAT_1020b430[slot].hikariSound = 0;
    if (DAT_1021bdf8[slot][0] != 0)
        DAT_1021bdf8[slot][0]->RestartColourPlugs("stop", DAT_104bf8a4);
    return 0;
}
