// A guard-like NPC that watches for the player (EN build). Ported from the JP draft decomp/src/char_move.cpp
// (Npc_WatchForPlayer, 0x100b1520) and the Ghidra output of the EN twin, which also plays the alarm sound by distance.
#include <math.h>

#include "anim_model.h"
#include "camera.h"
#include "char_move.h"
#include "dialogs.h"
#include "npc_facing.h"
#include "npc_greet.h"
#include "sound_effects.h"

// NPC states: not started, watching, alarmed (turning to the player).
const int WATCH_NPC_STATE_START = 0;
const int WATCH_NPC_STATE_WATCHING = 1;
const int WATCH_NPC_STATE_ALARMED = 2;
// CharState::values entries: the sight line (talk code, in units of ten) and the frames since the alarm.
const int WATCH_NPC_VALUE_WAIT = 0;
const int WATCH_NPC_VALUE_SIGHT = 1;
// Sight line: units per talk code step, the half angle in radians (63 degrees), the distance within which the player is
// always seen, and the wall test flag. The eyes sit this far above the NPC's and the player's origins.
const int WATCH_NPC_SIGHT_UNIT = 10;
const float WATCH_NPC_SIGHT_ANGLE = 1.099525f;
const float WATCH_NPC_SIGHT_NEAR = 250.0f;
const int WATCH_NPC_CHECK_WALLS = 1;
const float WATCH_NPC_PLAYER_EYE_HEIGHT = 50.0f;
const float WATCH_NPC_SELF_EYE_HEIGHT = 75.0f;
// The alarm: key prompt and its frames, sound, the squared camera distance within which it is heard (slots below
// WATCH_NPC_ALWAYS_SOUND_SLOT always play it), the radius of the enemies it wakes, and the frames the NPC turns before
// its state is reset.
const int WATCH_NPC_ALARM_PROMPT = 15;
const int WATCH_NPC_ALARM_PROMPT_FRAMES = 60;
const int WATCH_NPC_ALARM_SOUND = 0x20c;
const float WATCH_NPC_SOUND_RANGE_SQUARED = 6250000.0f;
const int WATCH_NPC_ALWAYS_SOUND_SLOT = 0x3a;
const float WATCH_NPC_WAKE_RADIUS = 1500.0f;
const int WATCH_NPC_TURN_FRAMES = 30;
// The NPC counts as facing the player below this angle (one degree, in radians).
const float WATCH_NPC_FACING_ANGLE = 0.01745f;
// Motions: idle, and the turn on the spot.
const int WATCH_NPC_MOTION_IDLE = 2;
const int WATCH_NPC_MOTION_TURN = 0;

// A guard-like NPC that idles (motion 2) until it sees the player: from its head (75 above its origin) towards the player's
// head (50 above), within its talk code * 10 units, 63 degrees either side of its facing or 250 units. When it has seen the
// player (remembered in sawPlayer) or was told to react, it shows prompt 15 for 60 frames, plays sound 0x20c (within 2500
// units of the camera target, or always for slots below 0x3a), wakes the enemies within 1500 and starts turning: from then
// on it faces the player (idle motion once within one degree, else the turn motion) and after 30 frames its state is
// reset. The told-to-react flag is cleared every frame. Returns 0. Called by the NPC behaviour dispatcher.
extern "C" int Npc_WatchForPlayer(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    (player - self).Length();
    (player - self).Length2D();
    if (g_charState[index].state == WATCH_NPC_STATE_START)
    {
        g_characterModels[index].PlayMotion(WATCH_NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
        g_charState[index].state = WATCH_NPC_STATE_WATCHING;
    }
    else if (g_charState[index].state == WATCH_NPC_STATE_WATCHING)
    {
        g_characterModels[index].PlayMotion(WATCH_NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
        player.z += WATCH_NPC_PLAYER_EYE_HEIGHT;
        self.z += WATCH_NPC_SELF_EYE_HEIGHT;
        if ((g_charState[index].sawPlayer |=
             Char_CanSee(&self, &player, g_characterModels[index].heading,
                         (float)(g_charState[index].values[WATCH_NPC_VALUE_SIGHT] * WATCH_NPC_SIGHT_UNIT),
                         WATCH_NPC_SIGHT_ANGLE, WATCH_NPC_SIGHT_NEAR, WATCH_NPC_CHECK_WALLS)) != 0 ||
            g_charToldToReact[index] != 0)
        {
            if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != WATCH_NPC_ALARM_PROMPT)
            {
                g_keyPromptShown[index] = 1;
                g_keyPromptKey[index] = WATCH_NPC_ALARM_PROMPT;
            }
            g_keyPromptFrames[index] = WATCH_NPC_ALARM_PROMPT_FRAMES;
            float cameraSquared = (g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
            if (cameraSquared < WATCH_NPC_SOUND_RANGE_SQUARED || index < WATCH_NPC_ALWAYS_SOUND_SLOT)
            {
                Vec4 position;
                position.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(WATCH_NPC_ALARM_SOUND, &position, 1.0f);
            }
            g_charState[index].state = WATCH_NPC_STATE_ALARMED;
            g_charState[index].values[WATCH_NPC_VALUE_WAIT] = 0;
            Enemies_WakeNearby(index, WATCH_NPC_WAKE_RADIUS, 0);
        }
    }
    else if (g_charState[index].state == WATCH_NPC_STATE_ALARMED)
    {
        float target = (float)atan2(player.x - self.x, player.y - self.y) + NPC_TURN_HALF;
        g_charState[index].values[WATCH_NPC_VALUE_WAIT]++;
        g_characterModels[index].targetHeading = target;
        float turn = g_characterModels[index].heading - target;
        while (turn > ANGLE_HALF_TURN)
            turn -= ANGLE_FULL_TURN;
        while (turn < -ANGLE_HALF_TURN)
            turn += ANGLE_FULL_TURN;
        if (turn < WATCH_NPC_FACING_ANGLE)
            g_characterModels[index].PlayMotion(WATCH_NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
        else
            g_characterModels[index].PlayMotion(WATCH_NPC_MOTION_TURN, 0, NPC_MOTION_BLEND, 1.0f);
        if (g_charState[index].values[WATCH_NPC_VALUE_WAIT] == WATCH_NPC_TURN_FRAMES)
            Char_ResetState(index);
    }
    g_charToldToReact[index] = 0;
    return 0;
}
