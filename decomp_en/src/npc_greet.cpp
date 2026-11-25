// Character greetings (EN build). Ported from the matched JP source decomp/src/npc_greet.cpp.
#include <math.h>
#include "npc_greet.h"
#include "char.h"
#include "camera.h"
#include "dialogs.h"
#include "engine.h"
#include "scene_start.h"
#include "game_vars.h"
#include "sound_effects.h"

// Areas where characters greet the player.
const int GREET_AREA_A = 0x17;
const int GREET_AREA_B = 0x19;
const int GREET_AREA_C = 0x16;
const int GREET_AREA_D = 0x10;
const int GREET_AREA_E = 0x14;
const int GREET_AREA_F = 0x18;
const int GREET_AREA_G = 0x28;
const int GREET_AREA_H = 0x15;
// Distance within which the player starts a greeting.
const float GREET_RANGE = 1500.0f;
// Countdown value that makes the character greet now; a restart picks GREET_RESTART_BASE plus 0..GREET_RESTART_SPREAD-1.
const int GREET_NOW = 10000;
const int GREET_RESTART_BASE = 10020;
const int GREET_RESTART_SPREAD = 20;
// Countdown after a greeting.
const int GREET_COOLDOWN = 30;
// Greeting balloons: the two plain ones, the costume ones and the ones of single characters.
const int GREET_BALLOON_FIRST = 0x7d4;
const int GREET_BALLOON_COSTUME_FIRST = 0x7d6;
const int GREET_BALLOON_COSTUME_3 = 0x7d6;
const int GREET_BALLOON_COSTUME_15 = 0x7d7;
const int GREET_BALLOON_COSTUME_14 = 0x7d8;
const int GREET_BALLOON_CHARACTER_18 = 0x7d0;
const int GREET_BALLOON_CHARACTER_58 = 0x7d1;
const int GREET_BALLOON_CHARACTER_59 = 0x7dd;
const int GREET_CHARACTER_18 = 0x12;
const int GREET_CHARACTER_58 = 0x3a;
const int GREET_CHARACTER_59 = 0x3b;
const int GREET_COSTUME_A = 3;
const int GREET_COSTUME_B = 0xe;
const int GREET_COSTUME_C = 0xf;
// Maps of the special balloons of characters 0x3a (before map 13, except 6) and 0x3b (maps 6 and 0x1a).
const int GREET_MAP_LIMIT = 0xd;
const int GREET_MAP_SIX = 6;
const int GREET_MAP_LAST = 0x1a;
// Frames a greeting balloon stays and the sound that goes with it.
const int GREET_BALLOON_FRAMES = 0x3c;
const int GREET_SOUND = 0x356;
const float GREET_SOUND_VOLUME = 1.0f;
// Picks of the balloon roll: from this one on the costume balloons are possible.
const int GREET_PICK_COUNT = 3;
const int GREET_COSTUME_PICK = 2;

// Npc_Greet: greeting of character `index`: while greetings are on in one of the greeting areas and the player is within
// 1500 units, a finished countdown (<= 0) restarts at 10020-10039. At 10000 the character turns to face the player and
// shows greeting balloon 0x7d4 or 0x7d5 (one in three times 0x7d6-0x7d8 for the player's costumes 3, 15 and 14), replaced
// for characters 0x12, 0x3a and 0x3b in some maps, for 60 frames with a sound; the countdown restarts at 30. Returns 0.
extern "C" int Npc_Greet(int index)
{
    if (g_greetingFrames != 0 &&
        (g_greetingArea == GREET_AREA_A || g_greetingArea == GREET_AREA_B || g_greetingArea == GREET_AREA_C ||
         g_greetingArea == GREET_AREA_D || g_greetingArea == GREET_AREA_E || g_greetingArea == GREET_AREA_F ||
         g_greetingArea == GREET_AREA_G || g_greetingArea == GREET_AREA_H))
    {
        Vec4 player;
        Vec4 self;
        player.Copy(g_characterModels[0].matrix.GetTranslation());
        self.Copy(g_characterModels[index].matrix.GetTranslation());
        if ((player - self).Length() < GREET_RANGE && g_greetCountdown[index] <= 0)
            g_greetCountdown[index] = Random() % GREET_RESTART_SPREAD + GREET_RESTART_BASE;
    }
    if (g_greetCountdown[index] != GREET_NOW)
        return 0;
    {
        Vec4 player;
        Vec4 self;
        player.Copy(g_characterModels[0].matrix.GetTranslation());
        self.Copy(g_characterModels[index].matrix.GetTranslation());
        g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
    }
    int pick = Random() % GREET_PICK_COUNT;
    int balloon = 0;
    if (pick >= GREET_COSTUME_PICK)
    {
        switch (g_charRecords[0].costume)
        {
        case GREET_COSTUME_A:
            balloon = GREET_BALLOON_COSTUME_3;
            break;
        case GREET_COSTUME_B:
            balloon = GREET_BALLOON_COSTUME_14;
            break;
        case GREET_COSTUME_C:
            balloon = GREET_BALLOON_COSTUME_15;
            break;
        default:
            pick = Random() % GREET_COSTUME_PICK;
            break;
        }
    }
    if (balloon == 0)
    {
        balloon = pick + GREET_BALLOON_FIRST;
        if (balloon < GREET_BALLOON_COSTUME_FIRST)
        {
            if (index == GREET_CHARACTER_18)
                balloon = GREET_BALLOON_CHARACTER_18;
            else if (index == GREET_CHARACTER_58)
            {
                if (g_currentMap < GREET_MAP_LIMIT && g_currentMap != GREET_MAP_SIX)
                    balloon = GREET_BALLOON_CHARACTER_58;
            }
            else if (index == GREET_CHARACTER_59 && (g_currentMap == GREET_MAP_LAST || g_currentMap == GREET_MAP_SIX))
                balloon = GREET_BALLOON_CHARACTER_59;
        }
    }
    if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != balloon)
    {
        g_keyPromptShown[index] = 1;
        g_keyPromptKey[index] = balloon;
    }
    g_keyPromptFrames[index] = GREET_BALLOON_FRAMES;
    (g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
    Vec4 soundPosition;
    soundPosition.Set(g_characterModels[index].matrix.m[12], g_characterModels[index].matrix.m[13], g_characterModels[index].matrix.m[14], 0);
    Sound_PlayEffect(GREET_SOUND, &soundPosition, GREET_SOUND_VOLUME);
    g_greetCountdown[index] = GREET_COOLDOWN;
    return 0;
}
