// The per-kind character update (0x100f9670, run once a frame for every character slot by the scene loops). It
// decides whether the slot is in play (enemy slots 0x3a..0x5b are parked far away until they are seen), drops
// characters that fell out of the map, plays the effects of the hukusou/head/fire-node kinds, handles the
// menu-open pause, resets the tables of an enemy slot that was just woken, and finally calls the behaviour of the
// character's kind (CharSlot::kind).

#include <math.h>
#include <string.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "visibility.h"
#include "effect_manager.h"
#include "anim_model.h"
#include "char_state.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "sound_vars.h"
#include "world.h"
#include "enemy_queue.h"
#include "anim_step_vars.h"
#include "hop_spit_enemy.h"
#include "enemy_chase_hop.h"
#include "pounce_enemy.h"
#include "pod_enemy.h"
#include "hikari_warp.h"
#include "enemy_chase_lunge.h"
#include "enemy_cast_teleport.h"
#include "shy_ghost.h"
#include "enemy_roam_spit.h"
#include "pulse_charger.h"
#include "rolling_tire.h"
#include "lunge_enemy.h"
#include "hover_shooter.h"
#include "breath_enemy.h"
#include "attach_slots.h"
#include "drifter.h"
#include "swing_platform.h"
#include "map_cursor.h"
#include "rolling_ball.h"
#include "route_flier.h"
#include "claw_enemy.h"
#include "char_hikari.h"
#include "weapon_fetch.h"
#include "spit_charge_enemy.h"
#include "char_update.h"

// Behaviours without a header yet.
extern "C" int FUN_10068f70(int index, int variant);
extern "C" int FUN_10071ee0(int index);
extern "C" int FUN_100e2080(int index);   // Enemy_UpdateAI_A
extern "C" int FUN_100e2110(int index);   // Enemy_UpdateAI_B
extern "C" int FUN_100e2440(int index);   // Enemy_UpdateAI_C
extern "C" int FUN_100e24d0(int index);   // Enemy_UpdateAI_D
extern "C" int FUN_100e2560(int index);
extern "C" int FUN_100e21a0(int index);   // Enemy_DefaultStep
extern "C" int FUN_1002a110(int index, int run);   // Char_WalkRoute
extern "C" int FUN_1002a740(int index);   // Char_SpiritUpdate
extern "C" int FUN_1002a490(int index);   // Block_Update
extern "C" int FUN_1002ebf0(int index);   // DamageZone_Update
extern "C" int FUN_100347d0(int index);   // Char_FollowLeader
extern "C" int FUN_10034c80(int index);   // Char_StartDefeat
extern "C" int FUN_10079470(int index, int halfSpeed);
extern "C" int FUN_1007b870(int index);   // Npc_GatedRouteCarrier
extern "C" int FUN_10026730(int index);   // Watcher_Update
extern "C" int FUN_10086380(int index);
extern "C" int FUN_10049ec0(int index);
extern "C" int FUN_10051e20(int index);
extern "C" int FUN_1001c210(int index);   // Npc_TalkFacing
extern "C" int FUN_1002ee70(int index);   // Char_UpdateShoves
extern "C" int FUN_1002a340(int index);   // Char_RunScript
extern "C" int FUN_100d2110(int index);
extern "C" int FUN_100d24a0(int index);
extern "C" int FUN_100bdad0(int index);
extern "C" int FUN_100c39c0(int index);
extern "C" int FUN_100cc2a0(int index);
extern "C" int FUN_100edea0(int index);
extern "C" int FUN_100e7280(int index);
extern "C" int FUN_100dfcf0(int index);   // Char_RunBehaviour
extern "C" int FUN_100bcb30(int index);   // Enemy_UpdateBehaviour
extern "C" int FUN_1009e2d0(int index);
extern "C" int FUN_100b8220(int index);   // Npc_Greet
extern "C" int FUN_100627d0(int index);   // Enemy_Defeat
extern "C" int FUN_10024ac0(Node *node, const Vec4 *offset, Matrix4 *matrix, Vec4 *normal, Vec4 *average, int *count,
                            float *depth);

// Enemy slots (parked far away until they are seen).
#define ENEMY_SLOT_FIRST 0x3a
#define ENEMY_SLOT_END 0x5c
// Positions below this x are parked (the slot is out of the world).
#define PARKED_X -20000.0f
#define PARKED_X_LIMIT -19999.0f
#define PARKED_X_PLACE -200000.0f
#define FALL_LIMIT_Z -3400.0f
#define FAR_DISTANCE_SQUARED 16000000.0f
#define SLEEP_DISTANCE 2500.0f
#define SLEEP_FRAMES 0x5b
#define GREET_NOW 10000
#define MOTION_PLAY_IDLE 2
#define MOTION_BLEND_FRAMES 5
#define EFFECT_GROW_HEAD 0x1459
#define EFFECT_FIRE_NODE 0x59
#define EFFECT_HEAD_SMOKE 0x352
#define EFFECT_PUFF 0x732
#define EFFECT_RING 0x7c0
#define NODE_VALUE_FALL 0x2d
#define NODE_VALUE_NORMAL 0x2f
#define NODE_MASK_A 0x400
#define NODE_MASK_B 0x200
#define COUNTER_WRAP 3

// CharState words used by the kind steps below (the words mean different things per kind).
#define WORD_COUNTER 3
#define WORD_ACTIVE 4
#define WORD_TICK 5
#define WORD_FLAG_A 6
#define WORD_FLAG_B 7
#define WORD_FLAG_C 8
#define WORD_HIT 10
#define WORD_FALL_LEVEL 11
#define WORD_TIMER 14
#define WORD_BEHAVIOUR 15

// Remembers the motion that plays so it can resume after a menu or overlay closed (once per hold).
static __forceinline void RememberMotion(int index)
{
    if (DAT_10db6ee8[index].holding == 0 && DAT_10daaae8[index].kind != 0x2d && DAT_10daaae8[index].kind != -0x2d &&
        DAT_10daaae8[index].kind != 0x46 && DAT_10daaae8[index].kind != 0x3f && DAT_10daaae8[index].kind != 0x43 &&
        DAT_10db6ee8[index].motion->blending == 0)
    {
        DAT_10db6ee8[index].holding = 1;
        DAT_1020e138[index] = DAT_10db6ee8[index].motion->layer;
        DAT_1042dba8[index] = DAT_10db6ee8[index].motionFrame;
        DAT_10253a58[index] = DAT_10db6ee8[index].motion->nextLayer;
    }
}

// 0x100f9670 (Char_UpdateByKind). Updates character `index`: parks, wakes or drops it, runs the effects of its script
// code and then the behaviour of its kind. Returns 0.
int Char_UpdateByKind(int index)
{
    if (DAT_104c037c != DAT_104bf8b8)
    {
        DAT_104c0380++;
        DAT_104c037c = DAT_104bf8b8;
    }
    Vec4 playerPosition = DAT_10db6ee8[0].matrix.GetTranslation();
    Vec4 position = DAT_10db6ee8[index].matrix.GetTranslation();
    if (DAT_10c635ec != 0)
    {
        DAT_10c635ec = 0;
        FUN_100f5a80();
    }
    DAT_1021f6c0[index] = 0;
    if (DAT_10254090[index] == 1 && index >= ENEMY_SLOT_FIRST && index < ENEMY_SLOT_END && DAT_1021f3c0[index] != 2)
        return 0;
    if (DAT_10daaae8[index].kind == 0x37)
        return 0;
    if (DAT_10daaae8[index].kind != 0x28 && DAT_10daaae8[index].kind != 0x21 && DAT_10daaae8[index].kind != 0x20 &&
        DAT_10daaae8[index].kind != 0x23 && DAT_10daaae8[index].kind != 0x24 && DAT_10daaae8[index].kind != 0x2f &&
        DAT_10daaae8[index].kind != 0x58 && DAT_10daaae8[index].kind != 0x2d && DAT_10daaae8[index].kind != -0x2d &&
        DAT_10daaae8[index].kind != 0x46 && DAT_10daaae8[index].kind != 0x3f && DAT_10daaae8[index].kind != 0x43 &&
        DAT_10daaae8[index].kind != 0x2b && DAT_10daaae8[index].kind != 0x2c && DAT_10daaae8[index].kind != 0x32 &&
        DAT_10daaae8[index].kind != 0x2a && DAT_10daaae8[index].kind > -1 && index < ENEMY_SLOT_END &&
        index >= ENEMY_SLOT_FIRST)
    {
        if (DAT_1020d540[index] != 0)
        {
            DAT_1021b4c8[index]++;
            if (position.x > PARKED_X)
            {
                if (DAT_1021f3c0[index] == 3 && DAT_10255f10[index] != -1 &&
                    ((DAT_10db6ee8[index].shadowOpacity == 0 &&
                      (FUN_10015860(position) == 0 || position.DistanceSquared(DAT_118e6cb8) > FAR_DISTANCE_SQUARED)) ||
                     DAT_102199b8[index] == 0 || DAT_10254598[index] > 300))
                {
                    int visible = 1;
                    if (DAT_10255f08 != 0 && DAT_10255f08->visibilityCells != 0)
                    {
                        Box2 *box = DAT_10255f08->visibilityCells->boxes;
                        for (int boxCount = 0x40; boxCount != 0; boxCount--)
                        {
                            if (position.InBox(box) != 0)
                                visible = 2;
                            box++;
                        }
                        if (visible != 2)
                            visible = 0;
                    }
                    else if (DAT_10db6ee8[index].m_238c == 0 &&
                             (DAT_10db6ee8[index].mode == 1 || DAT_10daaae8[index].kind == 0xc) &&
                             FUN_10024750(index, 0, 100000.0f, 1, 0, 1, 0) == 0 && position.z < playerPosition.z)
                    {
                        Node *loaded = DAT_10db6ee8[index].loadedRoot;
                        if (FUN_101ae4d0(&(loaded->bounds + loaded->worldMatrix.GetTranslation()), &DAT_108392d8,
                                         loaded->cullMargin) == 0)
                            visible = 0;
                    }
                    if (DAT_10daaae8[index].kind == 0xb ||
                        (DAT_10daaae8[index].kind == 6 && DAT_1020b430[index].words[WORD_COUNTER] == 0))
                        visible = 1;
                    if (DAT_10210d30[index] == 1000 || visible == 0)
                    {
                        if (DAT_10daaae8[index].kind != 0x2a && DAT_10daaae8[index].kind != 0x48)
                            FUN_100627d0(index);
                    }
                    else
                    {
                        DAT_10867e18[index].Copy(position);
                        DAT_10db6ee8[index].SetPosition(PARKED_X_PLACE, 0, 0);
                        DAT_10db6ee8[index].PlayMotion(MOTION_PLAY_IDLE, 0, MOTION_BLEND_FRAMES, 1.0f);
                        DAT_10210d30[index] = 0;
                        position.x = PARKED_X_PLACE;
                    }
                }
            }
            else if (DAT_1021b4c8[index] % 5 == 0)
            {
                if ((FUN_10015860(DAT_10867e18[index]) != 0 || DAT_10256420[index] != 0) &&
                    DAT_10210d30[index] == 0 && DAT_102199b8[index] != 0 &&
                    (DAT_10256420[index] != 0 || DAT_10867e18[index].DistanceSquared(DAT_118e6cb8) < FAR_DISTANCE_SQUARED))
                {
                    DAT_10db6ee8[index].SetPosition(DAT_10867e18[index].x, DAT_10867e18[index].y,
                                                    DAT_10867e18[index].z);
                    DAT_10db6ee8[index].matrix._41 = DAT_10867e18[index].x;
                    DAT_10db6ee8[index].matrix._42 = DAT_10867e18[index].y;
                    DAT_10db6ee8[index].matrix._43 = DAT_10867e18[index].z;
                    DAT_10db6ee8[index].PlayMotion(MOTION_PLAY_IDLE, 0, MOTION_BLEND_FRAMES, 1.0f);
                    if (DAT_10254598[index] > SLEEP_FRAMES)
                        DAT_10254598[index] = SLEEP_FRAMES;
                    if (DAT_102199b8[index] == 1)
                    {
                        DAT_102199b8[index] = 2;
                        DAT_10db6ee8[index].alpha = 0.0f;
                    }
                    if (DAT_102199b8[index] > 0x10)
                        DAT_102199b8[index] = 1;
                    if (DAT_102199b8[index] == 1)
                        DAT_10db6ee8[index].alpha = 1.0f;
                    DAT_1042dea8[index] = 0x5a;
                }
            }
            if (DAT_1021b4c8[index] % 5 == 0)
                DAT_1021b4c8[index] = 1;
        }
        else
        {
            if (position.x > PARKED_X && DAT_10db6ee8[index].shadowOpacity != 0 && DAT_1021f3c0[index] == 3)
            {
                DAT_1020d540[index] = 1;
                DAT_1021b4c8[index] = index % 5;
                DAT_10867e18[index].Copy(position);
            }
        }
        if (DAT_10db6ee8[index].shadowOpacity == 0 && DAT_10254090[index] == 0)
            return 0;
    }
    if (position.x < PARKED_X_LIMIT && index != DAT_10c66cf4 && index != DAT_10c66cf0 && index != DAT_10c66cec &&
        index != DAT_10c66ce8 && DAT_10daaae8[index].kind != -0x2d &&
        (DAT_10daaae8[index].kind != 0x62 || DAT_1020b430[index].words[WORD_BEHAVIOUR] != 0x1b))
    {
        DAT_1021bbf8[index] = 1;
        if (DAT_104c0110 == index)
            DAT_104c0110 = 0;
        if (DAT_10254090[index] != 0)
            DAT_10254090[index] = 0;
        return 0;
    }
    if (position.z < FALL_LIMIT_Z && DAT_10daaae8[index].kind != 0x32 &&
        (DAT_10daaae8[index].kind > -1 || DAT_10daaae8[index].kind == -0x2d || DAT_10daaae8[index].kind == -0x65) &&
        DAT_1021f3c0[index] == 3)
    {
        if (DAT_10daaae8[index].kind != 0x2d && DAT_10daaae8[index].kind != -0x2d && DAT_10daaae8[index].kind != 100 &&
            DAT_10daaae8[index].kind != -0x65)
        {
            FUN_100627d0(index);
            return 0;
        }
        DAT_10daaae8[index].stats[STAT_PICKUPS] = 100;
    }
    if (index >= ENEMY_SLOT_FIRST && index < ENEMY_SLOT_END)
    {
        int code = DAT_102548c0[index];
        int hundreds = (code / 100) % 100;
        switch (hundreds)
        {
        case 7:
        case 9:
        {
            Node *tail = DAT_10db6ee8[index].root->Find("hukusou", 0);
            if (tail != 0)
            {
                Vec4 rotation;
                Vec4 turn;
                rotation.Copy(QuatFromMatrix(tail->matrix));
                turn.Set(0.0349f, 0.0f, 0.0f, 0.99939f);
                rotation.Copy(QuatMultiply(rotation, turn));
                tail->matrix = RotationMatrix(rotation);
            }
            break;
        }
        }
        EffectOptions options;
        Matrix4 pose;
        Vec4 effectPosition;
        options.flags |= 2;
        pose.SetIdentity();
        switch (code % 100)
        {
        case 6:
        case 0x10:
            if (DAT_104c0380 % 3 == index % 3)
            {
                Node *head = DAT_10db6ee8[index].root->Find("Bip01 Head", 0);
                if (head != 0)
                {
                    pose.SetTranslation(head->worldMatrix.GetTranslation());
                    pose._41 = (float)(FUN_1013f880() % 40) - 20.0f + pose._41;
                    pose._42 = (float)(FUN_1013f880() % 40) - 20.0f + pose._42;
                    pose._43 = (float)(FUN_1013f880() % 40) - 20.0f + pose._43;
                    FUN_1001d8b0(index, EFFECT_GROW_HEAD, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                }
            }
            break;
        case 7:
            if (DAT_104c0380 % 5 == index % 5)
            {
                Node *fire = DAT_10db6ee8[index].root->Find("@firenull", 0);
                if (fire != 0)
                {
                    pose.SetTranslation(fire->worldMatrix.GetTranslation());
                    pose._41 = (float)(FUN_1013f880() % 40) - 20.0f + pose._41;
                    pose._42 = (float)(FUN_1013f880() % 40) - 20.0f + pose._42;
                    pose._43 = (float)(FUN_1013f880() % 40) - 20.0f + pose._43;
                    FUN_1001d8b0(index, EFFECT_FIRE_NODE, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                }
            }
            break;
        case 0x11:
            if (DAT_104c0380 % 0x50 == index % 0x50 && DAT_1032d69c != 0)
            {
                Node *head = DAT_10db6ee8[index].root->Find("Bip01 Head", 0);
                if (head != 0)
                {
                    pose.SetTranslation(head->worldMatrix.GetTranslation());
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    pose._43 = pose._43 - 50.0f;
                    options.flags = 0;
                    options.m_0000 = 0;
                    options.m_0008 = 1.5f;
                    options.m_000e = 6;
                    FUN_1001d8b0(index, EFFECT_HEAD_SMOKE, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                }
            }
        }
    }
    int greet = DAT_10254390[index];
    if (greet > 0 && (DAT_104bfe1c != 0 || DAT_104c0135 != 0))
        goto rememberMotion;
    if (greet != GREET_NOW)
    {
        if (greet < 1)
        {
            if (greet != 0)
            {
                DAT_10db6ee8[index].PlayMotion(MOTION_PLAY_IDLE, 0, MOTION_BLEND_FRAMES, 1.0f);
                return 0;
            }
        }
        else
        {
            DAT_10254390[index] = greet - 1;
            if (greet - 1 < GREET_NOW)
            {
                DAT_10db6ee8[index].PlayMotion(MOTION_PLAY_IDLE, 0, MOTION_BLEND_FRAMES, 1.0f);
                return 0;
            }
        }
    }
    if (DAT_104bfdf0 != 0 || DAT_104c0135 != 0 || DAT_104bfe1c != 0)
    {
        switch (DAT_10daaae8[index].kind)
        {
        case 6:
            if (DAT_1020b430[index].words[WORD_FLAG_C] > 0)
            {
                DAT_1020b430[index].state = 5;
                DAT_1020b430[index].words[WORD_FALL_LEVEL] = 0;
                return 0;
            }
            if (DAT_1020b430[index].state == 5)
                return 0;
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].words[WORD_FLAG_A] = 0;
            DAT_1020b430[index].words[WORD_FLAG_B] = 0;
            DAT_1020b430[index].words[WORD_FLAG_C] = 0;
            DAT_1020b430[index].words[WORD_FALL_LEVEL] = 0;
            return 0;
        case 9:
            if (FUN_1013f880() % 100 < 0x50 && DAT_1020b430[index].words[WORD_COUNTER] == 0)
            {
                Vec4 effectPosition;
                EffectOptions options;
                Matrix4 pose;
                float zero = 0.0f;
                effectPosition.Fill(zero);
                pose.SetIdentity();
                pose.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
                options.flags |= 2;
                options.m_0004 = 0;
                options.m_0000 = 0;
                options.owner = index;
                if (DAT_1020b430[index].words[WORD_FLAG_A] > 0)
                {
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    options.m_000e = 5;
                    FUN_1001d8b0(index, EFFECT_PUFF, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                }
                DAT_1020b430[index].words[WORD_COUNTER]++;
                return 0;
            }
            if (DAT_1020b430[index].words[WORD_COUNTER] > 0)
            {
                int next = DAT_1020b430[index].words[WORD_COUNTER] + 1;
                DAT_1020b430[index].words[WORD_COUNTER] = next;
                if (next == COUNTER_WRAP)
                    DAT_1020b430[index].words[WORD_COUNTER] = 0;
            }
            return 0;
        case 5:
        {
            float zero = 0.0f;
            DAT_10db6ee8[index].velocity.Fill(zero);
            FUN_100241e0(index, 0, 0, 1.5f);
            if (DAT_1020b430[index].state > 5)
                DAT_1020b430[index].words[WORD_FLAG_B] = 1;
            return 0;
        }
        case 8:
            if (DAT_10db6ee8[index].mode == 2 && DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
                DAT_10db6ee8[index].mode = 0;
            return 0;
        case 0x42:
            if (DAT_1020b430[index].words[WORD_ACTIVE] != 0 && DAT_1020b430[index].words[WORD_TICK] % 3 == 0)
            {
                Vec4 effectPosition;
                Matrix4 pose;
                pose.SetIdentity();
                pose.SetTranslation(position);
                pose._43 = pose._43 + 40.0f;
                EffectOptions options;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = 1.0f;
                options.m_000e = 3;
                if (DAT_1021bdf8[index][1] == 0)
                    FUN_1001d8b0(index, EFFECT_RING, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                else
                    FUN_1001d8b0(index, EFFECT_RING, effectPosition, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0,
                                 &pose, &options);
            }
            DAT_1020b430[index].words[WORD_TICK]++;
            return 0;
        case 0x32:
            if (DAT_1020b430[index].words[WORD_FLAG_A] == 0 && index >= ENEMY_SLOT_FIRST)
            {
                DAT_1020b430[index].words[WORD_HIT] = 1;
                DAT_1020b430[index].words[WORD_BEHAVIOUR] = DAT_10c635f8 != 2 ? -0x3c : -0x1e;
            }
            break;
        case 0x33:
            if (DAT_1020b430[index].words[WORD_COUNTER] != 0 && DAT_1020b430[index].words[WORD_FLAG_A] == 0)
            {
                DAT_1020b430[index].words[WORD_HIT] = 1;
                DAT_1020b430[index].words[WORD_BEHAVIOUR] = DAT_10c635f8 != 2 ? -0x3c : -0x1e;
            }
            break;
        case 0x34:
            DAT_1020b430[index].state++;
            break;
        case 0x2d:
            FUN_100bdad0(index);
            break;
        case -0x2d:
            FUN_100347d0(index);
            break;
        case 0x46:
            FUN_100c39c0(index);
            break;
        case 0x3f:
            FUN_100cc2a0(index);
            break;
        }
    rememberMotion:
        RememberMotion(index);
        return 0;
    }
    if (DAT_1021f3c0[index] == 3 && DAT_104c0380 % 0x14 == index % 0x14 && DAT_10db6ee8[index].m_008d == 0 &&
        DAT_10db6ee8[index].mode != 0 && DAT_10db6ee8[index].m_2384 != 0 && DAT_10daaae8[index].kind != 0x3f &&
        DAT_10daaae8[index].kind != 8)
    {
        Vec4 average;
        Vec4 normal;
        Vec4 offset;
        Matrix4 matrix;
        float depth;
        int count = 0;
        int tries = 0;
        matrix.SetIdentity();
        do
        {
            count = 0;
            FUN_10024ac0(DAT_10db6ee8[index].root, &offset, &matrix, &normal, &average, &count, &depth);
            if (count != 0)
            {
                average.Copy(average * 10.0f);
                offset.Copy(offset + average);
                average.Copy(average + position);
                DAT_10db6ee8[index].SetPosition(average.x, average.y, average.z);
                position.Copy(average);
            }
            tries++;
        } while (count != 0 && tries < 10);
    }
    if (DAT_10c635c8 == 0)
    {
        memset(DAT_10254090, 1, 0x3a);
        DAT_10c635c8 = 1;
        DAT_102028d4 = -1;
    }
    if (DAT_10254090[index] != 0 && index < ENEMY_SLOT_FIRST)
    {
        if (DAT_104c0350 == 0)
        {
            FUN_100f5a80();
            memset(DAT_10253a58, 0, sizeof(DAT_10253a58));
            DAT_104c0350 = 1;
            for (int slot = 0; slot < 6; slot++)
                DAT_102108d0[slot] = 0;
            for (int slot = 0; slot < 10; slot++)
                DAT_10447310[slot] = 0;
            memset(DAT_1020e138, 0xff, sizeof(DAT_1020e138));
            for (int model = 0; model < CHAR_SLOT_COUNT; model++)
                DAT_10db6ee8[model].holding = 0;
        }
        for (int word = 0; word < 16; word++)
            DAT_1020b430[index].words[word] = 0;
        DAT_10254598[index] = SLEEP_FRAMES;
        DAT_1030b8f8[index] = 0;
        DAT_103086b0[index] = 1;
        DAT_1020d540[index] = 0;
        DAT_1021b4c8[index] = index % 5 - 1;
        float zero = 0.0f;
        DAT_10867e18[index].Fill(zero);
        DAT_10db6ee8[index].shadowOpacity = 0xf;
        DAT_10db6ee8[index].holding = 0;
        DAT_10256420[index] = 0;
        DAT_1021f940[index] = 0;
        DAT_1020e138[index] = -1;
        DAT_10253a58[index] = 0;
        DAT_1042dba8[index] = 0;
        DAT_1030b8f8[index] = 0;
        DAT_104483e8[index] = 0;
        if (DAT_10daaae8[index].kind == 0x19)
            memset(DAT_10254598, 0, 0x32 * sizeof(int));
    }
    if (DAT_103086b0[index] != 0)
    {
        DAT_10255160[index][0] = 0;
        DAT_10255160[index][1] = 0;
        DAT_10255160[index][2] = 0;
        DAT_103086b0[index] = 0;
        DAT_10255160[index][3] = 0;
        for (int piece = 0; piece < 6; piece++)
            DAT_10868838[index][piece].SetIdentity();
    }
    if (DAT_104bfdf0 != 0 ||
        (DAT_104bfe78 != 0 && DAT_10daaae8[index].kind != 0x20 && DAT_10daaae8[index].kind != 0x23 &&
         DAT_10daaae8[index].kind != 0x24 && DAT_10daaae8[index].kind != 0x2f && DAT_10daaae8[index].kind != 0x58))
        goto afterBehaviour;
    if (index < ENEMY_SLOT_END)
    {
        if (DAT_1020e138[index] < 0)
        {
            if (DAT_1020e138[index] == -2)
            {
                DAT_10db6ee8[index].PlayMotion(DAT_10253a58[index], 1, MOTION_BLEND_FRAMES, 1.0f);
                DAT_10db6ee8[index].holding = 0;
                DAT_1020e138[index] = -1;
            }
        }
        else
        {
            DAT_10db6ee8[index].holding = 0;
            DAT_1020e138[index] = -1;
        }
    }
    if (DAT_10254090[index] == 0)
    {
        Node *loaded = DAT_10db6ee8[index].loadedRoot;
        DAT_1030b8f8[index] =
            FUN_101ae4d0(&(loaded->bounds + loaded->worldMatrix.GetTranslation()), &DAT_108392d8, loaded->cullMargin);
    }
    if (DAT_10254090[index] != 0)
    {
        DAT_10254090[index] = 0;
        if (index < ENEMY_SLOT_FIRST)
            return 0;
    }
    if (DAT_1030b8f8[index] == 0)
    {
        if (index >= ENEMY_SLOT_FIRST)
            DAT_10254598[index]++;
    }
    else if (index >= ENEMY_SLOT_FIRST && DAT_10254598[index] > 0)
        DAT_10254598[index] = 0;
    if ((playerPosition - position).Length() > SLEEP_DISTANCE && DAT_1030b8f8[index] == 0 &&
        DAT_10254598[index] > 0x5a && DAT_104483e8[index] >= 0)
        DAT_104483e8[index] = 0;
    else
        DAT_104483e8[index] = 1;
    switch (DAT_10daaae8[index].kind)
    {
    case 0:
        Enemy_ChaseLungeAndSpit(index, 0);
        break;
    case -0x65:
        FUN_10034c80(index);
        break;
    case 4:
        Enemy_WanderHopAndSpit(index);
        break;
    case 5:
        FUN_10068f70(index, 0);
        break;
    case 7:
        Enemy_PounceAndBreathe(index);
        break;
    case 8:
        MapCursor_Walk(index);
        break;
    case 10:
        Enemy_SpitAndCharge(index);
        break;
    case 0xb:
        Char_Drifter(index);
        break;
    case 0xc:
        Enemy_CastAndTeleport(index);
        break;
    case 0xd:
        FUN_10068f70(index, 1);
        break;
    case 0xe:
        Char_SwingPlatform(index);
        break;
    case 0xf:
        Enemy_PodLobber(index);
        break;
    case 0x10:
        FUN_10071ee0(index);
        break;
    case 0x11:
        Enemy_ShyGhost(index);
        break;
    case 0x14:
        FUN_100e2080(index);
        break;
    case 0x15:
        FUN_100e2110(index);
        break;
    case 0x17:
        FUN_100e2440(index);
        break;
    case 0x18:
        FUN_100e24d0(index);
        break;
    case 0x19:
        FUN_100e2560(index);
        break;
    case 0x1e:
        FUN_1002a110(index, 0);
        break;
    case -0x1e:
        FUN_1002a110(index, 1);
        break;
    case 0x1f:
        Char_HikariWarp(index, 0);
        break;
    case 0x25:
        FUN_1002a740(index);
        break;
    case 0x26:
        FUN_1002a490(index);
        break;
    case 0x22:
        Char_HikariWarp(index, 1);
        break;
    case 0x20:
        Char_RollingBall(index);
        break;
    case 0x24:
        FUN_1002ebf0(index);
        break;
    case 0x21:
        FUN_1007c050(index);
        break;
    case -0x2d:
        FUN_100347d0(index);
        break;
    case 0x27:
        FUN_10079470(index, 0);
        break;
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f:
    case 0x50:
    case 0x51:
    case 0x52:
        FUN_1009e2d0(index);
        break;
    case 0x2b:
        Enemy_RoamAndSpit(index);
        break;
    case 0x2c:
        FUN_10086380(index);
        break;
    case 0x54:
        Enemy_HoverAndShoot(index);
        break;
    case 0x42:
        Enemy_PulseAndCharge(index);
        break;
    case 0x31:
        FUN_100d2110(index);
        break;
    case 0x44:
        Enemy_RollingTire(index, 0);
        break;
    case 0x59:
        Enemy_RollingTire(index, 1);
        break;
    case 0x56:
        Enemy_StalkAndBreathe(index);
        break;
    case 0x45:
        Enemy_StalkAndLunge(index);
        break;
    case 0x55:
        FUN_100a8ac0(index);
        break;
    case 0x3c:
    {
        FUN_10049ec0(index);
        Vec4 otherPosition;
        Vec4 selfPosition;
        Vec4 spare;
        selfPosition.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        for (int other = 1; other < index; other++)
        {
            if (DAT_10daaae8[other].kind == 0x3c)
            {
                otherPosition.Copy(DAT_10db6ee8[other].matrix.GetTranslation());
                float turn = atan2f(DAT_10db6ee8[index].move.x, DAT_10db6ee8[index].move.y) -
                             atan2f(otherPosition.x - DAT_10db6ee8[index].matrix._41,
                                    otherPosition.y - DAT_10db6ee8[index].matrix._42);
                while (turn > 3.1415f)
                    turn -= 6.283f;
                while (turn < -3.1415f)
                    turn += 6.283f;
                if ((position - otherPosition).Length() < 120.0f && floorf(turn) < 0.94245f &&
                    DAT_10db6ee8[index].move.Length() > 0.1f)
                {
                    DAT_10db6ee8[index].move.RotateZ(1.0472f);
                    float heading = atan2f(DAT_10db6ee8[index].move.x, DAT_10db6ee8[index].move.y) + 3.1415f;
                    DAT_10db6ee8[index].targetHeading = heading;
                    DAT_10255160[index][1] = heading;
                    DAT_1020b430[index].words[WORD_TIMER] = 5;
                }
            }
        }
        if (DAT_1020b430[index].words[WORD_TIMER] != 0)
        {
            DAT_1020b430[index].words[WORD_TIMER]--;
            DAT_10db6ee8[index].targetHeading = DAT_10255160[index][1];
        }
        break;
    }
    case 0x3f:
        FUN_100cc2a0(index);
        break;
    case 0x16:
    case 0x40:
        FUN_100e21a0(index);
        break;
    case 0x57:
        FUN_1001c210(index);
        break;
    case 0x2e:
        FUN_10079470(index, 1);
        break;
    case 0x30:
        Char_HikariWarp(index, 2);
        break;
    case 0x41:
        FUN_10051e20(index);
        break;
    case 0x2f:
        FUN_1002ee70(index);
        break;
    case 0x58:
        Char_UpdateAttachSlots(index);
        break;
    case 0x28:
        FUN_1007b870(index);
        break;
    case 0x2a:
        FUN_10026730(index);
        break;
    case 0x2d:
        FUN_100bdad0(index);
        break;
    case 0x43:
        FUN_100edea0(index);
        break;
    case 0x46:
        FUN_100c39c0(index);
        break;
    case 0x32:
        if (index < ENEMY_SLOT_FIRST)
            FUN_100e7280(index);
        break;
    case 0x62:
        FUN_100dfcf0(index);
        break;
    case 0x61:
        FUN_100bcb30(index);
        break;
    case 0x6b:
        FUN_100d24a0(index);
        break;
    case 0x136:
        FUN_1002cc70(index, 0);
        break;
    case 0x140:
        FUN_1002a340(index);
        break;
    }
afterBehaviour:
    if (DAT_1021f3c0[index] == 2)
    {
        if (!(DAT_10c62ed0 == 0x19 && index == 0x3d) && !(DAT_10c62ed0 == 0x191 && index == 0x3a) &&
            DAT_10c655e0 == 0 && DAT_10c66cf4 != index && DAT_10db6ee8[index].alpha > 0.0f &&
            DAT_10db6ee8[index].shadowHidden == 0)
            FUN_100b8220(index);
    }
    Vec4 probe;
    probe.z = 5.0f;
    if (DAT_1021f3c0[index] == 3 && index >= ENEMY_SLOT_FIRST && index < ENEMY_SLOT_END &&
        DAT_10db6ee8[index].IsClearAny(&probe) == 0 && DAT_10daaae8[index].kind != 0xf)
    {
        DAT_1021f940[index]++;
        if (DAT_1021f940[index] > 0x1e)
        {
            DAT_10db6ee8[index].root->M_10194730(NODE_VALUE_FALL, NODE_MASK_A);
            DAT_10db6ee8[index].root->M_10194730(NODE_VALUE_FALL, NODE_MASK_B);
        }
        return 0;
    }
    if (DAT_1021f940[index] > 0x1e)
    {
        DAT_10db6ee8[index].root->M_10194730(NODE_VALUE_NORMAL, NODE_MASK_A);
        DAT_10db6ee8[index].root->M_10194730(NODE_VALUE_NORMAL, NODE_MASK_B);
        if (DAT_10db6ee8[index].IsClearAny(0) == 0)
        {
            DAT_10db6ee8[index].root->M_10194730(NODE_VALUE_FALL, NODE_MASK_A);
            DAT_10db6ee8[index].root->M_10194730(NODE_VALUE_FALL, NODE_MASK_B);
            return 0;
        }
    }
    DAT_1021f940[index] = 0;
    return 0;
}
