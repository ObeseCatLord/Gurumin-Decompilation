// The BS_003 boss: a winged lizard fought on the Tokatyu maps. Its slot (kind 0x37) runs the cut-scene states 0..10
// (introduction, three speeches, the texture variants of its three forms) and, in every frame, the scripted fight:
// the map model, the boss's body and its look point follow animation nodes of tokatyu.it3 at the boss's frame, and
// key frames on those nodes start the claw swipes, the breath, the flame, the roar and the thrown rocks.

#include <math.h>
#include <string.h>
#include <windows.h>
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "node_data.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "effect_manager.h"
#include "texture_manager.h"
#include "bs003.h"
#include "football_state.h"
#include "hop_spit_enemy.h"
#include "partner_follow.h"
#include "doppler_loops.h"
#include "target_race.h"
#include "route_flier.h"
#include "npc_lookout.h"
#include "text_window.h"
#include "map_placement.h"
#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "script_system.h"
#include "sound_vars.h"
#include "ui_vars.h"
#include "world.h"

// Cut-scene states of the boss (CharState::state).
#define BS003_STATE_SETUP 0
#define BS003_STATE_WAIT_FOR_LOOKOUT 1
#define BS003_STATE_INTRODUCTION 2
#define BS003_STATE_SPEECH_ONE 3
#define BS003_STATE_FIRST_FORM 6
#define BS003_STATE_SPEECH_TWO 7
#define BS003_STATE_SECOND_FORM 8
#define BS003_STATE_SPEECH_THREE 9
#define BS003_STATE_THIRD_FORM 10
#define BS003_STATE_LAST_SCENE_STATE 9

#define BS003_MAP_LIFT 6555.0f              // the model that follows the map node sits this far above it
#define BS003_MAP_MODEL_SLOT 18             // the model placed at the map node
#define BS003_PI 3.1415f
#define BS003_TWO_PI 6.283f
#define BS003_HALF_PI 1.57075f
#define BS003_LEVEL_ANGLE 0.01745f          // a turn below this (one degree) counts as no turn
#define BS003_TURN_PERIOD 30                // frames of a turning motion
#define BS003_SHARD_COUNT 6
#define BS003_THROWN_SHARDS 3
#define BS003_ARENA_X 2075.0f               // x of the arena's centre line (the effects mirror around it)
#define BS003_SOUND_VOLUME 1.0f

// The sound/effect ids of the boss.
#define BS003_SOUND_STEP 0x2ec
#define BS003_SOUND_ROAR 0x2ed
#define BS003_SOUND_BEAM 0x2ee
#define BS003_SOUND_BREATH_HEAD 0x2ef
#define BS003_SOUND_FLAME_ON 0x2f0
#define BS003_SOUND_FLAME_ON_B 0x2f1
#define BS003_SOUND_SHAKE 0x2f2
#define BS003_SOUND_BLAST 0x2f3
#define BS003_SOUND_THUNDER 0x2f4
#define BS003_SOUND_SHARD 0x2f5

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

static inline Node *Bs003Bone(int which)
{
    return (Node *)DAT_1042d918[which];
}

static __forceinline float FoldHeading(float angle)
{
    while (angle > BS003_PI)
        angle -= BS003_TWO_PI;
    while (angle < -BS003_PI)
        angle += BS003_TWO_PI;
    return angle;
}

// Loads the model's and the lizard's nodes the fight reads: the bones from the boss's own tree and the animation
// nodes from the model tree of tokatyu.it3 (DAT_104c0390).
static void FindBossNodes(int index)
{
    Node *root = DAT_10db6ee8[index].root;
    DAT_1042d918[BS003_BONE_HEAD] = (int)root->Find("Bip01 Head", 0);
    DAT_1042d918[BS003_BONE_JAW] = (int)root->Find("Bone_ago", 0);
    DAT_1042d918[BS003_BONE_SPINE] = (int)root->Find("Bip01 Spine", 0);
    DAT_1042d918[BS003_BONE_LEFT_TOE] = (int)root->Find("Bip01 L Toe0", 0);
    DAT_1042d918[BS003_BONE_RIGHT_TOE] = (int)root->Find("Bip01 R Toe0", 0);
    DAT_1042d918[BS003_BONE_RIGHT_WING] = (int)root->Find("Bone_r_wing2", 0);
    DAT_1042d918[BS003_BONE_LEFT_WING] = (int)root->Find("Bone_l_wing2", 0);
    DAT_1042d918[BS003_BONE_EYE_ROOT] = (int)root->Find("eyeroot", 0);
    DAT_1021bdf8[index][BS003_NODE_MAP_MOVE] = DAT_104c0390->Find("mapmove", 0);
    DAT_1021bdf8[index][BS003_NODE_BODY_MOVE] = DAT_104c0390->Find("tokamove", 0);
    DAT_1021bdf8[index][BS003_NODE_SIGHT] = DAT_104c0390->Find("tokasiten", 0);
    DAT_1021bdf8[index][BS003_NODE_BREATH_A] = DAT_104c0390->Find("bress1", 0);
    DAT_1021bdf8[index][BS003_NODE_LASER] = DAT_104c0390->Find("laser1", 0);
    DAT_1021bdf8[index][BS003_NODE_CHARGE] = DAT_104c0390->Find("totugeki", 0);
    DAT_1021bdf8[index][BS003_NODE_SHAKE] = DAT_104c0390->Find("yure", 0);
    DAT_1021bdf8[index][BS003_NODE_THUNDER] = DAT_104c0390->Find("rakurai", 0);
    DAT_1021bdf8[index][BS003_NODE_BREATH_B] = DAT_104c0390->Find("tokahika", 0);
    DAT_1021bdf8[index - 1][0] = DAT_104c0390->Find("bress2", 0);
    DAT_1021bdf8[index - 1][2] = DAT_10255f08->Find("mp_307", 0);
}

// Matrix of animation node `node` of the boss's slot at `frame`.
static __forceinline Matrix4 NodeAt(int index, int node, int frame)
{
    return DAT_1021bdf8[index][node]->extra->AnimatedMatrix(frame);
}

// The map model follows the "mapmove" node: its matrix goes to the map's two nodes and the map model slot.
static void PlaceMapModel(int index, const Matrix4 &mapPose)
{
    DAT_1021bdf8[index - 1][2]->matrix = mapPose;
    DAT_1021bdf8[index - 1][2]->worldMatrix = mapPose;
    DAT_10255f0c->matrix = mapPose;
    DAT_10db6ee8[BS003_MAP_MODEL_SLOT].SetPosition(mapPose._41, mapPose._42, mapPose._43 + BS003_MAP_LIFT);
}

// True when key frame `frame` of animation node `node` is set (the node's translation track).
static __forceinline int HasKey(int index, int node, int frame)
{
    return DAT_1021bdf8[index][node]->extra->tracks[NODE_TRACK_TRANSLATION].HasFrame(frame);
}

// Starts the BS_003 form `variant` on the boss's model and gives it its look (the texture sets of the three forms).
static void ChangeForm(int index, int variant)
{
    FUN_10034920(index, variant);
    FUN_1015c9e0(0xf);
    DAT_10836c70.ResetFrameTimer();
}

// Keeps the boss where it is but turns it upright again (the cut-scene hands over to the next state).
static void ResetBossRotation(int index)
{
    Vec4 position;
    position.Copy(DAT_10db6ee8[index].root->matrix.GetTranslation());
    DAT_10db6ee8[index].root->matrix.SetIdentity();
    DAT_10db6ee8[index].root->matrix.SetTranslation(position);
}

// Remembers where the player stands as the cut-scene's shot point (integers, as the scene scripts keep them).
static void StoreShotPoint(const Vec4 &point)
{
    DAT_10c6356c = (int)point.x;
    DAT_10c63570 = (int)point.y;
    DAT_10c63574 = (int)point.z;
}

// Options the boss's effects start with: its effect parameter, a random size around one and the list.
static void InitBossEffectOptions(EffectOptions &options, int index, float size, int list)
{
    options.m_0000 = 0;
    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
    options.m_0008 = size;
    options.m_000e = list;
}

// 0x100e25f0 (Bs003_Update). One frame of the BS_003 boss in slot `index`: the cut-scene states, the scripted
// movement of the map model, the body and the look point at the boss's frame, and the events the key frames start.
// Returns 0.
int Bs003_Update(int index)
{
    DAT_10daaae8[index].m_0141 = 1;
    DAT_104c02c5 = (unsigned char)index;
    if (DAT_1020b430[index].state < 3)
        DAT_104c02c7 = 100;
    else if (DAT_1020b430[index].state < 7)
        DAT_104c02c7 = 75;
    else
        DAT_104c02c7 = DAT_1020b430[index].state > 8 ? 25 : 50;
    Matrix4 mapPose;
    Matrix4 bodyPose;
    Matrix4 sightPose;
    Vec4 playerPos;
    Vec4 bossPos;
    Vec4 zero;
    Vec4 bodyPos;
    Vec4 sightPos;
    mapPose.m[12] = 2150.0f;
    mapPose.m[14] = 6400.0f;
    DAT_10db6ee8[index].uniformScale = 0.1f;
    DAT_10db6ee8[index].m_2390 = 0;
    DAT_10db6ee8[index - 1].m_2390 = 0;
    mapPose.SetIdentity();
    DAT_10daaae8[index - 1].m_0141 = 1;
    DAT_10255a88[index] = 0;
    DAT_10daaae8[index].stats[STAT_BUSY] = 1;
    DAT_104c0110 = index;
    playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    bossPos.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    DAT_10db6ee8[index].important = 1;
    (bossPos - playerPos).Length2D();
    float distance = (bossPos - playerPos).Length();
    if (distance >= 2000.0f)
        DAT_1020344c = 1.85f;
    else
        DAT_1020344c = distance * 0.0001f + 1.65f;
    DAT_1101dddc = 150.0f;
    int motionFrame = DAT_10db6ee8[index].motionFrame;
    if (((motionFrame - 5) % 0x1f == 0x1b && motionFrame < 0x5f) ||
        (motionFrame > 0x3e3 && motionFrame < 0x401 && (motionFrame - 0x3e3) % 0x1f == 0x1b) ||
        (motionFrame > 0x401 && motionFrame < 0x41f && (motionFrame - 0x401) % 0x1f == 0x1b))
        FUN_10065b10(index, BS003_SOUND_STEP, 1, 0);

    CharState &state = DAT_1020b430[index];
    if (state.state == BS003_STATE_SETUP)
    {
        if (DAT_10255f08 == 0)
            return 0;
        FindBossNodes(index);
        state.bossFrame = 0;
        state.state = BS003_STATE_WAIT_FOR_LOOKOUT;
        state.bossBeamSlot = -1;
        mapPose = NodeAt(index, BS003_NODE_MAP_MOVE, 0);
        PlaceMapModel(index, mapPose);
        DAT_10802c08[index][0].Fill(0.0f);
        bodyPose = NodeAt(index, BS003_NODE_BODY_MOVE, state.bossFrame) * mapPose;
        sightPose = NodeAt(index, BS003_NODE_SIGHT, state.bossFrame) * mapPose;
        bodyPos = bodyPose.GetTranslation();
        sightPos = sightPose.GetTranslation();
        bossPos.Copy(bodyPos);
        DAT_10255160[index][0] = Atan2(sightPos.x - bodyPos.x, sightPos.y - bodyPos.y);
        DAT_10255160[index][1] = DAT_10255160[index][0];
        while (DAT_10255160[index][1] > BS003_PI)
            DAT_10255160[index][1] -= BS003_TWO_PI;
        while (DAT_10255160[index][1] < -BS003_PI)
            DAT_10255160[index][1] += BS003_TWO_PI;
        float rise = sightPos.z - bodyPos.z;
        Matrix4 facing;
        if (rise > 0.0f)
            facing = RotationX(-(float)acos((sightPos - bodyPos).Length2D() / (sightPos - bodyPos).Length())) *
                     RotationZ(DAT_10255160[index][0]);
        else if (rise < 0.0f)
            facing = RotationX(acosf((sightPos - bodyPos).Length2D() / (sightPos - bodyPos).Length())) *
                     RotationZ(DAT_10255160[index][0]);
        else
            facing = RotationZ(DAT_10255160[index][0]);
        DAT_10db6ee8[index].root->matrix = facing;
        DAT_10daaae8[index].stats[STAT_HEALTH] = DAT_10daaae8[index].stats[STAT_MAX_HEALTH];
        DAT_10daaae8[index - 1].stats[STAT_HEALTH] = DAT_10daaae8[index - 1].stats[STAT_MAX_HEALTH];
        FUN_10034920(index, 2);
        DAT_10db6ee8[index].PlayMotion(0, 0, 0, 1.0f);
    }
    else if (state.state == BS003_STATE_WAIT_FOR_LOOKOUT)
    {
        if (DAT_10c63390 != 0)
        {
            DAT_1032d4b0 = -DAT_104bf8f0 / 30.0f;
            DAT_10c63394 = 1;
            DAT_10c62e50[DAT_10daaae8[index].routeRecord] = 0;
            DAT_10210d30[index] = 0;
            DAT_10db6ee8[index].alpha = 1.0f;
            state.state = BS003_STATE_INTRODUCTION;
            state.bossFrame = -30;
            DAT_104bf8e8 = 0;
            DAT_104bf8ec = 0;
            DAT_104bf8e4 = 30;
        }
        mapPose = NodeAt(index, BS003_NODE_MAP_MOVE, state.bossFrame);
        PlaceMapModel(index, mapPose);
        bodyPose = NodeAt(index, BS003_NODE_BODY_MOVE, state.bossFrame) * mapPose;
        sightPose = NodeAt(index, BS003_NODE_SIGHT, state.bossFrame) * mapPose;
        bodyPos = bodyPose.GetTranslation();
        sightPos = sightPose.GetTranslation();
        DAT_10255160[index][0] = Atan2(sightPos.x - bodyPos.x, sightPos.y - bodyPos.y) - BS003_HALF_PI;
        float rise = sightPos.z - bodyPos.z;
        Matrix4 facing;
        if (rise > 0.0f)
            facing = RotationX(-(float)acos((sightPos - bodyPos).Length2D() / (sightPos - bodyPos).Length())) *
                     RotationZ(DAT_10255160[index][0]);
        else if (rise < 0.0f)
            facing = RotationX(acosf((sightPos - bodyPos).Length2D() / (sightPos - bodyPos).Length())) *
                     RotationZ(DAT_10255160[index][0]);
        else
            facing = RotationZ(DAT_10255160[index][0]);
        DAT_10db6ee8[index].root->matrix = facing;
    }
    else if (state.state == BS003_STATE_INTRODUCTION)
    {
        if (state.bossFrame == 0)
            FUN_10065b10(index, BS003_SOUND_ROAR, 1, 0);
        state.bossFrame++;
        if (state.bossFrame == 0x352)
        {
            FUN_1000da40(0, 100, 30);
            DAT_104c00c8 = 1;
        }
        else if (state.bossFrame == 900)
            state.state = BS003_STATE_SPEECH_ONE;
    }
    else if (state.state == BS003_STATE_SPEECH_ONE)
    {
        if (DAT_104c00d0 == 0)
        {
            StoreShotPoint(playerPos);
            if (DAT_10c64f8c == 0)
            {
                FUN_1000da40(0, 0, 0);
                FUN_100cc1d0("houdai0.txt", DAT_10c6476c);
            }
            FUN_1015c9e0(0xf);
            DAT_10836c70.ResetFrameTimer();
            DAT_104c00c8 = 0;
            state.state = BS003_STATE_FIRST_FORM;
            state.bossFrame = 0x8fc;
            ResetBossRotation(index);
            DAT_10daaae8[index].stats[STAT_HEALTH] += DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 3;
        }
    }
    else if (state.state == BS003_STATE_FIRST_FORM)
    {
        if (state.bossFrame == 0x8fc)
        {
            ChangeForm(index, 1);
            FUN_1000da40(0, 0, 0x1e);
            playerPos.x = (float)DAT_10c6356c;
            playerPos.y = (float)DAT_10c63570;
            playerPos.z = (float)DAT_10c63574;
            DAT_10db6ee8[0].SetPosition(2587.0f, 0.0f, 6525.0f);
            DAT_104c00c8 = 0;
            DAT_10db6ee8[index].m_0294 = 0;
            DAT_10db6ee8[index].PlayMotion(10, 1, 0, 1.0f);
            state.bossFrame++;
        }
        else if (state.bossFrame == 0xea6)
        {
            FUN_1000da40(0, 100, 30);
            DAT_104c00c8 = 1;
            state.bossFrame++;
        }
        else
        {
            if (state.bossFrame == 0xed8)
                state.state = BS003_STATE_SPEECH_TWO;
            state.bossFrame++;
        }
    }
    else if (state.state == BS003_STATE_SPEECH_TWO)
    {
        if (DAT_104c00d0 == 0)
        {
            StoreShotPoint(playerPos);
            if (DAT_10c64f8c == 0)
            {
                FUN_1000da40(0, 0, 0);
                FUN_100cc1d0("houdai2.txt", DAT_10c6476c);
            }
            FUN_1015c9e0(0xf);
            DAT_10836c70.ResetFrameTimer();
            for (int shard = 0; shard < BS003_THROWN_SHARDS; shard++)
                DAT_10868838[index - 1][shard * 2 + 0]._24 = 0.0f;
            DAT_104c00c8 = 0;
            state.state = BS003_STATE_SECOND_FORM;
            state.bossFrame = 0xf3c;
            ResetBossRotation(index);
        }
    }
    else if (state.state == BS003_STATE_SECOND_FORM)
    {
        if (state.bossFrame == 0xf3c)
        {
            ChangeForm(index, 4);
            FUN_1000da40(0, 0, 0x1e);
            playerPos.x = (float)DAT_10c6356c;
            playerPos.y = (float)DAT_10c63570;
            playerPos.z = (float)DAT_10c63574;
            DAT_10db6ee8[0].SetPosition(2587.0f, 0.0f, 6525.0f);
            DAT_104c00c8 = 0;
            DAT_10db6ee8[index].PlayMotion(10, 1, 0, 1.0f);
            DAT_10daaae8[index].stats[STAT_HEALTH] += DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 3;
            DAT_10db6ee8[index].m_0294 = 0;
            state.bossFrame++;
        }
        else if (state.bossFrame == 0x1612)
        {
            FUN_1000da40(0, 100, 30);
            DAT_104c00c8 = 1;
            for (int slot = BS003_PEOPLE_FIRST; slot < BS003_PEOPLE_END; slot++)
                if (DAT_10daaae8[slot].kind == 0)
                    DAT_10daaae8[slot].stats[STAT_HEALTH] = 0;
            state.bossFrame++;
        }
        else
        {
            if (state.bossFrame == 0x1644)
            {
                state.state = BS003_STATE_SPEECH_THREE;
                state.bossFrame = 0x1644;
                DAT_104c00c8 = 0;
            }
            state.bossFrame++;
        }
    }
    else if (state.state == BS003_STATE_SPEECH_THREE)
    {
        if (DAT_104c00d0 == 0)
        {
            DAT_104c00c8 = 0;
            StoreShotPoint(playerPos);
            if (DAT_10c64f8c == 0)
            {
                FUN_1000da40(0, 0, 0);
                FUN_100cc1d0("houdai3.txt", DAT_10c6476c);
            }
            FUN_1015c9e0(0xf);
            DAT_10836c70.ResetFrameTimer();
            state.state = BS003_STATE_THIRD_FORM;
            state.bossFrame = 0x1644;
            ResetBossRotation(index);
        }
    }
    else if (state.state == BS003_STATE_THIRD_FORM && DAT_104c00d0 == 0)
    {
        ChangeForm(index, 3);
        DAT_10daaae8[index].stats[STAT_HEALTH] += DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 3;
        state.bossFrame = 0;
    }

    if (DAT_104bfe6c == 0)
    {
        int frame = (state.bossFrame > 0 || state.state >= 100) ? state.bossFrame : 0;
        mapPose = NodeAt(index, BS003_NODE_MAP_MOVE, frame);
        Vec4 &lookPoint = DAT_10802bb8[index][0];
        lookPoint.Copy(DAT_118e69e8);
        if ((lookPoint - mapPose.GetTranslation()).Length2D() > 2700.0f)
        {
            Vec4 offset;
            (lookPoint - mapPose.GetTranslation()).Length2D();
            offset.Copy(lookPoint - mapPose.GetTranslation());
            float around = Atan2(offset.x, offset.y) + BS003_PI;
            offset.Fill(0.0f);
            offset.z = -2700.0f;
            offset.RotateZ(-around);
            float reach = ((offset + mapPose.GetTranslation()) - playerPos).Length2D();
            offset.z = reach / 780.0f * 140.0f + playerPos.z - mapPose._43;
            lookPoint.Copy(offset + mapPose.GetTranslation());
            DAT_118e69e8.Copy(lookPoint * 0.5f + DAT_118e69e8 * 0.5f);
            DAT_1020344c = (lookPoint - DAT_118e6cb8).Length2D() / 780.0f;
            if (DAT_1020344c > 1.85f)
                DAT_1020344c = 1.85f;
        }
        if (state.state > BS003_STATE_LAST_SCENE_STATE)
            return 0;
        PlaceMapModel(index, mapPose);
        bodyPose = NodeAt(index, BS003_NODE_BODY_MOVE, state.bossFrame > 0 ? state.bossFrame : 0) * mapPose;
        sightPose = NodeAt(index, BS003_NODE_SIGHT, state.bossFrame > 0 ? state.bossFrame : 0) * mapPose;
        bodyPos = bodyPose.GetTranslation();
        sightPos = sightPose.GetTranslation();
        DAT_10255160[index][0] = Atan2(sightPos.x - bodyPos.x, sightPos.y - bodyPos.y) - BS003_HALF_PI;
        float rise = sightPos.z - bodyPos.z;
        float pitch;
        if (rise > 0.0f)
            pitch = (float)acos((sightPos - bodyPos).Length2D() / (sightPos - bodyPos).Length());
        else if (rise < 0.0f)
            pitch = -acosf((sightPos - bodyPos).Length2D() / (sightPos - bodyPos).Length());
        else
            pitch = 0.0f;
        DAT_10db6ee8[index].root->matrix = RotationX(pitch) * RotationZ(DAT_10255160[index][0]);
        DAT_10db6ee8[index].targetHeading = -BS003_HALF_PI;
        DAT_10db6ee8[index].heading = -BS003_HALF_PI;
    }

    DAT_10255160[index][0] = FoldHeading(DAT_10255160[index][0]);
    float turn = DAT_10255160[index][0] - DAT_10255160[index][1];
    if (turn > BS003_PI)
        turn -= BS003_TWO_PI;
    else if (turn < -BS003_PI)
        turn += BS003_TWO_PI;
    int playing = DAT_10db6ee8[index].motionFrame;
    if (turn > BS003_LEVEL_ANGLE)
    {
        if ((state.bossPose == 0 || state.bossPose == 2) && (playing - 5) % BS003_TURN_PERIOD == 0)
        {
            state.bossPose = 1;
            DAT_10db6ee8[index].PlayMotion(0x10, 0, 5, 1.0f);
        }
    }
    else if (turn < -BS003_LEVEL_ANGLE)
    {
        if ((state.bossPose == 0 || state.bossPose == 1) && (playing - 5) % BS003_TURN_PERIOD == 0)
        {
            state.bossPose = 2;
            DAT_10db6ee8[index].PlayMotion(0x11, 0, 5, 1.0f);
        }
    }
    else if ((state.bossPose == 0 || state.bossPose == 1) && (playing - 9) % BS003_TURN_PERIOD == 0)
        state.bossPose = 0;
    DAT_10255160[index][1] = DAT_10255160[index][0];

    if (HasKey(index, BS003_NODE_BREATH_A, state.bossFrame) && state.bossFrame > 0)
    {
        int motion;
        if (state.state < 5)
        {
            state.bossPose = 2;
            motion = 0xc;
        }
        else
        {
            motion = 0x1b;
            state.bossPose = state.state < 7 ? 3 : 4;
        }
        DAT_10db6ee8[index].PlayMotion(motion, 0, 5, 1.0f);
    }

    if (HasKey(index, BS003_NODE_LASER, state.bossFrame) && state.bossFrame > 0 && state.bossBreathHeld == 0)
    {
        Matrix4 muzzle;
        Vec4 mouth;
        Vec4 aim;
        Vec4 target;
        EffectOptions options;
        mouth.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        Vec4 eyeAt = Bs003Bone(BS003_BONE_EYE_ROOT)->worldMatrix.GetTranslation();
        target.z = 6538.0f;
        target.Copy(eyeAt);
        float aimHeading = Atan2(target.x - mouth.x, target.y - mouth.y);
        if (6538.0f <= target.z)
            muzzle = RotationX((float)acosf(mouth.Length2D() / mouth.Length())) * RotationZ(aimHeading + BS003_PI);
        else
            muzzle = RotationX(-acosf(mouth.Length2D() / mouth.Length())) * RotationZ(aimHeading + BS003_PI);
        muzzle.SetTranslation(target);
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0000 = 0;
        int beam = FUN_10034f70(&DAT_10db6ee8[0].matrix, 0.0f, 0.0f, -(DAT_10db6ee8[0].matrix._43 - 6538.0f), 0x32);
        target.Fill(0.0f);
        FUN_1001d8b0(index, 0x606, aim, -1, -1, &Bs003Bone(BS003_BONE_EYE_ROOT)->worldMatrix,
                     (int)&DAT_10f08880[beam], 0, &muzzle, 0);
        if (state.state < 7)
        {
            FUN_1001d8b0(index, 0x608, target, -1, -1, 0, 0, 0, 0, &options);
            FUN_10054550(BS003_SOUND_BREATH_HEAD, 0xf, &mouth);
        }
        else
        {
            Vec4 step;
            Vec4 side;
            state.bossBeamSlot = beam;
            Matrix4 &slot = DAT_10f08880[beam];
            step.Copy(slot.GetTranslation());
            step = step.Normalized3();
            side = step * -1.0f;
            slot.m[12] = target.x;
            slot.m[13] = target.y;
            slot.m[14] = 6550.0f;
            slot.m[15] = 50.0f;
            slot.m[0] = side.x;
            slot.m[1] = side.y;
        }
        FUN_1003ad20(BS003_SOUND_BEAM, &mouth, BS003_SOUND_VOLUME);
    }

    if (HasKey(index, BS003_NODE_CHARGE, state.bossFrame) && state.bossFrame > 0)
    {
        if (state.bossFlameOn == 0)
        {
            FUN_10065b10(index, BS003_SOUND_FLAME_ON, 1, 0);
            FUN_10065b10(index, BS003_SOUND_FLAME_ON_B, 1, 0);
            state.bossFlameOn = 1;
            DAT_10db6ee8[index].PlayMotion(0x18, 0, 5, 1.0f);
        }
        else
        {
            state.bossFlameOn = 0;
            DAT_10db6ee8[index].PlayMotion(0x1a, 0, 5, 1.0f);
        }
    }

    if (HasKey(index, BS003_NODE_SHAKE, state.bossFrame) && state.bossFrame > 0)
    {
        FUN_1016b230("\x33\x83\x92?\x82\xe0\x82\xa4\x93\x6e\x82\xc1\x82\xbd\x82\xbe\x81\x49");
        FUN_10055ee0();
        FUN_10065b10(index, BS003_SOUND_SHAKE, 1, 0);
        Vec4 cinder;
        Matrix4 at;
        cinder.x = BS003_ARENA_X;
        (cinder - DAT_10db6ee8[0].matrix.GetTranslation()).Length2D();
        if ((cinder - DAT_10db6ee8[0].matrix.GetTranslation()).Length2D() < 600.0f)
        {
            Vec4 none;
            none.Fill(0.0f);
            const int shake[3] = {0x44c, 0x44d, 0x44f};
            for (int delay = 0; delay <= 10; delay += 5)
                for (int effect = 0; effect < 3; effect++)
                    FUN_1001d8b0(index, shake[effect], cinder, -1, -1, 0, 0, delay, &at, 0);
        }
    }

    if (state.bossPose == 0)
    {
        Playback *motion = DAT_10db6ee8[index].motion;
        if ((motion->layer < 0x18 || motion->layer > 0x1b) && (motion->nextLayer < 0x18 || motion->nextLayer > 0x1b))
            DAT_10db6ee8[index].PlayMotion(10, 0, 5, 1.0f);
    }

    if (state.bossPose == 2 || state.bossPose == 3)
    {
        int playingNow = DAT_10db6ee8[index].motionFrame;
        if (playingNow == 0x4f || playingNow == 0x460)
        {
            Vec4 hitPoint;
            Matrix4 hitMatrix;
            EffectOptions options;
            hitPoint.Copy(Bs003Bone(BS003_BONE_JAW)->worldMatrix.GetTranslation());
            hitMatrix = RotationX(-0.1f) * RotationZ(DAT_10255160[index][0] - BS003_HALF_PI);
            hitMatrix.SetTranslation(hitPoint);
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0000 = 0;
            if (state.bossPose == 2)
            {
                Vec4 none;
                none.Fill(0.0f);
                FUN_1001d8b0(index, 10000, hitPoint, -1, -1, &Bs003Bone(BS003_BONE_JAW)->worldMatrix, 0, 0,
                             &hitMatrix, &options);
                FUN_10065b10(index, BS003_SOUND_BLAST, 1, 0);
            }
            else
            {
                Matrix4 &first = DAT_10868838[index - 1][0];
                Matrix4 &second = DAT_10868838[index - 1][1];
                Matrix4 &orient = DAT_10868838[index - 1][2];
                first.SetIdentity();
                first.SetTranslation(hitPoint);
                second.SetIdentity();
                second.SetTranslation(hitPoint);
                Vec4 spawn(BS003_ARENA_X, 0.0f, 6600.0f, 0.0f);
                spawn.RotateZ(((float)(FUN_1013f880() % 0x168) / 180.0f) * 3.14159f);
                orient = RotationZ(((float)(FUN_1013f880() % 0x168) / 180.0f) * 3.14159f + BS003_PI);
                orient.SetTranslation(spawn);
                FUN_1001d8b0(index, 0x6b9, hitPoint, -1, -1, &first, 0, 0, &hitMatrix, &options);
                FUN_1001d8b0(index, 0x6b9, hitPoint, -1, -1, &second, 0, 0, &hitMatrix, &options);
                first._11 = (orient._41 - first._41) / 60.0f;
                first._12 = (orient._42 - first._42) / 60.0f;
                first._13 = (orient._43 - first._43) / 60.0f;
                second._11 = ((BS003_ARENA_X - (orient._41 - BS003_ARENA_X)) - second._41) / 60.0f;
                second._12 = (-orient._42 - second._42) / 60.0f;
                second._13 = (orient._43 - second._43) / 60.0f;
                state.bossRockFrames = 180;
                FUN_10065b10(index, BS003_SOUND_THUNDER, 1, 0);
            }
        }
        else if (playingNow == 0x5e || playingNow == 0x469)
        {
            state.bossPose = 0;
            DAT_10db6ee8[index].PlayMotion(10, 0, 0, 1.0f);
        }
    }

    if (state.bossBeamSlot >= 0)
    {
        Matrix4 &slot = DAT_10f08880[state.bossBeamSlot];
        Vec4 playerHere = DAT_10db6ee8[0].matrix.GetTranslation();
        slot.m[12] += slot.m[4];
        slot.m[13] += slot.m[5];
        Vec4 tip(slot.m[12], slot.m[13], slot.m[14], 0.0f);
        float closeness = (playerHere - tip).Length2D();
        if (closeness < 500.0f && FUN_1013f880() % 5 == 0)
        {
            Vec4 here(tip.x + BS003_ARENA_X, tip.y, 6600.0f, 0.0f);
            EffectOptions options;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0000 = 0;
            FUN_1001d8b0(index, 0x69d, here, -1, -1, 0, 0, 0, &mapPose, &options);
            for (int delay = 5; delay <= 20; delay += 5)
                FUN_1001d8b0(index, 0x138a, here, -1, -1, 0, 0, delay, &mapPose, &options);
            FUN_1003ad20(0x2e6, &here, BS003_SOUND_VOLUME);
        }
        slot.m[3] -= 1.0f;
        if (slot.m[3] < 1.0f)
            state.bossBeamSlot = -1;
    }

    if (state.bossRockFrames > 0)
    {
        if (state.bossRockFrames < 0x79)
        {
            if (state.bossRockFrames == 0x5a)
            {
                EffectOptions options;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0000 = 0;
                options.m_0008 = (float)(FUN_1013f880() % 200) * 0.01f + 0.9f;
                Vec4 none;
                none.Fill(0.0f);
                FUN_1001d8b0(index, 0x6c0, DAT_10868838[index - 1][0].GetTranslation(), -1, -1, &DAT_10868838[index - 1][1],
                             0, 0, &DAT_10868838[index - 1][2], &options);
                FUN_1003ad20(0x28a, &none, BS003_SOUND_VOLUME);
                FUN_1003ad20(0x28a, &none, BS003_SOUND_VOLUME);
            }
            else if (state.bossRockFrames > 10 && state.bossRockFrames < 0x55 && state.bossRockFrames % 10 == 0)
            {
                Vec4 none;
                none.Fill(0.0f);
                FUN_1001d8b0(index, 0x6bd, none, -1, -1, &DAT_10868838[index - 1][1], 0, 0, &DAT_10868838[index - 1][2], 0);
            }
        }
        else
        {
            for (int shard = 0; shard < 2; shard++)
            {
                DAT_10868838[index - 1][shard].translation.x += DAT_10868838[index - 1][shard]._11;
                DAT_10868838[index - 1][shard].translation.y += DAT_10868838[index - 1][shard]._12;
                DAT_10868838[index - 1][shard].translation.z += DAT_10868838[index - 1][shard]._13;
            }
        }
        state.bossRockFrames--;
    }

    if (state.state > 6)
    {
        for (int shard = 0; shard < BS003_THROWN_SHARDS; shard++)
        {
            Matrix4 &piece = DAT_10868838[index][shard + 1];
            if (piece._14 > 1.0f)
            {
                piece._14 -= 1.0f;
                if (piece._14 <= 89.0f)
                {
                    if (piece._14 == 60.0f || piece._14 == 30.0f)
                    {
                        EffectOptions options;
                        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                        options.m_0000 = 0;
                        options.m_0008 = (float)(FUN_1013f880() % 0x14) * 0.01f + 0.9f;
                        Vec4 base;
                        base.Copy(piece.GetTranslation());
                        for (int spread = -2; spread < 3; spread++)
                        {
                            Matrix4 spin = RotationX(BS003_HALF_PI) * RotationZ(Atan2(base.x, base.y) + (float)spread);
                            spin.SetTranslation(piece.GetTranslation());
                            DAT_10daaa80.Start(0x6c2, piece.GetTranslation(), -1, -1, 0, 0, 0, &spin, &options,
                                               &DAT_10219770[index]);
                        }
                        FUN_1003ad20(BS003_SOUND_SHARD, &base, BS003_SOUND_VOLUME);
                    }
                }
                else
                {
                    piece.translation.x += piece._11;
                    piece.translation.y += piece._12;
                    piece.translation.z += piece._13;
                }
            }
        }
    }

    if (state.bossFlameOn != 0)
    {
        Vec4 flameAt;
        Vec4 mouthAt;
        flameAt.Fill(0.0f);
        flameAt.z = -310.0f;
        flameAt.RotateX(0.0f);
        flameAt.RotateZ(DAT_10255160[index][0]);
        mouthAt.Copy(flameAt + Bs003Bone(BS003_BONE_JAW)->worldMatrix.GetTranslation());
        Matrix4 flameMatrix;
        flameMatrix.SetIdentity();
        flameMatrix.SetTranslation(mouthAt);
        if (state.bossFrame % 5 == 0)
        {
            Matrix4 spray;
            EffectOptions options;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0000 = 0;
            options.settings[0] = 1;
            spray = RotationX(BS003_HALF_PI) * RotationZ(DAT_10255160[index][0] - BS003_HALF_PI);
            spray.SetTranslation(mouthAt);
            DAT_10daaa80.Start(0x4e2, mouthAt, -1, -1, &flameMatrix, 0, 0, &spray, &options, &DAT_10219770[index]);
        }
        EffectOptions options;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0000 = 0;
        options.m_000e = 5;
        DAT_10daaa80.Start(0x4e7, mouthAt, -1, -1, &DAT_10db6ee8[index].root->Find("Bip01", 0)->worldMatrix, 0, 0, 0,
                           &options, &DAT_10219770[index]);
    }

    if (state.state > 4)
    {
        state.bossBlastTimer++;
        if (state.bossBlastTimer == 300)
            state.bossBlastTimer = 0;
        if (state.bossBlastTimer % 3 == 0)
        {
            int bone[3];
            do
            {
                bone[0] = FUN_1013f880() % 7;
                bone[1] = FUN_1013f880() % 7;
                bone[2] = FUN_1013f880() % 7;
            } while (bone[0] == bone[1] || bone[0] == bone[2]);
            for (int pick = 0; pick < 3; pick++)
            {
                Matrix4 at;
                at.SetIdentity();
                at.SetTranslation(Bs003Bone(bone[pick])->worldMatrix.GetTranslation());
                EffectOptions options;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0000 = 0;
                if (state.state < 7)
                    DAT_10daaa80.Start(0x4e6, at.GetTranslation(), -1, -1, &Bs003Bone(bone[pick])->worldMatrix, 0, 0,
                                       &at, 0, &DAT_10219770[index]);
                else if (state.state < 9)
                    DAT_10daaa80.Start(0x138a, at.GetTranslation(), -1, -1, &Bs003Bone(bone[pick])->worldMatrix, 0,
                                       0, &at, 0, &DAT_10219770[index]);
            }
        }
    }

    if (HasKey(index, BS003_NODE_THUNDER, state.bossFrame + 0x1e) && state.bossFrame > 0 && state.state < 7)
    {
        int shard = 1;
        while (shard < BS003_SHARD_COUNT && DAT_10868838[index][shard]._14 >= 0.9f)
            shard++;
        if (shard != BS003_SHARD_COUNT)
        {
            Matrix4 &piece = DAT_10868838[index][shard];
            piece.SetIdentity();
            piece.SetTranslation(NodeAt(index, BS003_NODE_THUNDER, state.bossFrame + 0x1e).GetTranslation());
            piece.translation.z = 4540.0f;
            piece._14 = 120.0f;
            piece._24 = 1.0f;
            Vec4 launch = NodeAt(index, BS003_NODE_THUNDER, state.bossFrame + 0x1e).GetTranslation();
            Vec4 landing = NodeAt(index, BS003_NODE_THUNDER, state.bossFrame + 0x1e).GetTranslation();
            Vec4 delta = launch - landing;
            piece._21 = delta.x;
            piece._22 = delta.y;
            FUN_10054550(0x2f7, 0x1e, &launch);
            EffectOptions options;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0000 = 0;
            options.m_0008 = (float)(FUN_1013f880() % 10) * 0.02f + 1.0f;
            FUN_1001d8b0(index, 0x6b1, launch, -1, -1, &piece, 0, 0, 0, &options);
        }
    }

    if (HasKey(index, BS003_NODE_BREATH_B, state.bossFrame) && state.bossFrame > 0)
    {
        if (state.state == BS003_STATE_FIRST_FORM)
        {
            Matrix4 headMatrix;
            Vec4 none;
            headMatrix.SetIdentity();
            headMatrix.SetTranslation(Bs003Bone(BS003_BONE_HEAD)->worldMatrix.GetTranslation());
            none.Fill(0.0f);
            EffectOptions options;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0000 = 0;
            DAT_10daaa80.Start(0x6b8, headMatrix.GetTranslation(), -1, -1, &Bs003Bone(BS003_BONE_HEAD)->worldMatrix, 0,
                               0, &headMatrix, 0, &DAT_10219770[index]);
            DAT_10daaa80.Start(0x6b8, headMatrix.GetTranslation(), -1, -1, &Bs003Bone(BS003_BONE_HEAD)->worldMatrix, 0,
                               10, &headMatrix, 0, &DAT_10219770[index]);
        }
        else if (state.state == BS003_STATE_SECOND_FORM)
        {
            for (int shard = 0; shard < BS003_THROWN_SHARDS; shard++)
            {
                Matrix4 &piece = DAT_10868838[index][shard + 1];
                if (piece._14 < 1.0f)
                {
                    EffectOptions options;
                    Vec4 pick;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    piece.SetIdentity();
                    pick.Copy(piece.GetTranslation());
                    pick.x = BS003_ARENA_X;
                    pick.y = 0.0f;
                    pick.RotateZ((float)(FUN_1013f880() % 0xb4) / 180.0f * 3.14159f);
                    piece.m[12] = pick.x;
                    piece.m[13] = pick.y;
                    piece.m[14] = 6600.0f;
                    piece._14 = 120.0f;
                    DAT_10daaa80.Start(0x6c1, pick, -1, -1, 0, 0, 0, &piece, &options, &DAT_10219770[index]);
                    DAT_10daaa80.Start(0x6b8, pick, -1, -1, &piece, 0, 0, 0, &options, &DAT_10219770[index]);
                    FUN_10065b10(index, BS003_SOUND_SHARD, 1, 0);
                    break;
                }
            }
        }
    }

    for (int shard = 0; shard < BS003_SHARD_COUNT; shard++)
    {
        Matrix4 &piece = DAT_10868838[index][shard];
        Vec4 &drift = DAT_104c4bd8[5 - shard];
        if (piece._14 > 0.9f && state.state < 7)
        {
            piece.translation.x = mapPose.translation.x + drift.x;
            piece.translation.y = mapPose.translation.y + drift.y;
            piece.translation.z = mapPose.translation.z + drift.z;
            piece._14 -= 1.0f;
            if (piece._24 == 1.0f && FUN_1013f880() % 6 == 5)
            {
                Vec4 here(BS003_ARENA_X, 0.0f, 6600.0f, 0.0f);
                if ((here - DAT_10db6ee8[0].matrix.GetTranslation()).Length2D() < 400.0f)
                {
                    Matrix4 at;
                    EffectOptions options;
                    at.SetIdentity();
                    at.SetTranslation(Vec4(piece.translation.x, piece.translation.y, 6610.0f, 0.0f));
                    DAT_10daaa80.Start(0x6b5, at.GetTranslation(), -1, -1, 0, 0, 0, &at, 0, &DAT_10219770[index]);
                }
            }
        }
    }
    Vec4 &follow = DAT_10802c08[index][0];
    Vec4 smoothed = (bodyPos - bossPos) * 0.9f + follow * 0.1f;
    follow.Copy(smoothed);
    DAT_10db6ee8[index].SetPosition(follow.x + bossPos.x, follow.y + bossPos.y, follow.z + bossPos.z);
    return 0;
}
