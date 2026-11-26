// The BS_001 boss (kind 100): a fighter that walks, runs, lunges, punches, claws and smashes at the player, with an
// optional partner slot that copies its animation. Each frame it fades itself and its partner in and out, reacts to
// hits, runs the defeat scene (the four piece models fly apart while the camera swings round), picks its next move
// by chance and runs the move it is in, then spawns the footstep and hit effects of the motion it plays.

#include <math.h>
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "effect_manager.h"
#include "texture_manager.h"
#include "game.h"
#include "bs001.h"
#include "bs003.h"
#include "hop_spit_enemy.h"
#include "partner_follow.h"
#include "doppler_loops.h"
#include "target_race.h"
#include "route_flier.h"
#include "dodge_golem.h"
#include "game_main.h"
#include "texture_load.h"
#include "value_command.h"
#include "camera_vars.h"
#include "render_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "scene_reset.h"
#include "script_system.h"
#include "world.h"

#define BS001_PI 3.1415f
#define BS001_TWO_PI 6.283f

// Fade counters (Bs001Fade): frames of the fade out and of the fade in; the model is gone at the middle.
#define BS001_FADE_MIDDLE 15
#define BS001_FADE_END 30
// The fall below which the boss counts as lost and fades back in at its partner's side.
#define BS001_LOST_HEIGHT -3400.0f
#define BS001_LOST_LIMIT -10000.0f
// Where a model waits while it is hidden.
#define BS001_HIDDEN_X -200000.0f
// Height the faded-out models wait at.
#define BS001_FADE_PARK_Z 600.0f
#define BS001_FADE_PARK_PARTNER_Z 1200.0f

// Motions the boss plays (Bs001State::motion).
#define BS001_MOTION_WALK 0
#define BS001_MOTION_RUN 1
#define BS001_MOTION_IDLE 2
#define BS001_MOTION_TURN 3
#define BS001_MOTION_STOMP 4
#define BS001_MOTION_LUNGE 5
#define BS001_MOTION_SHOUT 10
#define BS001_MOTION_FALL 0xb
#define BS001_MOTION_DOWN 0x13
#define BS001_MOTION_SMASH 0x14
#define BS001_MOTION_HURT 0x19
#define BS001_MOTION_HIT 0x1a

// Voices (Enemy_PlayVoice).
#define BS001_VOICE_FADE 0x1e4
#define BS001_VOICE_PAIN_FIRST 0x1e6
#define BS001_VOICE_PAIN_COUNT 5
#define BS001_VOICE_SHOUT 500
#define BS001_VOICE_DEFEATED 0x1eb
#define BS001_VOICE_STAND 0x1ec
#define BS001_VOICE_RISE 0x1ed
#define BS001_VOICE_FALL_FIRST 0x1ee
#define BS001_VOICE_RETREAT 0x1f0
#define BS001_VOICE_PUNCH 0x1f1
#define BS001_VOICE_CLAW 0x1f2
#define BS001_VOICE_SMASH 499

// Sound effects (Character_PlaySound).
#define BS001_SOUND_DEFEAT_START 0x2fd
#define BS001_SOUND_DEFEAT_BLAST 0x2fe
#define BS001_SOUND_STOMP 0x2ff
#define BS001_SOUND_SHOUT_RING 0x300
#define BS001_SOUND_SHOUT 0x301
#define BS001_SOUND_PUNCH 0x302
#define BS001_SOUND_CLAW 0x303
#define BS001_SOUND_SMASH_HIT 0x304
#define BS001_SOUND_STEP_FIRST 0x305
#define BS001_SOUND_STEP_SECOND 0x306
#define BS001_SOUND_SMASH_END 0x307

// Effects.
#define BS001_EFFECT_FOOTSTEP 0x2de
#define BS001_EFFECT_SHOUT_RING 0x611
#define BS001_EFFECT_SHOUT_SPARK 0x19
#define BS001_EFFECT_SHOUT_SPARK_OUTER 0x1d
#define BS001_EFFECT_SHOUT_CLOUD 0x271a
#define BS001_EFFECT_STOMP_L 0x613
#define BS001_EFFECT_STOMP_R 0x614
#define BS001_EFFECT_PUNCH_FLASH 0x60e
#define BS001_EFFECT_PUNCH_TRAIL 0x5ef
#define BS001_EFFECT_CLAW_FLASH 0x661
#define BS001_EFFECT_SMASH_DUST 0x454
#define BS001_EFFECT_SMASH_FLASH 0x44e
#define BS001_EFFECT_TRAIL_STANDARD 0x664
#define BS001_EFFECT_TRAIL_SMASH 0x455
#define BS001_EFFECT_ATTACH 5
#define BS001_EFFECT_RISE_DUST 0x160
#define BS001_EFFECT_DEFEAT_SMOKE_A 0x321
#define BS001_EFFECT_DEFEAT_SMOKE_B 0x322
#define BS001_EFFECT_BLOW 0x960

// Frame ranges of the smash animation (motion frame of the model).
#define BS001_SMASH_FRAME_FIRST 0x234
#define BS001_SMASH_FRAME_LAST 0x24e
#define BS001_SMASH_END_FRAME 0x289

// Defeat scene (Bs001State::defeatTimer counts its frames).
#define BS001_DEFEAT_FADE_FRAMES 0x5a
#define BS001_DEFEAT_FLASH_FRAMES 0x3c
#define BS001_DEFEAT_BLAST_FRAME 0x72
#define BS001_DEFEAT_END_FRAME 0xa8
#define BS001_DEFEAT_SCENE_END 0xd2
#define BS001_DEFEAT_SPLIT_FIRST 0x2a
#define BS001_DEFEAT_FILTER_FRAME 0xb4
#define BS001_DEFEAT_FINISH_FRAME 0xc3
#define BS001_DEFEAT_DUMMY_FRAME 0x1e
#define BS001_DEFEAT_CAMERA_DELAY 0xa0
#define BS001_DEFEAT_START_VALUE -30

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

static inline Node *Bs001BoneNode(int which)
{
    return (Node *)DAT_1042d918[which];
}

// Brings `angle` into -pi..pi.
static __forceinline float WrapAngle(float angle)
{
    while (angle > BS001_PI)
        angle -= BS001_TWO_PI;
    while (angle < -BS001_PI)
        angle += BS001_TWO_PI;
    return angle;
}

// Starts effect `effect` for the boss at `position`, attached to `owner` (a node's world matrix) with the given
// matrix and options.
static void BossEffect(int index, int effect, const Vec4 &position, const Matrix4 *owner, const Matrix4 *matrix,
                       const EffectOptions *options)
{
    FUN_1001d8b0(index, effect, position, -1, -1, owner, 0, 0, matrix, options);
}

// Starts effect `effect` at the world matrix of bone `bone` (if the boss has it) with the boss's effect options.
static void BoneEffect(int index, int effect, const Vec4 &position, Node *bone, const EffectOptions *options)
{
    if (bone != 0)
        FUN_1001d8b0(index, effect, position, -1, -1, 0, 0, 0, &bone->worldMatrix, options);
}

// Hides model `model` far away.
static void HideModel(AnimModel &model)
{
    model.SetPosition(BS001_HIDDEN_X, 0.0f, 0.0f);
}

// Turns the boss's goal heading towards `direction` (x/y of a vector from the boss to somewhere).
static void FaceDirection(AnimModel &model, const Vec4 &direction)
{
    model.targetHeading = (float)atan2(direction.x, direction.y) + BS001_PI;
}

// Pushes the boss forwards (along its heading) by the 2D length of `direction`, unless enemies are frozen.
static void PushForwards(AnimModel &model, Vec4 &direction)
{
    direction.y = -direction.Length2D();
    direction.x = 0.0f;
    direction.RotateZ(-model.heading);
    model.Push(direction.x, direction.y, 0.0f);
}

// 0x1010c650 (Bs001_Update; C++ linkage). One frame of the BS_001 boss in slot `index`.
int Bs001_Update(int index)
{
    AnimModel &model = DAT_10db6ee8[index];
    AnimModel &partnerModel = DAT_10db6ee8[DAT_10daaae8[index].m_00a4];
    int &fadeTrigger = DAT_10daaae8[index].stats[STAT_PICKUPS];
    int restartMotion = 0;
    int quickBlend = 0;
    int syncFlag = 0;
    int hardMode = DAT_10c635f8 == 2;
    Vec4 moveDir;
    Vec4 selfPos;
    Vec4 temp;

    if (model.matrix._43 < BS001_LOST_HEIGHT && model.matrix._41 > BS001_LOST_LIMIT)
        fadeTrigger = 100;
    if (fadeTrigger == 100 && DAT_1020b430[index + 2].bs001Fade.fadeOut == 0)
    {
        DAT_1020b430[index + 2].bs001Fade.fadeOut = 1;
        if (DAT_10c66cc0 != 0)
            FUN_100b9bb0(index, BS001_VOICE_FADE, 1, 1);
    }
    if (DAT_10daaae8[DAT_10daaae8[index].m_00a4].stats[STAT_PICKUPS] == 100)
    {
        if (DAT_10daaae8[index].m_00a8 == 0)
        {
            if (DAT_1020b430[index + 2].bs001Fade.fadeIn == 0)
                DAT_1020b430[index + 2].bs001Fade.fadeIn = 1;
        }
        else
            DAT_10daaae8[DAT_10daaae8[index].m_00a4].stats[STAT_PICKUPS] = 0;
    }
    if (DAT_1020b430[index + 2].bs001Fade.fadeOut != 0)
    {
        if (DAT_1020b430[index + 2].bs001Fade.fadeOut < BS001_FADE_MIDDLE)
        {
            float step = (float)(BS001_FADE_MIDDLE - DAT_1020b430[index + 2].bs001Fade.fadeOut);
            DAT_1020b430[index + 2].bs001Fade.fadeOut++;
            model.alpha = step / 15.0f;
        }
        else if (DAT_1020b430[index + 2].bs001Fade.fadeOut == BS001_FADE_MIDDLE)
        {
            model.alpha = 0.0f;
            model.SetPosition(0.0f, 0.0f, BS001_FADE_PARK_Z);
            DAT_1020b430[index + 2].bs001Fade.fadeOut++;
        }
        else if (DAT_1020b430[index + 2].bs001Fade.fadeOut < BS001_FADE_END)
        {
            float step = (float)(DAT_1020b430[index + 2].bs001Fade.fadeOut - BS001_FADE_MIDDLE);
            DAT_1020b430[index + 2].bs001Fade.fadeOut++;
            model.alpha = step / 15.0f;
        }
        else
        {
            DAT_1020b430[index + 2].bs001Fade.fadeOut = -1;
            int next = DAT_1020b430[index + 2].bs001Fade.fadeOut;
            model.alpha = 1.0f;
            fadeTrigger = 0;
            DAT_1020b430[index + 2].bs001Fade.fadeOut = next + 1;
        }
    }
    if (DAT_1020b430[index + 2].bs001Fade.fadeIn != 0)
    {
        if (DAT_1020b430[index + 2].bs001Fade.fadeIn < BS001_FADE_MIDDLE)
        {
            float step = (float)(BS001_FADE_MIDDLE - DAT_1020b430[index + 2].bs001Fade.fadeOut);
            partnerModel.alpha = step / 15.0f;
        }
        else if (DAT_1020b430[index + 2].bs001Fade.fadeIn == BS001_FADE_MIDDLE)
        {
            partnerModel.alpha = 0.0f;
            partnerModel.SetPosition(0.0f, 0.0f, BS001_FADE_PARK_PARTNER_Z);
        }
        else if (DAT_1020b430[index + 2].bs001Fade.fadeIn < BS001_FADE_END)
        {
            float step = (float)(DAT_1020b430[index + 2].bs001Fade.fadeOut - BS001_FADE_MIDDLE);
            partnerModel.alpha = step / 15.0f;
        }
        else
        {
            partnerModel.alpha = 1.0f;
            DAT_1020b430[index + 2].bs001Fade.fadeIn = -1;
            DAT_10daaae8[DAT_10daaae8[index].m_00a4].stats[STAT_PICKUPS] = 0;
        }
        DAT_1020b430[index + 2].bs001Fade.fadeIn++;
    }
    if (DAT_1020b430[index + 1].bs001Aux.partnerCooldown == 0 && DAT_10daaae8[index].m_00a8 == 0)
        syncFlag = 1;
    if (DAT_1020b430[index + 1].bs001Aux.partnerCooldown != 0 && DAT_10daaae8[index].m_00a8 != 0)
        syncFlag = 2;
    int partnerMode = DAT_1020b430[index].bs001.initialised;
    DAT_11b04510 = 1;
    DAT_10219e40[index] = (char)0xff;
    DAT_104c0110 = index;
    DAT_1021f6c0[index] = 0;
    DAT_1020b430[index].bs001.motion = -1;
    model.uniformScale = 0.15f;
    if (partnerMode != 0)
    {
        moveDir.Copy(model.matrix.GetTranslation());
        temp.Copy(moveDir);
        temp.w = -40.0f;
        temp.RotateZ(model.heading);
        moveDir.Copy(temp + moveDir);
        selfPos.Copy(moveDir);
        selfPos.z += 30.0f;
        selfPos.Fill(0.0f);
        model.matrix.SetTranslation(selfPos);
    }
    if (DAT_104bfdf0 != 0 || DAT_104c0135 != 0 || DAT_104bfe1c != 0)
    {
        model.m_0294 = 1;
        if (DAT_10daaae8[index].m_00a8 != 0)
            partnerModel.m_0294 = 1;
        DAT_1020b430[index].bs001.cutsceneHeld = 1;
        return 0;
    }
    if (DAT_1020b430[index].bs001.cutsceneHeld != 0)
    {
        model.m_0294 = 0;
        DAT_1020b430[index].bs001.cutsceneHeld = 0;
        if (DAT_10daaae8[index].m_00a8 != 0)
            partnerModel.m_0294 = 0;
    }
    if (DAT_102550e0[index] == 0)
    {
        DAT_1020b430[index + 1].bs001Aux.lastHitLevel = 0;
        if (DAT_1020b430[index + 1].bs001Aux.m_0004 > 0)
            ;
        if (DAT_1020b430[index].bs001.hurtLevel > 0)
            DAT_1020b430[index].bs001.hurtLevel--;
    }
    else
    {
        int state = DAT_1020b430[index].bs001.state;
        if (state == BS001_STAGGER || state == BS001_DOWN || state == BS001_HURT_STAGGER)
        {
            DAT_102550e0[index] = 0;
            model.PlayMotion(BS001_MOTION_HIT, 1, 5, 1.0f);
            if (DAT_10daaae8[index].m_00a8 != 0)
                partnerModel.PlayMotion(BS001_MOTION_HIT, 1, 5, 1.0f);
            DAT_1020b430[index + 1].bs001Aux.lastHitLevel = DAT_102550e0[index];
        }
        else if ((int)DAT_102550e0[index] <= DAT_1020b430[index + 1].bs001Aux.lastHitLevel)
            DAT_1020b430[index + 1].bs001Aux.lastHitLevel = DAT_102550e0[index];
        else
        {
            DAT_1020b430[index].bs001.hurtLevel += 10;
            if (DAT_10c635f8 == 1)
            {
                unsigned char value = (unsigned char)((11 - DAT_1020b430[index].bs001.hurtLevel) * 4);
                unsigned int level = 0x78;
                if (value > 0x77)
                    level = value;
                DAT_102550e0[index] = (unsigned char)(level / 10);
            }
            else if (DAT_10c635f8 == 0)
            {
                unsigned char value = (unsigned char)(DAT_1020b430[index].bs001.hurtLevel * -10 + 0x2c);
                unsigned int level = 0x1e;
                if (value >= 0x1e)
                    level = value;
                DAT_102550e0[index] = (unsigned char)(level / 10);
            }
            else if (DAT_10c635f8 == 2)
            {
                unsigned int level = (unsigned char)(DAT_1020b430[index].bs001.hurtLevel * -0x14 + 0x2c);
                DAT_102550e0[index] = (unsigned char)(level / 10);
            }
            if (DAT_10daaae8[index].m_00a8 == 0)
            {
                if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
                {
                    int pain = FUN_1013f880();
                    FUN_100b9bb0(index, pain % BS001_VOICE_PAIN_COUNT + BS001_VOICE_PAIN_FIRST, 0, 0);
                }
                DAT_1020b430[index + 1].bs001Aux.lastHitLevel = DAT_102550e0[index];
            }
            else
            {
                Playback *playback = model.motion;
                if (playback->blending != 0)
                    partnerModel.PlayMotion(playback->nextLayer, 1, 2, 1.0f);
                else
                    partnerModel.PlayMotion(playback->layer, 1, 0, 1.0f);
                partnerModel.motion->m_0004 = (float)model.motionFrame;
                partnerModel.motion->layer = model.motion->layer;
                DAT_1020b430[index + 1].bs001Aux.lastHitLevel = DAT_102550e0[index];
            }
        }
    }

    moveDir.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    selfPos.Copy(model.matrix.GetTranslation());
    (selfPos - moveDir).Length2D();
    float distance = (selfPos - moveDir).Length();
    if (DAT_1020b430[index + 1].bs001Aux.partnerCooldown != 0)
        DAT_1020b430[index + 1].bs001Aux.partnerCooldown--;
    if (DAT_1020b430[index + 1].bs001Aux.knockActive != 0)
    {
        partnerModel.mode = 0;
        DAT_1020b430[index + 1].bs001Aux.knockZ -= 200;
        if (DAT_1020b430[index + 1].bs001Aux.knockZ < -0xdac)
            DAT_1020b430[index + 1].bs001Aux.knockZ = -0xdac;
        Vec4 knock;
        temp.Copy(partnerModel.matrix.GetTranslation());
        knock.x = (float)DAT_1020b430[index + 1].bs001Aux.knockX / 100.0f;
        knock.y = (float)DAT_1020b430[index + 1].bs001Aux.knockY / 100.0f;
        knock.z = (float)DAT_1020b430[index + 1].bs001Aux.knockZ / 100.0f;
        if (temp.z + knock.z < 10.0f && DAT_1020b430[index + 2].bs001Fade.fadeIn == 0)
        {
            Vec4 bounce;
            bounce.z = knock.z;
            if (partnerModel.IsClearOfMapLimited(&bounce) == 0)
            {
                DAT_1020b430[index + 1].bs001Aux.knockZ = (int)((float)DAT_1020b430[index + 1].bs001Aux.knockZ);
                knock.z = knock.z * -0.5f;
                DAT_1020b430[index + 1].bs001Aux.knockX = (int)((float)DAT_1020b430[index + 1].bs001Aux.knockX);
                DAT_1020b430[index + 1].bs001Aux.knockY = (int)((float)DAT_1020b430[index + 1].bs001Aux.knockY);
                if (knock.z < 4.0f)
                {
                    DAT_1020b430[index + 1].bs001Aux.knockActive = 0;
                    partnerModel.mode = 1;
                    knock.z = 0.0f;
                }
            }
        }
        if (partnerModel.IsClearOfMapLimited(&knock) == 0)
        {
            knock.x = knock.x * -1.0f;
            DAT_1020b430[index + 1].bs001Aux.knockY = -DAT_1020b430[index + 1].bs001Aux.knockY;
            knock.y = knock.y * -1.0f;
            DAT_1020b430[index + 1].bs001Aux.knockX = -DAT_1020b430[index + 1].bs001Aux.knockX;
            while (partnerModel.IsClearOfMapLimited(&knock) == 0)
            {
                Vec4 away;
                away.Copy(partnerModel.matrix.GetTranslation() * -1.0f);
                away.Copy(away.Normalized3());
                away.Copy(away * 15.0f);
                knock.x = away.x + knock.x;
                knock.y = away.y + knock.y;
                if ((knock + temp).Length2D() < 100.0f)
                    break;
            }
        }
        temp.Copy(knock + partnerModel.matrix.GetTranslation());
        partnerModel.SetPosition(temp.x, temp.y, temp.z);
    }

    if (DAT_10210d30[index] != 0)
    {
        // The defeat scene: the boss and its pieces fly apart while the camera swings round.
        int roomFlagWord = DAT_10c63ffc * 2 + 1000;
        DAT_10db6ee8[PLAYER_SLOT].shadowHidden = 0;
        DAT_10daaae8[index].stats[3] = 0;
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
        DAT_104c0354 = 2;
        DAT_10daaae8[index].m_0141 = 1;
        unsigned int roomFlags = DAT_10874848.TestScriptFlag(roomFlagWord);
        DAT_10874848.SetScriptFlag(roomFlagWord, roomFlags | (1 << ((index - 0x3a) & 0x1f)));
        if (DAT_1020b430[index].bs001.defeatTimer < 0x1e)
        {
            if (DAT_1020b430[index].bs001.defeatTimer % 10 == 0 || FUN_1013f880() % 100 == 0)
            {
                Vec4 smokePos = model.matrix.GetTranslation();
                if ((FUN_1013f880() & 1) == 0)
                    FUN_1001d8b0(index, BS001_EFFECT_DEFEAT_SMOKE_A, smokePos, -1, -1, 0, 0, 0, 0, 0);
                else
                    FUN_1001d8b0(index, BS001_EFFECT_DEFEAT_SMOKE_B, smokePos, -1, -1, 0, 0, 0, 0, 0);
            }
        }
        else if (DAT_1020b430[index].bs001.defeatTimer == BS001_DEFEAT_DUMMY_FRAME)
        {
            FUN_1013fb10(3, 200, 30);
            FUN_100b9bb0(index, BS001_VOICE_DEFEATED, 1, 0);
        }
        if (DAT_1020b430[index].bs001.defeatTimer == BS001_DEFEAT_FILTER_FRAME)
        {
            FUN_1000db30(0, 0, 0);
            FUN_1000db30(0, 100, 20);
        }
        else if (DAT_1020b430[index].bs001.defeatTimer == BS001_DEFEAT_FINISH_FRAME)
        {
            FUN_1013fb10(3, 0, 10);
            DAT_10874848.MarkKilled(DAT_10daaae8[index].routeRecord);
            model.SetPosition(BS001_HIDDEN_X, 0.0f, 0.0f);
            DAT_10db6ee8[index + 1].SetPosition(BS001_HIDDEN_X, 0.0f, 0.0f);
            for (int part = 0; part < BS001_PART_COUNT; part++)
                DAT_10db6ee8[BS001_PART_FIRST + part].SetPosition(BS001_HIDDEN_X, 0.0f, 0.0f);
            DAT_104c00c8 = 0;
            DAT_104c0354 = 0;
            DAT_11018734 = 1;
        }
        AnimModel *parts = &DAT_10db6ee8[BS001_PART_FIRST];
        if (DAT_1020b430[index].bs001.hitRecover == 0)
        {
            FUN_10011c60();
            model.PlayMotion(BS001_MOTION_SHOUT + 3, 1, 5, 1.0f);
            DAT_1020b430[index].bs001.motion = 0xd;
            if (DAT_10daaae8[index].m_00a8 != 0)
                partnerModel.PlayMotion(0xd, 1, 5, 1.0f);
            FUN_10065b10(index, BS001_SOUND_DEFEAT_START, 1, 0);
            if (DAT_104bfe78 == 0)
                DAT_1020b430[index].bs001.hitRecover = 1;
            parts[0].heading = model.heading;
            parts[0].targetHeading = model.heading;
            parts[2].heading = DAT_10db6ee8[index + 1].heading;
            parts[2].targetHeading = DAT_10db6ee8[index + 1].heading;
            parts[1].heading = model.targetHeading;
            parts[1].targetHeading = model.targetHeading;
            parts[3].heading = DAT_10db6ee8[index + 1].targetHeading;
            parts[3].targetHeading = DAT_10db6ee8[index + 1].targetHeading;
            DAT_1020b430[index].bs001.defeatTimer = BS001_DEFEAT_START_VALUE;
            if (DAT_10daaae8[index].m_00a8 == 0)
            {
                parts[2].PlayMotion(0xf, 1, 0, 1.0f);
                parts[3].PlayMotion(0xf, 1, 0, 1.0f);
            }
            DAT_10da9150.Copy(DAT_118e69e8);
            DAT_10868818.Copy(DAT_118e6cb8);
            model.root->M_101946b0(0, 0xffffffff);
            DAT_10db6ee8[index + 1].root->M_101946b0(0, 0xffffffff);
            for (int part = 0; part < BS001_PART_COUNT; part++)
                parts[part].root->M_101946b0(0, 0xffffffff);
            model.alpha = 1.0f;
            DAT_10db6ee8[index + 1].alpha = 1.0f;
            DAT_104c00c8 = 1;
            DAT_10453008[index] = -10001;
            for (int part = 0; part < BS001_PART_COUNT; part++)
                parts[part].alpha = 0.0f;
        }
        int timer = DAT_1020b430[index].bs001.defeatTimer;
        if (timer > 0)
        {
            float shown;
            if (timer < BS001_DEFEAT_FADE_FRAMES)
            {
                shown = 1.0f - (float)timer / 90.0f;
                model.alpha = shown;
            }
            else
            {
                model.m_0294 = 1;
                model.motionFrame = parts[0].motionFrame;
                shown = 0.0f;
                model.alpha = 0.0f;
                DAT_10db6ee8[index + 1].m_0294 = 1;
                DAT_10db6ee8[index + 1].motionFrame = parts[2].motionFrame;
            }
            DAT_10db6ee8[index + 1].alpha = shown;
            if (timer < BS001_DEFEAT_FLASH_FRAMES)
            {
                float rise = (float)(BS001_DEFEAT_FLASH_FRAMES - timer);
                parts[0].scale.x = 1.0f;
                parts[0].scale.y = 0.0f;
                parts[0].scale.z = 0.0f;
                parts[0].scale.w = rise / 1830.0f + parts[0].scale.w;
            }
            else if (timer < BS001_DEFEAT_FADE_FRAMES)
            {
                parts[0].scale.x = 1.0f;
                parts[0].scale.y = 0.0f;
                parts[0].scale.z = 0.0f;
                parts[0].scale.w = 1.0f;
            }
            else if (timer < BS001_DEFEAT_END_FRAME)
            {
                if (timer == BS001_DEFEAT_FADE_FRAMES)
                {
                    FUN_10065b10(index, BS001_SOUND_DEFEAT_BLAST, 1, 0);
                    if (DAT_10c636c8 != 0)
                    {
                        Script_ValueCommand(0x7549, 0x4b);
                        if (DAT_10c6356c != 0)
                        {
                            DAT_10c6356c = 100000;
                            DAT_10c63588 = 7;
                            Script_ValueCommand(0x753f, 0x4b);
                        }
                    }
                }
                float remaining = (float)(BS001_DEFEAT_END_FRAME - timer);
                float elapsed = (float)(timer - BS001_DEFEAT_FADE_FRAMES);
                parts[0].scale.x = 1.0f;
                parts[0].scale.y = remaining * remaining * 0.004213826f + parts[0].scale.y;
                parts[0].scale.z = parts[0].scale.z - elapsed * elapsed * 0.009481108f;
                parts[0].scale.w = parts[0].scale.w - elapsed / 3081.0f;
                if (parts[0].scale.w < 0.0f)
                    parts[0].scale.w = 0.0f;
            }
            else
                parts[0].scale.Fill(0.0f);
        }
        timer = DAT_1020b430[index].bs001.defeatTimer;
        if (timer > BS001_DEFEAT_SPLIT_FIRST - 1)
        {
            if (timer < BS001_DEFEAT_BLAST_FRAME)
            {
                float since = (float)(timer - BS001_DEFEAT_SPLIT_FIRST);
                parts[1].scale.z = 0.0f;
                parts[1].scale.x = (float)(BS001_DEFEAT_BLAST_FRAME - timer) * 0.000152207f + parts[1].scale.x;
                parts[1].scale.y = since / 8.0f;
                parts[1].scale.w = since / 144.0f + 0.5f;
                if (parts[1].scale.x < 0.0f)
                    parts[1].scale.x = 0.0f;
            }
            else if (timer < BS001_DEFEAT_SCENE_END)
            {
                if (timer == BS001_DEFEAT_BLAST_FRAME)
                    parts[1].scale.x = 0.3f;
                float since = (float)(timer - BS001_DEFEAT_BLAST_FRAME);
                float remaining = (float)(BS001_DEFEAT_SCENE_END - timer);
                parts[1].scale.x = parts[1].scale.x - since * 8.591065e-05f;
                parts[1].scale.y = remaining * 0.08591065f + parts[1].scale.y;
                parts[1].scale.z = since * 0.128866f + parts[1].scale.z;
                parts[1].scale.w = (remaining * remaining) / 9216.0f;
                if (parts[1].scale.x < 0.0f)
                    parts[1].scale.x = 0.0f;
                float spin = (float)(timer - 0x60) * 5e-05f;
                parts[1].heading -= spin;
                parts[3].heading -= spin;
                parts[1].targetHeading = parts[1].heading;
                parts[3].targetHeading = parts[3].heading;
            }
            else
                parts[1].scale.Fill(0.0f);
        }
        for (int part = 0; part < BS001_PART_COUNT; part++)
            parts[part].m_0294 = 1;
        parts[0].motionFrame = model.motionFrame;
        parts[1].motionFrame = model.motionFrame;
        parts[2].motionFrame = DAT_10db6ee8[index + 1].motionFrame;
        parts[3].motionFrame = DAT_10db6ee8[index + 1].motionFrame;
        parts[0].SetPosition(model.matrix._41, model.matrix._42, model.matrix._43);
        parts[1].SetPosition(model.matrix._41, model.matrix._42, model.matrix._43);
        parts[2].SetPosition(DAT_10db6ee8[index + 1].matrix._41, DAT_10db6ee8[index + 1].matrix._42,
                             DAT_10db6ee8[index + 1].matrix._43);
        parts[3].SetPosition(DAT_10db6ee8[index + 1].matrix._41, DAT_10db6ee8[index + 1].matrix._42,
                             DAT_10db6ee8[index + 1].matrix._43);
        parts[1].alpha = parts[1].scale.w;
        parts[3].alpha = parts[1].scale.w;
        parts[0].alpha = parts[0].scale.w;
        parts[2].alpha = parts[0].scale.w;
        DAT_1020b430[index].bs001.defeatTimer++;
        return 1;
    }

    DAT_1021f9c0[index] = 0;
    DAT_1021f9c0[index + 1] = 0;
    DAT_10daaae8[index].m_0141 = 0;
    if (DAT_1020b430[index].bs001.initialised == 0)
    {
        AnimModel *parts = &DAT_10db6ee8[BS001_PART_FIRST];
        parts[0].LoadFile(FUN_1013f8a0("bs_001en.it3", 0));
        parts[1].LoadFile(FUN_1013f8a0("bs_001pa.it3", 0));
        parts[2].LoadFile(FUN_1013f8a0("bs_001be.it3", 0));
        parts[3].LoadFile(FUN_1013f8a0("bs_001bp.it3", 0));
        for (int part = 0; part < BS001_PART_COUNT; part++)
        {
            parts[part].LoadModel(FUN_1013f8a0("bs_001m.it3", 0));
            parts[part].mode = 0;
            parts[part].scale.Fill(0.0f);
            HideModel(parts[part]);
        }
        DAT_1020b430[index].bs001.initialised = 1;
        DAT_10836c70.ResetFrameTimer();
        Node *root = model.root;
        DAT_1021bdf8[index][BS001_NODE_RIGHT_UPPER_ARM] = root->Find("Bip01 R UpperArm", 0);
        DAT_1021bdf8[index][BS001_NODE_LEFT_UPPER_ARM] = root->Find("Bip01 L UpperArm", 0);
        DAT_1021bdf8[index][BS001_NODE_LEFT_HAND] = root->Find("Bip01 L Hand", 0);
        DAT_1021bdf8[index][BS001_NODE_LEFT_FINGER] = root->Find("Bip01 L Finger0", 0);
        DAT_1021bdf8[index][BS001_NODE_RIGHT_HAND] = root->Find("Bip01 R Hand", 0);
        DAT_1042d918[BS001_BONE_LEFT_TOE] = (int)root->Find("Bip01 L Toe0", 0);
        DAT_1042d918[BS001_BONE_RIGHT_TOE] = (int)root->Find("Bip01 R Toe0", 0);
        DAT_1042d918[BS001_BONE_LEFT_FOOT] = (int)root->Find("Bip01 L Foot", 0);
        DAT_1042d918[BS001_BONE_RIGHT_FOOT] = (int)root->Find("Bip01 R Foot", 0);
        DAT_1042d918[BS001_BONE_HEAD] = (int)root->Find("Bip01 Head", 0);
        DAT_1042d918[BS001_BONE_FOOTSTEPS] = (int)root->Find("Bip01 Footsteps", 0);
        DAT_1042d918[BS001_BONE_SPINE] = (int)root->Find("Bip01 Spine", 0);
        DAT_1020b430[index].bs001.state = BS001_REST;
        DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
        model.PlayMotion(BS001_MOTION_IDLE, 1, 5, 1.0f);
        partnerModel.PlayMotion(BS001_MOTION_IDLE, 1, 5, 1.0f);
        DAT_10da9150.Copy(DAT_118e69e8);
        DAT_10868818.Copy(DAT_118e6cb8);
        DAT_10802c08[index][0].Copy(DAT_10da9150);
        DAT_10255160[index][1] = 0.0f;
        DAT_10daaae8[index + 2].kind = -1;
    }
    DAT_10255a88[index] = 0;
    DAT_10210d30[index] = 0;
    DAT_1020b430[index].bs001.idle = 0;
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].bs001.state = BS001_REST;
        DAT_1020b430[index].bs001.finished = 1;
    }
    if (DAT_1020b430[index].bs001.hitRecover < 0)
        DAT_1020b430[index].bs001.hitRecover = 0;
    else if (DAT_1020b430[index].bs001.hitRecover > 10)
        DAT_1020b430[index].bs001.hitRecover = 0;
    else if (DAT_1020b430[index].bs001.hitRecover != 0 && DAT_1020b430[index].bs001.hitRecover < 10)
        DAT_1020b430[index].bs001.hitRecover--;
    if (distance < 10000.0f)
        DAT_1020344c = distance * 0.0003f + 0.95f;
    DAT_1101dddc = 125.0f;
    if (DAT_10daaae8[index].m_00a8 != 0)
        DAT_10daaae8[index].held = 1;
    if (DAT_1020b430[index].bs001.phase == 1)
    {
        DAT_1020b430[index].bs001.state = BS001_HURT_STAGGER;
        DAT_1020b430[index].bs001.frame = 0;
        DAT_1020b430[index].bs001.phase = 2;
    }

    int hit = DAT_102550e0[index];
    if (hit == 0)
    {
        Bs001Aux &next = DAT_1020b430[index + 1].bs001Aux;
        switch (DAT_1020b430[index].bs001.state)
        {
        case BS001_REST:
            DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame > 0x3c && FUN_1013f880() % 0x32 < 5)
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
            if (syncFlag != 0)
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
            break;
        case BS001_STAND_TURN:
            DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
            if (DAT_1020b430[index].bs001.frame == 0 && (FUN_1013f880() & 1) != 0)
                FUN_100b9bb0(index, BS001_VOICE_STAND, 1, 1);
            if (DAT_1020b430[index].bs001.frame % 0x28 == 0x14)
                FUN_1001d8b0(index, BS001_EFFECT_BLOW, selfPos, -1, -1, 0, 0, 0, 0, 0);
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame > 0x3c && DAT_1020b430[index].bs001.frame % 0x50 == 0xf && FUN_1013f880() % 0x32 < 0x19)
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
            if (syncFlag == 2)
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
            break;
        case BS001_WALK:
        case BS001_RUN:
        {
            moveDir = moveDir - selfPos;
            moveDir = Vec3_Normalized(&moveDir);
            if (DAT_1020b430[index].bs001.state == BS001_WALK && DAT_10daaae8[index].held == 0)
                DAT_1020b430[index].bs001.state = BS001_RUN;
            else
                moveDir = moveDir * (DAT_1020b430[index].bs001.state == BS001_WALK ? 5.0f : 8.0f);
            FaceDirection(model, moveDir);
            if (DAT_104bfd08 == 0)
                PushForwards(model, moveDir);
            DAT_1020b430[index].bs001.motion = DAT_1020b430[index].bs001.state == BS001_WALK ? BS001_MOTION_WALK : BS001_MOTION_RUN;
            if (DAT_1020b430[index].bs001.frame == 0)
                restartMotion = 1;
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame > 0x3c)
                if (FUN_1013f880() % 100 < (DAT_1020b430[index].bs001.state == BS001_WALK ? 10 : 0x14))
                    DAT_1020b430[index].bs001.state = BS001_CHOOSE;
            if (syncFlag == 2)
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
            break;
        }
        case BS001_LUNGE:
        {
            Vec4 lunge;
            moveDir = moveDir - selfPos;
            moveDir = Vec3_Normalized(&moveDir);
            FaceDirection(model, moveDir);
            lunge = (lunge + moveDir * 5.0f).Normalized3() * (5.0f * 1.8f);
            if (DAT_1020b430[index].bs001.frame < 10)
                lunge = lunge * 0.05f;
            DAT_1020b430[index].bs001.motion = BS001_MOTION_STOMP;
            if (DAT_104bfd08 == 0)
                model.Push(lunge.x, lunge.y, lunge.z);
            if (DAT_1020b430[index].bs001.frame % 0x28 > 3 && DAT_1020b430[index].bs001.frame % 0x28 < 0x25)
            {
                EffectOptions options;
                options.flags |= 2;
                BoneEffect(index, BS001_EFFECT_STOMP_R, lunge, DAT_1021bdf8[index][BS001_NODE_RIGHT_UPPER_ARM], &options);
                BoneEffect(index, BS001_EFFECT_STOMP_L, lunge, DAT_1021bdf8[index][BS001_NODE_LEFT_UPPER_ARM], &options);
                DAT_1021f6c0[index] = 1;
            }
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame % 0x28 == 0 && FUN_1013f880() % (DAT_1020b430[index].bs001.frame / 0x28 + 1) > 0)
            {
                DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
                DAT_1020b430[index].bs001.frame = 0;
                DAT_1020b430[index].bs001.finished = 1;
            }
            if (DAT_1020b430[index].bs001.frame % 0x28 % 7 == 0 && DAT_1020b430[index].bs001.frame % 0x28 > 5)
                FUN_10065b10(index, BS001_SOUND_STOMP, 1, 0);
            break;
        }
        case BS001_SHOUT:
        {
            if (DAT_1020b430[index].bs001.frame == 0)
            {
                restartMotion = 1;
                EffectOptions options;
                options.owner = index;
                for (int ring = 0; ring < 3; ring++)
                    FUN_1001d8b0(index, BS001_EFFECT_SHOUT_RING, selfPos, -1, -1, 0, 0, 0, 0, &options);
                moveDir = moveDir - selfPos;
                moveDir = Vec3_Normalized(&moveDir);
                FaceDirection(model, moveDir);
            }
            DAT_1020b430[index].bs001.motion = BS001_MOTION_SHOUT;
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame == 10)
            {
                Vec4 spawn = model.matrix.GetTranslation();
                spawn.z += 50.0f;
                int rays = hardMode * 2 + 8;
                for (int ray = 0; ray < rays; ray++)
                {
                    Matrix4 turn = RotationZ((float)ray * (2.0f / (float)rays) * BS001_PI + model.heading);
                    Matrix4 placement = turn;
                    placement.SetTranslation(spawn);
                    FUN_1001d8b0(index, ray == 0 ? BS001_EFFECT_SHOUT_SPARK : BS001_EFFECT_SHOUT_SPARK_OUTER, Vec4(),
                                 -1, -1, 0, 0, 0, &placement, 0);
                }
                FUN_10065b10(index, BS001_SOUND_SHOUT_RING, 1, 0);
            }
            if (DAT_1020b430[index].bs001.frame == 0x23)
            {
                model.PlayMotion(BS001_MOTION_IDLE, 1, 5, 1.0f);
                DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
                if (DAT_10daaae8[index].m_00a8 != 0)
                    partnerModel.PlayMotion(BS001_MOTION_IDLE, 1, 5, 1.0f);
                DAT_1020b430[index].bs001.state = BS001_REST;
                DAT_1020b430[index].bs001.frame = 0;
                DAT_1020b430[index].bs001.finished = 1;
            }
            break;
        }
        case BS001_STAGGER:
            if (DAT_1020b430[index].bs001.frame == 0)
            {
                restartMotion = 1;
                DAT_1020b430[index + 1].bs001Aux.lastHitLevel = 2;
            }
            DAT_1020b430[index].bs001.motion = BS001_MOTION_HURT;
            if (DAT_1020b430[index].bs001.frame > 0xd2)
            {
                DAT_1020b430[index].bs001.state = BS001_DOWN;
                DAT_1020b430[index].bs001.frame = 0;
            }
            DAT_1020b430[index].bs001.frame++;
            break;
        case BS001_DOWN:
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame == 1)
            {
                DAT_1020b430[index].bs001.motion = BS001_MOTION_DOWN;
                restartMotion = 1;
            }
            else if (DAT_1020b430[index].bs001.frame > 9)
            {
                if (DAT_1020b430[index].bs001.frame < 0x1e)
                    model.SetPosition(model.matrix._41, model.matrix._42, model.matrix._43 + 50.0f);
                else
                {
                    DAT_1020b430[index].bs001.state = BS001_RISE;
                    DAT_1020b430[index].bs001.frame = 0;
                    DAT_10db6ee8[index + 1].alpha = 1.0f;
                    DAT_104c00c8 = 1;
                    FUN_100b9bb0(index, 0x1e5, 0, 0);
                    DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
                }
            }
            break;
        case BS001_RISE:
            DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
            if (DAT_1020b430[index].bs001.frame == 1)
            {
                restartMotion = 1;
                DAT_1020b430[index].bs001.homeX = (int)model.matrix._41;
                DAT_1020b430[index].bs001.homeY = (int)model.matrix._42;
                DAT_1020b430[index].bs001.homeZ = (int)model.matrix._43;
                model.SetPosition(-60000.0f, 0.0f, 0.0f);
                DAT_1020b430[index].bs001.frame++;
            }
            else
            {
                DAT_10daaae8[index].m_0141 = 1;
                if (DAT_1020b430[index].bs001.frame == 0xdc)
                {
                    model.SetPosition((float)DAT_1020b430[index].bs001.homeX, (float)DAT_1020b430[index].bs001.homeY, (float)DAT_1020b430[index].bs001.homeZ);
                    DAT_1020b430[index].bs001.state = BS001_REVIVE;
                    DAT_1020b430[index].bs001.frame = 0;
                    FUN_100b9bb0(index, BS001_VOICE_RISE, 0, 1);
                }
                DAT_1020b430[index].bs001.frame++;
            }
            break;
        case BS001_REVIVE:
            DAT_1020b430[index].bs001.frame++;
            DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
            if (DAT_1020b430[index].bs001.frame == 0x1e)
            {
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
                DAT_1020b430[index].bs001.frame = 0;
                DAT_10daaae8[index].m_0141 = 0;
                FUN_10065b10(index, BS001_SOUND_SHOUT, 1, 0);
            }
            break;
        case BS001_KNOCKED:
            DAT_1020b430[index + 1].bs001Aux.attackCount = 0;
            DAT_10daaae8[index].m_0141 = 1;
            if (DAT_1020b430[index].bs001.frame == 0)
            {
                restartMotion = 1;
                FUN_100b9bb0(index, BS001_VOICE_FALL_FIRST + (FUN_1013f880() & 1), 1, 0);
            }
            DAT_1020b430[index].bs001.frame++;
            DAT_1020b430[index].bs001.motion = BS001_MOTION_FALL;
            if (model.motionFrame == 0x15d)
            {
                DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
                DAT_1020b430[index].bs001.state = BS001_REST;
                DAT_1020b430[index].bs001.frame = 0;
                DAT_1020b430[index].bs001.finished = 1;
                DAT_10daaae8[index].m_0141 = 0;
                DAT_10daaae8[index].held = 0;
            }
            break;
        case BS001_RETREAT:
            DAT_10daaae8[index].m_0141 = 1;
            if (DAT_1020b430[index].bs001.frame == 0)
            {
                restartMotion = 1;
                FUN_100b9bb0(index, BS001_VOICE_RETREAT, 0, 0);
            }
            DAT_1020b430[index].bs001.motion = BS001_MOTION_DOWN;
            DAT_1020b430[index].bs001.frame++;
            if (DAT_1020b430[index].bs001.frame == 5)
                DAT_10daaae8[index].m_00a8 = 1;
            else if (DAT_1020b430[index].bs001.frame == 10)
            {
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
                DAT_1020b430[index].bs001.frame = 0;
                DAT_10daaae8[index].m_0141 = 0;
                DAT_10daaae8[index].held = 1;
            }
            break;
        case BS001_PUNCH:
        case BS001_CLAW:
        case BS001_SMASH:
        case BS001_CHASE:
        case BS001_SMASH_FAST:
            // The remaining moves share the same shape: face the player, push towards him inside a frame window
            // of the motion, flash the hand effects and return to the dispatcher when the motion ends.
            if (DAT_1020b430[index].bs001.frame == 0)
            {
                restartMotion = 1;
                FUN_100b9bb0(index, DAT_1020b430[index].bs001.state == BS001_PUNCH ? BS001_VOICE_PUNCH : DAT_1020b430[index].bs001.state == BS001_CLAW ? BS001_VOICE_CLAW : BS001_VOICE_SMASH, 0, 1);
            }
            DAT_1020b430[index].bs001.motion = DAT_1020b430[index].bs001.state == BS001_PUNCH ? 5 : DAT_1020b430[index].bs001.state == BS001_CLAW ? 0x15 : BS001_MOTION_SMASH;
            moveDir = moveDir - selfPos;
            moveDir = Vec3_Normalized(&moveDir);
            FaceDirection(model, moveDir);
            if (model.motionFrame > BS001_SMASH_FRAME_FIRST && model.motionFrame < BS001_SMASH_FRAME_LAST &&
                DAT_104bfd08 == 0)
            {
                PushForwards(model, moveDir);
                DAT_1021f6c0[index] = 1;
            }
            if (DAT_1020b430[index].bs001.frame == 3)
                FUN_10065b10(index, BS001_SOUND_SMASH_HIT, 1, 0);
            DAT_1020b430[index].bs001.frame++;
            if (model.motionFrame == BS001_SMASH_END_FRAME)
            {
                DAT_1020b430[index].bs001.motion = BS001_MOTION_IDLE;
                DAT_1020b430[index].bs001.state = BS001_CHOOSE;
                DAT_1020b430[index].bs001.frame = 0;
                DAT_1020b430[index].bs001.finished = 1;
            }
            break;
        case BS001_CHOOSE:
        {
            DAT_1020b430[index + 1].bs001Aux.attackCount++;
            DAT_1020b430[index + 1].bs001Aux.streak++;
            DAT_1020b430[index].bs001.frame = 0;
            float gap = (selfPos - moveDir).Length2D();
            if (DAT_1020b430[index].bs001.phase == 0)
            {
                if (DAT_10daaae8[index].m_00a8 != 0)
                {
                    int last = DAT_1020b430[index + 1].bs001Aux.lastState;
                    if (last == BS001_REST || last == BS001_STAND_TURN || last == BS001_WALK)
                    {
                        int pick = FUN_1013f880() % 3;
                        DAT_1020b430[index].bs001.state = pick == 0 ? BS001_WALK : pick == 1 ? BS001_LUNGE : BS001_SHOUT;
                    }
                    else
                    {
                        DAT_1020b430[index].bs001.state = BS001_STAND_TURN;
                        DAT_1020b430[index + 1].bs001Aux.attackCount = 0;
                    }
                }
                else if (DAT_1020b430[index + 1].bs001Aux.attackCount < 7 || FUN_1013f880() % 100 >= (DAT_1020b430[index + 1].bs001Aux.attackCount - 4) * 5)
                {
                    unsigned int pick = FUN_1013f880() & 3;
                    if (DAT_1020b430[index + 1].bs001Aux.attackCount != 1 || gap <= 650.0f)
                    {
                        if (pick == 0)
                            DAT_1020b430[index].bs001.state = BS001_WALK;
                        else if (pick == 1)
                            DAT_1020b430[index].bs001.state = BS001_LUNGE;
                        else if (pick == 2 || (pick == 3 && gap >= 400.0f))
                            DAT_1020b430[index].bs001.state = BS001_SHOUT;
                        else
                            DAT_1020b430[index].bs001.state = BS001_SMASH;
                    }
                }
                else
                {
                    DAT_1020b430[index].bs001.state = BS001_REST;
                    DAT_1020b430[index + 1].bs001Aux.attackCount = 0;
                }
                if (DAT_1020b430[index + 1].bs001Aux.streak > 6 && FUN_1013f880() % 0x32 < DAT_1020b430[index + 1].bs001Aux.streak * 2 - 12)
                    DAT_1020b430[index].bs001.state = BS001_STAGGER;
            }
            else if (DAT_1020b430[index].bs001.phase == 1)
            {
                DAT_1020b430[index].bs001.state = BS001_HURT_STAGGER;
                DAT_1020b430[index].bs001.phase = 2;
                DAT_1020b430[index + 1].bs001Aux.streak = 0;
            }
            else if (DAT_10daaae8[index].m_00a8 == 0)
            {
                if (DAT_1020b430[index + 1].bs001Aux.attackCount < 7 || FUN_1013f880() % 100 >= (DAT_1020b430[index + 1].bs001Aux.attackCount - 5) * 5)
                {
                    unsigned int pick = FUN_1013f880() % 9;
                    if (pick == 0 && FUN_1013f880() % 5 != 0)
                        pick = (FUN_1013f880() & 7) + 1;
                    if (DAT_1020b430[index + 1].bs001Aux.attackCount != 1 || gap <= 450.0f)
                    {
                        if (pick == 1)
                            DAT_1020b430[index].bs001.state = BS001_LUNGE;
                        else if (pick == 2)
                            DAT_1020b430[index].bs001.state = BS001_SHOUT;
                        else if (pick == 3 || pick == 6)
                            DAT_1020b430[index].bs001.state = BS001_PUNCH;
                        else if (pick == 4 || pick == 7)
                            DAT_1020b430[index].bs001.state = BS001_CLAW;
                        else if (pick == 5 || pick == 8)
                            DAT_1020b430[index].bs001.state = BS001_SMASH;
                        else
                            DAT_1020b430[index].bs001.state = BS001_WALK;
                    }
                }
                else
                {
                    DAT_1020b430[index].bs001.state = BS001_REST;
                    DAT_1020b430[index + 1].bs001Aux.attackCount = 0;
                }
                if (DAT_1020b430[index + 1].bs001Aux.streak >= 4 && FUN_1013f880() % 0x32 < DAT_1020b430[index + 1].bs001Aux.streak * 4 - 0xc)
                    DAT_1020b430[index].bs001.state = BS001_STAGGER;
            }
            else if (DAT_1020b430[index + 1].bs001Aux.attackCount > 2)
            {
                DAT_1020b430[index].bs001.state = BS001_REST;
                DAT_1020b430[index + 1].bs001Aux.attackCount = 0;
            }
            else
            {
                int pick = FUN_1013f880() % 7;
                if (DAT_1020b430[index + 1].bs001Aux.attackCount != 1 || gap <= 650.0f)
                {
                    if (pick == 1)
                        DAT_1020b430[index].bs001.state = BS001_LUNGE;
                    else if (pick == 2)
                        DAT_1020b430[index].bs001.state = BS001_SHOUT;
                    else if (pick == 3 || pick == 5)
                        DAT_1020b430[index].bs001.state = BS001_PUNCH;
                    else if (pick == 4 || pick == 6)
                        DAT_1020b430[index].bs001.state = BS001_CLAW;
                    else
                        DAT_1020b430[index].bs001.state = BS001_RUN;
                }
            }
            if (DAT_10daaae8[index].m_00a8 == 0 && DAT_1020b430[index + 1].bs001Aux.partnerCooldown == 0)
            {
                if (DAT_1020b430[index + 1].bs001Aux.waitFrames == 0)
                {
                    DAT_1020b430[index].bs001.state = BS001_CHASE;
                    DAT_1020b430[index + 1].bs001Aux.streak = 0;
                }
                else
                    DAT_1020b430[index + 1].bs001Aux.waitFrames--;
            }
            break;
        }
        default:
            DAT_1020b430[index].bs001.state = BS001_CHOOSE;
        }
    }
    else
    {
        if (DAT_1020b430[index].bs001.state != BS001_STAGGER && DAT_1020b430[index].bs001.state != BS001_HURT_STAGGER)
            DAT_1020b430[index].bs001.state = BS001_CHOOSE;
        DAT_1042d9e8[index] = 0;
    }

    if (DAT_1020b430[index].bs001.state != BS001_CHOOSE)
        DAT_1020b430[index + 1].bs001Aux.lastState = DAT_1020b430[index].bs001.state;
    if (DAT_1020b430[index].bs001.motion >= 0)
    {
        model.PlayMotion(DAT_1020b430[index].bs001.motion, restartMotion, quickBlend == 0 ? 5 : 0, 1.0f);
        if (DAT_10daaae8[index].m_00a8 != 0)
            partnerModel.PlayMotion(DAT_1020b430[index].bs001.motion, restartMotion, quickBlend == 0 ? 4 : 0, 1.0f);
    }
    if (DAT_1020b430[index].bs001.motion != DAT_1020b430[index].bs001.shownMotion)
    {
        DAT_1020b430[index].bs001.shownMotion = DAT_1020b430[index].bs001.motion;
        if (DAT_1020b430[index].bs001.motion == 0 || DAT_1020b430[index].bs001.motion == 1 || DAT_1020b430[index].bs001.motion == 4)
            DAT_1020b430[index].bs001.motionFrames = -8;
        else if (DAT_1020b430[index].bs001.motion == BS001_MOTION_FALL)
            DAT_1020b430[index].bs001.motionFrames = 0;
    }
    DAT_1020b430[index].bs001.motionFrames++;

    Vec4 effectPos;
    effectPos.Fill(0.0f);
    Matrix4 boneMatrix;
    boneMatrix.SetIdentity();
    switch (DAT_1020b430[index].bs001.motion)
    {
    case 0:
    case 1:
    case 4:
    {
        int cadence = DAT_1020b430[index].bs001.motion == 0 ? 0xf : 10;
        int phaseFrames = DAT_1020b430[index].bs001.motionFrames;
        if (DAT_1020b430[index].bs001.motion == 1)
        {
            if (phaseFrames > 12)
                phaseFrames -= 12;
            else
                cadence = 10;
        }
        if (phaseFrames % cadence == 0)
        {
            Node *foot = Bs001BoneNode(phaseFrames % (cadence * 2) == 0 ? BS001_BONE_LEFT_TOE : BS001_BONE_RIGHT_TOE);
            if (foot != 0)
            {
                boneMatrix = foot->worldMatrix;
                FUN_1001d8b0(index, BS001_EFFECT_FOOTSTEP, effectPos, -1, -1, 0, 0, 0, &boneMatrix, 0);
                Vec4 stepPosition = foot->worldMatrix.GetTranslation();
                FUN_10054550(0, 0, &stepPosition);
            }
        }
        break;
    }
    case BS001_MOTION_FALL:
    {
        int frame = DAT_1020b430[index].bs001.motionFrames;
        Node *spine = Bs001BoneNode(BS001_BONE_FOOTSTEPS);
        if (frame >= 0x11 && frame <= 0x1b && (frame - 0x11) % 5 == 0)
        {
            FUN_10065b10(index, BS001_SOUND_STEP_FIRST, 1, 0);
            if (spine != 0)
            {
                boneMatrix = spine->worldMatrix;
                FUN_1001d8b0(index, BS001_EFFECT_FOOTSTEP, effectPos, -1, -1, 0, 0, 0, &boneMatrix, 0);
            }
        }
        else if (frame == 0x23 || frame == 0x4a)
        {
            FUN_10065b10(index, BS001_SOUND_STEP_SECOND, 1, 0);
            if (spine != 0)
            {
                boneMatrix = spine->worldMatrix;
                FUN_1001d8b0(index, BS001_EFFECT_FOOTSTEP, effectPos, -1, -1, 0, 0, 0, &boneMatrix, 0);
            }
        }
        break;
    }
    case BS001_MOTION_SMASH:
    {
        int frame = model.motionFrame;
        if (frame > 0x244 && (frame & 1) != 0 && frame < BS001_SMASH_FRAME_LAST && Bs001BoneNode(BS001_BONE_SPINE) != 0)
        {
            boneMatrix = Bs001BoneNode(BS001_BONE_SPINE)->worldMatrix;
            effectPos = boneMatrix.GetTranslation();
            effectPos.z -= 50.0f;
            FUN_1001d8b0(index, BS001_EFFECT_SMASH_DUST, effectPos, -1, -1, 0, 0, 0, &boneMatrix, 0);
        }
        if (frame == BS001_SMASH_FRAME_LAST)
        {
            FUN_10065b10(index, BS001_SOUND_SMASH_END, 1, 0);
            FUN_1016b230("52915");
            FUN_10055ee0();
            Matrix4 &placement = Bs001BoneNode(BS001_BONE_SPINE) != 0 ? Bs001BoneNode(BS001_BONE_SPINE)->worldMatrix
                                                                      : model.matrix;
            boneMatrix.SetTranslation(placement.GetTranslation());
            boneMatrix._43 = 0.0f;
            FUN_1001d8b0(index, BS001_EFFECT_SHOUT_CLOUD, effectPos, -1, -1, 0, 0, 0, &boneMatrix, 0);
            EffectOptions options;
            options.m_0004 = DAT_10daaae8[index].stats[3];
            options.m_000e = 5;
            FUN_1001d8b0(index, BS001_EFFECT_SMASH_FLASH, effectPos, -1, -1, 0, 0, 0, &boneMatrix, &options);
        }
        break;
    }
    }

    EffectOptions trailOptions;
    if (DAT_1020b430[index].bs001.idle == 0 && DAT_102550e0[index] == 0 && DAT_1021f6c0[index] != 0)
    {
        trailOptions.m_0004 = DAT_10daaae8[index].stats[3];
        trailOptions.flags |= 2;
        trailOptions.m_000e = 5;
        int trail = DAT_1020b430[index].bs001.motion == 4 || DAT_1020b430[index].bs001.motion == 0x14 ? BS001_EFFECT_TRAIL_SMASH : BS001_EFFECT_TRAIL_STANDARD;
        if (DAT_1020b430[index].bs001.motion != BS001_MOTION_FALL)
        {
            BoneEffect(index, trail, effectPos, DAT_1021bdf8[index][BS001_NODE_LEFT_HAND], &trailOptions);
            BoneEffect(index, trail, effectPos, DAT_1021bdf8[index][BS001_NODE_RIGHT_HAND], &trailOptions);
        }
        DAT_1021f6c0[index] = 0;
    }
    if (model.motion->blending == 0 && DAT_10daaae8[index].m_00a8 != 0)
    {
        int layer = model.motion->layer;
        if (partnerModel.motion->layer != layer)
        {
            partnerModel.motion->layer = layer;
            partnerModel.motion->m_0004 = model.motion->m_0004;
            partnerModel.motionFrame = model.motionFrame;
            partnerModel.motion->frame = model.motion->frame;
            if (DAT_1020b430[index + 1].bs001Aux.advancePartner != 0)
                partnerModel.motion->Advance();
        }
    }
    float target = WrapAngle(model.targetHeading);
    float current = WrapAngle(model.heading);
    float turn = WrapAngle(current - target);
    if (DAT_1020b430[index].bs001.state == 0x21b)
        turn = turn / 3.5f;
    else if (turn != 0.0f && DAT_1020b430[index].bs001.state != BS001_LUNGE)
        turn = turn / 2.5f;
    else
        turn = 0.0f;
    if (!(turn == 0.0f && DAT_1020b430[index].bs001.state == BS001_LUNGE))
        model.targetHeading = model.heading - turn;
    if (DAT_1020b430[index + 2].bs001Fade.fadeOut != 0)
        DAT_10daaae8[index].m_0141 = 1;
    return 0;
}
