// The BS_002 boss (kind 105): a beast whose body is two slots (the slot before the boss holds the second half). After the
// game is over (DAT_10210d30) it runs the defeat scene (two effect models grow and fade while the camera slides along
// it), otherwise the fight: it circles the player on a ring (states 1 and 2), walks the ring (4), spits rings of shots (5),
// stomps (6), lunges at the player from the ring (8), roars (0xb), whirls (10), claws (0xc..0xf) and so on, climbing and
// dropping to the pit floor while a smoothed height follows its target. A hit makes it stagger or roar; balloons and the
// other characters in reach are blasted when it lands.

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
#include "bs002.h"
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

#define BS002_PI 3.1415f
#define BS002_TWO_PI 6.283f
#define BS002_ENDING_MODEL_A 88                 // the three models of the ending (bs_002en.it3, bs_002pa.it3 and the one after)
#define BS002_ENDING_MODEL_B 89
#define BS002_ENDING_MODEL_C 90
#define BS002_ENDING_START (-30)
#define BS002_PEOPLE_FIRST 0x3b
#define BS002_ROOM_FLAG_BASE 1000               // + 2 * room: flag word of the ending
#define BS002_FAR_X (-200000.0f)
#define BS002_DIFFICULTY_FLAG 0x1c7             // script flag holding the difficulty (0, 1 or 2)
#define BS002_HALF_HEALTH_FLAG 0xf79            // script flag tested before the first hit of the fight
#define BS002_BALLOON_FIRST 0x5c                // slots 0x5c..0x7b: balloons ('B') and the floor characters it may hit
#define BS002_BALLOON_END 124
#define BS002_BYSTANDER_FIRST 0x3c              // slots 0x3c..0x40: bystanders that come back after a long time away
#define BS002_BYSTANDER_END 0x41
#define BS002_PIECE_COUNT 5                     // slots index + 1 .. index + 5 are its pieces (kind 0x2c)
#define BS002_PIECE_KIND 0x2c
#define BS002_RING_STEPS 14                     // entries of the table of ring angles it may stop at
#define BS002_RING_WRAP 6283                    // 2 pi in 1/1000 radians (the ring angle wraps with 0x188b)
#define BS002_NODE_FLAGS_HIDE 0x20
#define BS002_BASE_KIND_PACKED_GROUP 0x31
#define BS002_BASE_KIND_HIDE_ARMOUR 0x10

// Bones (DAT_1042d918[...]).
#define BS002_BONE_HEAD 0
#define BS002_BONE_NECK 1
#define BS002_BONE_SPINE 2
#define BS002_BONE_PELVIS 3
#define BS002_BONE_LEFT_ARM 4
#define BS002_BONE_RIGHT_ARM 5
#define BS002_BONE_LEFT_THIGH 6
#define BS002_BONE_RIGHT_THIGH 7

// DAT_1021bdf8[index][...] and DAT_1021bdd4[index][...]: the nodes it keeps.
#define BS002_NODE_HEAD 0
#define BS002_NODE_FOOTSTEPS 1
#define BS002_NODE_NECK 2
#define BS002_NODE_HEAD_B 3
#define BS002_NODE_TAIL 4
#define BS002_NODE_SPHERE_1 5
#define BS002_NODE_SPHERE_4 6
#define BS002_NODE_SPHERE_2 0                   // DAT_1021bdd4[index][...]
#define BS002_NODE_SPHERE_3 1
#define BS002_NODE_PELVIS 2
#define BS002_NODE_TAIL_B 3
#define BS002_NODE_DUMMY 4

// States (Bs002State::state).
#define BS002_STATE_SET_UP 0
#define BS002_STATE_CIRCLE_LEFT 1
#define BS002_STATE_CIRCLE_RIGHT 2
#define BS002_STATE_RING_LUNGE 3
#define BS002_STATE_RING_WALK 4
#define BS002_STATE_RING_SHOTS 5
#define BS002_STATE_STOMP 6
#define BS002_STATE_ROAR_WAVES 7
#define BS002_STATE_CLIMB 8
#define BS002_STATE_GLARE 9
#define BS002_STATE_WHIRL 10
#define BS002_STATE_STAGGER 0xb
#define BS002_STATE_CLAW_RUN 0xc
#define BS002_STATE_SWIPE 0xd
#define BS002_STATE_CLAW_B 0xe
#define BS002_STATE_CLAW_C 0xf
#define BS002_STATE_PICK 100

#define BS002_MOVE_NONE 0
#define BS002_MOVE_TOWARDS 1                    // moves by the vector to the goal (at most 32 units a frame)
#define BS002_MOVE_BLEND 2                      // as 1 and blends in the first movement (the ring walk)

#define BS002_FRAMES_PART_TWO 1000
#define BS002_FRAMES_PART_THREE 2000
#define BS002_FRAMES_PART_FOUR 3000

static const int sRingAngles[BS002_RING_STEPS] = {0x260,  0x442,  0x514,  0x72b,  0x7d5,  0x9f6, 0xb45,
                                                  0xd48,  0xea6,  0x10bd, 0x1156, 0x1374, 0x143c, 0x1642};

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

static inline Node *Bs002Bone(int which)
{
    return (Node *)DAT_1042d918[which];
}

// Keeps the ring angle inside 0..0x188b (the full turn in 1/1000 radians plus a little).
static int WrapRingAngle(int angle)
{
    if (angle < 0)
        angle += 0x188b + ((unsigned)(-angle - 1) / 0x188b) * 0x188b;
    if (angle > 0x188b)
        angle += -0x188b + ((unsigned)(angle - 0x188c) / 0x188b) * -0x188b;
    return angle;
}

// Distance of `angle` to ring angle `step`.
static int RingAngleGap(int angle, int step)
{
    int gap = angle - step;
    return gap < 0 ? -gap : gap;
}


// Starts effect `effect` for the boss at `where`. `owner` is the node matrix it follows (0 = none), `pose` the matrix
// it is placed with and `options` the start options (0 = defaults).
static void StartEffect(int index, int effect, const Vec4 &where, const Matrix4 *owner, int value, int frame,
                        const Matrix4 *pose, const EffectOptions *options)
{
    FUN_1001d8b0(index, effect, where, -1, -1, owner, value, frame, pose, options);
}

// The goal on the ring round the arena's middle: (cos, sin) of `angle` times `radius` at `height`.
static void RingGoal(Vec4 &goal, float angle, float radius, int height)
{
    goal.Set((float)cos(angle) * radius, (float)sin(angle) * radius, (float)height, 0.0f);
}

// Dust and rubble (effect 0x2e5, twice: plain and 5 frames late) at `count` random places round the boss.
static void StartDebrisPuffs(int index, AnimModel &model, int count)
{
    Matrix4 pose;
    pose.SetIdentity();
    for (; count > 0; count--)
    {
        Vec4 spot;
        spot.Fill(0.0f);
        spot.x = (float)(FUN_1013f880() % 300 - 150);
        spot.y = (float)(FUN_1013f880() % 900 - 450);
        spot.z = 0.0f;
        spot.RotateZ(model.heading);
        spot.Copy(spot + model.matrix.GetTranslation());
        StartEffect(index, 0x2e5, spot, 0, 0, 0, &pose, 0);
        StartEffect(index, 0x2e5, spot, 0, 0, 5, &pose, 0);
    }
}

// A shock wave (effect 0x44e) between the two arm bones, `drop` below them and turned to the boss's back, followed
// by `count` puffs of rubble.
static void StartRoarBurst(int index, AnimModel &model, int boneA, int boneB, float drop, int count)
{
    Vec4 a;
    Vec4 b;
    Vec4 middle;
    Matrix4 pose;
    a.Copy(((Node *)boneA)->worldMatrix.GetTranslation());
    b.Copy(((Node *)boneB)->worldMatrix.GetTranslation());
    middle.Set((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f, (a.z + b.z) * 0.5f - drop, 0.0f);
    pose.SetIdentity();
    pose = RotationZ(model.heading + BS002_PI);
    pose.SetTranslation(middle);
    StartEffect(index, 0x44e, middle, 0, 0, 0, &pose, 0);
    StartDebrisPuffs(index, model, count);
}

// The glow (effect 0x454) between the arm bones, 200 below them, while the boss pushes up from the floor.
static void StartArmGlow(int index, AnimModel &model, int boneA, int boneB)
{
    Vec4 a;
    Vec4 b;
    Vec4 middle;
    Matrix4 pose;
    a.Copy(((Node *)boneA)->worldMatrix.GetTranslation());
    b.Copy(((Node *)boneB)->worldMatrix.GetTranslation());
    middle.Set((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f, (a.z + b.z) * 0.5f - 200.0f, 0.0f);
    pose.SetIdentity();
    pose = RotationZ(model.heading + BS002_PI);
    pose.SetTranslation(middle);
    StartEffect(index, 0x454, middle, 0, 0, 0, &pose, 0);
}

// Splashes (effects 0x707 twice, 0x9b and 0x708, each with its own chance) at 36 places along the pool's edge.
static void StartStaggerSplashes(int index, AnimModel &model)
{
    Matrix4 pose;
    pose.SetIdentity();
    for (int place = 0; place < 0x24; place++)
    {
        Vec4 spot;
        spot.Fill(0.0f);
        spot.x = (float)place;
        spot.y = -680.0f;
        spot.z = -750.0f;
        spot.RotateZ(model.heading);
        if (FUN_1013f880() % 0xf == 0)
        {
            StartEffect(index, 0x707, spot, 0, 0, 0, &pose, 0);
            StartEffect(index, 0x707, spot, 0, 0, 5, &pose, 0);
        }
        if (FUN_1013f880() % 0x14 == 0)
            StartEffect(index, 0x9b, spot, 0, 0, 0, &pose, 0);
        if (FUN_1013f880() % 0x32 == 0)
            StartEffect(index, 0x708, spot, 0, 0, 2, &pose, 0);
    }
}

// A pillar of light (effect 0x6fa) 300 above the boss that follows it, with its roar.
static void StartPillar(int index, AnimModel &model)
{
    Matrix4 pose;
    EffectOptions options;
    Vec4 where;
    options.flags |= 2;
    options.owner = index;
    where.Copy(model.matrix.GetTranslation());
    where.z += 300.0f;
    pose.SetIdentity();
    pose.SetTranslation(where);
    StartEffect(index, 0x6fa, where, 0, 0, 0, &pose, &options);
    FUN_10065b10(index, 0x2a0, 1, 0);
    FUN_100b9bb0(index, 0x23, 0, 1);
}

// Rubble thrown up on the floor round the boss while it whirls.
static void StartFloorSplash(int index, AnimModel &model, const Vec4 &ownPos)
{
    Matrix4 pose;
    Vec4 spot;
    pose.SetIdentity();
    spot.Fill(0.0f);
    spot.x = (float)(FUN_1013f880() % 300 - 150);
    spot.y = (float)(FUN_1013f880() % 900 - 450);
    spot.z = 0.0f;
    spot.RotateZ(model.heading);
    spot.Copy(spot + ownPos);
    StartEffect(index, 0x2e5, spot, 0, 0, 0, &pose, 0);
}

// The fan of shots (5, or 7 on the hardest setting) of the ring shots state: effect 0x451 straight ahead and 0x700 for
// the others, spread over a fan that is wider on the easier settings.
static void FireShotFan(int index, AnimModel &model, CharSlot &slot, Bs002Extra &ex)
{
    int difficulty = DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG);
    int count = difficulty == 2 ? 7 : 5;
    for (int shot = 0; shot < count; shot++)
    {
        Matrix4 pose;
        EffectOptions options;
        Vec4 where;
        float spread;
        if (difficulty == 2)
            spread = (float)(shot - 3) * 0.31415f;
        else if (difficulty >= 0)
            spread = (float)(shot - 2) * 0.6754225f;
        else
            spread = (float)(shot - 2) * 0.18849f;
        pose.SetIdentity();
        pose = RotationZ(spread + model.targetHeading);
        where.Copy(model.matrix.GetTranslation());
        where.z -= 50.0f;
        pose.SetTranslation(where);
        options.m_0000 = 0;
        options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
        options.m_0008 = 0.8f;
        StartEffect(index, shot == 0 ? 0x451 : 0x700, where, 0, 0, shot == 0 ? 0 : 0xf, &pose, &options);
        FUN_10065b10(index, 0x28b, 1, 0);
    }
    ex.variant = FUN_1013f880() % 4 + 1;
    FUN_100b9bb0(index, 0x20, 0, 1);
}

// The end of the ring shots and the glare: stomp when the player is close and above the floor, else take off while the
// boss is near the middle, else climb when it is hurt, else pick.
static void ChooseFollowUp(Bs002State &st, CharSlot &slot, float distance2D, Vec4 &ownPos, const Vec4 &playerPos)
{
    if (distance2D < 400.0f && DAT_104bfe24 < playerPos.z)
    {
        st.state = BS002_STATE_STOMP;
        st.frames = 0;
        return;
    }
    int half = slot.stats[STAT_MAX_HEALTH] / 2;
    bool takeOff = DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG) == 1 || slot.stats[STAT_HEALTH] > half;
    if (takeOff && ownPos.Length2D() < 600.0f)
    {
        st.state = BS002_STATE_ROAR_WAVES;
        st.frames = 0;
    }
    else if (slot.stats[STAT_HEALTH] < half)
    {
        st.state = BS002_STATE_CLIMB;
        st.frames = 0;
    }
    else
        st.state = BS002_STATE_PICK;
}

// The trail of the claw run: every third frame three effects (0x6a2..0x6a4) at the claw side, every sixth a dust cloud
// (0xbca) and a floor effect (0x714), and the balloons are blasted every frame.
static void ClawTrail(int index, AnimModel &model, CharSlot &slot, Bs002State &st, Vec4 &ownPos, const Vec4 &playerPos)
{
    EffectOptions options;
    options.flags |= 2;
    if (st.frames % 3 == 0)
    {
        Matrix4 pose;
        Vec4 where;
        Vec4 side;
        side.Fill(0.0f);
        side.x = (float)(st.direction * 100);
        side.RotateZ(model.heading);
        where.Copy(model.matrix.GetTranslation() + side);
        pose.SetIdentity();
        pose = RotationZ(Atan2(playerPos.x - where.x, playerPos.y - where.y) + BS002_PI);
        pose.SetTranslation(where);
        StartEffect(index, 0x6a2, where, 0, 0, 0, &pose, &options);
        StartEffect(index, 0x6a3, where, 0, 0, 0, &pose, &options);
        StartEffect(index, 0x6a4, where, 0, 0, 0, &pose, &options);
        if (st.frames % 6 == 0)
        {
            StartEffect(index, 0xbca, where, 0, 0, 0, &pose, &options);
            FUN_10065b10(index, 0x293, 1, 0);
        }
        pose._43 = DAT_104bfe24 + 5.0f;
        StartEffect(index, 0x714, where, 0, 0, 0, &pose, &options);
    }
    FUN_100543a0(ownPos);
}

// Brings a bystander slot (0x3c..0x40) that was gone for a while back: a random place on the ring of 1000, full health
// (at least 5), every table of the slot cleared like Char_SpawnEnemy (0x10050510) does and its equipment put back on.
static void RespawnBystander(int slot)
{
    AnimModel &bystander = DAT_10db6ee8[slot];
    CharSlot &bystanderSlot = DAT_10daaae8[slot];
    float angle = (float)(FUN_1013f880() % 0x168) / 180.0f * BS002_PI;
    bystander.root->M_10194630(slot, 0x400);
    bystander.root->M_101946b0(0xfe, 0x400);
    bystander.root->M_10194730(0x2f, 0x400);
    bystander.root->M_10194630(0, 0x200);
    bystander.root->M_101946b0(0xff, 0x200);
    bystander.root->M_10194730(0x2f, 0x200);
    DAT_10210d30[slot] = 0;
    bystander.alpha = 1.0f;
    bystanderSlot.stats[STAT_HEALTH] = bystanderSlot.stats[STAT_MAX_HEALTH];
    if (bystanderSlot.stats[STAT_HEALTH] < 1)
        bystanderSlot.stats[STAT_HEALTH] = 5;
    bystander.SetPosition((float)cos(angle) * 1000.0f, (float)sin(angle) * 1000.0f, 1000.0f);
    DAT_10453008[slot] = 0;
    DAT_1030b8f8[slot] = 0;
    for (int word = 0; word < 0x10; word++)
        DAT_1020b430[slot].words[word] = 0;
    DAT_1020d540[slot] = 0;
    DAT_10254090[slot] = 1;
    DAT_10254598[slot] = 0x5b;
    DAT_103086b0[slot] = 1;
    DAT_10256420[slot] = 0;
    bystander.shadowOpacity = 1;
    DAT_10254390[slot] = 0;
    DAT_1021f940[slot] = 0;
    DAT_1020e138[slot] = -1;
    bystander.m_0294 = 0;
    DAT_10253a58[slot] = 0;
    DAT_1021b4c8[slot] = (char)(slot % 5) - 1;
    DAT_10467510[slot] = 0;
    bystander.m_02a8 = 0;
    bystander.holdState = 0;
    bystander.holdWeight = 0.0f;
    DAT_1030c480[slot] = 0;
    bystander.m_022c.Fill(0.0f);
    DAT_10467ba0[slot] = 0;
    DAT_102239d0[slot] = 0;
    DAT_10210d30[slot] = 0;
    DAT_104686b0[slot] = 0;
    DAT_1020d4c0[slot] = 0;
    DAT_102550e0[slot] = 0;
    DAT_10254090[slot] = 1;
    DAT_10218fc0[slot] = 0;
    DAT_10210b30[slot] = 0;
    DAT_10802bf8[slot].position.Fill(0.0f);
    DAT_10802bf8[slot].step.Fill(0.0f);
    int packed = bystanderSlot.stats[9];
    DAT_10315350[slot] = 0;
    DAT_1021f6c0[slot] = 0;
    DAT_102548c0[slot] = 0;
    DAT_10219e40[slot] = 0;
    DAT_1042d9e8[slot] = 0;
    bool hidesArmour = bystanderSlot.kind == BS002_BASE_KIND_HIDE_ARMOUR || DAT_1030b668[slot] == BS002_BASE_KIND_HIDE_ARMOUR;
    if (packed / 10000 == 0 &&
        ((bystanderSlot.kind != BS002_BASE_KIND_PACKED_GROUP && DAT_1030b668[slot] != BS002_BASE_KIND_PACKED_GROUP) ||
         packed % 100 == 0))
    {
        if (hidesArmour)
        {
            Node *armour = bystander.root->Find("@EN_016ar.lwo", 0);
            if (armour != 0)
                armour->ClearFlags(BS002_NODE_FLAGS_HIDE);
        }
    }
    else
    {
        if (hidesArmour)
        {
            Node *armour = bystander.root->Find("@EN_016ar.lwo", 0);
            if (armour != 0)
                armour->SetFlags(BS002_NODE_FLAGS_HIDE);
        }
        int weapon = 0;
        if (bystanderSlot.kind == BS002_BASE_KIND_PACKED_GROUP || DAT_1030b668[slot] == BS002_BASE_KIND_PACKED_GROUP)
            weapon = packed % 100;
        FUN_100404e0(slot, packed / 10000000 % 100, packed / 100000 % 100, packed / 1000 % 100, weapon, 0, 0);
    }
}


// 0x10104640 (Bs002_Update). One frame of the boss in slot `index` (its second half is slot index - 1). Returns 0 (1
// while the defeat scene runs).
int Bs002_Update(int index)
{
    AnimModel &model = DAT_10db6ee8[index];
    int prev = index - 1;
    Bs002State &st = DAT_1020b430[index].bs002;
    Bs002Extra &ex = DAT_1020b3f0[index];
    CharSlot &slot = DAT_10daaae8[index];
    Vec4 ownPos;
    Vec4 playerPos;
    Vec4 goal;
    Vec4 move;
    Vec4 where;
    Vec4 temp;
    Vec4 delta;
    int motion = -1;
    int restart = 0;
    int blendFrames = 5;
    int moveMode = BS002_MOVE_NONE;
    int reaction = 0;
    int armed = 0;
    int checkTargets = 0;

    if (DAT_104c0135 != 0 || DAT_104bfe1c != 0 || DAT_104bfdf0 != 0)
        return 0;

    DAT_104c0110 = index;
    model.uniformScale = 0.05f;
    slot.stats[14] = 1;
    if (ex.tinted != 0 && st.state != BS002_STATE_ROAR_WAVES && st.state != BS002_STATE_CLAW_B &&
        st.state != BS002_STATE_CLAW_C)
    {
        FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        ex.tinted = 0;
    }
    DAT_10db6ee8[prev].locked = 1;
    model.locked = 1;

    // The defeat scene.
    if (DAT_10210d30[index] != 0)
    {
        AnimModel &endingA = DAT_10db6ee8[BS002_ENDING_MODEL_A];
        AnimModel &endingB = DAT_10db6ee8[BS002_ENDING_MODEL_B];
        AnimModel &endingC = DAT_10db6ee8[BS002_ENDING_MODEL_C];
        int room = DAT_10c63ffc;
        slot.stats[13] = 1;
        if (ex.endingSetUp == 0)
        {
            FUN_10011c60();
            int roomFlags = DAT_10874848.TestScriptFlag(room * 2 + BS002_ROOM_FLAG_BASE);
            DAT_10874848.SetScriptFlag(room * 2 + BS002_ROOM_FLAG_BASE, roomFlags | (1 << (index - BS002_PEOPLE_FIRST)));
            int nearMiddle = 0;
            slot.m_0141 = 1;
            if (ex.phase < 3 && model.matrix.GetTranslation().Length2D() < 550.0f)
                nearMiddle = 1;
            for (int word = 0; word < 0x10; word++)
            {
                DAT_1020b430[prev].words[word] = 0;
                DAT_1020b430[index].words[word] = 0;
            }
            DAT_10db6ee8[prev].SetPosition(BS002_FAR_X, 0.0f, 0.0f);
            ex.phase = nearMiddle;
            ex.endingSetUp = 1;
            return 1;
        }
        int roomFlags = DAT_10874848.TestScriptFlag(room * 2 + BS002_ROOM_FLAG_BASE);
        move.Fill(0.0f);
        DAT_10daaae8[slot.m_00a4].switchesHeld = 0;
        DAT_104c0354 = 2;
        slot.m_0141 = 1;
        slot.stats[STAT_EFFECT_PARAM] = 0;
        DAT_10874848.SetScriptFlag(room * 2 + BS002_ROOM_FLAG_BASE, roomFlags | (1 << (index - BS002_PEOPLE_FIRST)));
        int &timer = st.mode;
        if (timer < 0x1e)
        {
            if (timer % 10 == 0 || FUN_1013f880() % 100 == 0)
            {
                Vec4 spot = model.matrix.GetTranslation();
                FUN_1001d8b0(index, FUN_1013f880() % 2 == 0 ? 0x321 : 0x322, spot, -1, -1, 0, 0, 0, 0, 0);
            }
        }
        else if (timer == 0x1e)
        {
            FUN_1013fb10(3, 200, 30);
            FUN_100b9bb0(index, 0x30, 1, 0);
        }
        if (timer == 0xb4)
        {
            FUN_1000db30(0, 0, 0);
            FUN_1000db30(0, 100, 0x14);
            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else if (timer == 0xc3)
        {
            FUN_1013fb10(3, 0, 10);
            DAT_10874848.MarkKilled(slot.routeRecord);
            model.SetPosition(BS002_FAR_X, 0.0f, 0.0f);
            endingA.SetPosition(BS002_FAR_X, 0.0f, 0.0f);
            endingB.SetPosition(BS002_FAR_X, 0.0f, 0.0f);
            DAT_104c00c8 = 0;
            DAT_104c0354 = 0;
            DAT_11018734 = 1;
        }
        if (st.hits == 0)
        {
            FUN_10011c60();
            if (ex.phase == 0)
                model.PlayMotion(0x34, 1, 5, 1.0f);
            else
            {
                model.PlayMotion(0x36, 1, 5, 1.0f);
                model.mode = 1;
            }
            st.angle = 0xd;
            if (slot.m_00a8 != 0)
                DAT_10db6ee8[slot.m_00a4].PlayMotion(0xd, 1, 5, 1.0f);
            FUN_10065b10(index, 0x2fd, 1, 0);
            if (DAT_104bfe78 == 0)
                st.hits = 1;
            endingA.targetHeading = model.targetHeading;
            endingB.targetHeading = model.targetHeading;
            endingA.heading = model.heading;
            endingB.heading = model.heading;
            st.mode = BS002_ENDING_START;
            DAT_10da9150.Copy(DAT_118e69e8);
            DAT_10868818.Copy(DAT_118e6cb8);
            model.root->M_101946b0(0, 0xffffffff);
            endingA.root->M_101946b0(0, 0xffffffff);
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
            if (clock < 0x5a)
            {
                float fade = 1.0f - (float)clock / 90.0f;
                model.alpha = fade;
                DAT_10db6ee8[index + 1].alpha = fade;
            }
            else
            {
                model.m_0294 = 1;
                model.motionFrame = endingA.motionFrame;
                model.alpha = 0.0f;
                DAT_10db6ee8[index + 1].m_0294 = 1;
                DAT_10db6ee8[index + 1].motionFrame = endingC.motionFrame;
                DAT_10db6ee8[index + 1].alpha = 0.0f;
            }
            if (clock < 0x3c)
                endingA.scale.Set(1.0f, 0.0f, 0.0f, endingA.scale.w + (float)(0x3c - clock) / 1830.0f);
            else if (clock < 0x5a)
                endingA.scale.Set(1.0f, 0.0f, 0.0f, 1.0f);
            else if (clock < 0xa8)
            {
                if (clock == 0x5a)
                {
                    FUN_10065b10(index, 0x2fe, 1, 0);
                    if (DAT_10c64660 != 0)
                    {
                        Script_ValueCommand(0x7549, 0x4b);
                        if (DAT_10c6356c != 0)
                        {
                            endingC.SetPosition(-100000.0f - model.matrix._41, model.matrix._42, model.matrix._43);
                            DAT_10c6356c = 100000;
                            DAT_10c63588 = 7;
                            Script_ValueCommand(0x753f, 0x4b);
                        }
                    }
                }
                float fade = (float)(0xa8 - clock);
                float rise = (float)(clock - 0x5a);
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
                endingB.heading = endingB.heading - (float)(clock - 0x60) * 5e-05f;
                endingB.targetHeading = endingB.heading;
            }
            else
                endingB.scale.Fill(0.0f);
        }
        endingA.m_0294 = 1;
        endingB.m_0294 = 1;
        endingA.motionFrame = model.motionFrame;
        timer++;
        endingB.motionFrame = model.motionFrame;
        endingA.SetPosition(DAT_10db9240, DAT_10db9244, DAT_10db9248);
        endingB.SetPosition(DAT_10db9240, DAT_10db9244, DAT_10db9248);
        endingA.alpha = endingA.scale.w;
        endingB.alpha = endingB.scale.w;

        move.Fill(0.0f);
        move.y += 950.0f;
        move.z -= 200.0f;
        move.RotateZ(-(Atan2(DAT_10868818.x - DAT_10da9150.x, DAT_10868818.y - DAT_10da9150.y) + BS002_PI));
        DAT_10edbd48.Copy(model.matrix.GetTranslation());
        DAT_10edbd48.z += 400.0f;
        DAT_10866d98.Copy(DAT_10edbd48 + move * 1.6f);
        if (timer < 0xa1)
        {
            float k = (float)(0xa0 - timer);
            k = (k + 1.0f) * k * 0.5f;
            k = k * k;
            float blend = 1.0f - k / 3.292411e+08f;
            DAT_10866d98.Copy(DAT_10da9150 + (DAT_10866d98 - DAT_10da9150) * blend);
            DAT_10edbd48.Copy(DAT_10868818 + (DAT_10edbd48 - DAT_10868818) * blend);
        }
        temp.Fill(0.0f);
        if (DAT_10866d98.Length2D() > 1550.0f && DAT_10255f08 != 0 &&
            DAT_10255f08->CastSegment("CA", 4, &DAT_10866d98, &DAT_10edbd48, &where, 0) != 0)
        {
            float gap = (where - DAT_10edbd48).Length();
            gap = gap <= 60.0f ? gap * 0.75f : gap - 50.0f;
            delta.Copy(DAT_10866d98 - DAT_10edbd48);
            delta.Copy(delta.Normalized3() * gap);
            delta.Copy(delta + where);
            DAT_10866d98.Copy(delta);
            delta.Copy(DAT_10866d98 - DAT_10868818);
            delta.RotateZ(0.0f);
            DAT_10da9150.Copy(delta + where);
        }
        float distance = (DAT_10edbd48 - DAT_10866d98).Length();
        float &closest = DAT_10255160[index][0];
        if (distance <= closest)
            closest = distance;
        else
        {
            closest = (distance - closest) * 0.3f + closest;
            delta.Copy(DAT_10edbd48 - DAT_10866d98);
            DAT_10866d98.Copy(DAT_10edbd48 - delta.Normalized3() * closest);
        }
        if (DAT_10866d98.z < 150.0f)
            DAT_10866d98.z = 150.0f;
        if (DAT_10edbd48.z < 100.0f)
            DAT_10edbd48.z = 100.0f;
        return 1;
    }

    // The fight. `prev` is the second half of its body; both halves are held in place by the engine's reaction code.
    int lastHealthWas = ex.lastHealth;
    DAT_1021f9c0[index] = 0;
    DAT_1021f9c0[prev] = 0;
    DAT_10255a88[index] = 0;
    DAT_10255a88[prev] = 0;
    DAT_10210d30[index] = 0;
    DAT_10210d30[prev] = 0;
    if (lastHealthWas == 0)
        ex.lastHealth = slot.stats[STAT_HEALTH];
    int lungeWas = st.lunging;
    DAT_10daaae8[prev].m_0141 = 1;
    slot.m_0141 = 0;
    DAT_10db6ee8[prev].m_17e8 = 1;
    if (lungeWas != 0)
    {
        st.lunging = 0;
        armed = 1;
    }
    DAT_11b04510 = 1;
    DAT_1021f6c0[index] = 0;
    model.mode = 0;
    ownPos.Copy(model.matrix.GetTranslation());
    playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    model.important = 1;
    float distance2D = (ownPos - playerPos).Length2D();
    float weighted = (ownPos.z - playerPos.z) * (ownPos.z - playerPos.z) * 0.01f + distance2D;
    if (weighted >= 8000.0f)
        DAT_1020344c = 1.48f;
    else
        DAT_1020344c = weighted * 6e-05f + 1.0f;
    DAT_1101dddc = 150.0f;
    if (DAT_104c0380 % 0x1e == index % 0x1e)
    {
        model.m_0012 = 2;
        FUN_1001c090(index, 1);
    }
    int hitLevel = DAT_102550e0[index];
    ex.variant = 0;

    if (hitLevel == 0)
    {
        if (st.state == BS002_STATE_SET_UP)
        {
            DAT_10db6ee8[BS002_ENDING_MODEL_A].LoadFile(FUN_1013f8a0("bs_002en.it3", 0));
            DAT_10db6ee8[BS002_ENDING_MODEL_B].LoadFile(FUN_1013f8a0("bs_002pa.it3", 0));
            DAT_10db6ee8[BS002_ENDING_MODEL_A].LoadModel(FUN_1013f8a0("bs_002m.it3", 0));
            DAT_10db6ee8[BS002_ENDING_MODEL_B].LoadModel(FUN_1013f8a0("bs_002m.it3", 0));
            DAT_10db6ee8[BS002_ENDING_MODEL_A].mode = 0;
            DAT_10db6ee8[BS002_ENDING_MODEL_B].mode = 0;
            DAT_10db6ee8[BS002_ENDING_MODEL_A].scale.Fill(0.0f);
            DAT_10db6ee8[BS002_ENDING_MODEL_B].scale.Fill(0.0f);
            DAT_10db6ee8[BS002_ENDING_MODEL_A].SetPosition(BS002_FAR_X, 0.0f, 0.0f);
            DAT_10db6ee8[BS002_ENDING_MODEL_B].SetPosition(BS002_FAR_X, 0.0f, 0.0f);
            if (DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG) == 1)
                slot.stats[STAT_VALUE_4] -= 5;
            slot.stats[13] = 1;
            st.state = BS002_STATE_RING_WALK;
            st.frames = 0;
            st.height = -50;
            st.targetHeight = -50;
            st.direction = -1;
            st.angle = 0xc46;
            model.PlayMotion(0xb, 1, 0, 1.0f);
            motion = 10;
            DAT_10255160[index][1] = 1000.0f;
            DAT_1042d918[BS002_BONE_HEAD] = (int)model.root->Find("Bip01 Head", 0);
            DAT_1042d918[BS002_BONE_NECK] = (int)model.root->Find("Bip01 Neck", 0);
            DAT_1042d918[BS002_BONE_SPINE] = (int)model.root->Find("Bip01 Spine", 0);
            DAT_1042d918[BS002_BONE_PELVIS] = (int)model.root->Find("Bip01 Pelvis", 0);
            DAT_1042d918[BS002_BONE_LEFT_ARM] = (int)model.root->Find("Bip01 L UpperArm", 0);
            DAT_1042d918[BS002_BONE_RIGHT_ARM] = (int)model.root->Find("Bip01 R UpperArm", 0);
            DAT_1042d918[BS002_BONE_LEFT_THIGH] = (int)model.root->Find("Bip01 L Thigh", 0);
            DAT_1042d918[BS002_BONE_RIGHT_THIGH] = (int)model.root->Find("Bip01 R Thigh", 0);
            DAT_1021bdf8[index][BS002_NODE_HEAD] = model.root->Find("Bip01 Head", 0);
            DAT_1021bdf8[index][BS002_NODE_FOOTSTEPS] = model.root->Find("Bip01 Footsteps", 0);
            DAT_1021bdf8[index][BS002_NODE_NECK] = model.root->Find("Bip01 Neck", 0);
            DAT_1021bdf8[index][BS002_NODE_HEAD_B] = model.root->Find("Bip01 Head", 0);
            DAT_1021bdf8[index][BS002_NODE_TAIL] = model.root->Find("Bone_tail2", 0);
            DAT_1021bdf8[index][BS002_NODE_SPHERE_1] = model.root->Find("Sphere01", 0);
            DAT_1021bdf8[index][BS002_NODE_SPHERE_4] = model.root->Find("Sphere04", 0);
            DAT_1021bdd4[index][BS002_NODE_SPHERE_2] = model.root->Find("Sphere02", 0);
            DAT_1021bdd4[index][BS002_NODE_SPHERE_3] = model.root->Find("Sphere03", 0);
            DAT_1021bdd4[index][BS002_NODE_PELVIS] = model.root->Find("Bip01 Pelvis", 0);
            DAT_1021bdd4[index][BS002_NODE_TAIL_B] = model.root->Find("Bone_tail2", 0);
            DAT_1021bdd4[index][BS002_NODE_DUMMY] = model.root->Find("Dummy05", 0);
            st.height = -250;
            st.targetHeight = -250;
            for (int piece = index + 1; piece < index + 1 + BS002_PIECE_COUNT; piece++)
                DAT_10daaae8[piece].kind = BS002_PIECE_KIND;
            DAT_10db6ee8[prev].SetPosition(BS002_FAR_X, 0.0f, 0.0f);
        }
        else if (st.state == BS002_STATE_CIRCLE_LEFT || st.state == BS002_STATE_CIRCLE_RIGHT)
        {
            // Circles to the left (1) or right (2) while the ring grows, then goes back to the ring walk.
            bool left = st.state == BS002_STATE_CIRCLE_LEFT;
            slot.stats[13] = 1;
            int frames = st.frames;
            st.targetHeight = -250;
            st.angle = left ? st.angle + frames - 3 : st.angle + (3 - frames);
            float angle = (float)st.angle / 1000.0f;
            if (frames == 0)
                st.radiusBoost = 0;
            st.radiusBoost += ((0x25 - frames) * 8) / 0x25;
            RingGoal(goal, angle, (float)st.radiusBoost + 1000.0f, st.height);
            move.Copy(goal - ownPos);
            moveMode = BS002_MOVE_TOWARDS;
            model.targetHeading = Atan2(move.x, move.y) + BS002_PI;
            st.frames++;
            if (st.frames > 0x25)
            {
                st.state = BS002_STATE_RING_WALK;
                st.frames = 0;
                st.direction = left ? 1 : -1;
            }
            motion = left ? 4 : 5;
        }
        else if (st.state == BS002_STATE_RING_LUNGE)
        {
            // Pulls in to the ring's inner circle for a few frames, then runs round it before the ring walk.
            slot.stats[13] = 1;
            int frames = st.frames;
            st.targetHeight = -300;
            if (frames == 0)
                ex.variant = FUN_1013f880() % 6 - 1;
            st.frames++;
            if (st.frames < 6)
            {
                goal.Set(ownPos.x, ownPos.y, (float)st.height, 0.0f);
                st.mode = 1;
            }
            else
            {
                RingGoal(goal, (float)(st.angle + st.direction * 0x14) / 1000.0f, 1000.0f, st.height);
                move.Copy(goal - ownPos);
                model.targetHeading = Atan2(move.x, move.y) + BS002_PI;
                if (st.frames < 0x15)
                    goal.Set(ownPos.x, ownPos.y, (float)st.height, 0.0f);
                else
                {
                    st.angle += st.direction * -8;
                    RingGoal(goal, (float)st.angle / 1000.0f, 1000.0f, st.height);
                }
            }
            moveMode = BS002_MOVE_TOWARDS;
            if (st.frames < 0x2d)
                motion = 0xd;
            else
            {
                motion = 2;
                st.frames = 0;
                st.state = BS002_STATE_RING_WALK;
            }
        }
        else if (st.state == BS002_STATE_RING_WALK)
        {
            // Walks round the ring with the player in the middle, picking a balloon-side radius, and ends the walk
            // with a pick (or a lunge) now and then.
            slot.stats[13] = 0;
            ex.attacking = 1;
            motion = 0;
            st.targetHeight = -250;
            st.angle = st.direction * 0x1e - (int)(Atan2(-ownPos.y, -ownPos.x) * 1000.0f);
            float &radius = DAT_10255160[index][1];
            float ringAngle = (float)st.angle / 1000.0f;
            RingGoal(goal, ringAngle, radius, st.height);
            move.Copy(goal - ownPos);
            moveMode = BS002_MOVE_BLEND;
            model.targetHeading = Atan2(move.x, move.y) + BS002_PI;
            st.frames++;
            bool decided = false;
            if (st.frames >= 0x3d)
            {
                int fury = ex.furyLevel;
                int roll = FUN_1013f880();
                if (st.frames - fury * fury - 0x3c > roll % 5000 && armed == 0)
                {
                    st.state = BS002_STATE_PICK;
                    ex.furyLevel = fury + 1;
                    decided = true;
                }
            }
            if (!decided)
            {
                int frames = st.frames;
                if (frames >= 0x29 && (frames * frames) / 0x32 > FUN_1013f880() % 5000)
                {
                    armed = 1;
                    ex.furyLevel = 0;
                    st.lunging = 1;
                }
                else if (frames < 0x28)
                {
                    armed = 0;
                    st.lunging = 0;
                }
                else if (armed != 0)
                    st.lunging = 1;
            }
            {
                // Keeps the ring on the balloon side: the balloon that is next along the walk sets the radius.
                int nearest = -1;
                float nearestGap = 0.0f;
                for (int target = BS002_BALLOON_FIRST; target < BS002_BALLOON_END; target++)
                {
                    if (DAT_1021f3c0[target] == 'B')
                    {
                        Vec4 spot = DAT_10db6ee8[target].matrix.GetTranslation();
                        float gap = Atan2(spot.y, spot.x) - ringAngle;
                        if (st.direction < 1)
                        {
                            while (gap < -BS002_TWO_PI)
                                gap += BS002_TWO_PI;
                            while (gap > 0.0f)
                                gap -= 5.1415f;
                            gap = gap * -1.0f;
                        }
                        else
                        {
                            while (gap > BS002_TWO_PI)
                                gap -= BS002_TWO_PI;
                            while (gap < 0.0f)
                                gap += 5.1415f;
                        }
                        if (nearest == -1 || gap < nearestGap)
                        {
                            nearest = target;
                            nearestGap = gap;
                        }
                    }
                }
                if (nearest >= 0)
                {
                    float away = DAT_10db6ee8[nearest].matrix.GetTranslation().Length2D();
                    radius = away <= 1050.0f ? radius + 50.0f : radius - 50.0f;
                }
                if (radius > 1200.0f)
                    radius = 1200.0f;
                else if (radius < 900.0f)
                    radius = 900.0f;
                st.angle = WrapRingAngle(st.angle);
                for (int step = 0; step < BS002_RING_STEPS; step++)
                {
                    if (RingAngleGap(st.angle, sRingAngles[step]) < 0x14 && armed != 0)
                    {
                        st.angle = sRingAngles[step];
                        st.state = BS002_STATE_PICK;
                        break;
                    }
                }
            }
        }
        else if (st.state == BS002_STATE_RING_SHOTS)
        {
            // Stands still and spits a fan of shots (5, or 7 on the hardest setting) once after a wind-up.
            int frames = ++st.frames;
            slot.stats[13] = 1;
            motion = 10;
            goal.Copy(ownPos);
            goal.z = (float)st.height;
            moveMode = BS002_MOVE_TOWARDS;
            if (frames > 0x23)
                motion = 0xb;
            st.targetHeight = frames > 0xf ? 0 : -400;
            if (frames > 0x1e)
            {
                slot.stats[13] = 0;
                model.targetHeading = Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS002_PI;
            }
            ex.attacking = 0;
            frames = st.frames;
            if (frames == 0x5a)
                st.motion = 10;
            else if (frames >= 0x3c && frames < 0x5a)
            {
                motion = 0xc;
                ex.attacking = 1;
                if (frames == 0x3c)
                {
                    restart = 1;
                    FireShotFan(index, model, slot, ex);
                }
            }
            frames = st.frames;
            if (frames < 0xf0 && frames > 0x5a && frames % 0xf == 0 && slot.stats[STAT_HEALTH] < slot.stats[STAT_MAX_HEALTH] / 4)
                checkTargets = 1;
            if (frames == 400)
                ChooseFollowUp(st, slot, distance2D, ownPos, playerPos);
        }
        else if (st.state == BS002_STATE_STOMP)
        {
            // Plants its feet and stamps; the stamp sends a ring of dust from the spine and a pulse from the head.
            motion = 0xb;
            slot.stats[13] = 1;
            goal.Copy(ownPos);
            goal.z = (float)st.height;
            moveMode = BS002_MOVE_TOWARDS;
            st.targetHeight = 0;
            if (st.frames < 0x28 && st.frames % 4 == 0)
            {
                where.Fill(0.0f);
                StartEffect(index, 0x703, where, &Bs002Bone(BS002_BONE_SPINE)->worldMatrix, 0, 0, 0, 0);
            }
            if (st.frames == 0)
            {
                FUN_10065b10(index, 0x2e7, 1, 0);
                FUN_100b9bb0(index, FUN_1013f880() % 2 == 0 ? 0x24 : 0x20, 0, 1);
            }
            if (st.frames > 0x28)
            {
                motion = 0x18;
                if (st.frames == 0x29)
                {
                    restart = 1;
                    EffectOptions options;
                    where.Fill(0.0f);
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.flags |= 2;
                    options.m_000e = 5;
                    options.m_0000 = 0;
                    options.owner = index;
                    FUN_1001d8b0(index, 0x704, where, -1, -1, &DAT_1021bdf8[index][BS002_NODE_HEAD]->worldMatrix,
                                 (int)&DAT_1021bdd4[index][BS002_NODE_DUMMY]->worldMatrix, 0, 0, &options);
                    Vec4 spot = DAT_1021bdf8[index][BS002_NODE_HEAD]->worldMatrix.GetTranslation();
                    FUN_10054550(0x28d, 0xf, &spot);
                }
            }
            int frame = model.motionFrame;
            st.frames++;
            if (frame == 0x81)
            {
                st.state = BS002_STATE_CLIMB;
                st.frames = 0;
            }
        }
        else if (st.state == BS002_STATE_ROAR_WAVES)
        {
            // Roars with the screen tinted; at frame 0x79 of the motion it sends a fan of waves (3, or 5 on the
            // hardest setting) in front of it.
            ex.tinted = 1;
            FUN_1000ec70(0xf, 0.5f, 0.5f, 0.8f, 1.0f);
            slot.stats[13] = 1;
            motion = 0xb;
            if (model.motionFrame == 0x79)
            {
                int difficulty = DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG);
                int count = difficulty + 1 == 3 ? 5 : 3;
                FUN_10065b10(index, 0x2e8, 1, 0);
                for (int wave = -1; count > 0; wave++, count--)
                {
                    Matrix4 pose;
                    EffectOptions options;
                    Vec4 offset;
                    int lateral;
                    int roll = DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG) + 1;
                    pose.SetIdentity();
                    offset.Fill(0.0f);
                    if (roll < 1)
                        lateral = wave - 2;
                    else if (roll < 3)
                        lateral = wave;
                    else if (roll != 3)
                        lateral = wave - 2;
                    else
                        lateral = wave - 1;
                    pose = RotationZ((float)(roll == 3 ? wave - 1 : wave) * 0.2618f + model.targetHeading);
                    offset.y = (float)lateral * 100.0f;
                    offset.RotateZ(model.targetHeading);
                    offset.Copy(offset + model.matrix.GetTranslation());
                    offset.z = 0.0f;
                    pose.SetTranslation(offset);
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    options.m_0008 = 1.25f;
                    StartEffect(index, 0x69f, where, 0, 0, 0, &pose, &options);
                }
            }
            if (st.frames == 0xf)
                FUN_100b9bb0(index, 0x27, 0, 0);
            int frames = st.frames;
            if (frames < 0x2d)
            {
                Matrix4 pose;
                pose.SetIdentity();
                pose.SetTranslation(model.matrix.GetTranslation());
                StartEffect(index, 0x138a, where, 0, 0, 0, &pose, 0);
                model.targetHeading = Atan2(-ownPos.x, -ownPos.y) + BS002_PI;
            }
            else if (frames < 1000)
            {
                motion = 0x18;
                if (frames == 0x2d)
                    restart = 1;
            }
            else if (frames == 0x3fc)
            {
                FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                st.state = BS002_STATE_PICK;
                ex.tinted = 0;
            }
            int frame = model.motionFrame;
            st.frames++;
            if (frame == 0x81)
                st.frames = 1000;
        }
        else if (st.state == BS002_STATE_CLIMB)
        {
            // Rises out of the ring (frames 0..0x3c), shoots up out of sight, then drops back on the ring walk's
            // side of the player and lands with a blast.
            slot.stats[13] = 1;
            motion = 0x26;
            if (st.frames == 0)
            {
                if (ex.lungeReady == 0)
                {
                    float gap = Atan2(playerPos.y, playerPos.x) - (float)st.angle / 1000.0f;
                    while (gap < -BS002_PI)
                        gap += BS002_TWO_PI;
                    while (gap > BS002_PI)
                        gap -= BS002_TWO_PI;
                    st.direction = gap >= 0.0f ? 1 : -1;
                    DAT_10255160[index][0] = DAT_104bfe24 - 100.0f;
                }
                ex.variant = FUN_1013f880() % 5;
            }
            if (st.frames < 0x3c)
            {
                st.targetHeight = 0x708;
                st.mode = 0;
            }
            else if (st.frames == 0x3c)
            {
                st.targetHeight = -2000;
                st.mode = 1;
                FUN_10065b10(index, 0x2e9, 1, 0);
            }
            else
            {
                st.targetHeight = -600;
                if ((float)st.height <= DAT_104bfe24 + 100.0f)
                    motion = 0x20;
                else
                {
                    motion = 0x28;
                    Matrix4 pose;
                    EffectOptions options;
                    pose.SetIdentity();
                    pose = RotationZ(model.heading);
                    pose.SetTranslation(model.matrix.GetTranslation());
                    pose._43 -= 200.0f;
                    StartEffect(index, 0x454, where, 0, 0, 0, &pose, 0);
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.flags |= 2;
                    options.m_0008 = 1.1f;
                    options.m_000e = 5;
                    StartEffect(index, 0x45d, where, &Bs002Bone(BS002_BONE_HEAD)->worldMatrix, 0, 0, 0, &options);
                    StartEffect(index, 0x45d, where, &Bs002Bone(BS002_BONE_LEFT_ARM)->worldMatrix, 0, 0, 0, &options);
                    StartEffect(index, 0x45d, where, &Bs002Bone(BS002_BONE_RIGHT_ARM)->worldMatrix, 0, 0, 0, &options);
                    if (st.frames < 500 && Bs002Bone(BS002_BONE_HEAD)->worldMatrix._43 < DAT_104bfe24 + 100.0f)
                    {
                        st.frames = 500;
                        FUN_100543a0(ownPos);
                    }
                }
            }
            st.angle += st.direction * (st.frames / 0xf);
            RingGoal(goal, (float)st.angle / 1000.0f, 1000.0f, st.height);
            goal.w = 1.0f;
            move.Copy(goal - ownPos);
            moveMode = BS002_MOVE_TOWARDS;
            model.targetHeading = Atan2(move.x, move.y) + BS002_PI;
            int frame = model.motionFrame;
            st.frames++;
            if (frame == 499)
            {
                if (ex.lungeReady == 0 || FUN_1013f880() % 0x32 > 0x1d)
                {
                    st.state = BS002_STATE_RING_WALK;
                    st.frames = 0;
                }
                else
                {
                    st.state = BS002_STATE_PICK;
                    st.lunging = 1;
                }
            }
        }
        else if (st.state == BS002_STATE_GLARE)
        {
            // Rears up and glares (frames 0x3c..0x95: pillars of light round it), then fires one shot at the player
            // every half second until it picks again.
            int frames = ++st.frames;
            slot.stats[13] = 1;
            motion = 10;
            goal.Copy(ownPos);
            goal.z = (float)st.height;
            moveMode = BS002_MOVE_TOWARDS;
            if (frames > 0x23)
                motion = 0xb;
            st.targetHeight = frames < 0x10 ? -400 : -50;
            if (frames > 0x1e && (frames < 0x3c || frames > 0x96))
            {
                slot.stats[13] = 0;
                model.targetHeading = Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS002_PI;
            }
            frames = st.frames;
            ex.attacking = 0;
            if (frames >= 0x3c && frames < 0x96)
            {
                slot.stats[13] = 0;
                motion = 0x29;
                ex.attacking = 1;
                if (frames == 0x3c)
                {
                    restart = 1;
                    ex.variant = FUN_1013f880() % 4 + 1;
                    FUN_10065b10(index, 0x28e, 1, 0);
                    FUN_100b9bb0(index, 0x21, 0, 1);
                }
                if (st.frames % 0x16 == 0)
                {
                    Matrix4 pose;
                    EffectOptions options;
                    where.Copy(model.matrix.GetTranslation());
                    where.z += 300.0f;
                    pose.SetIdentity();
                    pose.SetTranslation(where);
                    options.flags |= 2;
                    options.owner = index;
                    StartEffect(index, 0x6fa, where, 0, 0, 0, &pose, &options);
                }
            }
            else if (frames >= 300)
            {
                if (frames < 0x14a)
                {
                    motion = 0x2f;
                    ex.attacking = 1;
                }
            }
            else if (frames >= 0x96)
            {
                if (frames == 0x96)
                    FUN_100b9bb0(index, 0x22, 0, 1);
                motion = 0xc;
                ex.attacking = 1;
                if (st.frames % 0x1e == 0)
                {
                    Matrix4 pose;
                    EffectOptions options;
                    restart = 1;
                    options.m_0000 = 0;
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.flags |= 2;
                    options.m_0008 = 1.1f;
                    options.m_000e = 5;
                    temp.Copy(playerPos);
                    temp.z = -35.0f;
                    where.Copy(model.matrix.GetTranslation());
                    where.z += 300.0f;
                    pose.SetIdentity();
                    pose = RotationZ(Atan2(temp.x - where.x, temp.y - where.y) + BS002_PI);
                    pose.SetTranslation(where);
                    StartEffect(index, 0x451, where, 0, 0, 0, &pose, &options);
                    FUN_10065b10(index, 0x28c, 1, 0);
                }
            }
            int half = slot.stats[STAT_MAX_HEALTH] / 2;
            if ((slot.stats[STAT_HEALTH] > half && st.frames >= 0x28a) || st.frames == 800)
                ChooseFollowUp(st, slot, distance2D, ownPos, playerPos);
        }
        else if (st.state == BS002_STATE_WHIRL)
        {
            // Spins round the ring at a growing speed with the ring radius shrinking, rains rocks on the floor and
            // lands back at the ring walk (frames 1000.. = spinning in, 2000.. = on the way out, 3000.. = dropping).
            int direction = st.direction;
            int frames = st.frames;
            slot.stats[13] = 1;
            ex.attacking = 1;
            motion = (direction > 0 ? 2 : 0) + 0x30;
            float &ringRadius = DAT_10255160[index][1];
            if (frames < BS002_FRAMES_PART_TWO)
            {
                st.targetHeight = 300;
                st.mode = 2;
                if (frames < 0x1e)
                    st.angle += ((0x1e - frames) * direction) / 2;
                RingGoal(goal, (float)st.angle / 1000.0f, 900.0f, st.height);
                goal.w = 1.0f;
                move.Copy(goal - ownPos);
                moveMode = BS002_MOVE_TOWARDS;
                if (st.frames < 0x19)
                    model.targetHeading = Atan2(move.x, move.y) + BS002_PI;
                if (DAT_104bfe24 < goal.z)
                {
                    st.frames = BS002_FRAMES_PART_TWO;
                    FUN_100543a0(goal);
                    ringRadius = 900.0f;
                }
                if (goal.z <= DAT_104bfe24 && st.frames != 0xf)
                    st.frames++;
                else
                {
                    FUN_100b9bb0(index, 0x23, 0, 0);
                    st.frames++;
                }
            }
            else if (frames < BS002_FRAMES_PART_THREE)
            {
                st.targetHeight = 300;
                st.mode = 2;
                if (ringRadius < 800.0f)
                {
                    st.targetHeight = 0;
                    st.mode = 0;
                }
                ringRadius -= (float)(frames - BS002_FRAMES_PART_TWO) * 1.5f;
                RingGoal(goal, (float)st.angle / 1000.0f, ringRadius, st.height);
                goal.w = 1.0f;
                move.Copy(goal - ownPos);
                moveMode = BS002_MOVE_TOWARDS;
                FUN_100543a0(goal);
                if (ringRadius >= 550.0f)
                    st.frames++;
                else
                {
                    st.frames = BS002_FRAMES_PART_THREE;
                    st.frames++;
                }
            }
            else if (frames < BS002_FRAMES_PART_FOUR)
            {
                st.targetHeight = 0;
                ringRadius -= 20.0f;
                RingGoal(goal, (float)st.angle / 1000.0f, ringRadius, st.height);
                goal.w = 1.0f;
                move.Copy(goal - ownPos);
                moveMode = BS002_MOVE_TOWARDS;
                if (ringRadius < -500.0f)
                    st.frames = BS002_FRAMES_PART_FOUR;
                if (st.frames % 2 == 0)
                    StartFloorSplash(index, model, ownPos);
                if (st.frames % 6 == 0)
                    FUN_10065b10(index, 0x28f, 1, 0);
                EffectOptions options;
                options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                options.flags |= 2;
                options.m_000e = 5;
                options.m_0000 = 0;
                where.Fill(0.0f);
                StartEffect(index, 0x456, where, &DAT_1021bdd4[index][BS002_NODE_SPHERE_2]->worldMatrix, 0, 0, 0,
                            &options);
                StartEffect(index, 0x455, where, &DAT_1021bdd4[index][BS002_NODE_SPHERE_3]->worldMatrix, 0, 0, 0,
                            &options);
                StartEffect(index, 0x456, where, &DAT_1021bdf8[index][BS002_NODE_SPHERE_4]->worldMatrix, 0, 0, 0,
                            &options);
                StartEffect(index, 0x456, where, &DAT_1021bdd4[index][BS002_NODE_PELVIS]->worldMatrix, 0, 0, 0,
                            &options);
                StartEffect(index, 0x456, where, &DAT_1021bdd4[index][BS002_NODE_TAIL_B]->worldMatrix, 0, 0, 0,
                            &options);
                st.frames++;
            }
            else
            {
                if (frames < 4000)
                {
                    st.targetHeight = 200;
                    if (frames > 0xbc0)
                        st.targetHeight = -600;
                    float fall = 20.0f - (float)(frames - BS002_FRAMES_PART_FOUR) * 0.66667f;
                    if (fall < 0.0f)
                        fall = 0.0f;
                    ringRadius -= fall;
                    RingGoal(goal, (float)st.angle / 1000.0f, ringRadius, st.height);
                    goal.w = 1.0f;
                    move.Copy(goal - ownPos);
                    moveMode = BS002_MOVE_TOWARDS;
                    if ((float)st.height < DAT_104bfe24)
                        FUN_100543a0(goal);
                    if ((float)st.height < DAT_104bfe24 - 100.0f)
                    {
                        int angle = st.angle;
                        st.direction = -st.direction;
                        st.state = BS002_STATE_RING_WALK;
                        st.frames = -1;
                        ringRadius = 1000.0f;
                        st.angle = angle + 0xc46;
                    }
                }
                st.frames++;
            }
        }
        else if (st.state == BS002_STATE_CLAW_RUN)
        {
            // Runs round the ring clawing at the player (one swing every few frames, rolled for the end).
            slot.stats[13] = 1;
            DAT_1021f6c0[index] = 1;
            motion = 0x21;
            st.angle += st.direction * 0xf;
            st.targetHeight = (int)(DAT_104bfe24 * 0.0f);
            RingGoal(goal, (float)st.angle / 1000.0f, DAT_10255160[index][1], st.height);
            move.Copy(goal - ownPos);
            moveMode = BS002_MOVE_TOWARDS;
            model.targetHeading = Atan2(move.x, move.y) + BS002_PI - (float)st.direction * 0.6f;
            st.frames++;
            st.angle = WrapRingAngle(st.angle);
            bool atStop = false;
            for (int step = 0; step < BS002_RING_STEPS; step++)
            {
                if (RingAngleGap(st.angle, sRingAngles[step]) < 0x14)
                    atStop = true;
            }
            if (st.frames == 1)
                FUN_100b9bb0(index, 0x23, 0, 1);
            int frames = st.frames;
            if (frames >= 0x97 && (frames - 0x96) > FUN_1013f880() % 5000)
                st.state = BS002_STATE_PICK;
            else if (frames > 299 && atStop)
            {
                st.state = BS002_STATE_PICK;
                st.lunging = 1;
            }
            if ((float)st.height > DAT_104bfe24 - 150.0f && st.frames > 10)
                ClawTrail(index, model, slot, st, ownPos, playerPos);
        }
        else if (st.state == BS002_STATE_SWIPE)
        {
            int frames = ++st.frames;
            slot.stats[13] = 1;
            motion = 10;
            goal.Copy(ownPos);
            goal.z = (float)st.height;
            moveMode = BS002_MOVE_TOWARDS;
            if (frames > 0x14)
            {
                st.state = BS002_STATE_CLIMB;
                st.frames = 0;
                ex.lungeReady = 1;
                FUN_100543a0(ownPos);
            }
            if (st.frames == 0xf)
                FUN_100b9bb0(index, 0x25, 0, 1);
            st.targetHeight = st.frames > 0xf ? 0 : -400;
        }
        else if (st.state == BS002_STATE_STAGGER)
        {
            // Staggers back and slumps (a long motion; the height follows a fixed table of frames), then comes
            // back on the ring. Dust and splashes at the burst frames.
            int splash = 0;
            float eased;
            ++st.frames;
            slot.stats[13] = 1;
            ex.lastHealth = slot.stats[STAT_HEALTH];
            armed = 0;
            st.mode = 3;
            st.targetHeight = -45;
            ex.attacking = 1;
            goal.Set(ownPos.x, ownPos.y, (float)st.height, 1.0f);
            moveMode = BS002_MOVE_TOWARDS;
            int frames = st.frames;
            if (frames < 100 && frames > 0x13)
            {
                float steps = (float)(100 - frames);
                goal.Set(ownPos.x - ownPos.x / steps, ownPos.y - ownPos.y / steps, (float)st.height, 0.0f);
                DAT_10802bf8[index].position.w = (float)atan2(-ownPos.x, -ownPos.y) + BS002_PI;
            }
            if (st.frames == 1)
            {
                motion = 0x12;
                ex.roarStep = 0;
                restart = 1;
            }
            frames = st.frames;
            if (frames < 0x14)
            {
                st.targetHeight = -400;
                st.mode = 0;
            }
            else if (frames > 0x14 && frames < 0x43)
            {
                FUN_10034810((float)st.height, 1100.0f, &eased, (float)(0x49 - frames), 1);
                float mark = DAT_104bfe24 - 100.0f;
                if (mark < eased && (float)st.height < mark)
                    FUN_100543a0(ownPos);
                st.height = (int)eased;
                st.targetHeight = st.height;
            }
            else if (frames == 0x50)
            {
                FUN_10065b10(index, 0x2ea, 0, 0);
                FUN_100b9bb0(index, 0x26, 0, 0);
            }
            else if (frames < 0x62 || frames > 0x71)
            {
                if (frames == 0x72)
                {
                    StartRoarBurst(index, model, DAT_1042d918[BS002_BONE_LEFT_ARM], DAT_1042d918[BS002_BONE_RIGHT_ARM],
                                   100.0f, 5);
                    if (DAT_10255f08 != 0)
                    {
                        Node *marker = DAT_10255f08->Find("dai", 0);
                        splash = 1;
                        if (marker != 0)
                            marker->RestartColourPlugs("sink", DAT_104bf8a4);
                    }
                    else
                        splash = 1;
                }
                else if (frames < 0x73 || frames > 0x7c)
                {
                    if (frames < 0x7d || frames > 0x84)
                    {
                        if (frames == 0x85)
                        {
                            StartRoarBurst(index, model, DAT_1042d918[BS002_BONE_LEFT_ARM],
                                           DAT_1042d918[BS002_BONE_RIGHT_ARM], 100.0f, 4);
                            splash = 1;
                        }
                        else if (frames < 0x86 || frames > 0x88)
                        {
                            if (frames > 0x88 && frames < 0x94)
                            {
                                ex.roarStep = 10;
                                FUN_10034810((float)st.height, -200.0f, &eased, (float)(0x94 - frames), 1);
                                st.height = (int)eased;
                                st.targetHeight = st.height;
                                splash = 2;
                            }
                            else if (frames == 0x94)
                            {
                                ex.phase = 3;
                                st.height = -200;
                                st.targetHeight = -200;
                                StartDebrisPuffs(index, model, 2);
                                splash = 2;
                            }
                            else if (frames >= 0x95 && frames <= 0x111)
                            {
                                st.mode = 0;
                                st.height = -200 - (frames - 0x94);
                                st.targetHeight = st.height;
                                if (frames > 0xb8)
                                {
                                    motion = 0;
                                    blendFrames = 90;
                                }
                            }
                            else if (frames == 0x112)
                            {
                                FUN_100b9bb0(index, 0x2f, 1, 0);
                                st.state = BS002_STATE_PICK;
                                st.mode = 0;
                                if (DAT_10255f08 != 0)
                                {
                                    Node *marker = DAT_10255f08->Find("dai", 0);
                                    if (marker != 0)
                                        marker->RestartColourPlugs("sink0", 0);
                                }
                                motion = 0;
                                temp.Fill(0.0f);
                                temp.y = -1.0f;
                                temp.RotateZ(model.heading);
                                st.angle = (int)(Atan2(temp.y, temp.x) * 1000.0f);
                            }
                        }
                        else
                        {
                            FUN_10034810((float)st.height, 150.0f, &eased, (float)(0x89 - frames), 1);
                            st.height = (int)eased;
                            st.targetHeight = st.height;
                            splash = 2;
                        }
                    }
                    else
                    {
                        FUN_10034810((float)st.height, -145.0f, &eased, (float)(0x85 - frames), 1);
                        st.height = (int)eased;
                        st.targetHeight = st.height;
                    }
                }
                else
                {
                    FUN_10034810((float)st.height, 300.0f, &eased, (float)(0x7d - frames), 1);
                    st.height = (int)eased;
                    st.targetHeight = st.height;
                }
            }
            else
            {
                StartArmGlow(index, model, DAT_1042d918[BS002_BONE_LEFT_ARM], DAT_1042d918[BS002_BONE_RIGHT_ARM]);
                FUN_10034810((float)st.height, -50.0f, &eased, (float)(0x72 - st.frames), 1);
                st.height = (int)eased;
                st.targetHeight = st.height;
            }
            frames = st.frames;
            if (frames < 0x3c)
                DAT_1101dddc = 150.0f - (float)(frames * 5);
            else if (frames < 0xf5)
                DAT_1101dddc = -150.0f;
            else
                DAT_1101dddc = (float)((frames * 5 - 0x4c4) * 2) - 150.0f;
            if (splash != 0)
                StartStaggerSplashes(index, model);
        }
        else if (st.state == BS002_STATE_CLAW_B)
        {
            int frames = ++st.frames;
            slot.stats[13] = 0;
            ex.attacking = 1;
            motion = 10;
            goal.Copy(ownPos);
            goal.z = (float)st.height;
            moveMode = BS002_MOVE_TOWARDS;
            if (frames > 0x23)
                motion = 0xb;
            st.targetHeight = frames < 0x10 ? -400 : 0;
            if (frames > 0x1e)
            {
                slot.stats[13] = 0;
                model.targetHeading = Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS002_PI;
            }
            frames = st.frames;
            ex.attacking = 0;
            if (frames == 0x3c)
                StartPillar(index, model);
            if (frames > 0x3c && frames < 0x5a && frames % 4 == 0)
            {
                where.Fill(0.0f);
                StartEffect(index, 0x703, where, &Bs002Bone(BS002_BONE_SPINE)->worldMatrix, 0, 0, 0, 0);
            }
            frames = st.frames;
            if (frames >= 0x3d && frames <= 0x149)
            {
                ex.tinted = 1;
                FUN_1000ec70(0xf, 0.45f, 0.45f, 0.45f, 1.0f);
            }
            else if (frames == 0x14a)
            {
                ex.tinted = 0;
                FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            frames = st.frames;
            if (frames >= 0x5a && frames <= 0x12a)
            {
                motion = 0xc;
                ex.attacking = 1;
                if (frames % 0x3c == 0)
                {
                    Matrix4 pose;
                    EffectOptions options;
                    pose.SetIdentity();
                    pose.SetTranslation(ownPos);
                    pose._43 = DAT_104bfe24;
                    st.motion = 0xb;
                    restart = 1;
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    StartEffect(index, 0x70c, where, 0, 0, 5, &pose, &options);
                    FUN_10065b10(index, 0x290, 1, 0);
                }
            }
            else if (frames == 299)
                st.motion = 10;
            frames = st.frames;
            if (frames < 0x3c)
                DAT_1101dddc = 150.0f - (float)(frames * 3);
            else if (frames >= 0x12d && frames <= 0x167)
                DAT_1101dddc = (float)(frames * 3 - 0x3b1) - 30.0f;
            else if (frames < 0x12d)
                DAT_1101dddc = -30.0f;
            if (frames == 0x28a)
            {
                st.state = distance2D >= 400.0f || playerPos.z <= DAT_104bfe24 ? BS002_STATE_PICK : BS002_STATE_STOMP;
                st.frames = 0;
            }
        }
        else if (st.state == BS002_STATE_CLAW_C)
        {
            slot.stats[13] = 0;
            motion = 10;
            int frames = ++st.frames;
            goal.Copy(ownPos);
            goal.z = (float)st.height;
            moveMode = BS002_MOVE_TOWARDS;
            if (frames > 0x23)
                motion = 0xb;
            st.targetHeight = frames < 0x10 ? -400 : 0;
            if (frames % 0xf == 0 && frames > 0x2d && frames < 0x10e)
                checkTargets = 1;
            ex.attacking = 0;
            if (frames >= 0x1f && frames <= 0x10d)
            {
                ex.tinted = 1;
                FUN_1000ec70(0xf, 0.5f, 0.5f, 0.8f, 1.0f);
                where.Copy(model.matrix.GetTranslation());
                where.x += (float)(FUN_1013f880() % 200 - 100);
                where.y += (float)(FUN_1013f880() % 200 - 100);
                where.z += (float)(FUN_1013f880() % 200);
                Matrix4 pose;
                pose.SetIdentity();
                pose.SetTranslation(where);
                StartEffect(index, 0x6d6, where, 0, 0, 0, &pose, 0);
                ex.attacking = 1;
                motion = 0x29;
                if (st.frames == 0x1f)
                {
                    restart = 1;
                    FUN_10065b10(index, 0x292, 1, 0);
                    FUN_100b9bb0(index, 0x27, 0, 1);
                }
            }
            else if (frames == 0x10e)
            {
                ex.tinted = 0;
                FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            frames = st.frames;
            if (frames < 0x3c)
                DAT_1101dddc = 150.0f - (float)(frames * 3);
            else if (frames >= 0x10f && frames <= 0x15d)
                DAT_1101dddc = (float)(frames * 3 - 0x438) - 30.0f;
            else if (frames < 0x10f)
                DAT_1101dddc = -30.0f;
            if (frames == 0x2a8)
                st.state = BS002_STATE_PICK;
            model.targetHeading = Atan2(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS002_PI;
        }
        else if (st.state == BS002_STATE_PICK)
        {
            st.frames = 0;
            ex.damageTaken = 0;
            if (armed == 0)
            {
                if (st.lastState == BS002_STATE_RING_WALK)
                {
                    if (st.direction > 0)
                    {
                        st.state = BS002_STATE_CIRCLE_RIGHT;
                        st.direction = -1;
                    }
                    else if (st.direction < 0)
                    {
                        st.state = BS002_STATE_CIRCLE_LEFT;
                        st.direction = 1;
                    }
                }
                else if (st.lastState == BS002_STATE_STAGGER || st.lastState == BS002_STATE_CLAW_RUN)
                    st.state = BS002_STATE_RING_WALK;
                else
                    st.state = BS002_STATE_RING_LUNGE;
            }
            else if (DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG) == 1 ||
                     slot.stats[STAT_MAX_HEALTH] / 2 <= slot.stats[STAT_HEALTH])
            {
                if (st.lastState == BS002_STATE_RING_WALK)
                {
                    int roll = FUN_1013f880() % 3;
                    if (roll == 0)
                        st.state = BS002_STATE_RING_SHOTS;
                    else if (roll == 1)
                        st.state = BS002_STATE_GLARE;
                    else if (roll == 2)
                        st.state = BS002_STATE_WHIRL;
                }
            }
            else
            {
                switch (st.lastState)
                {
                case BS002_STATE_RING_WALK:
                case BS002_STATE_CLIMB:
                    switch (FUN_1013f880() % 5)
                    {
                    case 0:
                        st.state = BS002_STATE_GLARE;
                        break;
                    case 1:
                        st.state = BS002_STATE_CLAW_RUN;
                        break;
                    case 2:
                        st.state = BS002_STATE_SWIPE;
                        break;
                    case 3:
                        st.state = BS002_STATE_CLAW_B;
                        break;
                    case 4:
                        st.state = BS002_STATE_CLAW_C;
                        break;
                    }
                    break;
                case BS002_STATE_CLAW_RUN:
                    switch (FUN_1013f880() % 3)
                    {
                    case 0:
                        st.state = BS002_STATE_GLARE;
                        break;
                    case 1:
                        st.state = BS002_STATE_CLAW_B;
                        break;
                    case 2:
                        st.state = BS002_STATE_CLAW_C;
                        break;
                    }
                    break;
                case BS002_STATE_CLAW_B:
                    switch (FUN_1013f880() % 5)
                    {
                    case 0:
                        st.state = BS002_STATE_GLARE;
                        break;
                    case 1:
                        st.state = BS002_STATE_CLAW_RUN;
                        break;
                    case 2:
                        st.state = BS002_STATE_CLIMB;
                        break;
                    case 3:
                        st.state = BS002_STATE_CLAW_B;
                        break;
                    case 4:
                        st.state = BS002_STATE_CLAW_C;
                        break;
                    }
                    break;
                }
            }
            if (ex.phase == 1)
            {
                st.state = BS002_STATE_STAGGER;
                ex.phase = 2;
            }
            ex.lungeReady = 0;
        }
    }
    else
    {
        // The reaction to a hit: the first time it falls below half health it roars (reaction 1), and after enough
        // damage since the last stagger it staggers (reaction 2); every hit gets a voice line.
        st.motion = -1;
        reaction = 0;
        if (DAT_10c635f8 != 1 && ex.phase == 0 && slot.stats[STAT_HEALTH] < slot.stats[STAT_MAX_HEALTH] / 2)
        {
            reaction = 1;
            ex.phase = 1;
            if (st.state > 0 && (st.state < 8 || st.state == 9))
                st.state = BS002_STATE_PICK;
            ex.lastHealth = slot.stats[STAT_HEALTH];
        }
        else if (ex.lastHealth != slot.stats[STAT_HEALTH])
        {
            if (ex.phase == 1 || st.state == BS002_STATE_STAGGER)
                ex.damageTaken = 0;
            else
            {
                ex.damageTaken += ex.lastHealth - slot.stats[STAT_HEALTH];
                bool normalPlay = DAT_10c635f8 != 1;
                if ((normalPlay && slot.stats[STAT_MAX_HEALTH] / 9 < ex.damageTaken) ||
                    slot.stats[STAT_MAX_HEALTH] * 3 / 10 < ex.damageTaken)
                {
                    reaction = 2;
                    st.state = BS002_STATE_CLIMB;
                    st.frames = 0;
                    ex.damageTaken = 0;
                }
            }
            ex.lastHealth = slot.stats[STAT_HEALTH];
        }
        if (hitLevel > 6)
            DAT_102550e0[index] = 6;
        if (slot.stats[13] != 0 || ex.attacking != 0)
        {
            int savedFrame = st.motionFrame;
            DAT_102550e0[index] = 0;
            if (savedFrame < 0)
                model.PlayMotion(st.motionLayer, 1, 3, 1.0f);
            else
            {
                model.PlayMotion(st.motionLayer, 1, 0, 1.0f);
                model.motion->frame = st.motionFrame;
                model.motion->m_0004 = (float)st.motionFrame;
                model.motion->Advance();
            }
        }
        if (reaction == 0)
        {
            if (DAT_10219770[index] == 0 || FUN_1013f880() % 3 != 1)
            {
                if (DAT_10874848.TestScriptFlag(BS002_DIFFICULTY_FLAG) == 0)
                    FUN_100b9bb0(index, FUN_1013f880() % 2 + 0x28, 0, 0);
                else
                    FUN_100b9bb0(index, FUN_1013f880() % 4 + 0x28, 0, 0);
            }
            else
                FUN_100b9bb0(index, 0x2e, 0, 0);
        }
        else if (reaction == 1 || reaction == 2)
            FUN_100b9bb0(index, FUN_1013f880() % 2 + 0x2c, 0, 0);
    }

    // The end of every frame: remember the state, vary the mood now and then, keep the lunge on the ring's steps.
    if (st.state != BS002_STATE_PICK)
    {
        st.lastState = st.state;
        int roll = FUN_1013f880();
        if (st.frames % 10 == 5 && ((ex.phase > 1 && roll % 500 < 10) || roll % 500 < 5))
            ex.variant = FUN_1013f880() % 3 + 5;
    }
    if (model.motionFrame > 0x82 && model.motionFrame < 0xab)
    {
        // During the lunge's frames the goal is pulled onto the nearest step of the ring and the angle follows it.
        Vec4 snapped;
        Vec4 pull;
        snapped.Fill(0.0f);
        pull.Fill(0.0f);
        int nearest = -1;
        int nearestGap = 0;
        int goalAngle = WrapRingAngle((int)(Atan2(goal.y, goal.x) * 1000.0f));
        for (int step = 0; step < BS002_RING_STEPS; step++)
        {
            if (nearest == -1 || RingAngleGap(goalAngle, sRingAngles[step]) < nearestGap)
            {
                nearestGap = RingAngleGap(goalAngle, sRingAngles[step]);
                nearest = step;
            }
        }
        int stepAngle = sRingAngles[nearest];
        float stepRadians = (float)stepAngle / 1000.0f;
        snapped.x = (float)cos(stepRadians) * 1000.0f;
        snapped.y = (float)sin(stepRadians) * 1000.0f;
        pull.Copy(snapped - goal);
        pull.z = 0.0f;
        if (pull.Length2D() > 1.0f)
        {
            if (pull.Length2D() > 5.0f)
                pull.Copy(pull.Normalized3() * 5.0f);
            goal.x = pull.x + goal.x;
            goal.y = pull.y + goal.y;
        }
        int gap = stepAngle - st.angle;
        if ((gap < 0 ? -gap : gap) > 1)
        {
            if (gap > 0xc46)
                gap += -0x1828 + ((unsigned)(gap - 0xc47) / 0x1828) * -0x1828;
            if (gap < -0xc46)
                gap += 0x1828 + ((unsigned)(-gap - 0xc47) / 0x1828) * 0x1828;
            if (gap > 0)
                st.angle += 10;
            if (gap < 0)
                st.angle -= 10;
        }
    }

    // The height follows its target, faster the further away it is.
    int height = st.height;
    int targetHeight = st.targetHeight;
    int heightGap = height - targetHeight;
    if ((heightGap < 0 ? -heightGap : heightGap) > 0x13 && st.mode != 3)
    {
        if (height < targetHeight)
        {
            st.riseSpeed = (int)(float)(targetHeight - height);
            if (st.riseSpeed < 100)
                st.riseSpeed = 100;
            st.height = st.riseSpeed / 100 + height;
        }
        else if (targetHeight < height)
        {
            if (st.riseSpeed < 100)
                st.riseSpeed = 100;
            st.height = height - st.riseSpeed / 100;
        }
    }
    st.mode = 0;

    // Dust where its bones brush the floor: up to four puffs a frame, going through the eight bones in turn.
    if ((DAT_104bf890 & 1) == 0)
    {
        Matrix4 pose;
        EffectOptions options;
        Vec4 where2;
        int spawned = 0;
        int first = ex.shotRing;
        int next = first;
        bool secondPass = true;
        pose.SetIdentity();
        pose = Scaling(1.5f, 1.5f, 1.5f);
        where2.Fill(0.0f);
        if (next < 8)
        {
            while (next < 8 && spawned < 4)
            {
                pose.SetTranslation(Bs002Bone(next)->worldMatrix.GetTranslation());
                if (DAT_104bfe24 - 120.0f < pose._43 && pose._43 <= DAT_104bfe24 + 120.0f)
                {
                    pose._43 = DAT_104bfe24 + 10.0f;
                    spawned++;
                    options.list = (unsigned char)((DAT_104bf890 & 1) + 1);
                    StartEffect(index, 3000, where2, 0, 0, 0, &pose, 0);
                }
                next++;
            }
            secondPass = spawned < 4;
        }
        if (secondPass)
        {
            next = 0;
            while (next < first && spawned < 4)
            {
                pose.SetTranslation(Bs002Bone(next)->worldMatrix.GetTranslation());
                if (DAT_104bfe24 - 120.0f < pose._43 && pose._43 <= DAT_104bfe24 + 120.0f)
                {
                    pose._43 = DAT_104bfe24 + 10.0f;
                    spawned++;
                    options.list = (unsigned char)((DAT_104bf890 & 1) + 1);
                    StartEffect(index, 3000, where2, 0, 0, 0, &pose, 0);
                }
                next++;
            }
        }
        ex.shotRing = next;
        if (next > 8)
            ex.shotRing = next - 8;
    }

    // Crossing the floor line (its head going up or down through 8 above the floor) splashes and blasts the balloons.
    if (st.state != BS002_STATE_CLAW_RUN)
    {
        float line = DAT_104bfe24 + 8.0f;
        float headBefore = DAT_10255160[index][0];
        float headNow = Bs002Bone(BS002_BONE_HEAD)->worldMatrix._43;
        if ((line < headBefore && headNow < line) || (headBefore < line && line < headNow))
        {
            Matrix4 pose;
            Vec4 splash;
            splash.Copy(model.matrix.GetTranslation());
            splash.z = DAT_104bfe24;
            pose.SetIdentity();
            StartEffect(index, 0xbbd, splash, 0, 0, 0, 0, 0);
            pose = RotationZ(model.heading + BS002_TWO_PI);
            pose.SetTranslation(splash);
            for (int puff = 0; puff < 5; puff++)
                StartEffect(index, 0x6c8, splash, 0, 0, 0, &pose, 0);
            pose._43 += 5.0f;
            for (int ring = 0; ring < 3; ring++)
                StartEffect(index, 0x6a6, splash, 0, 0, 0, &pose, 0);
            for (int balloon = BS002_BALLOON_FIRST; balloon < BS002_BALLOON_END - 1; balloon++)
            {
                if (DAT_1021f3c0[balloon] == 'B' && DAT_10db6ee8[balloon].burst == 0)
                {
                    Vec4 spot;
                    spot.Copy(DAT_10db6ee8[balloon].matrix.GetTranslation());
                    DAT_10db6ee8[balloon].blastAngle = (int)(Atan2(spot.x - ownPos.x, spot.y - ownPos.y) * 1000.0f);
                    float reach = 600.0f - (spot - ownPos).Length2D();
                    int push = reach > 0.0f ? (int)reach : 0;
                    DAT_10db6ee8[balloon].blastPush = push / 13 * 13 + 3;
                }
            }
        }
    }
    DAT_10255160[index][0] = Bs002Bone(BS002_BONE_HEAD)->worldMatrix._43;

    // The movement the state asked for: the step towards the goal is at most 32 long; the ring walk blends it with the
    // step of the frame before.
    if (moveMode == BS002_MOVE_TOWARDS || moveMode == BS002_MOVE_BLEND)
    {
        Vec4 step;
        step.Copy(goal - ownPos);
        float length = step.Length2D();
        float rise = step.z;
        if (length > 32.0f)
            length = 32.0f;
        if (length > 1.0f)
        {
            step.z = 0.0f;
            step.Copy(step.Normalized3() * length);
            step.z = rise;
        }
        if (moveMode == BS002_MOVE_BLEND)
            step.Copy(step * 0.8f + move * 0.2f);
        step.Copy(step + ownPos);
        model.SetPosition(step.x, step.y, step.z);
    }

    // Marks the nearest balloon or target the player is close to (the lock-on mark, effect 0x702).
    if (checkTargets == 1)
    {
        int nearest = -1;
        float nearestDistance = 0.0f;
        for (int target = BS002_BALLOON_FIRST; target < BS002_BALLOON_END; target++)
        {
            if (DAT_1021f3c0[target] == 'B' || DAT_10daaae8[target].kind == 0x26)
            {
                Vec4 spot;
                spot.Copy(DAT_10db6ee8[target].matrix.GetTranslation());
                if ((spot - playerPos).Length() < 200.0f)
                {
                    if (nearest != -1 && nearestDistance <= (spot - playerPos).Length2D())
                        continue;
                    nearest = target;
                    nearestDistance = (spot - playerPos).Length2D();
                }
            }
        }
        bool special = DAT_10db9380 == 2;
        if (nearest >= 0 && !special && nearest != st.markedTarget)
        {
            Matrix4 pose;
            EffectOptions options;
            Vec4 spot;
            Vec4 none;
            none.Fill(0.0f);
            options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
            options.m_0008 = 0.6f;
            pose = Scaling(1.5f, 1.5f, 1.5f);
            spot.Copy(DAT_10db6ee8[nearest].matrix.GetTranslation());
            pose.SetTranslation(spot);
            pose._43 += 15.0f;
            StartEffect(index, 0x702, none, 0, 0, 0, &pose, &options);
            st.markedTarget = nearest;
            Vec4 soundAt = pose.GetTranslation();
            FUN_10054550(0x291, 0x23, &soundAt);
        }
        else if (special && st.markedTarget != -1)
        {
            Matrix4 pose;
            EffectOptions options;
            Vec4 none;
            none.Fill(0.0f);
            options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
            options.m_0008 = 0.6f;
            pose = Scaling(1.5f, 1.5f, 1.5f);
            pose.SetTranslation(playerPos);
            pose._43 = -22.0f;
            DAT_10daaa80.Start(0x702, none, -1, -1, 0, 0, 0, &pose, &options, &DAT_10219770[index]);
            st.markedTarget = -1;
            Vec4 soundAt = pose.GetTranslation();
            FUN_10054550(0x291, 0x23, &soundAt);
        }
        else
            st.markedTarget = 0;
    }

    // The motion the state asked for (restarted when it changed or the state asked for a restart), and the frame it is at,
    // kept for the reaction branch of the next frame.
    if (motion > -1 && (st.motion != motion || restart != 0))
    {
        model.PlayMotion(motion, restart, blendFrames, 1.0f);
        st.motion = motion;
    }
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

    // Heading: turns towards the target heading by a third of the shortest way round (3.5 -> the original's divisor).
    float heading = model.heading;
    float target = model.targetHeading;
    while (heading > BS002_PI)
        heading -= BS002_TWO_PI;
    while (target > BS002_PI)
        target -= BS002_TWO_PI;
    while (heading < -BS002_PI)
        heading += BS002_TWO_PI;
    while (target < -BS002_PI)
        target += BS002_TWO_PI;
    float turn = heading - target;
    while (turn > BS002_PI)
        turn -= BS002_TWO_PI;
    while (turn < -BS002_PI)
        turn += BS002_TWO_PI;
    model.targetHeading = model.heading - turn / 3.5f;

    // Bystanders that were away for a long time (below -50000, or flagged) come back now and then.
    for (int slotNumber = BS002_BYSTANDER_FIRST; slotNumber < BS002_BYSTANDER_END; slotNumber++)
    {
        if ((DAT_10db6ee8[slotNumber].matrix._41 < -50000.0f || DAT_10210d30[slotNumber] == 1) && FUN_1013f880() % 1000 < 0x32)
            RespawnBystander(slotNumber);
    }
    return 0;
}
