// Debris pieces update (the pool described in debris.cpp): falling, bouncing off the map, and flying to the player.

#include <stdlib.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_vars.h"
#include "game_vars.h"

extern "C" int FUN_1000e750(void);                                                     // anti-tamper guard refresh
extern "C" int FUN_10012130(int *item, int *count);                                    // Inventory_Add
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);            // Sound_PlayEffect

#define DEBRIS_COUNT 0x100
#define DEBRIS_SIMPLE_COUNT 0x80
#define DEBRIS_MAX_FRAMES 0x384
#define DEBRIS_STATE_FALLING 0
#define DEBRIS_STATE_LANDED 1
#define DEBRIS_STATE_FIRST_FLY 2
#define DEBRIS_STATE_LAST_FLY 5
#define DEBRIS_STATE_FALLEN 0x14
#define DEBRIS_FLY_DONE_STATE 5
#define DEBRIS_TYPE_COIN_1 0x51
#define DEBRIS_TYPE_COIN_5 0x52
#define DEBRIS_TYPE_COIN_10 0x53
#define DEBRIS_TYPE_COIN_50 0x54
#define DEBRIS_TYPE_COIN_100 0x55
#define DEBRIS_TYPE_COIN_500 0x56
#define DEBRIS_TYPE_NO_ITEM 0x50
#define DEBRIS_MIN_FRAMES_TO_LAND 10
#define DEBRIS_LAND_RADIUS_BASE 80
#define DEBRIS_LAND_RADIUS_PER_SIZE 60
#define DEBRIS_PIT_Z -3450.0f
#define DEBRIS_PROBE_RADIUS 20.0f
#define DEBRIS_GRAVITY 1.5
#define DEBRIS_FALL_LIMIT -20.0f
#define DEBRIS_GROUND_NORMAL_Z -0.5
#define DEBRIS_FLAT_NORMAL_Z 0.8
#define DEBRIS_FLAT_BOUNCE_SCALE 0.6f
#define DEBRIS_BOUNCE_KICK 10.0f
#define DEBRIS_SLIDE_DRAG 0.8f
#define DEBRIS_REST_SPEED 4
#define DEBRIS_MAX_BOUNCES 8
#define DEBRIS_PICKUP_FLAG 8
#define SOUND_DEBRIS_COIN 0x390
#define SOUND_DEBRIS_ITEM 0x391

// 0x100564d0 (Debris_Update). Steps every used piece one frame; a piece lives at most 900 frames. Slots 0-127: a
// falling piece moves by its velocity unless the drop collision is in the way; on a hit it bounces off the surface
// (kicked along the normal, slowed on flat ground, resting after a few bounces) and after 10 frames it lands when it
// touches the player's node tree (state 1 first, then 2). Fallen pieces (state 0x14, below -3450) and landed ones
// (states 2-5) fly towards the player in 6 - state steps; at state 5 the piece is picked up: coins 0x51-0x56 add
// 1/5/10/50/100/500 to the player's coin count, other pieces except 0x50 go to the inventory, each with a sound
// unless the piece had fallen. Slots 128-255 (thrown pieces) only fall under gravity and spin. Returns 0.
int FUN_100564d0(void)
{
    Vec4 playerPosition = DAT_10db6ee8[0].matrix.GetTranslation();
    Matrix4 identity;
    identity.SetIdentity();
    int i;
    for (i = 0; i < DEBRIS_SIMPLE_COUNT; i++)
    {
        if (DAT_1030bdf8[i] == 0)
            continue;
        DAT_104469f8[i]++;
        DAT_104465f8[i]++;
        Vec4 next;
        Vec4 normal;
        Vec4 push;
        if ((unsigned int)DAT_104469f8[i] > DEBRIS_MAX_FRAMES)
        {
            DAT_1030bdf8[i] = 0;
            continue;
        }
        unsigned char state = DAT_1021b628[i];
        if ((state <= DEBRIS_STATE_LANDED || state > DEBRIS_STATE_LAST_FLY) && state != DEBRIS_STATE_FALLEN)
        {
            next.Copy(DAT_10da9400[i] + DAT_10c66ef0[i]);
            if ((unsigned int)DAT_104469f8[i] > DEBRIS_MIN_FRAMES_TO_LAND)
            {
                float radius = (float)(DAT_102195f0[i] * DEBRIS_LAND_RADIUS_PER_SIZE + DEBRIS_LAND_RADIUS_BASE);
                if (DAT_10db6ee8[0].root->FindTouchingSphere((int)&DAT_10db6ee8[0].matrix, &DAT_10da9400[i], radius, 0,
                                                             0) != 0)
                {
                    if (DAT_1021b628[i] == DEBRIS_STATE_FALLING)
                    {
                        DAT_1021b628[i] = DEBRIS_STATE_LANDED;
                        DAT_10c66ef0[i].x = 0;
                        DAT_10c66ef0[i].y = 0;
                    }
                    else
                        DAT_1021b628[i] = DEBRIS_STATE_FIRST_FLY;
                }
            }
            if (next.z < DEBRIS_PIT_Z)
            {
                DAT_1021b628[i] = DEBRIS_STATE_FALLEN;
                DAT_10c66ef0[i].x = 0;
                DAT_10c66ef0[i].y = 0;
            }
            if (DAT_104677a0[i] < DEBRIS_MAX_BOUNCES)
            {
                float depth;
                if (DAT_10db6ee8[0].dropCollider->Query(&identity, &next, DEBRIS_PROBE_RADIUS, &normal, &depth, 0, 0) ==
                    0)
                {
                    DAT_104677a0[i] = 0;
                    if (DAT_102195f0[i] == 0)
                    {
                        float fall = DAT_10c66ef0[i].z - DEBRIS_GRAVITY;
                        DAT_10c66ef0[i].z = fall;
                        if (fall < DEBRIS_FALL_LIMIT)
                            DAT_10c66ef0[i].z = DEBRIS_FALL_LIMIT;
                    }
                    else
                    {
                        float drag = DEBRIS_SLIDE_DRAG;
                        DAT_10c66ef0[i].Copy(DAT_10c66ef0[i] * drag);
                    }
                    DAT_10da9400[i].Copy(next);
                }
                else
                {
                    normal.Normalized3();
                    if (DAT_10db6ee8[0].dropCollider->Query(&identity, &DAT_10da9400[i], DEBRIS_PROBE_RADIUS, &push,
                                                            &depth, 0, 0) == 0)
                    {
                        if (normal.z < DEBRIS_GROUND_NORMAL_Z)
                        {
                            if (DAT_10c66ef0[i].z > 0.0f)
                                DAT_10c66ef0[i].z = -DAT_10c66ef0[i].z;
                        }
                        else
                        {
                            float speed = (1.0f - (normal.z + normal.z)) * DAT_10c66ef0[i].z;
                            if (DAT_104677a0[i] % 2 == 0)
                            {
                                if (normal.z > DEBRIS_FLAT_NORMAL_Z)
                                    speed = speed * DEBRIS_FLAT_BOUNCE_SCALE;
                                DAT_10c66ef0[i].z = speed;
                                DAT_10c66ef0[i].x = normal.x * DEBRIS_BOUNCE_KICK;
                                DAT_10c66ef0[i].y = normal.y * DEBRIS_BOUNCE_KICK;
                            }
                            else
                            {
                                if (normal.z > DEBRIS_FLAT_NORMAL_Z)
                                    speed = speed * DEBRIS_FLAT_BOUNCE_SCALE;
                                DAT_10c66ef0[i].x = 0;
                                DAT_10c66ef0[i].y = 0;
                                DAT_10c66ef0[i].z = speed;
                            }
                        }
                    }
                    else
                    {
                        push.Normalized3();
                        DAT_10c66ef0[i].x = push.x * DEBRIS_BOUNCE_KICK + DAT_10c66ef0[i].x;
                        DAT_10c66ef0[i].y = push.y * DEBRIS_BOUNCE_KICK + DAT_10c66ef0[i].y;
                    }
                    if (normal.z > DEBRIS_FLAT_NORMAL_Z && abs((int)DAT_10c66ef0[i].z) < DEBRIS_REST_SPEED)
                    {
                        DAT_10c66ef0[i].z = 0;
                        DAT_104677a0[i] = DEBRIS_MAX_BOUNCES;
                    }
                    DAT_104677a0[i]++;
                }
            }
            continue;
        }
        {
            if (state != DEBRIS_STATE_FALLEN)
            {
                float steps = (float)(DEBRIS_STATE_LAST_FLY + 1 - state);
                DAT_10da9400[i].Copy((playerPosition - DAT_10da9400[i]).Divided(steps) + DAT_10da9400[i]);
            }
            if (DAT_1021b628[i] != DEBRIS_STATE_FALLEN)
                DAT_1021b628[i]++;
            if (DAT_1021b628[i] >= DEBRIS_FLY_DONE_STATE)
            {
                int type = DAT_1030bdf8[i];
                if (type >= DEBRIS_TYPE_COIN_1 && type <= DEBRIS_TYPE_COIN_500)
                {
                    if (type == DEBRIS_TYPE_COIN_1)
                        DAT_10daaae8[0].coins += 1;
                    if (type == DEBRIS_TYPE_COIN_5)
                        DAT_10daaae8[0].coins += 5;
                    if (type == DEBRIS_TYPE_COIN_10)
                        DAT_10daaae8[0].coins += 10;
                    if (type == DEBRIS_TYPE_COIN_50)
                        DAT_10daaae8[0].coins += 50;
                    if (type == DEBRIS_TYPE_COIN_100)
                        DAT_10daaae8[0].coins += 100;
                    if (type == DEBRIS_TYPE_COIN_500)
                        DAT_10daaae8[0].coins += 500;
                    FUN_1000e750();
                    DAT_104c011e = DEBRIS_PICKUP_FLAG;
                    DAT_1030bdf8[i] = 0;
                    if (DAT_1021b628[i] != DEBRIS_STATE_FALLEN)
                    {
                        FUN_1003ad20(SOUND_DEBRIS_COIN, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                        DAT_1021b628[i] = DEBRIS_STATE_FALLING;
                        continue;
                    }
                }
                else if (type != DEBRIS_TYPE_NO_ITEM && (type < DEBRIS_TYPE_NO_ITEM || type > DEBRIS_TYPE_COIN_500))
                {
                    if (DAT_1021b628[i] != DEBRIS_STATE_FALLEN)
                        FUN_1003ad20(SOUND_DEBRIS_ITEM, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                    int item = DAT_1030bdf8[i];
                    int count = 1;
                    FUN_10012130(&item, &count);
                    DAT_1030bdf8[i] = 0;
                }
                DAT_1021b628[i] = DEBRIS_STATE_FALLING;
            }
        }
    }
    for (i = DEBRIS_SIMPLE_COUNT; i < DEBRIS_COUNT; i++)
    {
        Vec4 next;
        Vec4 unused;
        if (DAT_1030bdf8[i] != 0)
        {
            DAT_104469f8[i]++;
            next.Copy(DAT_10da9400[i] + DAT_10c66ef0[i]);
            if ((unsigned int)DAT_104469f8[i] > DEBRIS_MAX_FRAMES)
                DAT_1030bdf8[i] = 0;
            else if (DAT_102195f0[i] == 0)
            {
                float fall = DAT_10c66ef0[i].z - DEBRIS_GRAVITY;
                DAT_10c66ef0[i].z = fall;
                if (fall < DEBRIS_FALL_LIMIT)
                    DAT_10c66ef0[i].z = DEBRIS_FALL_LIMIT;
                DAT_104465f8[i]++;
                DAT_10da9400[i].Copy(next);
            }
        }
    }
    return 0;
}
