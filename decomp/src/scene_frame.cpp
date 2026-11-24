// The scene frame (0x100f5a80, called once a frame by the frame step 0x1011a060, by the character update 0x100f9670 when
// a new map starts and by 0x1011d810). It runs the map specific updates, the scripted walk, the character slot loops of
// the people and the party (slots 0x3a..0x5b: waking, parking, following), the camera modes, the look-at turn of the
// people and the groups' fade, and counts down the timers of sounds and events. C++ linkage: the function keeps an
// exception frame for its Matrix4 and EffectOptions locals.

#include <math.h>
#include <string.h>
#include <windows.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "last_position.h"
#include "effect_manager.h"
#include "mmv_player.h"
#include "text_window.h"
#include "visibility.h"
#include "game.h"
#include "enemy_queue.h"
#include "partner_follow.h"
#include "ui_vars.h"
#include "engine_memory.h"
#include "texture_object.h"
#include "world.h"
#include "scene_reset.h"
#include "anim_step_vars.h"

#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "sound_vars.h"
#include "script_system.h"
#include "value_command.h"
#include "scene_command.h"
#include "shaga.h"
#include "doppler_loops.h"
#include "script_flag.h"
#include "dodge_game.h"
#include "dodge_golem.h"
#include "homing_shots.h"
#include "pod_enemy.h"
#include "char_update.h"
#include "hop_spit_enemy.h"
#include "enemy_chase_hop.h"
#include "camera_modes.h"
#include "scene_frame_vars.h"
#include "scene_frame.h"

// Maps and ranges the frame treats specially.
#define MAP_WALLS 0x18                  // Map_UpdateWalls
#define MAP_LAVA 0x13b                  // Lava_Update (while DAT_10c655e0 is set)
#define MAP_ROOT_ROTATOR 0x136          // the "@cr" node under "@crroot" turns
#define MAP_DODGE_GAME 0x132            // the stone golem dodge game
#define MAP_TIMED_BATTLE 0x71           // the player's motion speed decays (DAT_1020b430)
#define MAP_FOUR_SWITCHES 0xc           // four switches that turn back when nobody stands near them
#define MAP_SHORT_START 0x25d
#define MAP_TOKATYU 0x133
#define MAP_TOKATYU_B 0x134
#define PEOPLE_SLOT_FIRST 0x3a          // the party and the people of the map: slots 0x3a..0x5b
#define PEOPLE_SLOT_END 0x5c
#define GOLEM_PIECE_FIRST 0x3d
#define GOLEM_PIECE_COUNT 0x10
#define GOLEM_EFFECT_OWNER 0x166        // EffectOptions::owner of the dust the golem fight starts
#define MARKER_SLOT_END 0x7c            // markers: slots 0x5c..0x7b
#define SLOT_NOT_IN_PLAY (-200000.0f)   // x a parked character is placed at
#define PARTNER_PARK_X (-100000.0f)
#define PI_UPPER 3.1415f
#define TWO_PI_SHORT 6.283f
#define HALF_PI_SHORT 1.5707f
#define CAMERA_NEAR_DISTANCE 10.0f      // the camera counts as arrived within this distance of its goals
#define CAMERA_FOLLOW_GOAL 0.2f         // share of the way to the goal moved each frame
#define CAMERA_FOLLOW_KEEP 0.8f         // share of the previous camera vector kept each frame
#define CAMERA_TURN_STEP 0.01f          // largest turn of the turning camera per frame
#define TEXT_WINDOW_FADE_PERCENT 20     // card windows blink while their opening is below this percent
#define FADE_STEP 0.066667f
#define EFFECT_OWNER_BASE 400          // EffectOptions::owner = character slot + this
#define PARTNER_LINK_BASE 10000         // DAT_10453008 above this = the partner slot + this
#define PARTNER_PARKED_WAIT (-10000)    // DAT_10453008 of a slot parked behind its partner
#define PARTNER_WOKEN_WAIT (-30)        // DAT_10453008 of a person just woken (fades in over 30 frames)
#define WAKE_LIFT_TRIES 0x5a            // a woken person is lifted 5 units this often while it stands in the map
#define NODE_MASK_BODY 0x400
#define NODE_MASK_SHADOW 0x200
#define NODE_STATE_FE 0xfe
#define NODE_STATE_FF 0xff
#define NODE_STATE_2F 0x2f
#define NODE_FLAGS_HIDE 0x20
#define LOOK_HEIGHT_SCALE 2.5f         // heights count this much more than ground distance when a person is rated
#define LOOK_TURN_WEIGHT 0.75f          // share of the rating that grows with the angle between camera and person
#define LOOK_BASE_WEIGHT 0.25f
#define STATE_VISIBILITY_CHECK 1000     // DAT_10210d30 value of a character that is removed when out of view
#define FRAMES_WAKE_RESET 300
#define PARTNER_CODE_MAX 0x1f4

extern "C" int FUN_10017aa0(int id);                          // Enemy_FindByKind
extern "C" int FUN_1001d7d0(int index);                       // Char_TickKindCooldown
extern "C" int FUN_1001d920(void);                            // Carry_StackOnObject
extern "C" int FUN_100182a0(void);                            // Map_UpdateWalls
extern "C" void FUN_10034fe0(void);                           // Groups_Clear
extern "C" int FUN_100ba040(void);                            // Npc_PlayGreeting
extern "C" int FUN_10034e80(int index, const Vec4 *offset, int range);
extern "C" int FUN_1005f640(int slot);                        // Switch_TurnOff
extern "C" int FUN_100627d0(int index);                       // Enemy_Defeat
#include "bs003.h"
#include "neo_galaxy.h"
extern "C" int FUN_100165e0(int mode);                        // Player_SetHeadTracking
extern "C" int FUN_1016a8d0(void);                            // Camera_SetMode2
extern "C" int FUN_1016a8e0(void);                            // Camera_SetModeFollow
extern "C" int FUN_1007ad20(int index);                       // AreaZone_Update
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);
extern "C" int FUN_10174c70(int slot, int force);             // Texture_Release
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);   // Sound_PlayEffect
extern "C" void FUN_10001c50(int);                            // SoundArc_Command224
extern "C" int FUN_1013f880(void);                            // the game's random number
extern "C" const char *FUN_1013f8a0(const char *name, int folder);
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove
extern "C" double FUN_101b2b50(double y, double x);            // atan2
int Map_UpdateSpecial();
int FUN_10049650(void);                                       // Wave_Update
int FUN_10042a30(void);                                       // Shaga_Update
int Char_DrawLinks();
int Lava_Update();
int Kind43_UpdateShots();
int FUN_100404e0(int index, int mount, int clothes, int head, int weapon, int skipKindSelect, int alternative);
int FUN_100615f0(int index, int scatter, int bothHands);
int Enemy_PodLobber(int index);

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Slot kinds whose clamp of the three per-slot counters differs.
static void ClampKindCounters(int slot, int kind)
{
    if (kind == 0x46 || kind == 0x69 || kind == 0x6b || kind == 0x32)
    {
        if (DAT_103153b0[slot] > 0xf)
            DAT_103153b0[slot] = 0xf;
        if (DAT_10219770[slot] > 0xf)
            DAT_10219770[slot] = 0xf;
        if (DAT_10315350[slot] > 0xf)
            DAT_10315350[slot] = 0xf;
    }
    else if (kind == 0x3f)
    {
        if (DAT_103153b0[slot] > 0xf)
            DAT_103153b0[slot] = 0xf;
        if (DAT_10315350[slot] > 0xf)
            DAT_10315350[slot] = 0xf;
    }
    else if (kind == 0x2d)
    {
        if (DAT_10219770[slot] > 0xf)
            DAT_10219770[slot] = 0xf;
        if (DAT_10315350[slot] > 0xf)
            DAT_10315350[slot] = 0xf;
    }
    else if (kind == 0x43)
    {
        if (DAT_103153b0[slot] > 0xf)
            DAT_103153b0[slot] = 0xf;
        if (DAT_10219770[slot] > 0xf)
            DAT_10219770[slot] = 0xf;
        int limit = DAT_1020b430[slot].sawPlayer;
        if (limit != 0 && limit < DAT_10315350[slot])
        {
            if (limit < 0xff && limit >= 1)
                DAT_10315350[slot] = (unsigned char)limit;
            else if (limit < 1)
                DAT_10315350[slot] = 0;
            else
                DAT_10315350[slot] = 0xff;
        }
    }
}

// Starts a character's behaviour over: the whole CharState and the last positions are cleared.
static void ClearBehaviour(int slot)
{
    memset(&DAT_1020b430[slot], 0, sizeof(CharState));
    DAT_10802bf8[slot].position.Fill(0.0f);
    DAT_10802bf8[slot].step.Fill(0.0f);
}

// Sets bit `bit` (mod 32) of script flag word `word`; flag 0 and the flags past 3999 are never written.
static void SetFlagBit(int word, int bit)
{
    int current = ScriptFlag(word);
    unsigned int updated = current | (1 << (bit & 0x1f));
    if ((unsigned int)current != updated && word <= 3999 && word != 0)
        DAT_10c62edc[word] = updated;
}



// The look-around of the slot loop: the angle between the bearing from the camera to its target and the bearing to a
// person, folded into -pi..pi.
static __forceinline float FoldAngle(float angle)
{
    while (angle > PI_UPPER)
        angle -= TWO_PI_SHORT;
    while (angle < -PI_UPPER)
        angle += TWO_PI_SHORT;
    return angle;
}

// Rates how near a person is to the camera's aim: distance, more the further the camera must turn to see the person.
static __forceinline void StoreLookDistance(int index, Vec4 &offset, float turn)
{
    DAT_1021b728[index] = (int)offset.Length();
    float turnSize = (float)fabs(turn);
    DAT_1021b728[index] = (int)((turnSize / PI_UPPER * LOOK_TURN_WEIGHT + LOOK_BASE_WEIGHT) * (float)DAT_1021b728[index]);
}

// 0x100f5a80 (Scene_Frame). Once a frame: the map specific updates (walls, lava, the turning "@cr" node, the dodge game
// golem pieces), the scripted walk of a character to a point, the timers of the status windows, the slot loop of the
// people and party (slots 0x3a..0x5b: clamps their counters, starts them over when parked, wakes them when the player
// comes near, follows their partners, removes them when out of view), the camera mode of DAT_104c0354 (0 = none, 1 = ease
// to the goals, 2 = jump to the goals, 3 = height only, 5 = turn about, 7/8 = slow ease, 9..0x12 = the modes of
// camera_follow.cpp and friends), the look-at weights of the people and the fade of their groups, and the countdowns of
// sounds, events and the timed run. Returns 0.
// The enemy tied to switch `which` is switched off when it is out of reach (150 units) while a switch is armed.
static __forceinline void ReleaseStuckSwitchEnemy(int which)
{
    if (DAT_10c66d40[which] != 0)
    {
        int enemy = FUN_10017aa0(which + 1);
        if (FUN_10034e80(enemy, 0, 0x96) == 0 && DAT_1032d69c != 0)
            FUN_1005f640(enemy);
    }
}

int Scene_Frame()
{
    int gauge = DAT_10c64ad8;
    int bank = DAT_10c64ad0;
    if (bank != 0 && gauge == 400)
        DAT_10c64f78 = 1;
    else if (bank == 0 && gauge == 300)
        DAT_10c64f78 = 1;
    else
        DAT_10c64f78 = 0;
    if (DAT_104bf8a4 % 30 == 0 && DAT_10c635a8 != 0)
        FUN_10049650();
    Char_DrawLinks();
    FUN_10042a30();
    if (DAT_10c62ed0 == MAP_WALLS)
        FUN_100182a0();
    else if (DAT_10c62ed0 == MAP_LAVA)
    {
        if (DAT_10c655e0 != 0)
            Lava_Update();
    }
    else if (DAT_10c62ed0 == MAP_ROOT_ROTATOR)
    {
        if (DAT_10446e00[0] == 0)
        {
            Node *root;
            if (DAT_10255f08 != 0 && (root = DAT_10255f08->Find("@crroot", 0)) != 0 &&
                (DAT_10446e00[0] = root->Find("@cr", 1)) != 0)
            {
                root->matrix = DAT_10446e00[0]->matrix;
                DAT_10446e00[0]->matrix.SetIdentity();
                DAT_1021d018[0] = -11000;
            }
        }
        else
        {
            int shagaAngle = DAT_1021d018[0];
            float speed;
            if (DAT_104bfe6c == 0)
                speed = (float)((DAT_1021d018[0] / -1000) % 10) * 6.0f;
            else if (DAT_10c65684 != 0)
                speed = -54.0f;
            else
                speed = -6.0f;
            DAT_1021d018[0] = shagaAngle - (int)speed;
            float angle = (float)(-DAT_1021d018[0] % 1000) * PI_UPPER / 180.0f;
            if (-DAT_1021d018[0] % 1000 > 0x167)
                DAT_1021d018[0] += 0x168;
            DAT_10446e00[0]->matrix = RotationZ(angle);
        }
    }
    if (DAT_1032d69c != 0 && DAT_104bfe6c == 0 && DAT_104c0135 == 0 && DAT_104bfe1c == 0 && DAT_104bfeb0 == 0 &&
        DAT_104bfdf0 == 0)
    {
        if (DAT_10c62ed0 == MAP_DODGE_GAME)
        {
            if (DAT_1020c2b0 > 1)
            {
                for (int n = 0; n < GOLEM_PIECE_COUNT; n++)
                    if (DAT_1020b430[GOLEM_PIECE_FIRST + n].state != 0)
                        DodgeGame_UpdateGolemPiece(GOLEM_PIECE_FIRST + n);
                if (DAT_10daaae8[PEOPLE_SLOT_FIRST].stats[STAT_HEALTH] > 0 && DAT_10daaae8[PEOPLE_SLOT_FIRST].held != 0 &&
                    DAT_104bf8a4 % 10 == 0)
                {
                    Matrix4 matrix;
                    Vec4 position;
                    EffectOptions options;
                    matrix.SetIdentity();
                    options.owner = GOLEM_EFFECT_OWNER;
                    matrix.SetTranslation(DAT_10db6ee8[PEOPLE_SLOT_FIRST].matrix.GetTranslation());
                    FUN_1001d8b0(PEOPLE_SLOT_FIRST, 0x7be, position, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10c62ed0 == MAP_DODGE_GAME)
                Kind43_UpdateShots();
        }
        if (DAT_104c0398 > 0 && --DAT_104c0398 == 0)
            DAT_104c00b8 = 0;
    }

    // The scripted walk: character DAT_10c6358c walks to DAT_104c4438[1], sidesteps when it stands still near it.
    if (DAT_104c0360 != 0)
    {
        int walker = DAT_10c6358c;
        Vec4 anchor;
        Vec4 goal;
        Vec4 move;
        Vec4 sidestep;
        anchor.Copy(DAT_104c4438[0]);
        goal.Copy(DAT_104c4438[1]);
        move.Copy(DAT_104c4438[2]);
        sidestep.Copy(DAT_104c4438[3]);
        int sidestepSign = DAT_10c6357c;
        int stillFrames = DAT_10c63580;
        int stepCount = DAT_10c63584;
        int sidestepFrames = DAT_10c63588;
        if ((DAT_10db6ee8[walker].matrix.GetTranslation() - anchor).Length() < 1.0f)
        {
            if (++stillFrames > 10)
            {
                sidestepFrames = 15;
                anchor.Copy(DAT_10db6ee8[walker].matrix.GetTranslation());
                sidestep.Copy(goal - anchor);
                sidestep.z = 0.0f;
                sidestep.Copy(sidestep.Normalized3() * 7.0f);
                if (FUN_1013f880() % 2 != 0 && sidestepSign == 0 || sidestepSign == -1)
                {
                    sidestep.RotateZ(-HALF_PI_SHORT);
                    sidestepSign = -1;
                }
                else
                {
                    sidestep.RotateZ(HALF_PI_SHORT);
                    sidestepSign = 1;
                }
                stillFrames = 0;
            }
        }
        else
            stillFrames = 0;
        anchor.Copy(DAT_10db6ee8[walker].matrix.GetTranslation());
        move.Copy(goal - anchor);
        move.z = 0.0f;
        move.Copy(move.Normalized3() * (float)DAT_10c63578);
        if (sidestepFrames > 0)
        {
            move.Copy(sidestep);
            sidestepFrames--;
        }
        DAT_10db6ee8[walker].targetHeading = Atan2(move.x, move.y) + PI_UPPER;
        float arrival = (float)DAT_10c63578 / 7.0f * 10.0f;
        double arrivalDistance = arrival > 10.0 ? arrival : 10.0;
        if ((goal - anchor).Length2D() <= arrivalDistance)
        {
            DAT_10db6ee8[walker].PlayMotion(2, 0, 5, 1.0f);
            DAT_104c0360 = 0;
        }
        else
        {
            if (walker > 0)
                FUN_100241e0(walker, move.x, move.y, move.z);
            else
            {
                DAT_104bfd0c = 2;
                DAT_104c0024 = 0;
                DAT_102559f0 = (int)(move.x + 0.5f);
                DAT_1042d9e4 = (int)(move.y + 0.5f);
            }
            if (DAT_10c63578 > 7)
            {
                if (DAT_104c0360 != 0)
                    DAT_10db6ee8[walker].PlayMotion(1, 0, 5, 1.0f);
            }
            else if (DAT_104c0360 != 0)
                DAT_10db6ee8[walker].PlayMotion(0, 0, 5, 1.0f);
            stepCount++;
        }
        DAT_10c6357c = sidestepSign;
        DAT_10c63580 = stillFrames;
        DAT_10c63588 = sidestepFrames;
        DAT_10c63584 = stepCount;
        DAT_104c4438[0].Copy(anchor);
        DAT_104c4438[1].Copy(goal);
        DAT_104c4438[2].Copy(move);
        DAT_104c4438[3].Copy(sidestep);
    }
    if (DAT_1032d69c == 0 && DAT_10c62ed0 == MAP_TIMED_BATTLE)
    {
        if (DAT_1020b430[0].state > 0)
        {
            DAT_10db6ee8[0].AddMotionSpeed((float)(DAT_1020b430[0].state * 2) + 6.0f);
            DAT_1020b430[0].state--;
        }
    }
    else if (DAT_10c62ed0 == MAP_FOUR_SWITCHES)
    {
        ReleaseStuckSwitchEnemy(0);
        ReleaseStuckSwitchEnemy(1);
        ReleaseStuckSwitchEnemy(2);
        ReleaseStuckSwitchEnemy(3);
    }
    if (DAT_10c6505c != 0)
        Map_UpdateSpecial();
    if (DAT_10c66cf4 == 0xb && DAT_1020b6f0 != 0)
    {
        Vec4 position;
        position.Copy(DAT_10db6ee8[0xb].matrix.GetTranslation());
        position.z += 10.0f;
        DAT_10db6ee8[0xb].SetPosition(position.x, position.y, position.z);
        DAT_10db6ee8[0xb].heading = DAT_10db6ee8[0].heading;
        DAT_10db6ee8[0xb].targetHeading = DAT_10db6ee8[0].targetHeading;
        DAT_10db6ee8[0xb].mode = 0;
    }
    if (DAT_104bf8c0 != 0xf && DAT_104c0394 != 0)
    {
        DAT_11b1ba40(DAT_104c0394);
        DAT_104c0394 = 0;
    }
    if (DAT_10453008[0] == 0)
    {
        if (DAT_1032d69c == 0)
        {
            DAT_10453008[0] = 1;
            DAT_104c4438[0].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        }
    }
    else if (DAT_1032d69c != 0)
        DAT_10453008[0] = 0;
    if (DAT_104c01e0 == 0)
    {
        if (DAT_10c635dc != 0)
        {
            DAT_10c635dc = 0;
            if (DAT_104c01b0 == 0)
            {
                if (DAT_104477b0 != -11)
                {
                    FUN_10001c50(DAT_104477b0);
                    DAT_104477b0 = -11;
                }
                if (DAT_104c01b0 == 0 && DAT_104477ae != -11)
                {
                    FUN_10001c50(DAT_104477ae);
                    DAT_104477ae = -11;
                }
            }
            DAT_10c635d4 = -1;
            DAT_10c635d8 = -1;
        }
    }
    else
        FUN_1004fbe0();
    if (DAT_104c036c != 0)
        FUN_100ba040();
    FUN_1001d920();
    if (DAT_104c0350 == 0)
    {
        FUN_10034fe0();
        DAT_10c636c4 = 1;
    }

    // The three status windows open and close.
    for (int window = 0; window < TEXT_WINDOW_COUNT; window++)
    {
        TextWindow &card = DAT_10255960[window];
        if (card.kind != 0)
        {
            int percent;
            if (card.m_0020 == 1)
            {
                card.m_0018++;
                if (card.m_001c < card.m_0018)
                    card.m_0018 = card.m_001c;
                percent = card.m_0018 * 100 / card.m_001c;
            }
            else
            {
                card.m_0018--;
                if (card.m_0018 < 0)
                {
                    card.kind = 0;
                    DAT_10c63574 = 0;
                    if (card.picture > -1)
                    {
                        FUN_10174c70(DAT_1049efd8[card.picture].slot, 1);
                        card.picture = -1;
                    }
                }
                percent = card.m_0018 * 100 / card.m_001c;
            }
            card.progress = percent;
        }
        if (card.kind == TEXT_WINDOW_CARD && card.progress < TEXT_WINDOW_FADE_PERCENT && (card.progress & 1) == 0)
        {
            DAT_10f08ab0.M_10161050();
            DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
        }
    }

    if (DAT_104c0350 == 0)
    {
        memset(DAT_10253a58, 0, sizeof(DAT_10253a58));
        for (int index = 0; index < CHAR_SLOT_COUNT; index++)
            DAT_1020e138[index] = -1;
        DAT_104c0350 = 1;
        for (int owner = 0; owner < 6; owner++)
            DAT_102108d0[owner] = 0;
        for (int delay = 0; delay < 10; delay++)
            DAT_10447310[delay] = 0;
        for (int index = 0; index < CHAR_SLOT_COUNT; index++)
            DAT_10db6ee8[index].m_0294 = 0;
    }

    // The people and the party: slots 0x3a..0x5b.
    for (int index = PEOPLE_SLOT_FIRST; index < PEOPLE_SLOT_END; index++)
    {
        FUN_1001d7d0(index);
        CharSlot &slot = DAT_10daaae8[index];
        AnimModel &model = DAT_10db6ee8[index];
        int kind = slot.kind;
        ClampKindCounters(index, kind);
        if (DAT_104c037c == DAT_104bf8b8 && kind == CHAR_KIND_ENEMY && DAT_1020b430[index].behaviour == 0x1b &&
            DAT_1020b430[index].state == 1)
            Char_UpdateByKind(index);
        if (DAT_10210d30[index] != 0 && DAT_1020b430[index].behaviour == 0xb && slot.kind == CHAR_KIND_ENEMY)
        {
            DAT_1020b430[index].behaviour = 0;
            DAT_10c67f60.Set(1.0f, 1.0f, 1.0f, 1.0f);
            DAT_10daa800.Copy((DAT_10c67f60 - DAT_10da90d0) * 0.06666667f);
            DAT_1042da68 = 15;
        }
        if (DAT_10c62ed0 == MAP_SHORT_START && DAT_10315350[index] != 0 && slot.kind == CHAR_KIND_ENEMY &&
            DAT_1020b430[index].behaviour == 0x26 && (DAT_1020b430[index].values[9] != 0 || DAT_1021bdf8[index][8] != 0))
        {
            DAT_1020b430[index].values[9] = 0;
            DAT_1021bdf8[index][8] = 0;
        }

        // A character that was told to start over (DAT_1021f3c0 = 3): everything of it is reset.
        if (DAT_10254090[index] == 1)
        {
            if (DAT_1021f3c0[index] == 3)
            {
                kind = slot.kind;
                if (kind == 9 || kind == 0xb || kind == 0x28 || kind == 0x2a || kind == 0x21 || kind == 4 || kind == 0xe ||
                    kind == 0x54 || kind == 0x48)
                    model.mode = 0;
                if (index == PEOPLE_SLOT_FIRST)
                {
                    for (int n = 0x22; n != 0; n--)
                    {
                        DAT_1021c620 = 0;
                        DAT_1021c624 = 0;
                        DAT_1021c628 = 0;
                        DAT_1021c62c = 0;
                        DAT_1021c630 = 0;
                    }
                    for (int n = 0x32; n != 0; n--)
                        DAT_10254598[PEOPLE_SLOT_FIRST] = -1;
                    for (Vec4 *colour = DAT_10edbd58; colour < DAT_10edbd58 + 6; colour++)
                        colour->Fill(0.0f);
                    for (int owner = 0; owner < 6; owner++)
                    {
                        DAT_10f08880[owner].SetIdentity();
                        DAT_102108d0[owner] = 0;
                    }
                    for (int delay = 0; delay < 10; delay++)
                        DAT_10447310[delay] = 0;
                    for (int sound = 0; sound < 10; sound++)
                        DAT_1021a398[sound] = 0;
                }
                DAT_10254598[index] = 0x5b;
                DAT_1030b8f8[index] = 0;
                ClearBehaviour(index);
                DAT_103086b0[index] = 1;
                DAT_1020d540[index] = 0;
                DAT_1021b4c8[index] = (char)(index % 5) - 1;
                DAT_10867e18[index].Fill(0.0f);
                DAT_10256420[index] = 0;
                model.m_0294 = 0;
                DAT_10253a58[index] = 0;
                DAT_1042dba8[index] = 0;
                DAT_102199b8[index] = 0;
                DAT_1042dea8[index] = 0;
                DAT_10255f10[index] = slot.stats[2];
                model.shadowOpacity = 0xf;
                DAT_1021f940[index] = 0;
                DAT_1020e138[index] = -1;
                if (slot.kind == 0x19)
                    for (int n = 0; n < 0x32; n++)
                        DAT_10254598[n] = 0;
                DAT_104c4438[index].Copy(model.matrix.GetTranslation());
                int code = slot.routeRecord;
                DAT_1030b8f8[index] = 0;
                DAT_104483e8[index] = 0;
                DAT_10453008[index] = 0;
                if (code < PARTNER_CODE_MAX + 1)
                {
                    if (code < 0x65)
                    {
                        if (code < 0)
                        {
                            DAT_10210d30[index] = 2;
                            DAT_10254090[index] = 0;
                            DAT_104c4438[index].Copy(model.matrix.GetTranslation());
                            model.SetPosition(SLOT_NOT_IN_PLAY, 0.0f, 0.0f);
                            if (FUN_1013f880() % 5 == 0)
                            {
                                for (int partner = PEOPLE_SLOT_FIRST; partner < index && DAT_10db6ee8[partner].root != 0; partner++)
                                {
                                    if (DAT_10daaae8[partner].routeRecord == -slot.routeRecord &&
                                        DAT_104c4438[partner].x >= -20000.0f && FUN_1013f880() % 2 == 0)
                                    {
                                        DAT_10254090[index] = DAT_10254090[partner];
                                        DAT_10453008[partner] = index + PARTNER_LINK_BASE;
                                        DAT_104c4438[index].Copy(DAT_104c4438[partner]);
                                        DAT_10db6ee8[index].SetPosition(DAT_10db6ee8[partner].matrix.translation.x,
                                                                        DAT_10db6ee8[partner].matrix.translation.y,
                                                                        DAT_10db6ee8[partner].matrix.translation.z);
                                        DAT_10db6ee8[partner].SetPosition(PARTNER_PARK_X,
                                                                          DAT_10db6ee8[partner].matrix.translation.y,
                                                                          DAT_10db6ee8[partner].matrix.translation.z);
                                        DAT_1021b4c8[index] = DAT_1021b4c8[partner];
                                        DAT_10210d30[index] = 0;
                                        DAT_10210d30[partner] = 2;
                                        DAT_1020d540[index] = DAT_1020d540[partner];
                                        DAT_10867e18[index].Copy(DAT_10867e18[partner]);
                                        DAT_10254390[index] = DAT_10254390[partner];
                                        DAT_10256420[index] = DAT_10256420[partner];
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        slot.m_00c0 = 0;
                        slot.m_00c4 = 0;
                        slot.m_00c8 = 0;
                        slot.money = code % 100;
                    }
                }
                else
                {
                    slot.routeRecord = code - 500;
                    if (FUN_1013f880() % 1000 < 100)
                    {
                        slot.m_00c0 = 0;
                        slot.m_00c4 = 0;
                        slot.m_00c8 = 0;
                        slot.money = slot.routeRecord % 100;
                    }
                }
            }
            if (DAT_10254090[index] == 1 && DAT_1021f3c0[index] == 3)
            {
                if (model.matrix.translation.x >= -20000.0f)
                    DAT_10453008[index] = -1;
                else
                {
                    DAT_10453008[index] = PARTNER_PARKED_WAIT;
                    DAT_10254090[index] = 2;
                }
                DAT_10254090[index] = 2;
            }
        }
        if (DAT_10210d30[index] == 5)
        {
            DAT_10daaa80.RemoveOwnedBy(index);
            DAT_10daaa80.RemoveOwnedBy(index + EFFECT_OWNER_BASE);
        }
        if (DAT_10254090[index] == 0)
        {
            if (DAT_10210d30[index] == 0 || slot.kind == 0x2d || slot.kind == 0x2b || slot.kind < -1 ||
                DAT_1021f3c0[index] != 3 || DAT_104bfe1c != 0 || DAT_104bfe6c != 0 || DAT_104bfeb0 != 0 ||
                DAT_104c0094 != 0 || DAT_104bfdf0 != 0 || DAT_104c0135 != 0 || DAT_10c62ed0 == MAP_TOKATYU ||
                DAT_10c62ed0 == MAP_TOKATYU_B || slot.kind == 0x21)
            {
                int wait = DAT_10453008[index];
                if (wait < 1 && DAT_1021f3c0[index] == 3 && DAT_104bfe1c == 0 && DAT_104bfe6c == 0 &&
                    DAT_104bfeb0 == 0 && DAT_104c0094 == 0 && DAT_104bfdf0 == 0 && DAT_104c0135 == 0 &&
                    wait > PARTNER_PARKED_WAIT)
                {
                    DAT_10453008[index] = wait + 1;
                    model.alpha = (float)(wait + 1) / 30.0f + 1.0f;
                }
            }
            else
            {
                if ((slot.stats[STAT_BUSY] == 0 && slot.routeRecord > -1) || (slot.kind == 0x11 && slot.routeRecord > -1))
                {
                    if (index > 0x58)
                        SetFlagBit(DAT_10c63ffc * 2 + 0x3e9, index - 0x59);
                    else
                        SetFlagBit(DAT_10c63ffc * 2 + 1000, index - PEOPLE_SLOT_FIRST);
                }
                if (slot.kind != 6 && DAT_104c4438[index].x > -50000.0f)
                {
                    if (DAT_10453008[index] > -1000 && DAT_10453008[index] < 10000)
                        DAT_10453008[index]++;
                    int timer = DAT_10453008[index];
                    if (timer <= PARTNER_LINK_BASE)
                    {
                        int delayClass = slot.m_00d8 % 10 - 1;
                        if (slot.m_00d8 % 10 != 0 && DAT_101d6564[delayClass > 0 ? delayClass : 0] <= timer &&
                            DAT_10210d30[index] == 2)
                        {
                            int target = index;
                            if (FUN_1013f880() % 10000 < 500 && slot.routeRecord != 0)
                            {
                                for (int other = PEOPLE_SLOT_FIRST; other < PEOPLE_SLOT_END; other++)
                                {
                                    if (DAT_10210d30[other] == 2 && DAT_10453008[other] < -1000 &&
                                        DAT_10daaae8[other].routeRecord == -slot.routeRecord)
                                    {
                                        DAT_10453008[index] = other + PARTNER_LINK_BASE;
                                        DAT_104c4438[other].Copy(DAT_104c4438[index]);
                                        target = other;
                                        DAT_1021b4c8[other] = DAT_1021b4c8[index];
                                        DAT_1020d540[other] = DAT_1020d540[index];
                                        DAT_10256420[other] = DAT_10256420[index];
                                        DAT_10867e18[other].Copy(DAT_10867e18[index]);
                                        DAT_10254390[other] = DAT_10254390[index];
                                        break;
                                    }
                                }
                            }
                            AnimModel &woken = DAT_10db6ee8[target];
                            woken.SetPosition(DAT_104c4438[target].x, DAT_104c4438[target].y, DAT_104c4438[target].z);
                            woken.matrix.translation.x = DAT_104c4438[target].x;
                            woken.matrix.translation.y = DAT_104c4438[target].y;
                            woken.matrix.translation.z = DAT_104c4438[target].z;
                            if ((woken.matrix.GetTranslation() - DAT_10db6ee8[0].matrix.GetTranslation()).Length2D() > 400.0f)
                            {
                                Vec4 spot;
                                DAT_10453008[target] = PARTNER_WOKEN_WAIT;
                                if (woken.root != 0)
                                {
                                    woken.root->M_10194630(target, NODE_MASK_BODY);
                                    woken.root->M_101946b0(NODE_STATE_FE, NODE_MASK_BODY);
                                    woken.root->M_10194730(NODE_STATE_2F, NODE_MASK_BODY);
                                    woken.root->M_10194630(0, NODE_MASK_SHADOW);
                                    woken.root->M_101946b0(NODE_STATE_FF, NODE_MASK_SHADOW);
                                    woken.root->M_10194730(NODE_STATE_2F, NODE_MASK_SHADOW);
                                }
                                int lifts = 0;
                                while (woken.IsClearOfMapLimited(0) == 0 && lifts < WAKE_LIFT_TRIES)
                                {
                                    spot.Copy(woken.matrix.GetTranslation());
                                    lifts++;
                                    woken.SetPosition(woken.matrix.translation.x, woken.matrix.translation.y,
                                                      woken.matrix.translation.z + 5.0f);
                                    woken.matrix.translation.x = spot.x;
                                    woken.matrix.translation.y = spot.y;
                                    woken.matrix.translation.z = spot.z;
                                }
                                int maxHealth = DAT_10daaae8[target].stats[STAT_MAX_HEALTH];
                                DAT_10210d30[target] = 0;
                                woken.alpha = 0.0f;
                                DAT_10daaae8[target].stats[STAT_HEALTH] = maxHealth;
                                if (maxHealth < 1)
                                    DAT_10daaae8[target].stats[STAT_HEALTH] = 5;
                                ClearBehaviour(target);
                                DAT_10daaae8[target].coins = 0;
                                DAT_10254598[target] = 0x5b;
                                DAT_1030b8f8[target] = 0;
                                DAT_103086b0[target] = 1;
                                DAT_102199b8[index] = 2;
                                model.alpha = 0.0f;
                                DAT_1042dea8[index] = 0x5a;
                                DAT_1030c480[index] = 0;
                                DAT_10467510[index] = 0;
                                model.m_02a8 = 0;
                                model.holdState = HOLD_FREE;
                                model.holdWeight = 0.0f;
                                model.m_022c.Fill(0.0f);
                                DAT_10210d30[index] = 0;
                                DAT_104686b0[index] = 0;
                                DAT_1020d4c0[index] = 0;
                                DAT_102550e0[index] = 0;
                                DAT_10218fc0[index] = 0;
                                DAT_10210b30[index] = 0;
                                DAT_10467ba0[index] = 0;
                                DAT_102239d0[index] = 0;
                                DAT_10254090[index] = 1;
                                DAT_10802bf8[index].position.Fill(0.0f);
                                DAT_10802bf8[index].step.Fill(0.0f);
                                int look = slot.stats[9];
                                DAT_10315350[index] = 0;
                                DAT_1021f6c0[index] = 0;
                                DAT_102548c0[index] = 0;
                                DAT_10219e40[index] = 0;
                                DAT_1042d9e8[index] = 0;
                                if (look < 1000 && ((slot.kind != 0x31 && DAT_1030b668[index] != 0x31) || look % 100 == 0))
                                {
                                    if (slot.kind == 0x10 || DAT_1030b668[index] == 0x10)
                                    {
                                        Node *armour = model.root->Find("@EN_016ar.lwo", 0);
                                        if (armour != 0)
                                            armour->ClearFlags(NODE_FLAGS_HIDE);
                                    }
                                }
                                else
                                {
                                    if (slot.kind == 0x10 || DAT_1030b668[index] == 0x10)
                                    {
                                        Node *armour = model.root->Find("@EN_016ar.lwo", 0);
                                        if (armour != 0)
                                            armour->SetFlags(NODE_FLAGS_HIDE);
                                    }
                                    int weapon = 0;
                                    if (slot.kind == 0x31 || DAT_1030b668[index] == 0x31)
                                        weapon = slot.stats[9] % 100;
                                    FUN_100404e0(index, slot.stats[9] / 10000000 % 100, slot.stats[9] / 100000 % 100,
                                                 slot.stats[9] / 1000 % 100, weapon, 0, 0);
                                }
                            }
                            else
                                woken.SetPosition(SLOT_NOT_IN_PLAY, 0.0f, 0.0f);
                        }
                    }
                    else if (DAT_10210d30[timer - PARTNER_LINK_BASE] == 2)
                    {
                        DAT_10254090[index] = DAT_10254090[timer - PARTNER_LINK_BASE];
                        DAT_1021b4c8[index] = DAT_1021b4c8[timer - PARTNER_LINK_BASE];
                        DAT_1020d540[index] = DAT_1020d540[timer - PARTNER_LINK_BASE];
                        DAT_10256420[index] = DAT_10256420[timer - PARTNER_LINK_BASE];
                        DAT_10867e18[index].Copy(DAT_10867e18[timer - PARTNER_LINK_BASE]);
                        int partner = DAT_10453008[index] - PARTNER_LINK_BASE;
                        DAT_10453008[partner] = PARTNER_PARKED_WAIT;
                        DAT_10254390[index] = DAT_10254390[partner];
                        DAT_104c4438[partner].x = PARTNER_PARK_X;
                        DAT_10453008[index] = PARTNER_WOKEN_WAIT;
                    }
                }
            }
        }
        if (slot.kind == -0x65)
            DAT_1020b46c[index].value = 0;
        if (DAT_1042d9e8[index] != 0 && DAT_10219e40[index] > -1)
        {
            int shots = DAT_102548c0[index];
            DAT_1042d9e8[index] = 0;
            if (shots != 0 && DAT_10219e40[index] == 0)
                FUN_100615f0(index, 0, 0);
        }
        if (DAT_10219e40[index] > 0)
            DAT_10219e40[index]--;
        if (DAT_10210d30[index] == STATE_VISIBILITY_CHECK)
        {
            Vec4 position;
            DAT_1021b4c8[index]++;
            position.Copy(model.matrix.GetTranslation());
            Vec4 centre = position;
            int visible = 1;
            bool seen = false;
            if (DAT_10255f08->visibilityCells != 0)
            {
                Box2 *box = DAT_10255f08->visibilityCells->boxes;
                for (int cell = 0; cell < 0x40; cell++)
                {
                    if (DAT_11afffe8[cell] != 0 && centre.InBox(box) != 0)
                    {
                        seen = true;
                        break;
                    }
                    box++;
                }
            }
            if (!seen)
            {
                if (DAT_10255f08 != 0 && DAT_10255f08->visibilityCells != 0)
                {
                    Box2 *box = DAT_10255f08->visibilityCells->boxes;
                    for (int cell = 0x40; cell != 0; cell--)
                    {
                        if (centre.InBox(box) != 0)
                            visible = 2;
                        box++;
                    }
                    if (visible != 2)
                        FUN_100627d0(index);
                }
                else
                {
                    Vec4 from;
                    Vec4 to;
                    Vec4 spare;
                    Vec4 hit;
                    hit.Fill(0.0f);
                    hit.x = SLOT_NOT_IN_PLAY;
                    if (model.headNode != 0)
                        from.Copy(model.headNode->worldMatrix.GetTranslation());
                    else
                    {
                        from.x = model.matrix.translation.x;
                        from.y = model.matrix.translation.y;
                        from.z = model.matrix.translation.z + 30.0f;
                        from.w = model.matrix._44;
                    }
                    to.Copy(from);
                    to.z -= 100000.0f;
                    if (DAT_10255f08 == 0 || DAT_10255f08->Collide(0, 0x4000000, &to, &from, &hit) == 0)
                    {
                        Node *loaded = model.loadedRoot;
                        if (FUN_101ae4d0(&(loaded->bounds + loaded->worldMatrix.GetTranslation()), &DAT_108392d8,
                                         loaded->cullMargin) == 0)
                            FUN_100627d0(index);
                    }
                }
            }
        }
    }

    // The player (slot 0) is turned by the scripted spin (DAT_104c038c > 0) or stands on the slot named by it (< 0).
    if (DAT_104c038c > 0.0f)
    {
        DAT_104c038c += 0.667f;
        DAT_10db6ee8[0].targetHeading = DAT_104c038c / 180.0f * PI_UPPER + DAT_10db6ee8[0].targetHeading;
        DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
        DAT_10db6ee8[0].SetPosition(DAT_10db6ee8[0].matrix.translation.x, DAT_10db6ee8[0].matrix.translation.y,
                                    DAT_10db6ee8[0].matrix.translation.z - 3.0f);
    }
    else if (DAT_104c038c < 0.0f)
    {
        int rider = -(int)DAT_104c038c;
        DAT_10db6ee8[rider].SetPosition(DAT_10db6ee8[rider].matrix.translation.x,
                                        DAT_10db6ee8[rider].matrix.translation.y,
                                        DAT_10db6ee8[rider].matrix.translation.z - 3.0f);
    }

    // The sparks round the player while the hurt effect lasts (DAT_10c66d00 counts down).
    if (DAT_10c66d00 != 0 && DAT_104bfe6c == 0 && DAT_104bfe1c == 0 && DAT_104bfdf0 == 0 && DAT_104bfe78 == 0)
    {
        DAT_10c66d00--;
        if (DAT_10c66d00 % 5 == 0)
        {
            Matrix4 matrix;
            Vec4 position;
            matrix.SetIdentity();
            matrix.SetTranslation(DAT_10db6ee8[0].matrix.GetTranslation());
            matrix.translation.z = (float)(FUN_1013f880() % 0x96 - 0x4b) + matrix.translation.z + 100.0f;
            matrix.translation.x = (float)(FUN_1013f880() % 0x96 - 0x4b) + matrix.translation.x;
            matrix.translation.y = (float)(FUN_1013f880() % 0x96 - 0x4b) + matrix.translation.y;
            DAT_10daaa80.Start(0x76f, position, -1, -1, 0, 0, 0, &matrix, 0, 0);
        }
        if (DAT_10daaae8[0].stats[STAT_HEALTH] < 1)
            DAT_10c66d00 = 0;
    }

    // Countdowns of the attached matrices, of the delayed sounds and of the delayed events.
    for (int owner = 0; owner < 6; owner++)
    {
        if (DAT_102108d0[owner] > 0)
            DAT_102108d0[owner]--;
        else
            DAT_102108d0[owner] = 0;
    }
    for (int delay = 0; delay < 10; delay++)
    {
        if (DAT_10447310[delay] > 0 && --DAT_10447310[delay] == 0)
        {
            if (DAT_1042d870[delay] != 0)
                FUN_1003ad20(DAT_1021a398[delay], &DAT_10f08a00[delay], 1.0f);
            else
                FUN_1003ad20(DAT_1021a398[delay], 0, 1.0f);
        }
    }
    if (DAT_104bfe6c == 0 && DAT_104bfe1c == 0 && DAT_104bfdf0 == 0 && DAT_104bfe78 == 0)
    {
        for (int event = 0; event < 10; event++)
        {
            if (DAT_1020dcd0[event] > 0 && --DAT_1020dcd0[event] == 0)
            {
                if (DAT_1021a370[event] != 0)
                    DAT_10874848.RegisterEvent(DAT_1021a370[event]);
                DAT_1021a370[event] = 0;
                DAT_1020dcd0[event] = 0;
            }
        }
    }
    if (DAT_104c03b4 > 0 && ((DAT_10c655e0 != 0 && DAT_104c03b4 > DAT_104c01e4) ||
                             (DAT_10c655e0 == 0 && DAT_104c03b4 < DAT_104c01e4)))
    {
        DAT_104c03b4 = 0;
        DAT_10874848.RegisterEvent(0x1f2);
    }

    // The character named by the player's state turns to face the one named after it.
    if (DAT_1020b430[0].lookActive != 0)
    {
        Vec4 offset;
        Vec4 from;
        offset.Copy(DAT_10db6ee8[DAT_1020b430[0].lookTarget].matrix.GetTranslation());
        from.Copy(DAT_10db6ee8[DAT_1020b430[0].state].matrix.GetTranslation());
        offset.Copy(offset - from);
        DAT_10db6ee8[DAT_1020b430[0].state].targetHeading = Atan2(offset.x, offset.y) + PI_UPPER;
    }
    if (DAT_102028d4 > -1)
        DAT_102028d4++;

    // The camera: DAT_104c0354 selects how the camera target (DAT_118e6cb8) and position (DAT_118e69e8) move.
    if (DAT_104c0354 == 0)
    {
        if (DAT_104c0358 != 0)
        {
            FUN_1016a8e0();
            DAT_104c0358 = 0;
        }
    }
    else if (DAT_104c0354 == 1)
    {
        if (DAT_104c0358 == 0)
        {
            FUN_1016a8d0();
            DAT_104c0358 = 1;
        }
        DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8) * CAMERA_FOLLOW_GOAL +
                          (DAT_10f08aa0 - DAT_118e6cb8) * CAMERA_FOLLOW_KEEP);
        DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * CAMERA_FOLLOW_GOAL +
                          (DAT_104d29d8 - DAT_118e69e8) * CAMERA_FOLLOW_KEEP);
        DAT_118e6c78.Copy(DAT_118e6cb8);
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_104d29d8.Copy(DAT_118e69e8);
        DAT_10f08aa0.Copy(DAT_118e6cb8);
        if ((DAT_118e69e8 - DAT_10866d98).Length() < CAMERA_NEAR_DISTANCE &&
            (DAT_118e6cb8 - DAT_10edbd48).Length() < CAMERA_NEAR_DISTANCE)
            DAT_104c0354 = 4;
    }
    else if (DAT_104c0354 == 2)
    {
        if (DAT_104c0358 == 0)
        {
            FUN_1016a8d0();
            DAT_104c0358 = 1;
        }
        DAT_11af3e28.Copy(DAT_10866d98);
        DAT_118e6c78.Copy(DAT_10edbd48);
        DAT_118e69e8.Copy(DAT_10866d98);
        DAT_118e6cb8.Copy(DAT_10edbd48);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
    }
    else if (DAT_104c0354 == 3)
        DAT_11af3e28.z = DAT_10866d98.z;
    else if (DAT_104c0354 == 4)
    {
    }
    else if (DAT_104c0354 == 5)
    {
        if (DAT_10ff1df0 == 0)
            DAT_104c0354 = 0;
        Vec4 targetStep;
        Vec4 eyeOffset;
        Vec4 eyeToGoal;
        Vec4 goalOffset;
        targetStep.Copy(DAT_118e6c78 - DAT_10f08aa0);
        eyeOffset.Copy(DAT_104d29d8 - DAT_10f08aa0);
        eyeToGoal.Copy(DAT_11af3e28 - DAT_118e6c78);
        goalOffset.Copy(DAT_10866d98 - DAT_10edbd48);
        float goalDistance = goalOffset.LengthSquared3();
        float eyeDistance = eyeToGoal.LengthSquared3();
        eyeOffset.Length2D();
        if (goalDistance > eyeDistance)
        {
            float targetAngle = Atan2(targetStep.x, targetStep.y);
            float eyeAngle = Atan2(eyeOffset.x, eyeOffset.y);
            float turn = FoldAngle(eyeAngle - targetAngle);
            if (turn < 0.0f)
                eyeOffset.RotateZ(turn > -CAMERA_TURN_STEP ? -turn : CAMERA_TURN_STEP);
            else
                eyeOffset.RotateZ(turn < CAMERA_TURN_STEP ? -turn : -CAMERA_TURN_STEP);
            DAT_104d29d8.Copy(DAT_10f08aa0 + eyeOffset);
            DAT_11af3f48.Copy(DAT_104d29d8);
            DAT_11af3e28.Copy(DAT_104d29d8);
        }
        DAT_104d29d8.Copy(DAT_11af3e28);
        DAT_10f08aa0.Copy(DAT_118e6c78);
    }
    else if (DAT_104c0354 == 6)
    {
        DAT_10db6ee8[0].matrix.translation.x = DAT_10866d98.x;
        DAT_10db6ee8[0].matrix.translation.y = DAT_10866d98.y;
        DAT_10db6ee8[0].matrix.translation.z = DAT_10866d98.z;
    }
    else if (DAT_104c0354 == 7)
    {
        if (DAT_104c0358 == 0)
        {
            FUN_1016a8d0();
            DAT_104c0358 = 1;
        }
        DAT_11af3e28.Copy(DAT_118e69e8 * CAMERA_FOLLOW_KEEP + DAT_10866d98 * CAMERA_FOLLOW_GOAL);
        DAT_118e69e8.Copy(DAT_11af3e28);
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_10edbd48.Copy(DAT_10edbd48 * CAMERA_FOLLOW_KEEP +
                          DAT_10db6ee8[0].matrix.GetTranslation() * CAMERA_FOLLOW_GOAL);
        DAT_118e6c78.Copy(DAT_10edbd48);
        DAT_118e6cb8.Copy(DAT_10edbd48);
        DAT_10f668e8.Copy(DAT_118e6cb8);
    }
    else if (DAT_104c0354 == 8)
    {
        if (DAT_104c0358 == 0)
        {
            FUN_1016a8d0();
            DAT_104c0358 = 1;
        }
        DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10868838[0][0].GetTranslation() - DAT_118e6cb8) * CAMERA_FOLLOW_GOAL +
                          (DAT_10f08aa0 - DAT_118e6cb8) * CAMERA_FOLLOW_KEEP);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_118e6c78.Copy(DAT_118e69e8);
        DAT_10f08aa0.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * CAMERA_FOLLOW_GOAL +
                          (DAT_104d29d8 - DAT_118e69e8) * CAMERA_FOLLOW_KEEP);
        DAT_104d29d8.Copy(DAT_118e69e8);
        if (DAT_10255160[0][0] > 0.0f)
        {
            DAT_10255160[0][0] += 0.015f;
            DAT_10868838[0][0].translation.z += DAT_10255160[0][0];
        }
    }
    else if (DAT_104c0354 == 9)
        FUN_100184e0();
    else if (DAT_104c0354 == 10)
        FUN_10018870();
    else if (DAT_104c0354 == 0xb)
        FUN_10018fb0();
    else if (DAT_104c0354 == 0xc)
        FUN_10019210();
    else if (DAT_104c0354 == 0xd)
    {
        DAT_10802ac8.Copy(DAT_10866d98);
        FUN_100165e0(1);
    }
    else if (DAT_104c0354 == 0xe)
        FUN_10019310();
    else if (DAT_104c0354 == 0xf)
        FUN_10019670();
    else if (DAT_104c0354 == 0x10)
        FUN_10019d80();
    else if (DAT_104c0354 == 0x11)
    {
        int chased = (int)(DAT_10866d98.x + 0.5f);
        FUN_10035050(&DAT_10db6ee8[0].matrix.GetTranslation(), &DAT_10db6ee8[chased].matrix.GetTranslation(), chased);
    }
    else if (DAT_104c0354 == 0x12)
        FUN_1001a0d0();

    // Heading of the camera, and how far each person turns and stands from it.
    Vec4 aim;
    aim.Copy(DAT_118e6cb8);
    float aimHeading = Atan2(aim.x - DAT_118e69e8.x, aim.y - DAT_118e69e8.y);

    // How far each person stands from the player in the line of sight, weighted by how far the camera must turn.
    for (int index = PEOPLE_SLOT_FIRST; index < PEOPLE_SLOT_END; index++)
    {
        AnimModel &model = DAT_10db6ee8[index];
        if (DAT_10210d30[index] != 2 && model.root != 0 && DAT_10254090[index] != 1 && DAT_1020d540[index] != 0)
        {
            if (model.matrix.translation.x > -20000.0f)
            {
                float turn = FoldAngle(aimHeading - Atan2(model.matrix.translation.x - DAT_118e69e8.x,
                                                          model.matrix.translation.y - DAT_118e69e8.y));
                Vec4 offset = aim - model.matrix.GetTranslation();
                offset.z = offset.z * LOOK_HEIGHT_SCALE;
                StoreLookDistance(index, offset, turn);
            }
            else
            {
                bool inView = model.shadowOpacity != 0;
                if (!inView)
                {
                    Vec4 probe = DAT_10867e18[index];
                    if (DAT_10255f08->visibilityCells == 0)
                        inView = true;
                    else
                    {
                        Box2 *box = DAT_10255f08->visibilityCells->boxes;
                        for (int cell = 0; cell < 0x40; cell++)
                        {
                            if (DAT_11afffe8[cell] != 0 && probe.InBox(box) != 0)
                            {
                                inView = true;
                                break;
                            }
                            box++;
                        }
                    }
                }
                if (!inView)
                {
                    DAT_102199b8[index] = 0;
                    continue;
                }
                float turn = FoldAngle(aimHeading - Atan2(DAT_10867e18[index].x - DAT_118e69e8.x,
                                                          DAT_10867e18[index].y - DAT_118e69e8.y));
                Vec4 offset = aim - DAT_10867e18[index];
                offset.z = offset.z * LOOK_HEIGHT_SCALE;
                StoreLookDistance(index, offset, turn);
            }
        }
        else if (DAT_10210d30[index] == 2 && DAT_10daaae8[index].kind == 0xf && DAT_1020b430[index].podRipeness != 0)
            Enemy_PodLobber(index);
        if (DAT_10daaae8[index].kind == 0x37 && DAT_104bfe6c == 0 && DAT_104bfe1c == 0 && DAT_104bfdf0 == 0 &&
            DAT_104bfe78 == 0 && DAT_10c62ed0 == MAP_TOKATYU && DAT_104c0135 == 0)
        {
            if (DAT_104c0390 == 0)
            {
                DAT_104c0390 = (Node *)DAT_11b0b938(0x18a0, "tsm_lprootnode");
                DAT_104c0390->Init();
                DAT_104c0390->LoadIt3(FUN_1013f8a0("tokatyu.it3", 0), 0, 0);
            }
            Bs003_Update(index);
        }
        if (DAT_10daaae8[index].kind == 0x32 && DAT_104bfe6c == 0 && DAT_104bfe1c == 0 && DAT_104bfdf0 == 0 &&
            DAT_104bfe78 == 0 && DAT_104c0135 == 0 && DAT_104c0094 == 0 && model.matrix.translation.x > -20000.0f &&
            (DAT_10210d30[index] == 0 || DAT_1021f9c0[index] == 0))
            NeoGalaxy_Update(index);
    }

    // The crowd weights: a person keeps a level (0..1, DAT_102199b8 phase) while it is one of the nearest in its group.
    for (int index = PEOPLE_SLOT_FIRST; index < PEOPLE_SLOT_END; index++)
    {
        AnimModel &model = DAT_10db6ee8[index];
        if (DAT_10210d30[index] == 2 || model.root == 0 || DAT_10254090[index] != 0 || DAT_1020d540[index] == 0)
            DAT_102199b8[index] = 0;
        else
        {
            char seen[8] = {0};
            int closer = 0;
            int other = 0;
            if ((DAT_1042dea8[index] == 0 && DAT_102199b8[index] < 2) || DAT_10210d30[index] > 0)
            {
                for (other = PEOPLE_SLOT_FIRST; other < PEOPLE_SLOT_END; other++)
                {
                    int group = DAT_10255f10[index];
                    if (other != index && group != -1 && DAT_10210d30[other] == 0 &&
                        (DAT_10daaae8[other].stats[STAT_HEALTH] > 0 || DAT_10210d30[other] != 2) &&
                        DAT_10db6ee8[other].root != 0 && DAT_10254090[other] == 0 && DAT_1020d540[other] != 0 &&
                        !(group > 0 && group < 100 && group == DAT_10255f10[other] && DAT_102199b8[other] != 0) &&
                        DAT_102199b8[other] <= 0x10 && DAT_10255f10[other] != -1)
                    {
                        bool distinct = true;
                        if (DAT_10255f10[other] > 0 && DAT_10255f10[other] < 100)
                        {
                            for (int n = 0; n < 8; n++)
                            {
                                if (seen[n] == 0)
                                {
                                    seen[n] = (char)DAT_10255f10[other];
                                    break;
                                }
                                if (seen[n] == DAT_10255f10[other])
                                {
                                    distinct = false;
                                    break;
                                }
                            }
                        }
                        if (DAT_1021b728[index] - group > DAT_1021b728[other] - DAT_10255f10[other] && distinct)
                            closer++;
                        if (closer >= DAT_104bfe80 * 2 + 4 && DAT_1032d69c != 0)
                        {
                            if (DAT_102199b8[index] == 1)
                            {
                                DAT_102199b8[index] = 0x11;
                                DAT_1042dea8[index] = 0x2d;
                            }
                            break;
                        }
                    }
                }
                if (other == PEOPLE_SLOT_END && DAT_102199b8[index] == 0)
                {
                    DAT_102199b8[index] = 2;
                    DAT_1042dea8[index] = 300;
                }
            }
            if (DAT_1042dea8[index] > 0)
                DAT_1042dea8[index]--;
            int phase = DAT_102199b8[index];
            if (phase > 1)
            {
                float level = phase > 0x10 ? (float)((0x1f - phase) % 0xf) : (float)((phase - 2) % 0xf);
                DAT_102199b8[index] = phase + 1;
                model.alpha = level / 15.0f;
                if (phase + 1 == 0x11)
                {
                    DAT_102199b8[index] = 1;
                    model.alpha = 1.0f;
                }
                else if (phase + 1 == 0x20)
                {
                    model.alpha = 0.0f;
                    DAT_102199b8[index] = 0;
                }
            }
        }
    }
    for (int index = PEOPLE_SLOT_FIRST; index < PEOPLE_SLOT_END; index++)
    {
        AnimModel &model = DAT_10db6ee8[index];
        int group = DAT_10255f10[index];
        if (group > 0 && group < 100 && DAT_10210d30[index] != 2 && model.root != 0 && DAT_10254090[index] == 0 &&
            DAT_1020d540[index] != 0)
        {
            int other;
            for (other = PEOPLE_SLOT_FIRST; other < PEOPLE_SLOT_END; other++)
            {
                if (other != index && DAT_10210d30[other] == 0 &&
                    (DAT_10daaae8[other].stats[STAT_HEALTH] > 0 || DAT_10210d30[other] != 2) &&
                    DAT_10db6ee8[other].root != 0 && DAT_10254090[other] == 0 && DAT_1020d540[other] != 0 &&
                    group == DAT_10255f10[other] && DAT_102199b8[other] < 0x11 && DAT_102199b8[other] != 0 &&
                    DAT_1021b728[index] - group > DAT_1021b728[other] - DAT_10255f10[other])
                    break;
            }
            if (other != PEOPLE_SLOT_END && DAT_102199b8[index] == 0)
            {
                DAT_102199b8[index] = 2;
                DAT_1042dea8[index] = 300;
            }
            if (DAT_1042dea8[index] > 0)
                DAT_1042dea8[index]--;
            int phase = DAT_102199b8[index];
            if (phase > 1)
            {
                if (phase > 0x10)
                    model.alpha = (float)((0x1f - phase) % 0xf) / 15.0f;
                else
                    model.alpha = (float)((phase - 2) % 0xf) / 15.0f;
                DAT_102199b8[index] = phase + 1;
                if (phase + 1 == 0x11)
                {
                    DAT_102199b8[index] = 1;
                    model.alpha = 1.0f;
                }
                else if (phase + 1 == 0x20)
                {
                    DAT_102199b8[index] = 0;
                    model.alpha = 0.0f;
                }
            }
        }
    }

    // The fade of every slot's group (1 = fading in from nothing, 2 = in, 3 = out) and the area zones.
    for (int index = 1; index < MARKER_SLOT_END; index++)
    {
        if (index == PEOPLE_SLOT_FIRST)
            index = PEOPLE_SLOT_END;
        if (DAT_102199b8[index] != 0)
        {
            if (DAT_102199b8[index] == 1)
            {
                DAT_10db6ee8[index].alpha = 0.0f;
                DAT_102199b8[index] = 2;
            }
            else if (DAT_102199b8[index] == 3)
            {
                float fade = DAT_10db6ee8[index].alpha - FADE_STEP;
                DAT_102199b8[index] = 3;
                DAT_10db6ee8[index].alpha = fade;
                if (fade < 0.0f)
                {
                    DAT_10db6ee8[index].alpha = 0.0f;
                    DAT_102199b8[index] = 0;
                }
            }
            else
            {
                float fade = DAT_10db6ee8[index].alpha + FADE_STEP;
                DAT_102199b8[index] = 2;
                DAT_10db6ee8[index].alpha = fade;
                if (fade > 1.0f)
                {
                    DAT_10db6ee8[index].alpha = 1.0f;
                    DAT_102199b8[index] = 0;
                }
            }
        }
        if (DAT_10daaae8[index].kind == 0x23 && DAT_104bfe1c == 0 && DAT_104bfe6c == 0 && DAT_104bfeb0 == 0 &&
            DAT_104c0094 == 0 && DAT_104bfdf0 == 0 && DAT_104c0135 == 0)
            FUN_1007ad20(index);
    }

    // The hits the player took in the timed run, and its clock (it stops while a cutscene or the pause shows).
    if (DAT_10c64014 != DAT_10daaae8[0].stats[STAT_HEALTH])
    {
        if (DAT_10c64014 > DAT_10daaae8[0].stats[STAT_HEALTH])
            DAT_10c64008 += DAT_10c64014 - DAT_10daaae8[0].stats[STAT_HEALTH];
        DAT_10c64014 = DAT_10daaae8[0].stats[STAT_HEALTH];
    }
    if (DAT_104bfdf0 == 0 && DAT_104c0135 == 0 && DAT_104bfe1c == 0)
    {
        if (DAT_10c635ac != 0)
        {
            DAT_10c64004 = GetTickCount();
            DAT_10c635ac = 0;
        }
    }
    else if (DAT_10c635ac == 0)
    {
        DAT_10c6400c = GetTickCount() + (DAT_10c6400c - DAT_10c64004);
        DAT_10c635ac = 1;
    }
    return 0;
}
