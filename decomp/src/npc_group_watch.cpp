// A group of NPCs that watch for the player together (0x100b1800, behaviour 3 of Char_RunBehaviour 0x100dfcf0): like
// the lone watcher (Npc_WatchForPlayer) it raises the alarm when it sees the player, and while it idles it turns to
// look at random groupmates and shows an emote.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_walker[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"

#define GROUP_FIRST_SLOT 0x3a
#define GROUP_SLOT_LIMIT 0x5c
#define GROUP_BEHAVIOUR 3
#define GROUP_IDLE_MOTION 2
#define GROUP_TURN_MOTION 0
#define GROUP_STATE_SETUP 0
#define GROUP_STATE_WATCHING 1
#define GROUP_STATE_ALARMED 2
#define GROUP_PHASE_IDLE 0
#define GROUP_PHASE_TURNING 1
#define GROUP_PHASE_EMOTE 2
#define GROUP_RECOUNT_PERIOD 0x5c
#define GROUP_LOOK_AFTER_FRAMES 0x3c
#define GROUP_LOOK_CHANCE 200
#define GROUP_LOOK_CHANCE_RANGE 10000
#define GROUP_EMOTE_CHOICES 6
#define GROUP_EMOTE_FRAMES 0x3c
#define GROUP_SOUND_FIRST 0x209
#define GROUP_SOUND_CHOICES 3
#define GROUP_SIGHT_UNIT 20
#define GROUP_SIGHT_ANGLE 1.099525f
#define GROUP_SIGHT_NEAR 250.0f
#define GROUP_EYE_HEIGHT_PLAYER 50.0f
#define GROUP_EYE_HEIGHT_SELF 75.0f
#define GROUP_ALARM_PROMPT 15
#define GROUP_ALARM_PROMPT_FRAMES 60
#define GROUP_ALARM_SOUND 0x20c
#define GROUP_ALARM_WAKE_RANGE 1500.0f
#define GROUP_ALARM_RESET_FRAMES 0x1e
#define GROUP_TURN_OFFSET 3.1415f
#define GROUP_FULL_TURN 6.283f
#define GROUP_TURN_STEP 0.17453f
#define GROUP_FACING_ANGLE 0.01745f
#define GROUP_FAR_SQUARED 6250000.0f
#define GROUP_PLAYER_SLOTS 0x3a

extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1000d620(int index, int key, int frames);                // KeyPrompt_Show
extern "C" int FUN_1001b670(int index, float range, int count, ...);       // Enemies_WakeNearby
extern "C" int FUN_10043cc0(int index);                                     // Char_ResetState
extern "C" float FUN_10001f80(float y, float x);                            // Atan2
extern "C" int FUN_10043e20(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange,
                            float nearRange, int checkWalls);           // Char_CanSee
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);  // Sound_PlayEffect

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x100b1800 (Npc_GroupWatch). State 0 counts the groupmates (characters of kind 0x62 with behaviour 3 and the same
// talk id), splits the talk id into its group digit and sight scale and starts watching (state 1). Watching, it
// recounts its group (same group digit) once every 92 frames, and after a minute, with a 2 % chance a frame, turns
// to a random groupmate (phase 1: in steps of 10 degrees; phase 2: a random emote prompt of the six and a voice
// 0x209..0x20b, back to phase 0). Whenever it sees the player (within its sight scale * 20 units, 63 degrees either
// side, or 250 units) or is told to react, it shows prompt 15, wakes the enemies within 1500 and is alarmed (state
// 2): it faces the player, the idle motion once within one degree, else the turn motion, and resets after 30 frames.
// Returns 0.
extern "C" int FUN_100b1800(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    (player - self).Length();
    (player - self).Length2D();
    int state = DAT_1020b430[index].state;
    if (state == GROUP_STATE_SETUP)
    {
        DAT_10db6ee8[index].PlayMotion(GROUP_IDLE_MOTION, 0, 5, 1.0f);
        DAT_1020b430[index].state = GROUP_STATE_WATCHING;
        for (int slot = GROUP_FIRST_SLOT; slot < GROUP_SLOT_LIMIT; slot++)
        {
            if (slot != index && DAT_10db6ee8[slot].root != 0 && DAT_10daaae8[slot].kind == CHAR_KIND_ENEMY &&
                DAT_1020b430[slot].behaviour == GROUP_BEHAVIOUR && DAT_1020b430[index].talk == DAT_1020b430[slot].talk)
                DAT_1020b430[index].groupSize++;
        }
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].sightScale = DAT_1020b430[index].talk / 10;
        DAT_1020b430[index].groupDigit = DAT_1020b430[index].talk % 10;
        DAT_10255f10[index] = DAT_1020b430[index].talk % 10;
        return 0;
    }
    if (state == GROUP_STATE_WATCHING)
    {
        if (DAT_1020b430[index].watchPhase == GROUP_PHASE_IDLE)
        {
            DAT_10db6ee8[index].PlayMotion(GROUP_IDLE_MOTION, 0, 5, 1.0f);
            DAT_1020b430[index].wait++;
            if (DAT_104c0380 % GROUP_RECOUNT_PERIOD == index)
            {
                DAT_1020b430[index].groupSize = 0;
                for (int slot = GROUP_FIRST_SLOT; slot < GROUP_SLOT_LIMIT; slot++)
                {
                    if (slot != index && DAT_10db6ee8[slot].root != 0 && DAT_10daaae8[slot].kind == CHAR_KIND_ENEMY &&
                        DAT_1020b430[slot].behaviour == GROUP_BEHAVIOUR &&
                        DAT_1020b430[index].groupDigit == DAT_1020b430[slot].groupDigit)
                        DAT_1020b430[index].groupSize++;
                }
            }
            if (DAT_1020b430[index].groupSize != 0 && DAT_1020b430[index].wait > GROUP_LOOK_AFTER_FRAMES &&
                FUN_1013f880() % GROUP_LOOK_CHANCE_RANGE < GROUP_LOOK_CHANCE)
            {
                DAT_1020b430[index].wait = 0;
                int pick = FUN_1013f880() % DAT_1020b430[index].groupSize;
                int counted = 0;
                int slot;
                for (slot = GROUP_FIRST_SLOT; slot < GROUP_SLOT_LIMIT; slot++)
                {
                    if (slot != index && DAT_10db6ee8[slot].root != 0 && DAT_10daaae8[slot].kind == CHAR_KIND_ENEMY &&
                        DAT_1020b430[slot].behaviour == GROUP_BEHAVIOUR &&
                        DAT_1020b430[index].groupDigit == DAT_1020b430[slot].groupDigit)
                    {
                        if (pick == counted)
                            break;
                        counted++;
                    }
                }
                if (slot != GROUP_SLOT_LIMIT)
                {
                    DAT_1020b430[index].watchTarget = slot;
                    DAT_1020b430[index].watchPhase = GROUP_PHASE_TURNING;
                    DAT_1020b430[index].wait = 0;
                }
            }
        }
        else if (DAT_1020b430[index].watchPhase == GROUP_PHASE_TURNING)
        {
            float target = FUN_10001f80(DAT_10db6ee8[DAT_1020b430[index].watchTarget].matrix.m[12] - self.x,
                                        DAT_10db6ee8[DAT_1020b430[index].watchTarget].matrix.m[13] - self.y) +
                           GROUP_TURN_OFFSET;
            float turn = target - DAT_10db6ee8[index].heading;
            while (turn > GROUP_TURN_OFFSET)
                turn = turn - GROUP_FULL_TURN;
            while (turn < -GROUP_TURN_OFFSET)
                turn = turn + GROUP_FULL_TURN;
            if (turn > GROUP_TURN_STEP)
            {
                target = DAT_10db6ee8[index].heading + GROUP_TURN_STEP;
                DAT_10db6ee8[index].PlayMotion(GROUP_TURN_MOTION, 0, 5, 1.0f);
            }
            else if (turn < -GROUP_TURN_STEP)
            {
                target = DAT_10db6ee8[index].heading - GROUP_TURN_STEP;
                DAT_10db6ee8[index].PlayMotion(GROUP_TURN_MOTION, 0, 5, 1.0f);
            }
            else
            {
                DAT_1020b430[index].watchPhase = GROUP_PHASE_EMOTE;
                DAT_1020b430[index].wait = 0;
            }
            DAT_10db6ee8[index].targetHeading = target;
        }
        else if (DAT_1020b430[index].watchPhase == GROUP_PHASE_EMOTE)
        {
            switch (FUN_1013f880() % GROUP_EMOTE_CHOICES)
            {
            case 0:
                FUN_1000d620(index, 13, GROUP_EMOTE_FRAMES);
                break;
            case 1:
                FUN_1000d620(index, 15, GROUP_EMOTE_FRAMES);
                break;
            case 2:
                FUN_1000d620(index, 8, GROUP_EMOTE_FRAMES);
                break;
            case 4:
                FUN_1000d620(index, 4, GROUP_EMOTE_FRAMES);
                break;
            case 5:
                FUN_1000d620(index, 9, GROUP_EMOTE_FRAMES);
                break;
            }
            FUN_10065b10(index, FUN_1013f880() % GROUP_SOUND_CHOICES + GROUP_SOUND_FIRST, 0, 0);
            DAT_1020b430[index].watchPhase = GROUP_PHASE_IDLE;
            DAT_1020b430[index].wait = 0;
        }
        player.z += GROUP_EYE_HEIGHT_PLAYER;
        self.z += GROUP_EYE_HEIGHT_SELF;
        if ((DAT_1020b430[index].sawPlayer |=
             FUN_10043e20(&self, &player, DAT_10db6ee8[index].heading,
                          (float)(DAT_1020b430[index].sightScale * GROUP_SIGHT_UNIT), GROUP_SIGHT_ANGLE,
                          GROUP_SIGHT_NEAR, 1)) != 0 ||
            DAT_10210b30[index] != 0)
        {
            if (DAT_104bf8f8[index] == 0 || DAT_1021d138[index] != GROUP_ALARM_PROMPT)
            {
                DAT_104bf8f8[index] = 1;
                DAT_1021d138[index] = GROUP_ALARM_PROMPT;
            }
            DAT_1021f740[index] = GROUP_ALARM_PROMPT_FRAMES;
            float distanceSquared = (DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3();
            if (distanceSquared < GROUP_FAR_SQUARED || index < GROUP_PLAYER_SLOTS)
            {
                Vec4 sound;
                sound.Set(DAT_10db6ee8[index].matrix.m[12], DAT_10db6ee8[index].matrix.m[13],
                          DAT_10db6ee8[index].matrix.m[14], 0);
                FUN_1003ad20(GROUP_ALARM_SOUND, &sound, 1.0f);
            }
            DAT_1020b430[index].state = GROUP_STATE_ALARMED;
            DAT_1020b430[index].wait = 0;
            FUN_1001b670(index, GROUP_ALARM_WAKE_RANGE, 0);
            DAT_10210b30[index] = 0;
        }
    }
    else if (state == GROUP_STATE_ALARMED)
    {
        float target = Atan2(player.x - self.x, player.y - self.y) + GROUP_TURN_OFFSET;
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].targetHeading = target;
        float turn = DAT_10db6ee8[index].heading - target;
        while (turn > GROUP_TURN_OFFSET)
            turn = turn - GROUP_FULL_TURN;
        while (turn < -GROUP_TURN_OFFSET)
            turn = turn + GROUP_FULL_TURN;
        if (turn < GROUP_FACING_ANGLE)
            DAT_10db6ee8[index].PlayMotion(GROUP_IDLE_MOTION, 0, 5, 1.0f);
        else
            DAT_10db6ee8[index].PlayMotion(GROUP_TURN_MOTION, 0, 5, 1.0f);
        if (DAT_1020b430[index].wait == GROUP_ALARM_RESET_FRAMES)
            FUN_10043cc0(index);
    }
    return 0;
}
