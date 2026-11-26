// The BS_005 boss: the stone golem of the dodge game (map 0x132). Its slot (kind 0x3f) first runs the ending scene when
// the game is over (DAT_10210d30), otherwise the fight: the golem rests, picks an attack from how many of its rock
// pieces (slots 0x3d..0x4c, dodge_golem.h) are gone and how hurt it is, turns towards the player, and swings its two
// fists (the nodes Dummy01 and Dummy02) while effects follow their movement.

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
#include "bs005.h"
#include "bgm_track.h"
#include "dodge_golem.h"
#include "doppler_loops.h"
#include "game.h"
#include "game_main.h"
#include "hikari_button.h"
#include "hop_spit_enemy.h"
#include "last_position.h"
#include "route_flier.h"
#include "target_race.h"
#include "texture_load.h"
#include "title_screen.h"
#include "camera_vars.h"
#include "char_links.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "script_system.h"
#include "text_window.h"
#include "world.h"

#define BS005_PI 3.1415f
#define BS005_TWO_PI 6.283f
#define BS005_QUARTER_TURN 1.5708f
#define BS005_FIRST_PIECE 0x3d                  // the fists (0x3d, 0x3e) and rocks: slots 0x3d..0x4c
#define BS005_PIECE_COUNT 16
#define BS005_FIRST_ROCK 0x3f
#define BS005_ROCK_COUNT 14
#define BS005_PIECE_TARGET_WORD 2               // CharState::words[2] of a piece: the character slot it stands for
#define BS005_FIST_A_PIECE 0x3e                 // the piece of Dummy01
#define BS005_FIST_B_PIECE 0x3d                 // the piece of Dummy02
#define BS005_SCRIPT_FLAG_DIFFICULTY 0x1c7      // script flag holding the difficulty (0, 1 or 2)
#define BS005_ROOM_FLAG_BASE 1000               // + 2 * room: flag word of the ending
#define BS005_PEOPLE_FIRST 0x3a
#define BS005_DESTROYED_SLOT_FLAG 0x29a
#define BS005_OFFSTAGE_X (-250000.0f)
#define BS005_FAR_X (-200000.0f)
#define BS005_ENDING_MODEL_A 88                 // the two models of the ending (bs_005en.it3 and bs_005pa.it3)
#define BS005_ENDING_MODEL_B 89
#define BS005_ENDING_START (-30)
#define BS005_HIT_CAMERA_FLAG 4
#define BS005_EFFECT_OWNER_BASE 300

// States of the golem (Bs005State::state).
#define BS005_STATE_SET_UP 0
#define BS005_STATE_WAKE_PIECES 1
#define BS005_STATE_REST 2
#define BS005_STATE_SLAM 3                      // lunges at the player (hits on frames 0x88..0x92 and 0x99..0xa4)
#define BS005_STATE_BACKHAND 4
#define BS005_STATE_SWEEP 5
#define BS005_STATE_STOMP 6
#define BS005_STATE_ROCK_THROW 7
#define BS005_STATE_ROAR_WAVES 8
#define BS005_STATE_SHATTER 9                   // sends its rock pieces at the player
#define BS005_STATE_BEAM 10
#define BS005_STATE_LIFT 11                     // lifts the player and rains rock
#define BS005_STATE_TURN 12
#define BS005_STATE_CLOSE_ATTACK_A 13
#define BS005_STATE_CLOSE_ATTACK_B 14
#define BS005_STATE_CLOSE_STOMP 15
#define BS005_STATE_PICK 100

// Values of Bs005State::frames that mark the later parts of a state.
#define BS005_FRAMES_PART_TWO 1000
#define BS005_FRAMES_PART_THREE 2000
#define BS005_FRAMES_PART_FOUR 3000

#define BS005_MOTION_IDLE 2
#define BS005_MOTION_NONE (-1)

#define BS005_VOICE_COOLDOWN_SHORT 0x1e
#define BS005_VOICE_COOLDOWN_LONG 0x2d

static const Bs005StepSound sBs005StepSounds[] = {
    {0x88, 0x20e, 0},  {0x99, 0x20e, 0},  {0x4ad, 0x20e, 0}, {0x4bd, 0x20e, 0},
    {0xb7, 0x20f, 1},  {0xed, 0x20f, 1},  {0x4e0, 0x20f, 1}, {0x51a, 0x20f, 1}, {0x56a, 0x20f, 1}, {0x57d, 0x20f, 1},
    {0x5a4, 0x20f, 1}, {0x5cc, 0x20f, 1}, {0x590, 0x20f, 1}, {0x5b7, 0x20f, 1},
    {0x570, 0x210, 0}, {0x584, 0x210, 0}, {0x5ac, 0x210, 0}, {0x5d4, 0x210, 0}, {0x598, 0x210, 0}, {0x5c0, 0x210, 0},
    {0x216, 0x210, 0}, {0x243, 0x210, 0}, {0x270, 0x210, 0}, {0x22d, 0x210, 0}, {0x25a, 0x210, 0}, {0x200, 0x210, 0},
    {0x12d, 0x211, 1}, {0x18b, 0x211, 1}, {0x1f5, 0x211, 1}, {0x20c, 0x211, 1}, {0x23b, 0x211, 1}, {0x267, 0x211, 1},
    {0x224, 0x211, 1}, {0x251, 0x211, 1},
    {0x142, 0x21b, 0}, {0x19c, 0x21b, 0},
};

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Turns an angle into -pi..pi.
static float WrapAngle(float angle)
{
    while (angle > BS005_PI)
        angle -= BS005_TWO_PI;
    while (angle < -BS005_PI)
        angle += BS005_TWO_PI;
    return angle;
}

// The boss kinds share a voice arbitration: each says its line only when the neighbouring boss's counter for the same
// kind is clear. Returns the CharState word of the golem's own counter when `index` may speak now, else -1.
static int VoiceCounterWord(int index)
{
    int kind = DAT_10daaae8[index].kind;
    int word;
    if (kind == 100)
        word = 10;
    else if (kind == 0x46)
        word = 0xb;
    else if (kind == 0x69)
        word = 8;
    else if (kind == 0x2d)
        word = 1;
    else if (kind == BS005_KIND)
        word = 9;
    else if (kind == 0x43)
        word = 0xf;
    else
        return -1;
    int neighbourBusy;
    if (kind == BS005_KIND || kind == 0x43)
        neighbourBusy = DAT_1020b430[index].words[word];
    else if (kind == 0x2d)
        neighbourBusy = DAT_1020b430[index + 2].words[word];
    else if (kind == 0x69)
        neighbourBusy = DAT_1020b430[index - 1].words[word];
    else
        neighbourBusy = DAT_1020b430[index + 1].words[word];
    return neighbourBusy == 0 ? word : -1;
}

// Says voice line `sound` unless a neighbouring boss is speaking, then blocks the counter for `cooldown` frames.
static void SayLine(int index, int sound, int cooldown)
{
    int word = VoiceCounterWord(index);
    if (word >= 0)
    {
        FUN_10065b10(index, sound, 1, 1);
        DAT_1020b430[index].words[word] = cooldown;
    }
}

// Starts effect `effect` on the golem with `pose` as its placement.
static void StartPoseEffect(int index, int effect, Matrix4 &pose, float scale, int frame, const Matrix4 *owner)
{
    EffectOptions options;
    Vec4 position;
    options.m_0000 = 0;
    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
    options.m_0008 = scale;
    options.m_000e = 0;
    position.Fill(0.0f);
    FUN_1001d8b0(index, effect, position, -1, -1, owner, 0, frame, &pose, &options);
}

// A burst of effect 0x7a4 turned outward from `where`, sometimes jolted by up to 300 units (the debris of the golem's
// stomps); it starts when a roll of `rollRange` reaches `rollMin`.
static void StartDebrisBurst(int index, const Vec4 &where, int rollRange, int rollMin, float scale, int frame)
{
    Matrix4 pose;
    pose.SetIdentity();
    pose = RotationZ(FUN_10001f80(where.x, where.y));
    pose.SetTranslation(where);
    if (FUN_1013f880() % 3 == 1)
    {
        FUN_10055ee0();
        pose.m[12] += (float)(FUN_1013f880() % 600 - 300);
        pose.m[13] += (float)(FUN_1013f880() % 600 - 300);
        pose.m[14] = 0.0f;
    }
    pose.SetTranslation(where);
    pose.m[14] = 0.0f;
    if (FUN_1013f880() % rollRange >= rollMin)
        StartPoseEffect(index, 0x7a4, pose, scale, frame, 0);
}

// 0x100cc2a0 (Bs005_Update). One frame of the stone golem in slot `index`. Before the fight (DAT_10210d30) it runs
// the ending scene: the camera slides along the golem while the screen fades. In the fight the states of the golem
// run as described at the BS005_STATE_ constants, the motion and fist effects follow, and the golem turns towards
// the player by at most a limited angle per frame. Returns 0 (1 in the first frame of the ending scene).
int Bs005_Update(int index)
{
    if (DAT_104bfdf0 != 0)
        return 0;
    if (DAT_104bfe1c != 0)
        return 0;
    if (DAT_104c0135 != 0)
        return 0;
    AnimModel &model = DAT_10db6ee8[index];
    Vec4 playerPos;
    Vec4 ownPos;
    Vec4 toPlayer;
    Vec4 look;
    Vec4 away;
    Vec4 reach;
    int difficulty = DAT_10c635f8;
    int period = difficulty == 1 ? 60 : (difficulty == 2 ? 15 : 30);
    int motion = BS005_MOTION_NONE;
    int restart = 0;
    int refreshMotion = 1;
    int rightHit = 0;
    int leftHit = 0;
    int power = 0;
    int flameA = 0;
    int flameB = 0;
    float turnScale = 1.0f;
    float effectScale = 1.0f;
    float turnLimit = 0.0f;
    model.locked = 1;
    DAT_10255a88[index] = 0;
    DAT_104c0110 = index;
    DAT_1021f6c0[index] = 0;
    model.uniformScale = 0.15f;
    DAT_11b04510 = 1;
    playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    ownPos.Copy(model.matrix.GetTranslation());
    toPlayer = ownPos - playerPos;
    toPlayer.Length2D();
    toPlayer = ownPos - playerPos;
    toPlayer.Length();
    DAT_1101dddc = 150.0f;
    DAT_1020344c = 1.5f;
    if (DAT_10210d30[index] != 0)
    {
        if (DAT_1020b430[index].bs005.endingSetUp == 0)
        {
            for (int word = 0; word < 0x10; word++)
            {
                DAT_1020b430[index].words[word] = 0;
                if (word < 4)
                    DAT_10255160[index][word] = 0.0f;
                if (word < 6)
                    DAT_10868838[index][word].SetIdentity();
            }
            DAT_1020b430[index].bs005.endingSetUp = 1;
            DAT_10802bf8[index].position.Fill(0.0f);
            DAT_10802bf8[index].step.Fill(0.0f);
            if (DAT_10255f08 != 0)
            {
                Node *camera = DAT_10255f08->Find("CAboss", 0);
                if (camera != 0)
                    camera->ClearFlags(BS005_HIT_CAMERA_FLAG);
                camera = DAT_10255f08->Find("CAboss2", 0);
                if (camera != 0)
                    camera->ClearFlags(BS005_HIT_CAMERA_FLAG);
            }
            return 1;
        }
        AnimModel &endingA = DAT_10db6ee8[BS005_ENDING_MODEL_A];
        AnimModel &endingB = DAT_10db6ee8[BS005_ENDING_MODEL_B];
        int room = DAT_10c63ffc;
        DAT_104c0354 = 2;
        DAT_10daaae8[index].m_0141 = 1;
        DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] = 0;
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
        int roomFlags = DAT_10874848.TestScriptFlag(room * 2 + BS005_ROOM_FLAG_BASE);
        DAT_10874848.SetScriptFlag(room * 2 + BS005_ROOM_FLAG_BASE, roomFlags | (1 << (index - BS005_PEOPLE_FIRST)));
        int t = DAT_1020b430[index].bs005.endingTimer;
        if (t < 30)
        {
            if (t % 10 == 0 || FUN_1013f880() % 100 == 0)
            {
                Vec4 spot = model.matrix.GetTranslation();
                FUN_1001d8b0(index, FUN_1013f880() % 2 == 0 ? 0x321 : 0x322, spot, -1, -1, 0, 0, 0, 0, 0);
            }
        }
        else if (t == 30)
        {
            FUN_1013fb10(3, 200, 30);
            FUN_100b9bb0(index, 0x46, 1, 0);
        }
        if (DAT_1020b430[index].bs005.endingTimer == 0xb4)
        {
            FUN_1000db30(0, 0, 0);
            FUN_1000db30(0, 100, 20);
        }
        else if (DAT_1020b430[index].bs005.endingTimer == 0xc3)
        {
            FUN_1013fb10(3, 0, 10);
            DAT_10874848.MarkKilled(DAT_10daaae8[index].routeRecord);
            model.SetPosition(BS005_FAR_X, 0.0f, 0.0f);
            endingA.SetPosition(BS005_FAR_X, 0.0f, 0.0f);
            endingB.SetPosition(BS005_FAR_X, 0.0f, 0.0f);
            DAT_104c00c8 = 0;
            DAT_104c0354 = 0;
            DAT_11018734 = 1;
        }
        if (DAT_1020b430[index].bs005.endingMotionStarted == 0)
        {
            FUN_10011c60();
            model.PlayMotion(0x24, 1, 5, 1.0f);
            FUN_10065b10(index, 0x2fd, 1, 0);
            DAT_1020b430[index].bs005.cameraHidden = 0x24;
            if (DAT_104bfe78 == 0)
                DAT_1020b430[index].bs005.endingMotionStarted = 1;
            endingA.heading = model.targetHeading;
            endingB.targetHeading = model.targetHeading;
            endingA.targetHeading = model.heading;
            endingB.heading = model.heading;
            DAT_1020b430[index].bs005.endingTimer = BS005_ENDING_START;
            endingA.m_0000.Copy(model.matrix.GetTranslation());
            endingB.m_0000.Copy(model.matrix.GetTranslation());
            model.root->M_101946b0(0, 0xffffffff);
            endingA.root->M_101946b0(0, 0xffffffff);
            endingB.root->M_101946b0(0, 0xffffffff);
            model.alpha = 1.0f;
            DAT_104c00c8 = 1;
            DAT_10453008[index] = -10001;
            endingA.alpha = 0.0f;
            endingB.alpha = 0.0f;
        }
        int clock = DAT_1020b430[index].bs005.endingTimer;
        if (clock > 0)
        {
            if (clock < 90)
            {
                model.alpha = 1.0f - (float)clock / 90.0f;
                if (clock == 1)
                {
                    for (int person = BS005_PEOPLE_FIRST; person < BS005_PEOPLE_FIRST + 0x1c; person++)
                    {
                        if (DAT_10daaae8[person].kind == 0x43)
                        {
                            DAT_10255f08->Find("@prince", 0);
                            DAT_10daaae8[person].m_0098 = -1;
                            DAT_10daaae8[person].stats[STAT_BUSY] = 1;
                            DAT_10daaae8[person].m_0141 = 1;
                            DAT_10db6ee8[person].heading = DAT_10db6ee8[0].targetHeading;
                            DAT_10db6ee8[person].targetHeading = DAT_10db6ee8[0].targetHeading;
                            DAT_10db6ee8[person].SetPosition(BS005_OFFSTAGE_X, 0.0f, 0.0f);
                            break;
                        }
                    }
                    if (DAT_10c64f8c == 0)
                    {
                        DAT_104c012c = 0;
                        Bgm_Play(0, 0);
                    }
                }
            }
            else
            {
                model.m_0294 = 1;
                model.motionFrame = endingA.motionFrame;
                model.alpha = 0.0f;
            }
        }
        clock = DAT_1020b430[index].bs005.endingTimer;
        if (clock > 0)
        {
            if (clock < 0x3c)
                endingA.scale.Set(1.0f, 0.0f, 0.0f, endingA.scale.w + (float)(0x3c - clock) / 1830.0f);
            else if (clock < 90)
                endingA.scale.Set(1.0f, 0.0f, 0.0f, 1.0f);
            else if (clock < 0xa8)
            {
                if (clock == 90)
                    FUN_10065b10(index, 0x2fe, 1, 0);
                float fade = (float)(0xa8 - clock);
                float rise = (float)(clock - 90);
                endingA.scale.Set(1.0f, endingA.scale.y + fade * fade * 0.004213826f,
                                  endingA.scale.z - rise * rise * 0.009481108f, endingA.scale.w - rise / 3081.0f);
                if (endingA.scale.w < 0.0f)
                    DAT_10db6ee8[124].scale.w = 0.0f;
            }
            else
                endingA.scale.Fill(0.0f);
        }
        clock = DAT_1020b430[index].bs005.endingTimer;
        if (clock > 0x29)
        {
            if (clock < 0x72)
            {
                float step = (float)(clock - 0x2a);
                endingB.scale.z = 0.0f;
                endingB.scale.x = (float)(0x72 - clock) * 0.000152207f + endingB.scale.x;
                endingB.scale.y = step / 8.0f;
                endingB.scale.w = step / 144.0f + 0.5f;
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
        DAT_1020b430[index].bs005.endingTimer++;
        endingB.motionFrame = model.motionFrame;
        endingA.SetPosition(DAT_10db9240, DAT_10db9244, DAT_10db9248);
        endingB.SetPosition(DAT_10db9240, DAT_10db9244, DAT_10db9248);
        endingA.alpha = endingA.scale.w;
        endingB.alpha = endingB.scale.w;
        look.Fill(0.0f);
        look.y += 1425.0f;
        look.z -= 50.0f;
        look.RotateZ(-(Atan2(DAT_10868818.x - DAT_10da9150.x, DAT_10868818.y - DAT_10da9150.y) + BS005_PI));
        DAT_10edbd48.Copy(model.matrix.GetTranslation());
        DAT_10edbd48.z += 600.0f;
        DAT_10866d98.Copy(DAT_10edbd48 + look * 1.6f);
        if (DAT_1020b430[index].bs005.endingTimer < 0xa1)
        {
            float k = (float)(0xa0 - DAT_1020b430[index].bs005.endingTimer);
            k = (k + 1.0f) * k * 0.5f;
            k = k * k;
            float blend = 1.0f - k / 3.292411e+08f;
            DAT_10866d98.Copy(DAT_10da9150 + (DAT_10866d98 - DAT_10da9150) * blend);
            DAT_10edbd48.Copy(DAT_10868818 + (DAT_10edbd48 - DAT_10868818) * blend);
        }
        reach.Fill(0.0f);
        if (DAT_10866d98.Length2D() > 2325.0f && DAT_10255f08 != 0 &&
            DAT_10255f08->CastSegment("CA", 4, &DAT_10866d98, &DAT_10edbd48, &toPlayer, 0) != 0)
        {
            float gap = (DAT_10edbd48 - toPlayer).Length();
            gap = gap <= 60.0f ? gap * 0.75f : gap - 50.0f;
            away.Copy(DAT_10edbd48 - DAT_10866d98);
            away.Copy(away.Normalized3() * gap);
            DAT_10866d98.Copy(toPlayer + away);
            away.Copy(DAT_10866d98 - DAT_10868818);
            away.RotateZ(0.0f);
            DAT_10da9150.Copy(DAT_10868818 + away);
        }
        float distance = (DAT_10edbd48 - DAT_10866d98).Length();
        float &closest = DAT_10255160[index][0];
        if (distance <= closest)
            closest = distance;
        else
        {
            closest = (distance - closest) * 0.3f + closest;
            away.Copy(DAT_10edbd48 - DAT_10866d98);
            away.Copy(away.Normalized3() * closest);
            DAT_10866d98.Copy(DAT_10edbd48 - away);
        }
        return 1;
    }
    DAT_1021f9c0[index] = 0;
    if (DAT_10daaae8[index].held == 0)
        DAT_1020b430[index].bs005.calmFrames++;
    unsigned char level = DAT_102550e0[index];
    if (level == 0)
    {
        if (model.m_0294 != 0)
            model.m_0294 = 0;
    }
    else
    {
        if ((difficulty == 2 && DAT_10c66cc0 == 2 && level > 2) || level > 0x14)
        {
            if (DAT_1020b430[index].bs005.motionFrame < 0)
                model.PlayMotion(DAT_1020b430[index].bs005.motionLayer, 1, 3, 1.0f);
            else
            {
                model.PlayMotion(DAT_1020b430[index].bs005.motionLayer, 1, 0, 1.0f);
                model.motion->m_0004 = (float)DAT_1020b430[index].bs005.motionFrame;
                model.motion->frame = DAT_1020b430[index].bs005.motionFrame;
                model.motionFrame = DAT_1020b430[index].bs005.motionFrame;
                model.motion->Advance();
            }
            model.m_0294 = 1;
            if (difficulty == 0)
                DAT_102550e0[index] = 4;
            else
                DAT_102550e0[index] = difficulty == 1 ? 7 : 1;
            if (DAT_1020b430[index].bs005.attacking != 0)
                DAT_102550e0[index] = 1;
        }
        if (DAT_102550e0[index] == 1)
        {
            DAT_102550e0[index] = 0;
            model.m_0294 = 0;
        }
    }
    int frame = model.motionFrame;
    if (model.m_0294 == 0)
    {
        if (frame == 0x2b4)
            FUN_10065b10(index, 0x217, 1, 0);
        else if (frame == 0x2c2)
            FUN_10065b10(index, 0x21c, 1, 0);
        else if (frame == 0x422)
            FUN_10065b10(index, 0x216, 1, 0);
        else if (frame == 0x458)
            FUN_10065b10(index, 0x215, 1, 0);
        else if (frame == 0x6d || frame == 0x77)
            FUN_10065b10(index, 0x21a, 1, 0);
        else
        {
            for (int entry = 0; entry < (int)(sizeof(sBs005StepSounds) / sizeof(sBs005StepSounds[0])); entry++)
            {
                if (sBs005StepSounds[entry].frame == frame)
                {
                    Node *fist = DAT_1021bdf8[index][sBs005StepSounds[entry].fist];
                    Vec4 heard = fist->worldMatrix.GetTranslation();
                    FUN_1003ad20(sBs005StepSounds[entry].sound, &heard, 1.0f);
                    break;
                }
            }
        }
    }
    if (DAT_102550e0[index] != 0)
        goto finish;
    if (DAT_1020b430[index].bs005.state == BS005_STATE_SET_UP)
    {
        DAT_1020b430[index].bs005.state = BS005_STATE_WAKE_PIECES;
        model.PlayMotion(BS005_MOTION_IDLE, 1, 0, 1.0f);
        DAT_1021bdf8[index][0] = model.root->Find("Dummy01", 0);
        DAT_1021bdf8[index][1] = model.root->Find("Dummy02", 0);
        DAT_1021bdf8[index][2] = model.root->Find("Bip01 Pelvis", 0);
        DAT_10db6ee8[BS005_ENDING_MODEL_A].LoadFile(FUN_1013f8a0("bs_005en.it3", 0));
        DAT_10db6ee8[BS005_ENDING_MODEL_B].LoadFile(FUN_1013f8a0("bs_005pa.it3", 0));
        DAT_10db6ee8[BS005_ENDING_MODEL_A].LoadModel(FUN_1013f8a0("bs_005m.it3", 0));
        DAT_10db6ee8[BS005_ENDING_MODEL_B].LoadModel(FUN_1013f8a0("bs_005m.it3", 0));
        DAT_10db6ee8[BS005_ENDING_MODEL_A].mode = 0;
        DAT_10db6ee8[BS005_ENDING_MODEL_B].mode = 0;
        DAT_10db6ee8[BS005_ENDING_MODEL_A].scale.Fill(0.0f);
        DAT_10db6ee8[BS005_ENDING_MODEL_B].scale.Fill(0.0f);
        DAT_10db6ee8[BS005_ENDING_MODEL_A].SetPosition(BS005_FAR_X, 0.0f, 0.0f);
        DAT_10db6ee8[BS005_ENDING_MODEL_B].SetPosition(BS005_FAR_X, 0.0f, 0.0f);
        DAT_10836c70.ResetFrameTimer();
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_WAKE_PIECES)
    {
        for (int piece = 0; piece < BS005_PIECE_COUNT; piece++)
            DodgeGame_UpdateGolemPiece(BS005_FIRST_PIECE + piece);
        DAT_1020b430[index].bs005.state = BS005_STATE_REST;
        DAT_1020b430[index].bs005.frames = 0;
        if (DAT_10255f08 != 0)
        {
            Node *camera = DAT_10255f08->Find("CAboss", 0);
            if (camera != 0)
            {
                camera->ClearFlags(BS005_HIT_CAMERA_FLAG);
                DAT_1020b430[index].bs005.cameraHidden = 0;
            }
            camera = DAT_10255f08->Find("CAboss2", 0);
            if (camera != 0)
                camera->SetFlags(BS005_HIT_CAMERA_FLAG);
        }
        DAT_1020b430[index].bs005.lastHealth = DAT_10daaae8[index].stats[STAT_HEALTH];
        DAT_1020b430[index].bs005.healthMark = DAT_10daaae8[index].stats[STAT_HEALTH];
        if (difficulty == 1)
            DAT_10daaae8[index].stats[4] -= 8;
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_REST)
    {
        DAT_1020b430[index].bs005.frames++;
        motion = BS005_MOTION_IDLE;
        if (!((difficulty != 0 || DAT_1020b430[index].bs005.frames < 0x5b || FUN_1013f880() % 100 > 9) &&
              (difficulty != 1 || DAT_1020b430[index].bs005.frames < 0x97 || FUN_1013f880() % 100 > 9) &&
              (difficulty != 2 || DAT_1020b430[index].bs005.frames < 0x2e || FUN_1013f880() % 100 > 9) &&
              DAT_10daaae8[index].stats[STAT_HEALTH] == DAT_1020b430[index].bs005.lastHealth))
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_SLAM)
    {
        DAT_1020b430[index].bs005.frames++;
        motion = 9;
        if (DAT_1020b430[index].bs005.frames == 1 && DAT_1020b430[index].bs005.motionLayer == 2 && DAT_10c66cc0 != 0)
        {
            restart = 1;
            SayLine(index, 0x45, BS005_VOICE_COOLDOWN_SHORT);
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        frame = model.motionFrame;
        if (frame >= 0x88 && frame <= 0x92)
        {
            power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 3;
            rightHit = 1;
        }
        else if (frame > 0x98 && frame < 0xa5)
        {
            power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 3;
            leftHit = 1;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_BACKHAND)
    {
        DAT_1020b430[index].bs005.frames++;
        if (DAT_1020b430[index].bs005.frames == 1)
        {
            DAT_1020b430[index].bs005.variant = FUN_1013f880() % 2;
            restart = 1;
            if (DAT_10c66cc0 != 0)
                SayLine(index, 0x43, BS005_VOICE_COOLDOWN_SHORT);
        }
        frame = model.motionFrame;
        if (DAT_1020b430[index].bs005.variant == 0)
        {
            motion = 0xc;
            if (frame > 0xec && frame < 0x109)
            {
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 2;
                leftHit = 1;
            }
        }
        else
        {
            motion = 0xb;
            if (frame > 0xb6 && frame < 0xce)
            {
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 2;
                rightHit = 1;
            }
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_SWEEP)
    {
        DAT_1020b430[index].bs005.frames++;
        if (DAT_1020b430[index].bs005.frames == 1)
        {
            DAT_1020b430[index].bs005.variant = FUN_1013f880() % 2;
            restart = 1;
            FUN_100b9bb0(index, 0x42, 0, 0);
        }
        frame = model.motionFrame;
        if (DAT_1020b430[index].bs005.variant == 0)
        {
            motion = 0xe;
            if ((frame >= 0x18b && frame <= 0x198) || (frame >= 0x19c && frame < 0x1b3))
            {
                leftHit = 1;
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            }
        }
        else
        {
            motion = 0xd;
            if ((frame >= 0x12d && frame <= 0x13f) || (frame >= 0x142 && frame <= 0x15d))
            {
                rightHit = 1;
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            }
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_STOMP || DAT_1020b430[index].bs005.state == BS005_STATE_CLOSE_STOMP)
    {
        bool closeStomp = DAT_1020b430[index].bs005.state == BS005_STATE_CLOSE_STOMP;
        DAT_1020b430[index].bs005.frames++;
        motion = closeStomp ? 0x21 : 0xf;
        if (DAT_1020b430[index].bs005.frames == 1 && DAT_1020b430[index].bs005.motionLayer == 2 && (closeStomp || DAT_10c66cc0 != 0))
        {
            restart = 1;
            if (closeStomp)
                FUN_100b9bb0(index, 0x41, 0, 1);
            else
                SayLine(index, 0x41, BS005_VOICE_COOLDOWN_LONG);
        }
        if (DAT_1020b430[index].bs005.frames == 0x1e)
            FUN_100b9bb0(index, 0x43, 0, 0);
        frame = model.motionFrame;
        bool burstA = closeStomp ? (frame == 0x570 || frame == 0x598 || frame == 0x5c0)
                                 : (frame == 0x200 || frame == 0x22d || frame == 0x25a);
        bool burstB = closeStomp ? (frame == 0x584 || frame == 0x5ac || frame == 0x5d4)
                                 : (frame == 0x216 || frame == 0x243 || frame == 0x270);
        if (burstA)
        {
            FUN_1016b230("46283746");
            effectScale = 0.9f;
            flameB = 1;
            StartDebrisBurst(index, ownPos, 5, 3, 0.9f, FUN_1013f880() % 0x1e);
        }
        else if (burstB)
        {
            FUN_1016b230("46283746");
            effectScale = 0.9f;
            flameA = 1;
            StartDebrisBurst(index, ownPos, 3, 2, 0.9f, FUN_1013f880() % 0x1e);
        }
        frame = model.motionFrame;
        if (closeStomp)
        {
            if ((frame > 0x57c && frame < 0x585) || (frame > 0x5a3 && frame < 0x5ad) ||
                (frame > 0x5cb && frame < 0x5d5))
            {
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                rightHit = 1;
            }
            if ((frame > 0x569 && frame < 0x571) || (frame > 0x58f && frame < 0x599) ||
                (frame >= 0x5b7 && frame < 0x5c1))
            {
                leftHit = 1;
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            }
        }
        else
        {
            if ((frame > 0x20b && frame < 0x219) || (frame > 0x23a && frame < 0x247) ||
                (frame > 0x266 && frame < 0x273))
            {
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                rightHit = 1;
            }
            if ((frame >= 0x1f5 && frame <= 0x202) || (frame >= 0x224 && frame <= 0x22f) ||
                (frame >= 0x251 && frame <= 0x25b))
            {
                leftHit = 1;
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            }
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_ROCK_THROW)
    {
        DAT_1020b430[index].bs005.attacking = 1;
        DAT_1020b430[index].bs005.frames++;
        motion = 0x10;
        if (DAT_1020b430[index].bs005.frames == 1)
        {
            restart = 1;
            DAT_1020b430[index].bs005.variant = FUN_1013f880() % BS005_ROCK_COUNT;
            FUN_100b9bb0(index, 0x44, 0, 0);
        }
        frame = model.motionFrame;
        if (frame > 0x2b3 && frame < 0x2ff)
        {
            rightHit = 1;
            leftHit = 1;
            power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] + 2;
        }
        if (frame >= 0x2c2 && frame < 0x2d0)
        {
            int rock = ((DAT_1020b430[index].bs005.variant - 0x2c2 + frame) * 3) % BS005_ROCK_COUNT + BS005_FIRST_ROCK;
            int target = DAT_1020b430[rock].words[BS005_PIECE_TARGET_WORD];
            if (DAT_1021f3c0[target] > 0)
            {
                DAT_10db6ee8[target].m_0288 = 6;
                DAT_10db6ee8[target].m_028c = 0;
                DAT_1021f3c0[target] = SLOT_KIND_DESTROYED;
                DAT_10874848.SetScriptFlag(DAT_10db6ee8[target].scriptFlag, BS005_DESTROYED_SLOT_FLAG);
                DAT_10db6ee8[target].SetPosition(BS005_OFFSTAGE_X, 0.0f, 0.0f);
                DAT_1020b430[rock].state = 4;
                DAT_1020b430[rock].words[1] = 0;
            }
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            if (DAT_10255f08 != 0)
            {
                Node *camera = DAT_10255f08->Find("CAboss", 0);
                if (camera != 0)
                {
                    camera->ClearFlags(BS005_HIT_CAMERA_FLAG);
                    DAT_1020b430[index].bs005.cameraHidden = 0;
                }
            }
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_ROAR_WAVES)
    {
        DAT_1020b430[index].bs005.frames++;
        DAT_1020b430[index].bs005.attacking = 1;
        int limit = difficulty == 0 ? 0x78 : (difficulty == 1 ? 0xb4 : 0x3c);
        if (DAT_1020b430[index].bs005.frames < limit)
        {
            motion = 0x11;
            if (DAT_1020b430[index].bs005.frames == 1 && DAT_1020b430[index].bs005.motionLayer == 2 && DAT_10c66cc0 != 0)
            {
                restart = 1;
                SayLine(index, 0x40, BS005_VOICE_COOLDOWN_LONG);
            }
        }
        else if (DAT_1020b430[index].bs005.frames < BS005_FRAMES_PART_TWO)
        {
            motion = 0x19;
            restart = 1;
            DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_TWO;
        }
        else
        {
            int hardness = DAT_10874848.TestScriptFlag(BS005_SCRIPT_FLAG_DIFFICULTY);
            if ((hardness == 0 && DAT_1020b430[index].bs005.frames < 0x44c) || (hardness == 1 && DAT_1020b430[index].bs005.frames < 0x415) ||
                (hardness == 2 && DAT_1020b430[index].bs005.frames < 0x47e))
            {
                if (DAT_1020b430[index].bs005.motionLayer == 0x19)
                    motion = 0x19;
                model.targetHeading = Atan2(ownPos.x - playerPos.x, ownPos.y - playerPos.y) + BS005_PI;
                turnScale = DAT_10874848.TestScriptFlag(BS005_SCRIPT_FLAG_DIFFICULTY) == 2 ? 0.4f : 0.5f;
                if (DAT_1020b430[index].bs005.frames % 0x1e == 0)
                {
                    Matrix4 pose = RotationX(-BS005_QUARTER_TURN) * RotationZ(model.heading);
                    Vec4 spot = model.matrix.GetTranslation();
                    pose.SetTranslation(spot);
                    StartPoseEffect(index, 0x7ba, pose, 1.3f, 0, 0);
                    FUN_10065b10(index, 0x21d, 1, 0);
                }
            }
            else
            {
                motion = 0x1c;
                if (DAT_1020b430[index].bs005.frames < BS005_FRAMES_PART_THREE)
                {
                    restart = 1;
                    DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_THREE;
                }
            }
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_SHATTER)
    {
        DAT_1020b430[index].bs005.attacking = 1;
        if (DAT_1020b430[index].bs005.cameraHidden == 0)
        {
            away.Copy((ownPos - playerPos).Normalized3() * 1000.0f);
            away.z = 300.0f;
            if (DAT_10255f08 != 0 && DAT_10255f08->Find("CAboss", 0) != 0 &&
                DAT_10255f08->Collide(0, 0xffffffff, &look, &toPlayer, &reach) == 0)
            {
                DAT_10255f08->Find("CAboss", 0)->SetFlags(BS005_HIT_CAMERA_FLAG);
                DAT_1020b430[index].bs005.cameraHidden = 1;
            }
        }
        DAT_1020b430[index].bs005.frames++;
        if (DAT_1020b430[index].bs005.frames == 1)
        {
            DAT_1020b430[index].bs005.savedStat4 = DAT_10daaae8[index].stats[4];
            motion = 0x12;
            restart = 1;
            DAT_10daaae8[index].stats[4] = (DAT_10874848.TestScriptFlag(BS005_SCRIPT_FLAG_DIFFICULTY) != 1 ? 7 : 0) + 3;
            FUN_10065b10(index, 0x18b, 1, 0);
            FUN_100b9bb0(index, 0x43, 0, 0);
        }
        else if (DAT_1020b430[index].bs005.frames < 100)
        {
            motion = 0x12;
            if (DAT_1020b430[index].bs005.motionLayer == 0x13)
            {
                motion = 0x13;
                DAT_1020b430[index].bs005.frames = 100;
            }
        }
        else
        {
            motion = 0x13;
            int hardness = DAT_10874848.TestScriptFlag(BS005_SCRIPT_FLAG_DIFFICULTY);
            if (DAT_1020b430[index].bs005.frames < BS005_FRAMES_PART_TWO)
            {
                if (model.motionFrame == 0x387)
                    DAT_1020b430[index].bs005.variant++;
                if ((DAT_1020b430[index].bs005.variant > 0 && hardness == 2) || (DAT_1020b430[index].bs005.variant > 1 && hardness == 1) ||
                    (DAT_1020b430[index].bs005.variant > 2 && hardness == 0))
                {
                    DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_TWO;
                    DAT_1020b430[index].bs005.variant = 0;
                }
                if (DAT_1020b430[index].bs005.frames < 0x75)
                {
                    int piece = (DAT_1020b430[index].bs005.frames * 3 - 0x12f) % 0x10 + BS005_FIRST_PIECE;
                    if (DAT_1021f3c0[DAT_1020b430[piece].words[BS005_PIECE_TARGET_WORD]] < 0)
                    {
                        DAT_1020b430[piece].state = 2;
                        DAT_1020b430[piece].words[1] = 0;
                    }
                }
            }
            else
            {
                if (model.motionFrame == 0x387)
                {
                    DAT_1020b430[index].bs005.variant++;
                    if (DAT_1020b430[index].bs005.variant > 2)
                    {
                        DAT_10daaae8[index].stats[4] = DAT_1020b430[index].bs005.savedStat4;
                        DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
                        if (DAT_1020b430[index].bs005.cameraHidden == 0 && DAT_10255f08 != 0 && DAT_10255f08->Find("CAboss", 0) != 0)
                        {
                            DAT_10255f08->Find("CAboss", 0)->SetFlags(BS005_HIT_CAMERA_FLAG);
                            DAT_1020b430[index].bs005.cameraHidden = 1;
                        }
                    }
                }
                if (DAT_1020b430[index].bs005.frames < 0x40a)
                {
                    int piece = (DAT_1020b430[index].bs005.frames * 3 - 0xbbb) % 0x10 + BS005_FIRST_PIECE;
                    if (DAT_1021f3c0[DAT_1020b430[piece].words[BS005_PIECE_TARGET_WORD]] < 0)
                    {
                        DAT_1020b430[piece].state = 3;
                        DAT_1020b430[piece].words[1] = 0;
                    }
                }
            }
        }
        if (DAT_1020b430[index].bs005.frames >= 0x65 && DAT_1020b430[index].bs005.frames <= 0x405 && DAT_1020b430[index].bs005.frames % period == 0 && FUN_1013f880() % 3 == 0)
        {
            Matrix4 pose = RotationZ(FUN_10001f80(ownPos.x, ownPos.y));
            pose.SetTranslation(ownPos);
            StartPoseEffect(index, 0x7a4, pose, 0.9f, 0, 0);
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_BEAM)
    {
        DAT_1020b430[index].bs005.frames++;
        motion = 0x14;
        if (DAT_1020b430[index].bs005.frames == 1 && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            restart = 1;
            FUN_100b9bb0(index, 0x41, 0, 0);
        }
        if (DAT_1020b430[index].bs005.frames == 0x1e)
            FUN_100b9bb0(index, 0x43, 0, 1);
        if (model.motionFrame == 0x3bd)
        {
            FUN_1016b230("19191919191919191928283737464646");
            FUN_10055ee0();
            Matrix4 pose;
            Vec4 middle = (DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() +
                           DAT_1021bdf8[index][1]->worldMatrix.GetTranslation()) * 0.5f;
            middle.z = 0.0f;
            pose.SetIdentity();
            pose.SetTranslation(middle);
            float scale = 1.1f;
            if (DAT_1021f3c0[DAT_1020b430[BS005_FIST_A_PIECE].words[BS005_PIECE_TARGET_WORD]] < 0)
                scale *= 0.9f;
            if (DAT_1021f3c0[DAT_1020b430[BS005_FIST_B_PIECE].words[BS005_PIECE_TARGET_WORD]] < 0)
                scale *= 0.9f;
            StartPoseEffect(index, 0x70c, pose, scale, 0, 0);
        }
        if (model.motionFrame > 0x3b3 && model.motionFrame < 0x3be)
        {
            rightHit = 1;
            leftHit = 1;
            power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] + 2;
        }
        int hardness = DAT_10874848.TestScriptFlag(BS005_SCRIPT_FLAG_DIFFICULTY);
        if ((hardness == 0 && DAT_1020b430[index].bs005.frames > 0xa5) || (hardness == 1 && DAT_1020b430[index].bs005.frames > 0x78) ||
            (hardness == 2 && DAT_1020b430[index].bs005.frames > 0x5a))
            motion = 0x23;
        if (DAT_1020b430[index].bs005.frames < 0x1e)
            model.targetHeading = Atan2(ownPos.x - playerPos.x, ownPos.y - playerPos.y) + BS005_PI;
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_LIFT)
    {
        DAT_1020b430[index].bs005.attacking = 1;
        DAT_1020b430[index].bs005.frames++;
        if (DAT_1020b430[index].bs005.frames == 0x1e)
            FUN_100b9bb0(index, 0x43, 0, 0);
        if (DAT_1020b430[index].bs005.frames == 1)
        {
            motion = 0x15;
            restart = 1;
            FUN_100b9bb0(index, 0x40, 0, 0);
            goto finish;
        }
        if (DAT_1020b430[index].bs005.frames < 100)
        {
            motion = 0x15;
            if (model.motionFrame == 0x421)
                DAT_1020b430[index].bs005.frames = 100;
            goto finish;
        }
        if (DAT_1020b430[index].bs005.frames < BS005_FRAMES_PART_TWO)
        {
            motion = 0x15;
            float lift = 95.0f - (float)(DAT_1020b430[index].bs005.frames - 100);
            if (lift > 0.0f)
                DAT_10db9248 += lift;
            if (DAT_1020b430[index].bs005.frames % (period / 2) == 0)
            {
                Matrix4 pose;
                pose.SetIdentity();
                pose.SetTranslation(ownPos);
                StartPoseEffect(index, 0x7b9, pose, 0.65f, FUN_1013f880() % period, &model.root->worldMatrix);
            }
            if (model.motionFrame == 0x455)
            {
                motion = 0x16;
                DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_TWO;
            }
            model.targetHeading = Atan2(ownPos.x - playerPos.x, ownPos.y - playerPos.y) + BS005_PI;
            goto finish;
        }
        if (DAT_1020b430[index].bs005.frames >= BS005_FRAMES_PART_FOUR)
        {
            if (DAT_1020b430[index].bs005.frames < 4000)
            {
                motion = 0x17;
                if (model.motionFrame == 0x458)
                {
                    FUN_1016b230("191919191919283746");
                    FUN_10055ee0();
                    Matrix4 pose;
                    pose.SetIdentity();
                    pose.m[12] = playerPos.x;
                    pose.m[13] = playerPos.y;
                    StartPoseEffect(index, 0x6ec, pose, 0.8f, 0, 0);
                }
                if (model.motionFrame == 0x472)
                {
                    DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
                    DAT_1020b430[index].bs005.attacking = 0;
                }
            }
            goto finish;
        }
        motion = 0x16;
        if (playerPos.z >= 600.0f)
            model.targetHeading = Atan2(ownPos.x - playerPos.x, ownPos.y - playerPos.y) + BS005_PI;
        else
        {
            DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_FOUR;
            motion = 0x17;
            model.targetHeading = model.heading;
        }
        if (DAT_1020b430[index].bs005.frames % (period / 2) == 0)
        {
            Matrix4 pose;
            pose.SetIdentity();
            pose.SetTranslation(ownPos);
            StartPoseEffect(index, 0x7b9, pose, 0.65f, FUN_1013f880() % period, &model.root->worldMatrix);
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_TURN)
    {
        int maxHealth = DAT_10daaae8[index].stats[STAT_MAX_HEALTH];
        if (DAT_1020b430[index].bs005.frames < 100)
        {
            motion = 4;
            if (DAT_1020b430[index].bs005.motionLayer == 5)
                DAT_1020b430[index].bs005.frames = 100;
            if (maxHealth / 4 < DAT_1020b430[index].bs005.healthMark - DAT_10daaae8[index].stats[STAT_HEALTH])
                DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_TWO;
        }
        else if (DAT_1020b430[index].bs005.frames < BS005_FRAMES_PART_TWO)
        {
            float wanted = Atan2(ownPos.x - playerPos.x, ownPos.y - playerPos.y) + BS005_PI;
            float error = WrapAngle(wanted - model.heading);
            if (error > 0.0f)
                motion = 6;
            else if (error < 0.0f)
                motion = 7;
            model.targetHeading = wanted;
            if (fabs(error) < 0.17453f)
                DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_TWO;
            if (maxHealth / 4 < DAT_1020b430[index].bs005.healthMark - DAT_10daaae8[index].stats[STAT_HEALTH])
                DAT_1020b430[index].bs005.frames = BS005_FRAMES_PART_TWO;
        }
        else
        {
            motion = 8;
            if (DAT_1020b430[index].bs005.motionLayer == 2)
                DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_CLOSE_ATTACK_A)
    {
        DAT_1020b430[index].bs005.frames++;
        motion = 0x1d;
        if (DAT_1020b430[index].bs005.frames == 1 && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            restart = 1;
            FUN_100b9bb0(index, 0x45, 0, 1);
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        frame = model.motionFrame;
        if (frame >= 0x4ac && frame <= 0x4b3)
            rightHit = 1;
        else if (frame >= 0x4bd && frame <= 0x4c2)
            leftHit = 1;
        else
            goto finish;
        power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 3;
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state == BS005_STATE_CLOSE_ATTACK_B)
    {
        DAT_1020b430[index].bs005.frames++;
        if (DAT_1020b430[index].bs005.frames == 1)
        {
            DAT_1020b430[index].bs005.variant = FUN_1013f880() % 2;
            restart = 1;
            FUN_100b9bb0(index, 0x43, 0, 1);
        }
        frame = model.motionFrame;
        if (DAT_1020b430[index].bs005.variant == 0)
        {
            motion = 0x20;
            if (frame > 0x519 && frame < 0x52d)
            {
                leftHit = 1;
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 2;
            }
        }
        else
        {
            motion = 0x1f;
            if (frame > 0x4df && frame < 0x4f5)
            {
                rightHit = 1;
                power = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] - 2;
            }
        }
        if (DAT_1020b430[index].bs005.frames > 0x1e && DAT_1020b430[index].bs005.motionLayer == 2)
        {
            DAT_1020b430[index].bs005.state = BS005_STATE_PICK;
            motion = BS005_MOTION_NONE;
        }
        goto finish;
    }
    if (DAT_1020b430[index].bs005.state != BS005_STATE_PICK)
        goto finish;
    DAT_1020b430[index].bs005.frames = 0;
    DAT_1020b430[index].bs005.variant = 0;
    DAT_1020b430[index].bs005.attacking = 0;
    {
        int hardness = DAT_10874848.TestScriptFlag(BS005_SCRIPT_FLAG_DIFFICULTY);
        int maxHealth = DAT_10daaae8[index].stats[STAT_MAX_HEALTH];
        int lost = DAT_1020b430[index].bs005.healthMark - DAT_10daaae8[index].stats[STAT_HEALTH];
        unsigned pick;
        if (DAT_10daaae8[index].held == 0)
        {
            pick = FUN_1013f880() % 10;
            bool angry = pick == 4 || DAT_1020b430[index].bs005.calmFrames > FUN_1013f880() % 10000 || (lost > maxHealth / 4 && hardness != 1) ||
                         (lost > maxHealth / 2 && hardness == 1);
            if (angry)
            {
                if (DAT_1020b430[index].bs005.calmFrames < FUN_1013f880() % 10000 &&
                    ((lost < maxHealth / 4 && hardness != 1) || (lost < maxHealth / 2 && hardness == 1)))
                    pick = FUN_1013f880() % 10;
                else
                    pick = 4;
            }
            switch (pick)
            {
            case 0:
            case 5:
                turnLimit = 1.4f;
                DAT_1020b430[index].bs005.state = BS005_STATE_BEAM;
                break;
            case 1:
            case 7:
                turnLimit = 0.8f;
                DAT_1020b430[index].bs005.state = BS005_STATE_SLAM;
                break;
            case 2:
            case 6:
            case 8:
                turnLimit = 2.4f;
                DAT_1020b430[index].bs005.state = BS005_STATE_SWEEP;
                break;
            case 3:
                turnLimit = 1.5f;
                DAT_1020b430[index].bs005.state = BS005_STATE_ROAR_WAVES;
                break;
            case 9:
                DAT_1020b430[index].bs005.state = BS005_STATE_LIFT;
                turnLimit = BS005_PI;
                break;
            default:
                DAT_1020b430[index].bs005.state = BS005_STATE_SHATTER;
                turnLimit = BS005_PI;
                break;
            }
        }
        else
        {
            pick = FUN_1013f880() % 9;
            int gone = 0;
            for (int piece = BS005_FIRST_ROCK; piece < BS005_FIRST_ROCK + BS005_ROCK_COUNT; piece++)
                if (DAT_1021f3c0[DAT_1020b430[piece].words[BS005_PIECE_TARGET_WORD]] < 0)
                    gone++;
            if (pick == 4 || FUN_1013f880() % 100 < gone * 8)
            {
                if (gone * 8 < FUN_1013f880() % 100)
                {
                    while (pick == 4)
                        pick = FUN_1013f880() % 9;
                }
                else
                    pick = 4;
            }
            switch (pick)
            {
            case 0:
                turnLimit = 0.17453f;
                DAT_1020b430[index].bs005.state = BS005_STATE_REST;
                break;
            case 1:
            case 7:
                turnLimit = 0.8f;
                DAT_1020b430[index].bs005.state = BS005_STATE_SLAM;
                break;
            case 2:
            case 6:
            case 8:
                turnLimit = 1.5f;
                DAT_1020b430[index].bs005.state = BS005_STATE_BACKHAND;
                break;
            case 3:
                DAT_1020b430[index].bs005.state = BS005_STATE_STOMP;
                turnLimit = BS005_PI;
                break;
            case 5:
                turnLimit = 1.4f;
                DAT_1020b430[index].bs005.state = BS005_STATE_BEAM;
                break;
            default:
                DAT_1020b430[index].bs005.state = BS005_STATE_ROCK_THROW;
                turnLimit = BS005_PI;
                break;
            }
        }
        toPlayer = ownPos - playerPos;
        if (toPlayer.Length2D() < 700.0f)
        {
            switch (DAT_1020b430[index].bs005.state)
            {
            case BS005_STATE_SLAM:
                DAT_1020b430[index].bs005.state = BS005_STATE_CLOSE_ATTACK_A;
                break;
            case BS005_STATE_BACKHAND:
            case BS005_STATE_SWEEP:
                turnLimit = 1.4f;
                DAT_1020b430[index].bs005.state = BS005_STATE_CLOSE_ATTACK_B;
                break;
            case BS005_STATE_STOMP:
            case BS005_STATE_ROCK_THROW:
                if (DAT_1020b430[index].bs005.state == BS005_STATE_ROCK_THROW && FUN_1013f880() % 2 == 0)
                    break;
                turnLimit = 1.6f;
                DAT_1020b430[index].bs005.state = BS005_STATE_CLOSE_STOMP;
                break;
            }
        }
        float error = WrapAngle(Atan2(ownPos.x - playerPos.x, ownPos.y - playerPos.y) + BS005_PI - model.targetHeading);
        if (turnLimit < (float)fabs(error))
            DAT_1020b430[index].bs005.state = BS005_STATE_TURN;
    }
finish:
    if (refreshMotion != 0)
    {
        Playback *playing = model.motion;
        if (playing->blending == 0)
        {
            DAT_1020b430[index].bs005.motionFrame = (int)playing->m_0004;
            DAT_1020b430[index].bs005.motionLayer = playing->layer;
        }
        else
        {
            DAT_1020b430[index].bs005.motionFrame = -1;
            DAT_1020b430[index].bs005.motionLayer = playing->nextLayer;
        }
    }
    if (motion >= 0)
        model.PlayMotion(motion, restart, 5, 1.0f);
    DAT_1020b430[index].bs005.hitLevel = DAT_102550e0[index];
    int pieceNo = 2;
    for (int piece = BS005_FIRST_PIECE + 2; piece < BS005_FIRST_PIECE + BS005_PIECE_COUNT; piece++, pieceNo++)
    {
        if (DAT_1021f3c0[DAT_1020b430[piece].words[BS005_PIECE_TARGET_WORD]] > 0)
        {
            DAT_10daaae8[index].held = 1;
            DAT_1020b430[index].bs005.healthMark = DAT_10daaae8[index].stats[STAT_HEALTH];
            DAT_1020b430[index].bs005.calmFrames = 0;
            break;
        }
    }
    if (pieceNo == 0x10)
    {
        char wasHeld = DAT_10daaae8[index].held;
        DAT_10daaae8[index].held = 0;
        if (wasHeld != 0)
            DAT_10daaa80.RemoveOwnedBy(index + BS005_EFFECT_OWNER_BASE);
    }
    float heading = WrapAngle(model.heading);
    float wanted = WrapAngle(model.targetHeading);
    float turn = WrapAngle(heading - wanted);
    float turnCap = difficulty == 0 ? 0.225f : (difficulty == 1 ? 0.15f : 0.3f);
    if ((float)fabs(turn) > turnCap)
        turn = turn <= 0.0f ? -turnCap : turnCap;
    turn = turn * turnScale;
    DAT_1020b430[index].bs005.lastHealth = DAT_10daaae8[index].stats[STAT_HEALTH];
    model.targetHeading = model.heading - turn;
    for (int fist = 0; fist < 2; fist++)
    {
        if ((fist == 0 && rightHit == 0) || (fist == 1 && leftHit == 0))
            continue;
        Node *hand = DAT_1021bdf8[index][fist];
        Vec4 &last = fist == 0 ? DAT_10802bf8[index].position : DAT_10802bf8[index].step;
        int pieceSlot = fist == 0 ? BS005_FIST_A_PIECE : BS005_FIST_B_PIECE;
        Vec4 swing = hand->worldMatrix.GetTranslation() - last;
        float yaw = Atan2(swing.x, swing.y) + BS005_PI;
        float pitch = (float)acos(swing.Length2D() / swing.Length());
        swing.Copy(swing.Normalized3() * 100.0f);
        if (swing.z < 0.0f)
            pitch = pitch * -1.0f;
        Matrix4 pose = RotationX(pitch) * RotationZ(yaw);
        Vec4 at = hand->worldMatrix.GetTranslation() + swing;
        pose.SetTranslation(at);
        at.Fill(0.0f);
        bool broken = DAT_1021f3c0[DAT_1020b430[pieceSlot].words[BS005_PIECE_TARGET_WORD]] > 0;
        EffectOptions options;
        options.m_0000 = 0;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = 1.0f;
        options.followOwner = 1;
        FUN_1001d8b0(index, broken ? 0x7a6 : 0x7a8, at, -1, -1, 0, 0, 0, &pose, broken ? 0 : &options);
        Matrix4 sideways = RotationX(1.5708f) * pose;
        pose = sideways;
        pose.SetTranslation(hand->worldMatrix.GetTranslation() - swing * 0.5f);
        FUN_1001d8b0(index, broken ? 0x7a5 : 0x7a7, at, -1, -1, 0, 0, 0, &pose, &options);
        if (power > 0)
        {
            EffectOptions strike;
            strike.m_0000 = 0;
            strike.m_0004 = power;
            strike.m_000e = 5;
            strike.m_0008 = broken ? 1.0f : 0.75f;
            Matrix4 strikePose;
            strikePose.SetIdentity();
            strikePose.SetTranslation(hand->worldMatrix.GetTranslation());
            FUN_1001d8b0(index, broken ? 0x7a9 : 0x7aa, at, -1, -1, 0, 0, 0, &strikePose, &strike);
        }
    }
    for (int fist = 0; fist < 2; fist++)
    {
        if ((fist == 0 && flameA == 0) || (fist == 1 && flameB == 0))
            continue;
        EffectOptions options;
        Matrix4 pose;
        Vec4 at;
        int pieceSlot = fist == 0 ? BS005_FIST_A_PIECE : BS005_FIST_B_PIECE;
        options.m_0000 = 0;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = effectScale;
        pose.SetIdentity();
        pose.m[12] = DAT_1021bdf8[index][fist]->matrix.m[12];
        pose.m[13] = DAT_1021bdf8[index][fist]->matrix.m[13];
        bool broken = DAT_1021f3c0[DAT_1020b430[pieceSlot].words[BS005_PIECE_TARGET_WORD]] < 0;
        if (broken)
            options.m_0008 = effectScale * 0.8f;
        at.Fill(0.0f);
        FUN_1001d8b0(index, broken ? 0x7b2 : 0x7ad, at, -1, -1, 0, 0, 0, &pose, &options);
    }
    for (int fist = 0; fist < 2; fist++)
    {
        Vec4 &last = fist == 0 ? DAT_10802bf8[index].position : DAT_10802bf8[index].step;
        Vec4 moved = DAT_1021bdf8[index][fist]->worldMatrix.GetTranslation() - last;
        if (moved.LengthSquared3() > 5.0f)
            last.Copy(DAT_1021bdf8[index][fist]->worldMatrix.GetTranslation());
    }
    if (DAT_104c0380 % 5 == 0)
    {
        Vec4 aimAway = ownPos - playerPos;
        aimAway.z = 0.0f;
        aimAway = aimAway.Normalized3() * 1000.0f;
        aimAway.z = 300.0f;
        if (DAT_10255f08 != 0)
        {
            Node *first = DAT_10255f08->Find("CAboss", 0);
            Node *second = DAT_10255f08->Find("CAboss2", 0);
            Vec4 from = playerPos;
            Vec4 hit;
            if ((first != 0 && first->Collide(0, 4, &from, &aimAway, &hit) != 0) ||
                (second != 0 && second->Collide(0, 4, &from, &aimAway, &hit) != 0))
            {
                away.Copy(hit);
                away.y -= 900.0f;
                away.z = 0.0f;
                away.Copy(away.Normalized3() * 700.0f);
                DAT_10db9244 = away.y + 900.0f;
                DAT_10db9240 = away.x;
            }
        }
    }
    return 0;
}
