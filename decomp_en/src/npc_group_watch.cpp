// A guard NPC that works in a group (EN build). The EN function at 0x6b3b20 is the longer sibling of Npc_WatchForPlayer
// (npc_watch_player.cpp): besides watching for the player it keeps a group number and now and then turns to look at a
// random group mate and shows a prompt.
#include <math.h>

#include "anim_model.h"
#include "camera.h"
#include "char.h"
#include "char_move.h"
#include "collide_query.h"
#include "dialogs.h"
#include "engine.h"
#include "sound_effects.h"

// CharState::state values.
const int GROUP_WATCH_STATE_START = 0;
const int GROUP_WATCH_STATE_WATCHING = 1;
const int GROUP_WATCH_STATE_ALARMED = 2;
// CharState::values entries of this behaviour: frames since the last glance or the alarm, the talk code (group number in
// the units digit, sight line in the tens), which glance phase the NPC is in, how many group mates exist, the slot of the
// mate it looks at, the group number and the sight line in units of ten.
const int GROUP_WATCH_VALUE_WAIT = 0;
const int GROUP_WATCH_VALUE_TALK = 1;
const int GROUP_WATCH_VALUE_PHASE = 2;
const int GROUP_WATCH_VALUE_MATES = 3;
const int GROUP_WATCH_VALUE_TARGET = 4;
const int GROUP_WATCH_VALUE_GROUP = 5;
const int GROUP_WATCH_VALUE_SIGHT = 6;
// Glance phases: waiting for the next glance, turning towards the mate, and the mate is reached (prompt and voice).
const int GROUP_WATCH_PHASE_WAIT = 0;
const int GROUP_WATCH_PHASE_TURN = 1;
const int GROUP_WATCH_PHASE_SPEAK = 2;
// The talk code splits into the group number (units digit) and the sight line (the rest).
const int GROUP_WATCH_TALK_BASE = 10;
// Slots below this one are searched for group mates (the first enemy slot up to the end of the character slots).
const int GROUP_WATCH_SLOT_END = 0x5c;
// Behaviour number of a group mate.
const int GROUP_WATCH_MATE_BEHAVIOUR = 3;
// Idle and walking motions the NPC plays, with the blend frames.
const int GROUP_WATCH_MOTION_IDLE = 2;
const int GROUP_WATCH_MOTION_TURN = 0;
const int GROUP_WATCH_BLEND_FRAMES = 5;
// Frame counter modulus that lets one NPC per frame count its mates, the frames it waits and the chance (out of
// GROUP_WATCH_CHANCE_RANGE) per frame that it glances at a mate.
const int GROUP_WATCH_COUNT_PERIOD = 0x5c;
const int GROUP_WATCH_GLANCE_WAIT = 60;
const int GROUP_WATCH_CHANCE = 200;
const int GROUP_WATCH_CHANCE_RANGE = 10000;
// Angle (about ten degrees) the NPC turns per frame towards the mate, and the full turn as the game rounds it.
const float GROUP_WATCH_TURN_STEP = 0.17453f;
const float GROUP_WATCH_FULL_TURN = 6.283f;
// The glance picks one of this many cases at random; each shows its own prompt key, except case 3 which shows none.
const int GROUP_WATCH_PROMPT_CHOICES = 6;
const int GROUP_WATCH_PROMPT_KEY_0 = 13;
const int GROUP_WATCH_PROMPT_KEY_1 = 15;
const int GROUP_WATCH_PROMPT_KEY_2 = 8;
const int GROUP_WATCH_PROMPT_KEY_4 = 4;
const int GROUP_WATCH_PROMPT_KEY_5 = 9;
const int GROUP_WATCH_VOICE_FIRST = 0x209;
const int GROUP_WATCH_VOICE_COUNT = 3;
// Sight line: units per sight value, the half angle in radians (63 degrees), the distance within which the player is
// always seen, and the eye heights above the origins of the player and of the NPC.
const int GROUP_WATCH_SIGHT_UNIT = 20;
const float GROUP_WATCH_SIGHT_ANGLE = 1.099525f;
const float GROUP_WATCH_SIGHT_NEAR = 250.0f;
const float GROUP_WATCH_PLAYER_EYE_HEIGHT = 50.0f;
const float GROUP_WATCH_SELF_EYE_HEIGHT = 75.0f;
// The alarm: key prompt and its frames, the voice, the squared camera distance within which that voice is heard and the
// radius of the enemies the alarm wakes.
const int GROUP_WATCH_ALARM_PROMPT = 15;
const int GROUP_WATCH_PROMPT_FRAMES = 60;
const int GROUP_WATCH_ALARM_VOICE = 0x20c;
const float GROUP_WATCH_VOICE_RANGE_SQUARED = 6250000.0f;
const float GROUP_WATCH_WAKE_RADIUS = 1500.0f;
// Frames the NPC turns to the player after the alarm before its state is reset, and the angle (one degree) below which it
// faces the player.
const int GROUP_WATCH_TURN_FRAMES = 30;
const float GROUP_WATCH_FACING_ANGLE = 0.01745f;

// Npc_GroupWatch: a guard that belongs to a group (talk code units digit). On its first frame it counts the other
// behaviour 3 characters of its group. Then it idles; now and then (one chance in fifty per frame, at least 60 frames
// apart) it picks a random group mate, turns to it ten degrees per frame and shows a random prompt with a random voice.
// When it sees the player (63 degrees either side of its facing, or within 250 units) or is told to react it shows prompt
// 15, plays voice 0x20c, wakes the enemies within 1500 and turns to the player for 30 frames before its state is reset.
// Returns 0. Called by the NPC update.
int Npc_GroupWatch(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    (player - self).Length();
    (player - self).Length2D();
    if (g_charState[index].state == GROUP_WATCH_STATE_START)
    {
        g_characterModels[index].PlayMotion(GROUP_WATCH_MOTION_IDLE, 0, GROUP_WATCH_BLEND_FRAMES, 1.0f);
        g_charState[index].state = GROUP_WATCH_STATE_WATCHING;
        for (int slot = ENEMY_SLOT_FIRST; slot < GROUP_WATCH_SLOT_END; slot++)
        {
            if (slot != index && g_characterModels[slot].root != 0 && g_charRecords[slot].kind == CHAR_KIND_PACKED &&
                g_charState[slot].behaviour == GROUP_WATCH_MATE_BEHAVIOUR &&
                g_charState[index].values[GROUP_WATCH_VALUE_TALK] == g_charState[slot].values[GROUP_WATCH_VALUE_TALK])
                g_charState[index].values[GROUP_WATCH_VALUE_MATES]++;
        }
        g_charToldToReact[index] = 0;
        g_charGroup[index] = g_charState[index].values[GROUP_WATCH_VALUE_TALK] % GROUP_WATCH_TALK_BASE;
        g_charState[index].values[GROUP_WATCH_VALUE_SIGHT] = g_charState[index].values[GROUP_WATCH_VALUE_TALK] / GROUP_WATCH_TALK_BASE;
        g_charState[index].values[GROUP_WATCH_VALUE_GROUP] = g_charState[index].values[GROUP_WATCH_VALUE_TALK] % GROUP_WATCH_TALK_BASE;
        return 0;
    }
    if (g_charState[index].state == GROUP_WATCH_STATE_WATCHING)
    {
        if (g_charState[index].values[GROUP_WATCH_VALUE_PHASE] == GROUP_WATCH_PHASE_WAIT)
        {
            g_characterModels[index].PlayMotion(GROUP_WATCH_MOTION_IDLE, 0, GROUP_WATCH_BLEND_FRAMES, 1.0f);
            g_charState[index].values[GROUP_WATCH_VALUE_WAIT]++;
            if (g_frameCounter % GROUP_WATCH_COUNT_PERIOD == index)
            {
                g_charState[index].values[GROUP_WATCH_VALUE_MATES] = 0;
                for (int slot = ENEMY_SLOT_FIRST; slot < GROUP_WATCH_SLOT_END; slot++)
                {
                    if (slot != index && g_characterModels[slot].root != 0 && g_charRecords[slot].kind == CHAR_KIND_PACKED &&
                        g_charState[slot].behaviour == GROUP_WATCH_MATE_BEHAVIOUR &&
                        g_charState[index].values[GROUP_WATCH_VALUE_GROUP] == g_charState[slot].values[GROUP_WATCH_VALUE_GROUP])
                        g_charState[index].values[GROUP_WATCH_VALUE_MATES]++;
                }
            }
            if (g_charState[index].values[GROUP_WATCH_VALUE_MATES] != 0 &&
                g_charState[index].values[GROUP_WATCH_VALUE_WAIT] > GROUP_WATCH_GLANCE_WAIT &&
                Random() % GROUP_WATCH_CHANCE_RANGE < GROUP_WATCH_CHANCE)
            {
                g_charState[index].values[GROUP_WATCH_VALUE_WAIT] = 0;
                int pick = Random() % g_charState[index].values[GROUP_WATCH_VALUE_MATES];
                int found = 0;
                int slot;
                for (slot = ENEMY_SLOT_FIRST; slot < GROUP_WATCH_SLOT_END; slot++)
                {
                    if (slot != index && g_characterModels[slot].root != 0 && g_charRecords[slot].kind == CHAR_KIND_PACKED &&
                        g_charState[slot].behaviour == GROUP_WATCH_MATE_BEHAVIOUR &&
                        g_charState[index].values[GROUP_WATCH_VALUE_GROUP] == g_charState[slot].values[GROUP_WATCH_VALUE_GROUP])
                    {
                        if (pick == found)
                            break;
                        found++;
                    }
                }
                if (slot != GROUP_WATCH_SLOT_END)
                {
                    g_charState[index].values[GROUP_WATCH_VALUE_TARGET] = slot;
                    g_charState[index].values[GROUP_WATCH_VALUE_PHASE] = GROUP_WATCH_PHASE_TURN;
                    g_charState[index].values[GROUP_WATCH_VALUE_WAIT] = 0;
                }
            }
        }
        else if (g_charState[index].values[GROUP_WATCH_VALUE_PHASE] == GROUP_WATCH_PHASE_TURN)
        {
            float target = (float)atan2(g_characterModels[g_charState[index].values[GROUP_WATCH_VALUE_TARGET]].matrix.m[MATRIX_TRANSLATION_X] - self.x,
                                        g_characterModels[g_charState[index].values[GROUP_WATCH_VALUE_TARGET]].matrix.m[MATRIX_TRANSLATION_Y] - self.y) +
                ANGLE_HALF_TURN;
            float turn = target - g_characterModels[index].heading;
            while (turn > ANGLE_HALF_TURN)
                turn = turn - GROUP_WATCH_FULL_TURN;
            while (turn < -ANGLE_HALF_TURN)
                turn = turn + GROUP_WATCH_FULL_TURN;
            if (turn > GROUP_WATCH_TURN_STEP)
            {
                target = g_characterModels[index].heading + GROUP_WATCH_TURN_STEP;
                g_characterModels[index].PlayMotion(GROUP_WATCH_MOTION_TURN, 0, GROUP_WATCH_BLEND_FRAMES, 1.0f);
            }
            else if (turn < -GROUP_WATCH_TURN_STEP)
            {
                target = g_characterModels[index].heading - GROUP_WATCH_TURN_STEP;
                g_characterModels[index].PlayMotion(GROUP_WATCH_MOTION_TURN, 0, GROUP_WATCH_BLEND_FRAMES, 1.0f);
            }
            else
            {
                g_charState[index].values[GROUP_WATCH_VALUE_PHASE] = GROUP_WATCH_PHASE_SPEAK;
                g_charState[index].values[GROUP_WATCH_VALUE_WAIT] = 0;
            }
            g_characterModels[index].targetHeading = target;
        }
        else if (g_charState[index].values[GROUP_WATCH_VALUE_PHASE] == GROUP_WATCH_PHASE_SPEAK)
        {
            switch (Random() % GROUP_WATCH_PROMPT_CHOICES)
            {
            case 0:
                KeyPrompt_Show(index, GROUP_WATCH_PROMPT_KEY_0, GROUP_WATCH_PROMPT_FRAMES);
                break;
            case 1:
                KeyPrompt_Show(index, GROUP_WATCH_PROMPT_KEY_1, GROUP_WATCH_PROMPT_FRAMES);
                break;
            case 2:
                KeyPrompt_Show(index, GROUP_WATCH_PROMPT_KEY_2, GROUP_WATCH_PROMPT_FRAMES);
                break;
            case 4:
                KeyPrompt_Show(index, GROUP_WATCH_PROMPT_KEY_4, GROUP_WATCH_PROMPT_FRAMES);
                break;
            case 5:
                KeyPrompt_Show(index, GROUP_WATCH_PROMPT_KEY_5, GROUP_WATCH_PROMPT_FRAMES);
                break;
            }
            Character_PlaySound(index, Random() % GROUP_WATCH_VOICE_COUNT + GROUP_WATCH_VOICE_FIRST, 0, 0);
            g_charState[index].values[GROUP_WATCH_VALUE_PHASE] = GROUP_WATCH_PHASE_WAIT;
            g_charState[index].values[GROUP_WATCH_VALUE_WAIT] = 0;
        }
        player.z += GROUP_WATCH_PLAYER_EYE_HEIGHT;
        self.z += GROUP_WATCH_SELF_EYE_HEIGHT;
        if ((g_charState[index].sawPlayer |= Char_CanSee(&self, &player, g_characterModels[index].heading,
                                                       (float)(g_charState[index].values[GROUP_WATCH_VALUE_SIGHT] * GROUP_WATCH_SIGHT_UNIT),
                                                       GROUP_WATCH_SIGHT_ANGLE, GROUP_WATCH_SIGHT_NEAR, 1)) != 0 ||
            g_charToldToReact[index] != 0)
        {
            if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != GROUP_WATCH_ALARM_PROMPT)
            {
                g_keyPromptShown[index] = 1;
                g_keyPromptKey[index] = GROUP_WATCH_ALARM_PROMPT;
            }
            g_keyPromptFrames[index] = GROUP_WATCH_PROMPT_FRAMES;
            float distanceSquared = (g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
            if (distanceSquared < GROUP_WATCH_VOICE_RANGE_SQUARED || index < ENEMY_SLOT_FIRST)
            {
                Vec4 position;
                position.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(GROUP_WATCH_ALARM_VOICE, &position, 1.0f);
            }
            g_charState[index].state = GROUP_WATCH_STATE_ALARMED;
            g_charState[index].values[GROUP_WATCH_VALUE_WAIT] = 0;
            Enemies_WakeNearby(index, GROUP_WATCH_WAKE_RADIUS, 0);
            g_charToldToReact[index] = 0;
            return 0;
        }
    }
    else if (g_charState[index].state == GROUP_WATCH_STATE_ALARMED)
    {
        float target = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
        g_charState[index].values[GROUP_WATCH_VALUE_WAIT]++;
        g_characterModels[index].targetHeading = target;
        float turn = g_characterModels[index].heading - target;
        while (turn > ANGLE_HALF_TURN)
            turn -= GROUP_WATCH_FULL_TURN;
        while (turn < -ANGLE_HALF_TURN)
            turn += GROUP_WATCH_FULL_TURN;
        if (turn < GROUP_WATCH_FACING_ANGLE)
            g_characterModels[index].PlayMotion(GROUP_WATCH_MOTION_IDLE, 0, GROUP_WATCH_BLEND_FRAMES, 1.0f);
        else
            g_characterModels[index].PlayMotion(GROUP_WATCH_MOTION_TURN, 0, GROUP_WATCH_BLEND_FRAMES, 1.0f);
        if (g_charState[index].values[GROUP_WATCH_VALUE_WAIT] == GROUP_WATCH_TURN_FRAMES)
            Char_ResetState(index);
    }
    g_charToldToReact[index] = 0;
    return 0;
}
