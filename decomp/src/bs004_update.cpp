// The BS_004 boss: the moss root creature (kind 0x46). Once the game is over (DAT_10210d30) it runs the ending scene
// (the camera slides along the creature while the screen fades), otherwise the fight: the creature hovers over a path
// of nodes, picks attack states by random odds that depend on how many of its three minions (the slots after it) are
// down, dives at the player, sweeps its tail, breathes fire and calls the minions back. The moves are written as the
// original blends them: the movement vector is smoothed towards a per-state step vector every frame.

#include <math.h>
#include <windows.h>
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "effect_manager.h"
#include "texture_manager.h"
#include "bs004.h"
#include "bgm_track.h"
#include "char_equip_models.h"
#include "doppler_loops.h"
#include "dodge_golem.h"
#include "enemy_chase_hop.h"
#include "enemy_fighter.h"
#include "engine_memory.h"
#include "game.h"
#include "game_main.h"
#include "hop_spit_enemy.h"
#include "last_position.h"
#include "partner_follow.h"
#include "route_flier.h"
#include "target_race.h"
#include "title_screen.h"
#include "value_command.h"
#include "bs003.h"
#include "action_select_vars.h"
#include "char_ground.h"
#include "map_placement.h"
#include "texture_load.h"
#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "script_system.h"
#include "world.h"

#define BS004_PI 3.1415f
#define BS004_TWO_PI 6.283f
#define BS004_ENDING_MODEL_A 88                 // the two models of the ending (bs_004en.it3 and bs_004pa.it3)
#define BS004_ENDING_MODEL_B 89
#define BS004_ENDING_START (-30)
#define BS004_PEOPLE_FIRST 0x3a
#define BS004_ROOM_FLAG_BASE 1000               // + 2 * room: flag word of the ending
#define BS004_FAR_X (-200000.0f)
#define BS004_HOVER_HEIGHT 700.0f
#define BS004_FLOOR_FIRST 92                    // slots 92..123: the kind 0x25 characters it stops when it is hit
#define BS004_FLOOR_END 124
#define BS004_SPECTATOR_FIRST 93                // slots 93..95: the three characters it checks for nearness
#define BS004_SPECTATOR_END 96
#define BS004_MINION_FIRST_OFFSET 2             // minions are the slots index + 2 .. index + 4
#define BS004_FLAG_DEFEAT_MODEL 0x1a4           // script flag holding the model that leaves with the defeat scene
#define BS004_PATH_NODE 0                       // DAT_1021bdf8[slot][0]: "@MossBeeRoot", the node whose frames are the path
#define BS004_BONE_FOOT_L 0                     // DAT_1042d918[...]: bones of the creature
#define BS004_BONE_FOOT_R 1
#define BS004_BONE_TAIL 2
#define BS004_BONE_TORCH 3
#define BS004_BONE_FOREARM_L 4
#define BS004_BONE_FINGER_L 5
#define BS004_BONE_FOREARM_R 6
#define BS004_BONE_FINGER_R 7
#define BS004_NODE_FLAGS_HIDE 0x20
#define BS004_BASE_KIND_PACKED_GROUP 0x31
#define BS004_BASE_KIND_HIDE_ARMOUR 0x10
#define BS004_KIND_FLOOR 0x25

#define BS004_MOVE_NONE 0
#define BS004_MOVE_WALK 1                       // move by the vector, scaled by the motion speed (Char_Move 0x100241e0)
#define BS004_MOVE_FLY 2                        // move by the vector, shrunk until the way is clear of the map

// States (Bs004State::state).
#define BS004_STATE_HOVER 0
#define BS004_STATE_FOLLOW_PATH 1
#define BS004_STATE_DRIFT 2
#define BS004_STATE_RECOIL 3
#define BS004_STATE_PICK_POINT 4
#define BS004_STATE_APPROACH_POINT 5
#define BS004_STATE_WAIT 6
#define BS004_STATE_ROAR 7
#define BS004_STATE_DIVE_A 8
#define BS004_STATE_SWIPE 9
#define BS004_STATE_APPROACH_POINT_B 10
#define BS004_STATE_DIVE_B 11
#define BS004_STATE_TAIL_SWEEP 12
#define BS004_STATE_SLAM 13
#define BS004_STATE_RISE_A 14
#define BS004_STATE_PLUNGE_A 15
#define BS004_STATE_RISE_B 16
#define BS004_STATE_PLUNGE_B 17
#define BS004_STATE_GROUND_BLAST 18
#define BS004_STATE_FIRE_RAIN 19
#define BS004_STATE_PICK 100

#define BS004_EFFECT_SPARK 0x6d4
#define BS004_EFFECT_PAIR 0x6dd

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

static inline Node *Bs004Bone(int which)
{
    return (Node *)DAT_1042d918[which];
}

// Smooths the movement vector towards a state's step vector: move = move * keep + step * give.
static void BlendMove(Vec4 &move, const Vec4 &step, float keep, float give)
{
    move.Copy(move * keep + step * give);
}

// Starts effect `effect` for the creature at `where`; `owner` is the node matrix the effect follows (0 = none).
static void StartEffect(int index, int effect, const Vec4 &where, const Matrix4 *owner, const Matrix4 *pose,
                        const EffectOptions *options)
{
    FUN_1001d8b0(index, effect, where, -1, -1, owner, 0, 0, pose, options);
}

// Fills the options of the creature's larger effects: its effect parameter, a scale and the follow flag byte.
static void SetUpOptions(int index, EffectOptions &options, float scale)
{
    options.m_0000 = 0;
    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
    options.m_0008 = scale;
    options.m_000e = 5;
}

// A turn about z of `heading` placed at `where`.
static void MakePose(Matrix4 &pose, float heading, const Vec4 &where)
{
    pose.SetIdentity();
    pose = RotationZ(heading);
    pose.SetTranslation(where);
}

// Two small burst effects (0x6dd) at the hands, as the tail sweeps start them.
static void StartHandBursts(int index, Matrix4 &poseA, Matrix4 &poseB)
{
    Vec4 where;
    where.Copy(poseA.GetTranslation());
    StartEffect(index, BS004_EFFECT_PAIR, where, 0, &poseA, 0);
    where.Copy(poseB.GetTranslation());
    StartEffect(index, BS004_EFFECT_PAIR, where, 0, &poseB, 0);
}

// The two hand poses of the bursts: the creature's heading placed at two bones.
static void MakeHandPoses(Matrix4 &poseA, Matrix4 &poseB, int boneA, int boneB)
{
    poseA = RotationZ(DAT_10db9364);
    poseB = poseA;
    poseA.SetTranslation(Bs004Bone(boneA)->worldMatrix.GetTranslation());
    poseB.SetTranslation(Bs004Bone(boneB)->worldMatrix.GetTranslation());
}

// Starts a minion slot over: clears the tables of its slot like Char_SpawnEnemy (0x10050510) does, tags its model nodes
// and puts its equipment back on. `fan` is the minion's place in the fan of three (-1, 0 or 1).
static void ActivateMinion(int slot, int fan)
{
    AnimModel &minion = DAT_10db6ee8[slot];
    CharSlot &minionSlot = DAT_10daaae8[slot];
    minionSlot.stats[STAT_HEALTH] = minionSlot.stats[STAT_MAX_HEALTH];
    minion.root->M_10194630(slot, 0x400);
    minion.root->M_101946b0(0xfe, 0x400);
    minion.root->M_10194730(0x2f, 0x400);
    minion.root->M_10194630(0, 0x200);
    minion.root->M_101946b0(0xff, 0x200);
    minion.root->M_10194730(0x2f, 0x200);
    minion.alpha = 1.0f;
    DAT_10254090[slot] = 0;
    DAT_10453008[slot] = 0;
    DAT_10254598[slot] = 0x5b;
    DAT_1030b8f8[slot] = 0;
    DAT_1020d540[slot] = 0;
    DAT_10256420[slot] = 1;
    minion.shadowOpacity = 1;
    DAT_10254390[slot] = 0;
    DAT_1021f940[slot] = 0;
    DAT_1020e138[slot] = -1;
    minion.m_0294 = 0;
    DAT_10253a58[slot] = 0;
    DAT_1021b4c8[slot] = (char)(slot % 5) - 1;
    for (int word = 0; word < 0x10; word++)
        DAT_1020b430[slot].words[word] = 0;
    DAT_1020b430[slot].words[2] = fan;
    FUN_10065b10(slot, 0x2d5, 1, 0);
    minionSlot.money = 0;
    minionSlot.m_00c0 = 0;
    minionSlot.m_00c4 = 0;
    minionSlot.m_00c8 = 0;
    DAT_1030c480[slot] = 0;
    DAT_10467510[slot] = 0;
    minion.m_02a8 = 0;
    minion.holdState = 0;
    minion.holdWeight = 0.0f;
    minion.m_022c.Fill(0.0f);
    DAT_10467ba0[slot] = 0;
    DAT_102239d0[slot] = 0;
    DAT_10210d30[slot] = 0;
    DAT_104686b0[slot] = 0;
    DAT_1020d4c0[slot] = 0;
    DAT_102550e0[slot] = 0;
    DAT_10218fc0[slot] = 0;
    DAT_10210b30[slot] = 0;
    DAT_10802bf8[slot].position.Fill(0.0f);
    DAT_10802bf8[slot].step.Fill(0.0f);
    int packed = minionSlot.stats[9];
    DAT_10315350[slot] = 0;
    DAT_1021f6c0[slot] = 0;
    DAT_102548c0[slot] = 0;
    DAT_10219e40[slot] = 0;
    DAT_1042d9e8[slot] = 0;
    bool hidesArmour = minionSlot.kind == BS004_BASE_KIND_HIDE_ARMOUR || DAT_1030b668[slot] == BS004_BASE_KIND_HIDE_ARMOUR;
    if (packed / 10000 == 0 &&
        ((minionSlot.kind != BS004_BASE_KIND_PACKED_GROUP && DAT_1030b668[slot] != BS004_BASE_KIND_PACKED_GROUP) ||
         packed % 100 == 0))
    {
        if (hidesArmour)
        {
            Node *armour = minion.root->Find("@EN_016ar.lwo", 0);
            if (armour != 0)
                armour->ClearFlags(BS004_NODE_FLAGS_HIDE);
        }
    }
    else
    {
        if (hidesArmour)
        {
            Node *armour = minion.root->Find("@EN_016ar.lwo", 0);
            if (armour != 0)
                armour->SetFlags(BS004_NODE_FLAGS_HIDE);
        }
        int weapon = 0;
        if (minionSlot.kind == BS004_BASE_KIND_PACKED_GROUP || DAT_1030b668[slot] == BS004_BASE_KIND_PACKED_GROUP)
            weapon = packed % 100;
        FUN_100404e0(slot, packed / 10000000 % 100, packed / 100000 % 100, packed / 1000 % 100, weapon, 0, 0);
    }
}

// Sets the target heading to face the player (the creature's model faces backwards: turned by pi).
static void FacePlayer(AnimModel &model, const Vec4 &ownPos, const Vec4 &playerPos)
{
    model.targetHeading = Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS004_PI;
}

// Turns an angle into -pi..pi.
static float WrapAngle(float angle)
{
    while (angle > BS004_PI)
        angle -= BS004_TWO_PI;
    while (angle < -BS004_PI)
        angle += BS004_TWO_PI;
    return angle;
}

// Starts a spark effect at a random bone (not the torch bones 2 and 3 of the index list).
static void StartBoneSpark(int index)
{
    Matrix4 pose;
    pose.SetIdentity();
    int pick = FUN_1013f880() % 6;
    if (pick == 2 || pick == 3)
        pick++;
    Vec4 where = Bs004Bone(pick)->worldMatrix.GetTranslation();
    StartEffect(index, BS004_EFFECT_SPARK, where, 0, &pose, 0);
}

// 0x100c39c0 (Bs004_Update). One frame of the moss root creature in slot `index`; see the comment at the top of the
// file. While the game is over it runs the ending scene (returns 1); otherwise it returns 0.
int Bs004_Update(int index)
{
    AnimModel &model = DAT_10db6ee8[index];
    Bs004State &st = DAT_1020b430[index].bs004;
    CharSlot &slot = DAT_10daaae8[index];
    Vec4 ownPos;
    Vec4 playerPos;
    Vec4 offset;
    Vec4 step;
    Vec4 move;
    Vec4 target;
    Vec4 probe;
    int moveMode = BS004_MOVE_NONE;
    int turn = 1;

    DAT_1021f6c0[index] = 0;
    model.mode = 0;
    model.m_238c = 0;
    DAT_10218fc0[index] = 0;
    DAT_11b04510 = 1;
    model.locked = 1;
    st.motion = -1;
    DAT_104c0110 = index;
    model.uniformScale = 0.06f;
    if (DAT_104c0380 % 0x1e == index % 0x1e)
    {
        model.m_0012 = 2;
        FUN_1001c090(index, 1);
    }
    ownPos.Copy(model.matrix.GetTranslation());
    playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    offset = ownPos - playerPos;
    float distance2D = offset.Length2D();
    offset = ownPos - playerPos;
    float distance3D = offset.Length();
    DAT_1020344c = 1.8f;
    DAT_10866d98.z = playerPos.z / 3.0f + 300.0f;
    if (DAT_104bfdf0 != 0 || DAT_104c0135 != 0 || DAT_104bfe1c != 0)
    {
        model.m_0294 = 1;
        st.paused = 1;
        return 0;
    }
    if (st.paused != 0)
    {
        st.paused = 0;
        model.m_0294 = 0;
    }

    if (DAT_10210d30[index] != 0)
    {
        if (DAT_1020b478[index].value == 0)
        {
            DAT_1020b478[index].value = 1;
            for (int word = 0; word < 0xf; word++)
                DAT_1020b430[index].words[word] = 0;
        }
        AnimModel &endingA = DAT_10db6ee8[BS004_ENDING_MODEL_A];
        AnimModel &endingB = DAT_10db6ee8[BS004_ENDING_MODEL_B];
        int room = DAT_10c63ffc;
        DAT_104c0354 = 2;
        slot.m_0141 = 1;
        slot.stats[STAT_EFFECT_PARAM] = 0;
        slot.stats[STAT_BUSY] = 1;
        int roomFlags = DAT_10874848.TestScriptFlag(room * 2 + BS004_ROOM_FLAG_BASE);
        DAT_10874848.SetScriptFlag(room * 2 + BS004_ROOM_FLAG_BASE, roomFlags | (1 << (index - BS004_PEOPLE_FIRST)));
        endingB.scale.w = DAT_10255160[index][1];
        int &timer = st.hitLevel;
        if (timer < 0x1e)
        {
            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
            if (timer % 10 == 0 || FUN_1013f880() % 100 == 0)
            {
                Vec4 spot = model.matrix.GetTranslation();
                FUN_1001d8b0(index, FUN_1013f880() % 2 == 0 ? 0x321 : 0x322, spot, -1, -1, 0, 0, 0, 0, 0);
            }
            if (timer >= 0)
                FUN_100241e0(index, 0.0f, 0.0f, (550.0f - ownPos.z) / (float)(0x1e - timer));
        }
        else if (timer == 0x1e)
        {
            FUN_1013fb10(3, 200, 30);
            FUN_100b9bb0(index, 0x1ff, 1, 0);
        }
        if (timer == 0xb4)
        {
            FUN_1000db30(0, 0, 0);
            FUN_1000db30(0, 100, 20);
        }
        else if (timer == 0xc3)
        {
            FUN_1013fb10(3, 0, 10);
            DAT_10874848.MarkKilled(slot.routeRecord);
            model.SetPosition(BS004_FAR_X, 0.0f, 0.0f);
            endingA.SetPosition(BS004_FAR_X, 0.0f, 0.0f);
            endingB.SetPosition(BS004_FAR_X, 0.0f, 0.0f);
            DAT_104c00c8 = 0;
            DAT_104c0354 = 0;
            DAT_11018734 = 1;
        }
        if (st.hits == 0)
        {
            FUN_10011c60();
            for (int floor = BS004_FLOOR_FIRST; floor < BS004_FLOOR_END; floor++)
            {
                if (DAT_10daaae8[floor].kind == BS004_KIND_FLOOR && DAT_1020b430[floor].words[8] != 0)
                    DAT_1020b430[floor].words[8] = 0;
            }
            model.PlayMotion(0x12, 1, 5, 1.0f);
            st.pathPoint = 0x12;
            FUN_10065b10(index, 0x2fd, 1, 0);
            if (DAT_104bfe78 == 0)
                st.hits = 1;
            endingA.targetHeading = model.targetHeading;
            endingB.targetHeading = model.targetHeading;
            endingA.heading = model.heading;
            endingB.heading = model.heading;
            timer = BS004_ENDING_START;
            endingA.m_0000.Copy(model.matrix.GetTranslation());
            endingB.m_0000.Copy(model.matrix.GetTranslation());
            if (model.root != 0)
                model.root->M_101946b0(0, 0xffffffff);
            if (endingA.root != 0)
                endingA.root->M_101946b0(0, 0xffffffff);
            if (endingB.root != 0)
                endingB.root->M_101946b0(0, 0xffffffff);
            model.alpha = 1.0f;
            DAT_104c00c8 = 1;
            DAT_10453008[index] = -10001;
            endingA.alpha = 0.0f;
            endingB.alpha = 0.0f;
        }
        int clock = timer;
        if (clock > 0)
        {
            if (clock < 0x3c)
            {
                model.alpha = 1.0f - (float)clock / 60.0f;
            }
            else
            {
                model.m_0294 = 1;
                model.motionFrame = endingA.motionFrame;
                model.alpha = 0.0f;
            }
            if (clock < 0x3c)
                endingA.scale.Set(1.0f, 0.0f, 0.0f, endingA.scale.w + (float)(0x3c - clock) / 1830.0f);
            else if (clock < 90)
                endingA.scale.Set(1.0f, 0.0f, 0.0f, 1.0f);
            else if (clock < 0xa8)
            {
                if (clock == 0x5a)
                {
                    FUN_10065b10(index, 0x2fe, 1, 0);
                    if (DAT_10c646f4 != 0)
                    {
                        Script_ValueCommand(0x7549, 0x4b);
                        int modelId = DAT_10874848.TestScriptFlag(BS004_FLAG_DEFEAT_MODEL);
                        if (modelId != 0)
                        {
                            DAT_10db6ee8[modelId].SetPosition(-100000.0f - model.matrix._41, model.matrix._42,
                                                              model.matrix._43);
                            DAT_10c6356c = 100000;
                            DAT_10c63588 = 7;
                            Script_ValueCommand(0x753f, 0x4b);
                        }
                    }
                }
                float fade = (float)(0xa8 - clock);
                float rise = (float)(clock - 90);
                endingA.scale.Set(1.0f, endingA.scale.y + fade * fade * 0.004213826f,
                                  endingA.scale.z - rise * rise * 0.009481108f, endingA.scale.w - rise / 3081.0f);
                if (endingA.scale.w < 0.0f)
                    endingA.scale.w = 0.0f;
            }
            else
                endingA.scale.Fill(0.0f);
        }
        clock = timer;
        if (clock > 0x29)
        {
            if (clock < 0x72)
            {
                float since = (float)(clock - 0x2a);
                endingB.scale.z = 0.0f;
                endingB.scale.x = (float)(0x72 - clock) * 0.000152207f + endingB.scale.x;
                endingB.scale.y = since / 8.0f;
                endingB.scale.w = since / 144.0f + 0.5f;
                if (endingB.scale.x < 0.0f)
                    endingB.scale.x = 0.0f;
            }
            else if (clock < 0xd2)
            {
                if (clock == 0x72)
                    endingB.scale.x = 0.3f;
                float since = (float)(clock - 0x72);
                float left = (float)(0xd2 - clock);
                endingB.scale.x = endingB.scale.x - since * 8.591065e-05f;
                endingB.scale.y = left * 0.08591065f + endingB.scale.y;
                endingB.scale.z = since * 0.128866f + endingB.scale.z;
                endingB.scale.w = (left * left) / 9216.0f;
                if (endingB.scale.x < 0.0f)
                    endingB.scale.x = 0.0f;
                endingB.heading = endingB.heading - (float)(clock - 0x60) * 0.00035f;
                endingB.targetHeading = endingB.heading;
            }
            else
                endingB.scale.Fill(0.0f);
        }
        endingA.m_0294 = 1;
        endingB.m_0294 = 1;
        endingA.motionFrame = model.motionFrame;
        endingB.motionFrame = model.motionFrame;
        endingA.SetPosition(DAT_10db9240, DAT_10db9244, DAT_10db9248);
        endingB.SetPosition(DAT_10db9240, DAT_10db9244, DAT_10db9248);
        timer++;
        endingA.alpha = endingA.scale.w;
        DAT_10255160[index][1] = endingB.scale.w;
        endingB.scale.w = DAT_10255160[index][1] * 1.5f;
        endingB.alpha = endingB.scale.w;

        step.Fill(0.0f);
        step.y += 1050.0f;
        step.RotateZ(-(Atan2(DAT_10868818.y - DAT_10da9150.y, DAT_10868818.x - DAT_10da9150.x) + BS004_PI));
        DAT_10edbd48.Copy(model.matrix.GetTranslation());
        DAT_10edbd48.z += 600.0f;
        DAT_10866d98.Copy(DAT_10edbd48 + step * 1.6f);
        if (timer < 0xa1)
        {
            float k = (float)(0xa0 - timer);
            k = (k + 1.0f) * k * 0.5f;
            k = k * k;
            float blend = 1.0f - k / 3.292411e+08f;
            DAT_10866d98.Copy(DAT_10da9150 + (DAT_10866d98 - DAT_10da9150) * blend);
            DAT_10edbd48.Copy(DAT_10868818 + (DAT_10edbd48 - DAT_10868818) * blend);
        }
        probe.Fill(0.0f);
        if (DAT_10866d98.Length2D() > 1550.0f && DAT_10255f08 != 0 &&
            DAT_10255f08->CastSegment("CA", 4, &DAT_10866d98, &DAT_10edbd48, &target, 0) != 0)
        {
            float gap = (DAT_10edbd48 - target).Length();
            gap = gap <= 60.0f ? gap * 0.75f : gap - 50.0f;
            offset.Copy(DAT_10edbd48 - DAT_10866d98);
            offset.Copy(offset.Normalized3() * gap);
            DAT_10866d98.Copy(target + offset);
            if (gap < 300.0f)
            {
                offset.Copy(DAT_10866d98 - DAT_10868818);
                offset.RotateZ(0.0f);
                DAT_10da9150.Copy(DAT_10868818 + offset);
            }
        }
        float distance = (DAT_10edbd48 - DAT_10866d98).Length();
        float &closest = DAT_10255160[index][0];
        if (distance <= closest)
            closest = distance;
        else
        {
            closest = (distance - closest) * 0.3f + closest;
            offset.Copy(DAT_10edbd48 - DAT_10866d98);
            offset.Copy(offset.Normalized3() * closest);
            DAT_10866d98.Copy(DAT_10edbd48 - offset);
        }
        return 1;
    }

    // Fight. The first frame loads the models and finds the nodes.
    DAT_1021f9c0[index] = 0;
    slot.m_0141 = 0;
    slot.stats[STAT_BUSY] = 0;
    if (st.setUp == 0)
    {
        if (DAT_10255f08 == 0)
            return 0;
        DAT_1020b48c[index].value = slot.stats[STAT_VALUE_4];
        DAT_104c0354 = 3;
        st.playingMotion = -1;
        st.motion = 2;
        st.state = BS004_STATE_HOVER;
        st.frames = 0x118;
        st.requested = 1;
        Node *crown = DAT_10db6ee8[index].root->Find("futa", 0);
        if (crown != 0)
            crown->SetFlags(BS004_NODE_FLAGS_HIDE);
        st.setUp = 1;
        if (DAT_104c0390 == 0)
        {
            DAT_104c0390 = (Node *)(*DAT_11b0b938)(0x18a0, "tsm_lprootnode");
            DAT_104c0390->Init();
            DAT_104c0390->LoadIt3(FUN_1013f8a0("mossroot.it3", 0), 0, 0);
        }
        st.pathPoint = 0;
        st.previousPoint = 0;
        DAT_1021bdf8[index][BS004_PATH_NODE] = DAT_104c0390->Find("@MossBeeRoot", 0);
        DAT_1042d918[BS004_BONE_FOOT_L] = (int)model.root->Find("Bip01 L Foot", 0);
        DAT_1042d918[BS004_BONE_FOOT_R] = (int)model.root->Find("Bip01 R Foot", 0);
        DAT_1042d918[BS004_BONE_TAIL] = (int)model.root->Find("Bone_tail_top", 0);
        DAT_1042d918[BS004_BONE_TORCH] = (int)model.root->Find("Bone_torch_top", 0);
        DAT_1042d918[BS004_BONE_FOREARM_L] = (int)model.root->Find("Bip01 L Forearm", 0);
        DAT_1042d918[BS004_BONE_FINGER_L] = (int)model.root->Find("Bip01 L Finger0", 0);
        DAT_1042d918[BS004_BONE_FOREARM_R] = (int)model.root->Find("Bip01 R Forearm", 0);
        DAT_1042d918[BS004_BONE_FINGER_R] = (int)model.root->Find("Bip01 R Finger0", 0);
        DAT_10db6ee8[BS004_ENDING_MODEL_A].LoadFile(FUN_1013f8a0("bs_004en.it3", 0));
        DAT_10db6ee8[BS004_ENDING_MODEL_B].LoadFile(FUN_1013f8a0("bs_004pa.it3", 0));
        DAT_10db6ee8[BS004_ENDING_MODEL_A].LoadModel(FUN_1013f8a0("bs_004m.it3", 0));
        DAT_10db6ee8[BS004_ENDING_MODEL_B].LoadModel(FUN_1013f8a0("bs_004m.it3", 0));
        DAT_10db6ee8[BS004_ENDING_MODEL_A].mode = 0;
        DAT_10db6ee8[BS004_ENDING_MODEL_B].mode = 0;
        DAT_10db6ee8[BS004_ENDING_MODEL_A].scale.Fill(0.0f);
        DAT_10db6ee8[BS004_ENDING_MODEL_B].scale.Fill(0.0f);
        DAT_10255160[index][0] = 0.0f;
        DAT_10db6ee8[BS004_ENDING_MODEL_A].SetPosition(BS004_FAR_X, 0.0f, 0.0f);
        DAT_10db6ee8[BS004_ENDING_MODEL_B].SetPosition(BS004_FAR_X, 0.0f, 0.0f);
        DAT_10836c70.ResetFrameTimer();
    }
    DAT_10255a88[index] = 0;
    DAT_10210d30[index] = 0;
    if (DAT_10210b30[index] != 0)
        DAT_10210b30[index] = 0;

    // Health bookkeeping: the damage taken adds up, a drop below half health starts the angry phase once.
    int health = slot.stats[STAT_HEALTH];
    if (DAT_1020b474[index].value != health)
    {
        if (health < DAT_1020b474[index].value)
            DAT_1020b470[index].value += DAT_1020b474[index].value - health;
        DAT_1020b474[index].value = health;
    }
    if (DAT_1020b47c[index].value == 0 && health < slot.stats[STAT_MAX_HEALTH] / 2)
    {
        if (st.state == BS004_STATE_WAIT || st.state == BS004_STATE_APPROACH_POINT)
            st.state = BS004_STATE_PICK;
        if (st.state != BS004_STATE_PICK && st.state != BS004_STATE_APPROACH_POINT_B)
        {
            st.state = BS004_STATE_PICK;
            DAT_1020b480[index].value = 0x10;
            st.minionsDown = 0;
            DAT_1020b47c[index].value = 1;
        }
    }
    Matrix4 &lastPose = DAT_10868838[index][0];
    offset = lastPose.GetTranslation() - ownPos;
    if (offset.Length() >= 5.0f || st.state == BS004_STATE_PICK)
        DAT_1020b494[index].value = 0;
    else
    {
        int stuck = ++DAT_1020b494[index].value;
        if (stuck > 1000 ||
            (st.state != 0x13 && st.state != 0x12 && st.state != 0 && st.state != 2 && st.state != 3 &&
             st.state != 6 && stuck > 300))
        {
            st.state = BS004_STATE_APPROACH_POINT_B;
            st.frames = 0;
            st.lastState = 1;
            st.previousPoint = st.pathPoint;
            st.pathPoint = FUN_1013f880() % 0x11;
            DAT_1020b494[index].value = 0;
        }
    }
    lastPose.SetTranslation(ownPos);
    if (DAT_10db9274 == 0)
    {
        if (DAT_104c00e8 == 0x23 || DAT_104c00e8 == 0x26 || DAT_104c00e8 == 0x2a)
            DAT_1020b498[index].value = 1;
    }
    else
        DAT_1020b498[index].value = 0;
    DAT_102192c0[index] = (DAT_1020b498[index].value != 0 ? 0x177 : 0) - 0xaf;

    // Hit reaction.
    int hitLevel = DAT_102550e0[index];
    if (hitLevel != 0)
    {
        int forceDown = 0;
        st.playingMotion = -1;
        if (st.hitLevel < hitLevel)
        {
            if (st.state == BS004_STATE_WAIT)
            {
                int difficulty = DAT_10874848.TestScriptFlag(0x1c7);
                if (difficulty == 1 && st.frames < 0x7d)
                    st.frames = 0x7d;
                else if (difficulty == 0 && st.frames < 0xaf)
                    st.frames = 0xaf;
                else if (difficulty == 2 && st.frames < 0xd2)
                    st.frames = 0xd2;
            }
            st.hits++;
        }
        if (hitLevel > 10)
            DAT_102550e0[index] = 10;
        if (st.state == BS004_STATE_WAIT && slot.stats[STAT_MAX_HEALTH] / 3 < DAT_1020b470[index].value)
        {
            DAT_102550e0[index] = 0;
            st.state = BS004_STATE_PICK;
            st.lastState = BS004_STATE_WAIT;
            st.minionsDown = 0;
            forceDown = 1;
        }
        if (st.state == 2 || st.state == 3 || (st.motionLayer > 3 && st.motionLayer < 7) || slot.stats[STAT_BUSY] != 0)
        {
            if (st.motionFrame < 0)
                model.PlayMotion(st.motionLayer, 1, 3, 1.0f);
            else
            {
                model.PlayMotion(st.motionLayer, 1, 0, 1.0f);
                model.motion->frame = st.motionFrame;
                model.motion->m_0004 = (float)st.motionFrame;
                model.motion->Advance();
            }
            DAT_102550e0[index] = 0;
        }
        int current = st.state;
        if (current == 6 || current == 5 || current == 0x10 || current == 0x11)
        {
            st.hits = 0;
            if (forceDown == 0)
                FUN_100b9bb0(index, FUN_1013f880() % 3 + 0x1fa, 0, 0);
        }
        else
        {
            if (st.hits >= 5 || (DAT_1020b488[index].value >= 0x2e && st.minionsDown >= 3))
            {
                st.minionsDown = 3;
                st.state = BS004_STATE_PICK;
                DAT_1020b494[index].value = 0;
                st.hits = 0;
                DAT_102550e0[index] = 0;
            }
            else if (st.minionsDown < 3)
            {
                st.state = BS004_STATE_APPROACH_POINT_B;
                DAT_1020b494[index].value = 0;
                st.hits = 0;
                st.frames = 0;
                st.previousPoint = st.pathPoint;
                st.pathPoint = FUN_1013f880() % 0x11;
                DAT_102550e0[index] = 0;
                slot.m_0141 = 1;
            }
            if (forceDown == 0)
                FUN_100b9bb0(index, 0x1fd, 1, 0);
        }
    }

    Node *path = DAT_1021bdf8[index][BS004_PATH_NODE];
    switch (st.state)
    {
    case BS004_STATE_HOVER:
        step.Fill(0.0f);
        step.z = BS004_HOVER_HEIGHT - ownPos.z;
        step.Copy(step.Normalized3() * 5.0f);
        BlendMove(move, step, 0.9f, 0.1f);
        moveMode = BS004_MOVE_WALK;
        st.motion = 2;
        if (st.frames == 0)
            DAT_1020b488[index].value = 0;
        st.frames++;
        if (st.frames > 0x1c2 && FUN_1013f880() % st.frames > 0x172)
            st.state = BS004_STATE_PICK;
        if (st.hits > 4 || (DAT_1020b488[index].value > 0x2d && st.minionsDown > 2))
        {
            st.minionsDown = 3;
            st.state = BS004_STATE_PICK;
            DAT_1020b494[index].value = 0;
            st.hits = 0;
        }
        FacePlayer(model, ownPos, playerPos);
        break;
    case BS004_STATE_FOLLOW_PATH:
        target.Fill(0.0f);
        target.z = BS004_HOVER_HEIGHT;
        if (st.pathPoint < path->frameMatrixCount)
            target.Copy(path->frameMatrices[st.pathPoint].GetTranslation());
        step = target - ownPos;
        if (step.Length2D() >= 15.0f)
            moveMode = BS004_MOVE_WALK;
        else
            st.state = BS004_STATE_PICK;
        step.Copy(step.Normalized3() * 10.0f);
        BlendMove(move, step, 0.8f, 0.2f);
        st.motion = 0;
        model.targetHeading = Atan2(move.x, move.y) + BS004_PI;
        break;
    case BS004_STATE_DRIFT:
        step.Fill(0.0f);
        step.z = BS004_HOVER_HEIGHT - ownPos.z;
        step.Copy(step.Normalized3() * 5.0f);
        BlendMove(move, step, 0.9f, 0.1f);
        moveMode = BS004_MOVE_WALK;
        st.motion = 4;
        st.frames++;
        if (st.frames == 1)
        {
            st.minionsDown = 0;
            FUN_100b9bb0(index, 0x1f8, 0, 0);
        }
        DAT_1020b494[index].value = 0;
        if ((st.frames < 0x5b || FUN_1013f880() % st.frames < 0x5b) && (st.frames < 0x2e || st.minionsAlive > 0) &&
            st.minionsDown < 1)
        {
            if (st.frames % 0x1e == 0 && st.frames < 0x5b)
            {
                int minion = index + st.frames / 0x1e + 1;
                if (DAT_10db6ee8[minion].matrix._41 < -50000.0f || DAT_10210d30[minion] != 0)
                {
                    int fan = minion - index - 2;
                    int tries = 0;
                    target.Fill(0.0f);
                    target.z = 300.0f;
                    target.RotateY((float)fan);
                    target.Copy(target + ownPos);
                    while (target.Length2D() > 1450.0f && tries < 30)
                    {
                        fan++;
                        tries++;
                        target.Fill(0.0f);
                        target.z = 300.0f - (float)(tries * 10);
                        target.RotateY((float)fan);
                        target.Copy(target + ownPos);
                    }
                    if (target.Length2D() >= 1450.0f)
                    {
                        target.Fill(0.0f);
                        target.z = 800.0f;
                    }
                    if (target.Length2D() < 1450.0f)
                    {
                        Matrix4 pose;
                        DAT_10db6ee8[minion].SetPosition(target.x, target.y, target.z);
                        ActivateMinion(minion, fan);
                        pose.SetIdentity();
                        pose.SetTranslation(target);
                        StartEffect(index, 5, target, 0, &pose, 0);
                    }
                }
            }
        }
        else
        {
            st.state = BS004_STATE_PICK;
            st.minionsDown = 3;
            st.frames = 0;
        }
        if (st.hits > 4 || (DAT_1020b488[index].value > 0x2d && st.minionsDown > 2))
        {
            st.minionsDown = 3;
            st.state = BS004_STATE_PICK;
            DAT_1020b494[index].value = 0;
            st.hits = 0;
        }
        if (st.frames > 0x1e)
            slot.held = 1;
        FacePlayer(model, ownPos, playerPos);
        break;
    case BS004_STATE_RECOIL:
        slot.held = 0;
        st.motion = 6;
        st.frames++;
        if (model.motionFrame == 0xa0 || st.frames > 0x28)
            st.state = BS004_STATE_PICK;
        break;
    case BS004_STATE_PICK_POINT:
        st.state = BS004_STATE_APPROACH_POINT;
        st.frames = 0;
        st.previousPoint = st.pathPoint;
        st.pathPoint = FUN_1013f880() % 3 + 0x32;
        break;
    case BS004_STATE_APPROACH_POINT:
    case BS004_STATE_APPROACH_POINT_B:
    {
        bool second = st.state == BS004_STATE_APPROACH_POINT_B;
        slot.m_0141 = 1;
        st.hits = 0;
        target.Fill(0.0f);
        target.z = BS004_HOVER_HEIGHT;
        if (st.pathPoint < path->frameMatrixCount)
            target.Copy(path->frameMatrices[st.pathPoint].GetTranslation());
        step = target - ownPos;
        float distanceSquared = step.LengthSquared3();
        if (st.frames == 0)
            FUN_10065b10(index, 0x2d6, 1, 0);
        st.frames++;
        if (distanceSquared >= 100.0f)
        {
            step.Copy(step.Normalized3() * 100.0f);
            if ((step + move).LengthSquared3() <= distanceSquared)
            {
                BlendMove(move, step, 0.2f, 0.8f);
                moveMode = BS004_MOVE_FLY;
                st.motion = 7;
            }
            else
            {
                step.Fill(0.0f);
                model.SetPosition(target.x, target.y, target.z);
                st.state = second ? BS004_STATE_PICK : BS004_STATE_WAIT;
                if (!second)
                    st.lastState = 6;
                st.frames = 0;
                slot.m_0141 = 0;
            }
        }
        else
        {
            slot.m_0141 = 0;
            st.state = second ? BS004_STATE_PICK : BS004_STATE_WAIT;
            if (!second)
                st.lastState = 6;
            model.SetPosition(target.x, target.y, target.z);
        }
        model.targetHeading = Atan2(move.x, move.y);
        {
            Matrix4 pose;
            MakePose(pose, 0.0f, probe);
            probe.Copy(model.matrix.GetTranslation());
            StartEffect(index, 0x6cb, probe, 0, &pose, 0);
        }
        break;
    }
    case BS004_STATE_WAIT:
        st.hits = 0;
        st.motion = 2;
        st.frames++;
        if ((st.frames > 300 && FUN_1013f880() % st.frames > 300) ||
            slot.stats[STAT_MAX_HEALTH] / 3 < DAT_1020b470[index].value)
        {
            st.state = BS004_STATE_PICK;
            st.lastState = 6;
            st.minionsDown = 0;
        }
        FacePlayer(model, ownPos, playerPos);
        for (int spectator = BS004_SPECTATOR_FIRST; spectator < BS004_SPECTATOR_END; spectator++)
        {
            AnimModel &watcher = DAT_10db6ee8[spectator];
            int &closeness = DAT_1020b430[spectator].words[8];
            Vec4 gap = watcher.matrix.GetTranslation() - playerPos;
            if (gap.Length2D() >= 500.0f)
                closeness = 0;
            else
            {
                if (closeness != 2 || watcher.matrix._43 + 330.0f <= playerPos.z)
                    closeness = 1;
                else
                    closeness = 2;
                if (DAT_1020b430[spectator].words[6] > 0x3c)
                    DAT_1020b430[spectator].words[6] = 0x3c;
                gap = watcher.matrix.GetTranslation() - ownPos;
                if (gap.Length2D() >= 100.0f)
                {
                    if (playerPos.z <= 150.0f)
                    {
                        gap = watcher.matrix.GetTranslation() - move;
                        if (gap.Length2D() < 300.0f && (DAT_10db9274 != 0 || closeness == 2))
                            closeness = 2;
                    }
                    else
                        closeness = 3;
                }
                else
                    closeness = 3;
            }
        }
        if (st.state == BS004_STATE_PICK)
        {
            for (int floor = BS004_FLOOR_FIRST; floor < BS004_FLOOR_END; floor++)
            {
                if (DAT_10daaae8[floor].kind == BS004_KIND_FLOOR && DAT_1020b430[floor].words[8] != 0)
                    DAT_1020b430[floor].words[8] = 0;
            }
        }
        break;
    case BS004_STATE_ROAR:
        FUN_1000ec70(0xf, 0.7f, 0.3f, 0.7f, 1.0f);
        st.motion = 0x11;
        if (st.frames == 0)
        {
            FUN_10065b10(index, 0x2d7, 1, 0);
            FUN_100b9bb0(index, 0x1f9, 0, 0);
        }
        if (st.frames == 0xf)
        {
            int first = 0x2d;
            for (int candidate = 0x2d; candidate < 0x7c; candidate++)
            {
                if (DAT_1021f3c0[candidate] == 'N')
                {
                    first = candidate;
                    break;
                }
            }
            EffectOptions options;
            Matrix4 pose;
            SetUpOptions(index, options, 1.2f);
            pose.SetIdentity();
            for (int victim = first; victim < first + 13 && victim < 0x7c; victim++)
            {
                if (FUN_1013f880() % 2 == 0)
                {
                    int roll = FUN_1013f880();
                    int &word = DAT_1020b430[victim].words[6];
                    word = word < 0x2d ? roll % 0x3c + 0x5fa : 0x5dc;
                    Vec4 where = DAT_10db6ee8[victim].matrix.GetTranslation();
                    if (DAT_1020b430[victim].words[4] != 200)
                        pose.m[14] -= 150.0f;
                    pose.SetTranslation(where);
                    FUN_1001d8b0(index, 0x6cc, where, -1, -1, 0, 0, roll % 0x3c, &pose, &options);
                }
            }
        }
        st.frames++;
        if (st.frames < 0x50)
        {
            Matrix4 pose;
            pose.SetIdentity();
            FUN_1013f880();
            probe.Copy(model.matrix.GetTranslation());
            FUN_1001d8b0(index, BS004_EFFECT_SPARK, probe, -1, -1, 0, 0, 0, &pose, 0);
        }
        if (model.motionFrame == 0x261)
        {
            st.state = BS004_STATE_PICK;
            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        break;
    case BS004_STATE_DIVE_A:
    case BS004_STATE_DIVE_B:
    {
        bool deep = st.state == BS004_STATE_DIVE_B;
        float hoverAbove = deep ? 550.0f : 400.0f;
        st.frames++;
        step = playerPos - ownPos;
        step.z = (playerPos.z + hoverAbove) - ownPos.z;
        step.Normalized3();
        if (playerPos.z + hoverAbove != ownPos.z)
            step.z *= deep ? 3.0f : 4.0f;
        float speed = st.frames < 0x3c ? ((float)st.frames / 60.0f) * (deep ? 25.0f : 20.0f) + 5.0f
                                       : (deep ? 30.0f : 25.0f);
        step.Copy(step * speed);
        BlendMove(move, step, 0.7f, 0.3f);
        if (deep)
        {
            if (distance2D < 200.0f)
            {
                st.state = BS004_STATE_TAIL_SWEEP;
                st.lastState = BS004_STATE_TAIL_SWEEP;
                st.frames = 0;
            }
        }
        else if (distance2D < 600.0f && fabsf(playerPos.z + hoverAbove - ownPos.z) < 500.0f)
        {
            st.state = BS004_STATE_SWIPE;
            st.lastState = BS004_STATE_SWIPE;
            st.frames = 0;
        }
        st.motion = 0;
        moveMode = BS004_MOVE_WALK;
        FacePlayer(model, ownPos, playerPos);
        if (!deep && (st.frames & 1) != 0)
        {
            Matrix4 pose;
            pose.SetIdentity();
            probe.Fill(0.0f);
            StartEffect(index, 0x6d5, probe, &Bs004Bone(BS004_BONE_TAIL)->worldMatrix, &pose, 0);
        }
        else if (deep && st.frames % 0xf == 0)
        {
            Matrix4 poseA;
            Matrix4 poseB;
            MakeHandPoses(poseA, poseB, BS004_BONE_FOREARM_L, BS004_BONE_FOREARM_R);
            StartHandBursts(index, poseA, poseB);
        }
        break;
    }
    case BS004_STATE_SWIPE:
        st.motion = 0xe;
        slot.stats[STAT_BUSY] = 1;
        st.frames++;
        step.Fill(0.0f);
        {
            int frame = model.motion->frame;
            if (frame < 0x1b3 || model.motion->blending != 0)
            {
                step.z = ((playerPos.z - ownPos.z) + 250.0f) / (float)st.frames;
                if (step.z < -20.0f)
                    step.z = -20.0f;
            }
            else if (frame < 0x1b4 || model.motionFrame > 0x1c1)
                step.z = 0.0f;
            else
            {
                step.z = (BS004_HOVER_HEIGHT - ownPos.z) / (float)(0x1c2 - model.motionFrame);
                if (step.z > 20.0f)
                    step.z = 20.0f;
            }
        }
        BlendMove(move, step, 0.5f, 0.5f);
        moveMode = BS004_MOVE_WALK;
        if (model.motionFrame == 0x1c1)
        {
            slot.stats[STAT_BUSY] = 0;
            st.state = BS004_STATE_PICK;
            st.motion = 2;
        }
        if (model.motionFrame == 0x1a4)
        {
            EffectOptions options;
            Matrix4 pose = RotationZ(Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS004_PI);
            SetUpOptions(index, options, 1.0f);
            options.m_000e = 5;
            probe.Fill(0.0f);
            StartEffect(index, 0x6d7, probe, &Bs004Bone(BS004_BONE_TAIL)->worldMatrix, &pose, &options);
            FUN_10065b10(index, 0x2d8, 1, 0);
            FUN_100b9bb0(index, 0x1f6, 0, 1);
        }
        FacePlayer(model, ownPos, playerPos);
        break;
    case BS004_STATE_TAIL_SWEEP:
    {
        Matrix4 poseA;
        Matrix4 poseB;
        st.motion = 8;
        st.frames++;
        step = playerPos - ownPos;
        step.z = (playerPos.z + 550.0f) - ownPos.z;
        step.Normalized3();
        step.Copy(step * 5.0f);
        BlendMove(move, step, 0.7f, 0.3f);
        moveMode = BS004_MOVE_WALK;
        FacePlayer(model, ownPos, playerPos);
        if (st.frames > 0x14 && model.motionFrame == 0x116)
        {
            st.state = BS004_STATE_PICK;
            st.requested = 1;
        }
        if (model.motionFrame == 0x104)
        {
            FUN_10065b10(index, 0x2d9, 1, 0);
            FUN_100b9bb0(index, 0x1f7, 0, 1);
        }
        if (model.motionFrame < 0xff || model.motionFrame > 0x108)
        {
            if ((model.motionFrame & 1) != 0)
            {
                MakeHandPoses(poseA, poseB, BS004_BONE_FINGER_L, BS004_BONE_FINGER_R);
                StartHandBursts(index, poseA, poseB);
            }
        }
        else
        {
            MakeHandPoses(poseA, poseB, BS004_BONE_FINGER_L, BS004_BONE_FINGER_R);
            StartHandBursts(index, poseA, poseB);
            if (model.motionFrame == 0x108)
            {
                EffectOptions options;
                Matrix4 pose = RotationX(-1.5708f);
                StartHandBursts(index, poseA, poseB);
                pose.SetTranslation(ownPos);
                pose.m[14] -= 350.0f;
                SetUpOptions(index, options, 0.9f);
                StartEffect(index, ownPos.z >= 525.0f ? 0x6e3 : 0x6e1, probe, 0, &pose, &options);
                if (ownPos.z >= 525.0f)
                    FUN_10065b10(index, 0x2da, 1, 0);
            }
            if (model.motionFrame > 0x103)
            {
                Vec4 spread;
                for (int burst = 0; burst < 4; burst++)
                {
                    float wide = burst < 2 ? 0.66731f : 0.333f;
                    float narrow = burst < 2 ? 0.333f : 0.66731f;
                    spread.Copy(DAT_10802bb8[index][0] * wide + move * narrow);
                    poseA.SetTranslation(spread);
                    StartEffect(index, BS004_EFFECT_PAIR, spread, 0, &poseA, 0);
                }
            }
        }
        break;
    }
    case BS004_STATE_SLAM:
    {
        st.motion = 9;
        st.frames++;
        step.Fill(0.0f);
        if (model.motionFrame == 0x124)
        {
            float pitch = acosf(offset.z / (distance3D > 0.0f ? distance3D : 1.0f));
            EffectOptions options;
            if (pitch > 0.7854f)
                pitch = 0.7854f;
            pitch = pitch * -1.0f;
            Matrix4 pose = RotationX(pitch) * RotationZ(Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS004_PI);
            SetUpOptions(index, options, 1.3f);
            pose.m[12] += ownPos.x;
            pose.m[13] += ownPos.y - 100.0f;
            pose.m[14] = ownPos.z + 100.0f;
            StartEffect(index, 0x6e3, probe, 0, &pose, &options);
            FUN_10065b10(index, 0x2da, 1, 0);
            FUN_100b9bb0(index, 0x1f7, 0, 1);
        }
        BlendMove(move, step, 0.8f, 0.2f);
        moveMode = BS004_MOVE_WALK;
        FacePlayer(model, ownPos, playerPos);
        if (model.motionFrame == 0x134)
            st.state = BS004_STATE_PICK;
        if (model.motionFrame < 300)
        {
            Matrix4 poseA;
            Matrix4 poseB;
            MakeHandPoses(poseA, poseB, BS004_BONE_FINGER_L, BS004_BONE_FINGER_R);
            StartHandBursts(index, poseA, poseB);
            if (model.motionFrame > 0x122)
            {
                Vec4 spread;
                for (int burst = 0; burst < 4; burst++)
                {
                    float wide = burst < 2 ? 0.66731f : 0.333f;
                    float narrow = burst < 2 ? 0.333f : 0.66731f;
                    spread.Copy(DAT_10802bb8[index][0] * wide + move * narrow);
                    poseB.SetTranslation(spread);
                    StartEffect(index, BS004_EFFECT_PAIR, spread, 0, &poseB, 0);
                }
            }
        }
        break;
    }
    case BS004_STATE_RISE_A:
    case BS004_STATE_RISE_B:
    case BS004_STATE_PLUNGE_A:
    case BS004_STATE_PLUNGE_B:
    {
        // 14/15 fly up to height 950 and drop; 16/17 go up to 2550 and drop harder (state 17 also fades the screen).
        bool high = st.state == BS004_STATE_RISE_B || st.state == BS004_STATE_PLUNGE_B;
        bool dropping = st.state == BS004_STATE_PLUNGE_A || st.state == BS004_STATE_PLUNGE_B;
        float top = high ? 2550.0f : 950.0f;
        st.motion = dropping ? 0xb : 2;
        slot.stats[STAT_BUSY] = dropping ? 0 : 1;
        moveMode = BS004_MOVE_FLY;
        st.frames++;
        if (high && !dropping)
            slot.stats[STAT_VALUE_4] = 5;
        if (!dropping)
        {
            step.Fill(0.0f);
            if (st.frames < 0x3c)
            {
                step.z = -2.0f;
                BlendMove(move, step, 0.8f, 0.2f);
            }
            else
            {
                st.motion = 10;
                if (st.frames == 0x3c)
                {
                    step = ownPos - ownPos;
                    step = target - ownPos;
                    step.Copy(step * 0.5f);
                    step.z = top - ownPos.z;
                    step.Copy(step.Divided(high ? 56.0f : 28.0f));
                    DAT_10802c08[index][0].z = (top - ownPos.z) / (high ? 56.0f : 28.0f);
                }
                BlendMove(move, step, 0.8f, 0.2f);
                if (model.motionFrame == 0x156 && (high ? st.frames > 0x5a && ownPos.z > 1000.0f
                                                         : playerPos.z + 500.0f < ownPos.z))
                {
                    st.state = high ? BS004_STATE_PLUNGE_B : BS004_STATE_PLUNGE_A;
                    st.lastState = st.state;
                    st.frames = 0;
                }
            }
            if (high && st.frames == 0x3c)
                FUN_10065b10(index, 0x2dd, 1, 0);
        }
        else
        {
            step.Fill(0.0f);
            if (high)
                slot.stats[STAT_VALUE_4] = DAT_1020b48c[index].value;
            if (st.frames < 0xf)
            {
                step.z = (float)(0xf - st.frames) * 1.6f;
                BlendMove(move, step, 0.7f, 0.3f);
            }
            else if (st.frames < 0x1e)
            {
                step.z = (float)(0xf - st.frames) * 2.4f;
                BlendMove(move, step, 0.7f, 0.3f);
                if (st.frames == 0x19)
                {
                    FUN_10065b10(index, high ? 0x2de : 0x2db, 1, 0);
                    FUN_100b9bb0(index, high ? 0x1f9 : 0x1fe, 0, high ? 0 : 1);
                }
            }
            else if (st.frames < 1000)
            {
                if (ownPos.z <= 10.0f)
                {
                    EffectOptions options;
                    Matrix4 pose;
                    FUN_1016b230("");
                    FUN_10055ee0();
                    FUN_1003ad20(high ? 0x2df : 0x2dc, 0, 1.0f);
                    st.motion = 0xd;
                    st.frames = 1000;
                    model.SetPosition(ownPos.x, ownPos.y, 0.0f);
                    SetUpOptions(index, options, 1.4f);
                    MakePose(pose, 0.0f, ownPos);
                    StartEffect(index, high ? 0x6ec : 0x6e7, ownPos, 0, &pose, &options);
                    StartEffect(index, 0x6db, ownPos, 0, &pose, &options);
                }
                else
                {
                    step.z = (float)(0x1e - st.frames) * (high ? 1.2f : 2.4f) - 36.0f;
                    BlendMove(move, step, 0.7f, 0.3f);
                }
            }
            else if (st.frames < 0x3eb)
                st.motion = 0xd;
            else if (st.frames < 2000)
            {
                st.motion = 0xd;
                st.frames = 2000;
                move.Copy(move * -2.0f);
                DAT_10802c08[index][0].z = high ? 80.0f : 40.0f;
            }
            else
            {
                if (high)
                    FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                st.motion = 0xd;
                BlendMove(move, step, 0.2f, 0.8f);
                move.Copy(move * 0.9f);
                if (st.frames > 0x7ed)
                {
                    st.state = BS004_STATE_PICK;
                    st.motion = 2;
                }
            }
            if (st.frames < 2000)
            {
                float spin = DAT_10db9364 + (high ? 0.6f : 0.2f);
                DAT_10db9364 = spin;
                model.targetHeading = spin;
                if (st.frames > 0xf && st.frames < 1000)
                {
                    EffectOptions options;
                    Matrix4 pose;
                    pose.SetIdentity();
                    SetUpOptions(index, options, 1.0f);
                    options.flags |= 2;
                    StartEffect(index, 0x6e5, ownPos, 0, &pose, 0);
                    StartEffect(index, 0x6e5, ownPos, 0, &pose, 0);
                    if (high)
                        StartEffect(index, 0x6e5, ownPos, 0, &pose, 0);
                    StartEffect(index, 0x45d, ownPos, 0, &pose, &options);
                }
            }
        }
        if (!dropping)
            FacePlayer(model, ownPos, playerPos);
        break;
    }
    case BS004_STATE_GROUND_BLAST:
        st.frames++;
        slot.stats[STAT_BUSY] = 1;
        st.motion = 0xf;
        step.Fill(0.0f);
        if (st.frames < 0x1e)
        {
            slot.stats[STAT_BUSY] = 0;
            st.motion = 2;
            if (ownPos.z >= 650.0f)
                st.frames = 0x1d;
            else
            {
                step.z = 10.0f;
                st.frames = 0x1c;
            }
        }
        else if (st.frames < 0x3c)
        {
            if (st.frames == 0x3b)
            {
                EffectOptions options;
                SetUpOptions(index, options, 1.0f);
                options.flags |= 2;
                probe.Fill(0.0f);
                FUN_1001d8b0(index, 0x6fa, probe, -1, -1, &model.root->Find("Bip01", 0)->worldMatrix, 0, 0, 0, &options);
            }
            else if (st.frames == 0x25)
                FUN_10065b10(index, 0x2e0, 1, 0);
        }
        else if (st.frames < 0x78)
        {
            step.z = -8.0f;
            StartBoneSpark(index);
        }
        else if (st.frames == 0x78)
            st.motion = 0x15;
        else if (model.motionFrame == 0x1e0)
        {
            float pitch = acosf(offset.z / (distance3D > 0.0f ? distance3D : 1.0f));
            EffectOptions options;
            SetUpOptions(index, options, 2.0f);
            if (pitch > 0.7854f)
                pitch = 0.7854f;
            pitch = pitch * -1.0f;
            Matrix4 pose = RotationX(pitch) * RotationZ(DAT_10db9364);
            StartEffect(index, 0x6f3, ownPos, 0, &pose, &options);
            FUN_10065b10(index, 0x2e1, 1, 0);
            FUN_100b9bb0(index, 0x1f5, 0, 1);
            probe.Fill(0.0f);
            probe.y = -1.0f;
            probe.RotateX(pitch);
            probe.RotateZ(DAT_10db9364);
            if (probe.z < -0.01f)
            {
                probe.Copy(probe * (ownPos.z / probe.z));
                probe.Copy(probe + ownPos);
                probe.z = 0.0f;
                pose.SetTranslation(probe);
                StartEffect(index, 0x6f6, probe, 0, &pose, &options);
            }
        }
        else if (model.motionFrame == 0x1fd)
        {
            st.state = BS004_STATE_PICK;
            slot.stats[STAT_BUSY] = 0;
        }
        if (st.frames < 0x79)
        {
            if (st.frames < 0x73)
                FacePlayer(model, ownPos, playerPos);
        }
        else
            st.motion = 0x15;
        BlendMove(move, step, 0.9f, 0.1f);
        break;
    case BS004_STATE_FIRE_RAIN:
        st.motion = 0x10;
        step.Fill(0.0f);
        moveMode = BS004_MOVE_WALK;
        st.frames++;
        step.z = (600.0f - ownPos.z) / 20.0f;
        BlendMove(move, step, 0.9f, 0.1f);
        if (st.frames == 0)
        {
            FUN_10065b10(index, 0x2e2, 1, 0);
            FUN_100b9bb0(index, 0x1f8, 0, 1);
        }
        else if (st.frames > 0x18 && st.frames % 7 == 0 && FUN_1013f880() % 0x1e < 0x14)
        {
            EffectOptions options;
            Matrix4 pose = RotationZ(DAT_10db9364);
            SetUpOptions(index, options, 0.75f);
            probe.Fill(0.0f);
            probe.y = -150.0f;
            probe.x = (float)(FUN_1013f880() % 600 - 300);
            probe.z = (float)(FUN_1013f880() % 600 - 300);
            probe.RotateX((float)(FUN_1013f880() % 60 - 30) / 180.0f * BS004_PI);
            probe.RotateZ(((float)(FUN_1013f880() % 60 - 30) / 180.0f) * BS004_PI + DAT_10db9364);
            probe.Copy(probe + ownPos);
            pose.SetTranslation(probe);
            StartEffect(index, 0x6fc, probe, 0, &pose, &options);
            FUN_1003ad20(0x2e3, &ownPos, 1.0f);
        }
        if (st.frames > 0x96 && (st.frames >> 1) < FUN_1013f880() % st.frames)
            st.state = BS004_STATE_PICK;
        if (st.frames % 3 == 0)
            StartBoneSpark(index);
        FacePlayer(model, ownPos, playerPos);
        break;
    case BS004_STATE_PICK:
    {
        // Picks the next state: the odds of each attack depend on whether it is angry (health below half), how often
        // it picked an attack in a row, and what it did last.
        int picked = DAT_1020b484[index].value;
        int angry = DAT_1020b47c[index].value;
        int handled = 0;
        bool picksAttack;
        DAT_1020b470[index].value = 0;
        if (angry == 0)
        {
            int divisor = 6 - picked;
            if (divisor < 2)
                divisor = 2;
            picksAttack = FUN_1013f880() % divisor == 0;
            if (!picksAttack && picked > 2)
                picksAttack = FUN_1013f880() % 100 < (picked * picked * picked) / 2;
        }
        else
        {
            int divisor = 0xc - picked;
            if (divisor < 3)
                divisor = 3;
            picksAttack = FUN_1013f880() % divisor == 0;
            if (!picksAttack && picked > 5)
                picksAttack = FUN_1013f880() % 100 < picked * picked;
        }
        switch (st.lastState)
        {
        case 0:
            DAT_1020b494[index].value = 0;
            if (FUN_1013f880() % 2 != 0)
            {
                st.state = BS004_STATE_FOLLOW_PATH;
                handled = 1;
            }
            else
                picksAttack = true;
            break;
        case 1:
            if (!picksAttack)
            {
                if (st.pathPoint < 0x12)
                    st.state = BS004_STATE_DRIFT;
                else
                {
                    st.previousPoint = st.pathPoint;
                    st.state = BS004_STATE_FOLLOW_PATH;
                    st.pathPoint = FUN_1013f880() % 0x11;
                }
                handled = 1;
            }
            break;
        case 2:
            st.state = BS004_STATE_RECOIL;
            DAT_1020b494[index].value = 0;
            handled = 1;
            break;
        case 3:
            DAT_1020b494[index].value = 0;
            st.state = BS004_STATE_HOVER;
            handled = 1;
            break;
        case 6:
            st.state = BS004_STATE_APPROACH_POINT_B;
            handled = 1;
            break;
        case 10:
            picksAttack = true;
            break;
        default:
            if (st.requested == 1)
            {
                st.state = BS004_STATE_FOLLOW_PATH;
                st.requested = 0;
                handled = 1;
            }
            else if (st.requested == 2)
            {
                st.state = BS004_STATE_APPROACH_POINT_B;
                st.requested = 0;
                handled = 1;
            }
            else if (!picksAttack)
            {
                st.state = BS004_STATE_FOLLOW_PATH;
                handled = 2;
            }
            break;
        }
        if (handled == 0 || picksAttack)
        {
            // The attacks in the order of their cumulative weights.
            static const int attacks[8] = {8, 7, 0xb, 0xd, 0xe, 0x10, 0x12, 0x13};
            int weight[7];
            int hard = DAT_1020b47c[index].value != 0;
            int scale = hard ? 2 : 1;
            int roarWeight = hard ? 0x96 : 200;
            int diveWeight = hard ? 0x96 : 100;
            int slamWeight = hard ? 0x96 : 100;
            int riseAWeight = hard ? 0x32 : 0;
            int riseBWeight = hard ? 0x28 : 0;
            int blastWeight = hard ? 0x4b : 0;
            int plungeWeight = hard ? 100 : 10;
            int fireWeight = 100;
            DAT_1020b484[index].value++;
            switch (st.lastState)
            {
            case 7:
                diveWeight += scale * 300;
                roarWeight += scale * 200;
                break;
            case 9:
                fireWeight = scale * 200 + 100;
                break;
            case 0xc:
                fireWeight = scale * 500 + 100;
                break;
            case 0xd:
                plungeWeight += scale * 100;
                break;
            case 0xf:
                roarWeight += scale * 100;
                break;
            case 0x11:
                blastWeight += scale * 200;
                break;
            case 0x12:
                blastWeight += scale * 200;
                fireWeight = scale * 200 + 100;
                break;
            case 0x13:
                riseAWeight += scale * 100;
                roarWeight += scale * 200;
                break;
            }
            weight[0] = roarWeight;
            weight[1] = weight[0] + riseAWeight;
            weight[2] = diveWeight + weight[1];
            weight[3] = fireWeight + weight[2];
            weight[4] = weight[3] + 0x32;
            weight[5] = riseBWeight + weight[4];
            weight[6] = slamWeight + weight[5];
            int total = weight[6] + plungeWeight + blastWeight;
            int roll = FUN_1013f880() % total;
            int pick = 0;
            do
            {
                if (roll < weight[pick])
                    break;
                pick++;
            } while (pick < 7);
            st.state = attacks[pick];
        }
        if (DAT_1020b480[index].value != 0)
        {
            st.state = DAT_1020b480[index].value;
            DAT_1020b480[index].value = 0;
        }
        st.frames = 0;
        if (st.minionsDown > 2 && st.state != BS004_STATE_RECOIL && st.lastState != BS004_STATE_RECOIL)
            st.state = BS004_STATE_PICK_POINT;
        st.lastState = st.state;
        if (st.state == BS004_STATE_FOLLOW_PATH && FUN_1013f880() % 1000 < 0x96)
            st.state = BS004_STATE_APPROACH_POINT_B;
        if ((st.state == BS004_STATE_FOLLOW_PATH && handled == 2) || st.state == BS004_STATE_APPROACH_POINT_B)
        {
            // Goes to one of the two path points nearest to where it moves.
            int nearest = -1;
            int second = -1;
            float nearestDistance = 0.0f;
            float secondDistance = 0.0f;
            for (int point = 0; point < 0x11; point++)
            {
                float d = (path->frameMatrices[point].GetTranslation() - ownPos).LengthSquared3();
                if (nearest == -1 || d < nearestDistance)
                {
                    second = nearest;
                    secondDistance = nearestDistance;
                    nearest = point;
                    nearestDistance = d;
                }
                else if (second == -1 || d < secondDistance)
                {
                    second = point;
                    secondDistance = d;
                }
            }
            st.previousPoint = st.pathPoint;
            st.pathPoint = FUN_1013f880() % 2 == 0 ? second : nearest;
        }
        else if (st.state == BS004_STATE_FOLLOW_PATH && handled == 1)
        {
            st.previousPoint = st.pathPoint;
            st.pathPoint = FUN_1013f880() % 0x11;
        }
        if (st.state == BS004_STATE_HOVER)
            DAT_1020b484[index].value = 0;
        break;
    }
    }

    // Start the motion the state asked for.
    if (st.motion > -1 && st.motion != st.playingMotion)
    {
        model.PlayMotion(st.motion, 0, 3, 1.0f);
        st.playingMotion = st.motion;
    }
    if (moveMode == BS004_MOVE_WALK)
    {
        FUN_100241e0(index, move.x * model.speedScale, move.y * model.speedScale, move.z * model.speedScale);
    }
    else if (moveMode == BS004_MOVE_FLY)
    {
        Vec4 shrunk = move;
        int tries = 0x1e;
        do
        {
            probe.Copy(ownPos + shrunk);
            if (model.IsClearOfMapLimited(&shrunk) != 0 && DAT_10255f08 != 0 &&
                DAT_10255f08->CastSegment(0, 4, &probe, &ownPos, &target, 0) == 0)
            {
                if (shrunk.Length2D() < 2000.0f)
                    break;
            }
            tries--;
            shrunk.x *= 0.9f;
            shrunk.y *= 0.9f;
            shrunk.z *= 0.95f;
        } while (tries != 0);
        probe.Copy(ownPos + shrunk);
        float height = DAT_10802c08[index][0].z + probe.z;
        if (tries != 0 || (height > 0.0f && height < 1000.0f) || height < 0.0f)
            model.SetPosition(probe.x, probe.y, probe.z);
        turn = 0;
    }

    // Count the minions that are down and standing, and note when none changed.
    int standing = 0;
    for (int minion = index + BS004_MINION_FIRST_OFFSET; minion < index + 5; minion++)
    {
        int &seen = DAT_1020b430[minion].words[4];
        if (seen == 0)
        {
            if (DAT_10210d30[minion] > 0)
            {
                seen = 1;
                st.minionsDown++;
            }
            if (seen == 0 && DAT_10db6ee8[minion].matrix._41 > -50000.0f)
                standing++;
        }
    }
    DAT_1020b488[index].value++;
    if (st.minionsAlive != standing)
        DAT_1020b488[index].value = 0;
    st.minionsAlive = standing;
    st.hitLevel = DAT_102550e0[index];
    if (model.motion->blending == 0)
    {
        st.motionLayer = model.motion->layer;
        st.motionFrame = model.motion->frame;
    }
    else
    {
        st.motionLayer = model.motion->nextLayer;
        st.motionFrame = -1;
    }

    // Turn towards the target heading, at most a third (2.5 while it flies) of the angle per frame.
    if (turn != 0)
    {
        float wanted = WrapAngle(model.targetHeading);
        float difference = WrapAngle(WrapAngle(model.heading) - wanted);
        if (difference != 0.0f && st.state != BS004_STATE_APPROACH_POINT)
            difference = difference / 2.5f;
        model.targetHeading = model.heading - difference;
    }

    // Footstep and flap sounds on the frames of its motions.
    switch (model.motionFrame)
    {
    case 0x219:
    case 0x230:
    case 0x242:
    case 0x256:
    case 0x276:
    case 0x28a:
    case 0x1ae:
    case 0x6e:
    case 10:
    case 0x26:
    case 0x4c:
    case 0x82:
    case 0x14a:
    case 0x197:
        FUN_10065b10(index, 0x2e4, 1, 0);
        break;
    }
    return 0;
}
