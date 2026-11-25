// A sleeping NPC (EN build). Ported from the matched JP source decomp/src/npc_sleep.cpp.
#include <math.h>
#include "anim_model.h"
#include "char.h"
#include "dialogs.h"
#include "scene_start.h"
#include "char_shots.h"

// Indices in CharState::values of the sleeper's frame counter and its talk id.
const int SLEEP_WAIT_VALUE = 0;
const int SLEEP_TALK_VALUE = 1;
// Talk ids hold a script number in the hundreds and the distance in tens of units in the last two digits.
const int SLEEP_TALK_SCRIPT_DIVISOR = 100;
const int SLEEP_WAKE_DISTANCE_UNIT = 10;
// Squared speed above which a running player wakes the sleeper (speed 16).
const float SLEEP_WAKE_SPEED_SQUARED = 256.0f;
// Behaviour steps.
const int SLEEP_STATE_START = 0;
const int SLEEP_STATE_SLEEPING = 1;
const int SLEEP_STATE_WAKING = 2;
// Motion layer of the sleeper and its motions (layer, first, last).
const int SLEEP_MOTION_LAYER = 2;
// Key prompts: the sleeping one and the ones shown on waking and when it turned to the player.
const int SLEEP_PROMPT_SLEEPING = 10;
const int SLEEP_PROMPT_SLEEPING_FRAMES = 15;
const int SLEEP_PROMPT_WOKEN = 4;
const int SLEEP_PROMPT_WOKEN_FRAMES = 30;
const int SLEEP_PROMPT_TURNED = 9;
const int SLEEP_PROMPT_TURNED_FRAMES = 60;
// Frames the waking step looks around (turning every SLEEP_LOOK_PERIOD frames, half a period apart) and when it turns to
// the player.
const int SLEEP_LOOK_FRAMES = 35;
const int SLEEP_LOOK_PERIOD = 12;
const int SLEEP_LOOK_HALF_PERIOD = 6;
const int SLEEP_TURN_FRAME = 45;
// Angle (40 degrees) the sleeper looks to either side of where it faced.
const float SLEEP_LOOK_ANGLE = 0.69813f;

// Npc_SleepUntilWoken: a sleeper (motion held at its current frame, prompt 10 shown) that wakes when it saw the player and
// its talk id has hundreds, when the player comes within the talk id's line * 10 units running fast (speed over 16) or
// while greetings are on, when told to react or when it was hit: it shows prompt 4, looks left and right (40 degrees
// either side of where it faced, every 6 frames for 35 frames), turns to the player at frame 45 and shows prompt 9
// before its state is reset. Returns 0.
extern "C" int Npc_SleepUntilWoken(int index)
{
    g_characterModels[index].mode = 0;
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    (player - self).Length();
    float distance = (player - self).Length2D();
    g_charTalkPending[index] = 0;
    int awake;
    if ((g_charState[index].sawPlayer != 0 &&
         g_charState[index].values[SLEEP_TALK_VALUE] / SLEEP_TALK_SCRIPT_DIVISOR != 0) ||
        (distance < (float)(g_charState[index].values[SLEEP_TALK_VALUE] % SLEEP_TALK_SCRIPT_DIVISOR *
                            SLEEP_WAKE_DISTANCE_UNIT) &&
         g_characterModels[0].idleMotionEnabled != 0 &&
         g_characterModels[0].velocity.LengthSquared3() > SLEEP_WAKE_SPEED_SQUARED) ||
        (distance < (float)(g_charState[index].values[SLEEP_TALK_VALUE] % SLEEP_TALK_SCRIPT_DIVISOR *
                            SLEEP_WAKE_DISTANCE_UNIT) &&
         g_greetingFrames != 0) ||
        g_charToldToReact[index] != 0 || g_charHitLevel[index] != 0)
    {
        awake = 1;
        g_charToldToReact[index] = 0;
    }
    else
        awake = 0;
    if (g_charState[index].state == SLEEP_STATE_START)
    {
        g_characterModels[index].PlayMotion(SLEEP_MOTION_LAYER, 1, 0, 1.0f);
        g_charState[index].state = SLEEP_STATE_SLEEPING;
    }
    else if (g_charState[index].state == SLEEP_STATE_SLEEPING)
    {
        if (g_characterModels[index].playback->layer == SLEEP_MOTION_LAYER &&
            g_characterModels[index].playback->blending == 0)
        {
            g_characterModels[index].motionFrame = g_characterModels[index].playback->frame;
            g_characterModels[index].walkingRoute = 1;
        }
        if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != SLEEP_PROMPT_SLEEPING)
        {
            g_keyPromptShown[index] = 1;
            g_keyPromptKey[index] = SLEEP_PROMPT_SLEEPING;
        }
        g_keyPromptFrames[index] = SLEEP_PROMPT_SLEEPING_FRAMES;
        if (awake != 0)
        {
            g_charState[index].state = SLEEP_STATE_WAKING;
            KeyPrompt_Show(index, SLEEP_PROMPT_WOKEN, SLEEP_PROMPT_WOKEN_FRAMES);
            g_shotParams[index][0] = g_characterModels[index].heading;
            g_characterModels[index].walkingRoute = 0;
        }
        g_shotSlots[index].position.Copy(player);
    }
    else if (g_charState[index].state == SLEEP_STATE_WAKING)
    {
        g_characterModels[index].PlayMotion(SLEEP_MOTION_LAYER, 0, 5, 1.0f);
        if (++g_charState[index].values[SLEEP_WAIT_VALUE] < SLEEP_LOOK_FRAMES)
        {
            if (g_charState[index].values[SLEEP_WAIT_VALUE] % SLEEP_LOOK_PERIOD == 0)
                g_characterModels[index].targetHeading = g_shotParams[index][0] + SLEEP_LOOK_ANGLE;
            else if (g_charState[index].values[SLEEP_WAIT_VALUE] % SLEEP_LOOK_PERIOD == SLEEP_LOOK_HALF_PERIOD)
                g_characterModels[index].targetHeading = g_shotParams[index][0] - SLEEP_LOOK_ANGLE;
        }
        if (g_charState[index].values[SLEEP_WAIT_VALUE] == SLEEP_TURN_FRAME)
            g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
        if (g_charState[index].values[SLEEP_WAIT_VALUE] == SLEEP_TURN_FRAME)
        {
            g_charTalkPending[index] = 1;
            KeyPrompt_Show(index, SLEEP_PROMPT_TURNED, SLEEP_PROMPT_TURNED_FRAMES);
            Char_ResetState(index);
        }
    }
    if (g_charHitLevel[index] != 0)
    {
        g_charTalkPending[index] = 1;
        if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != SLEEP_PROMPT_TURNED)
        {
            g_keyPromptShown[index] = 1;
            g_keyPromptKey[index] = SLEEP_PROMPT_TURNED;
        }
        g_keyPromptFrames[index] = SLEEP_PROMPT_TURNED_FRAMES;
        Char_ResetState(index);
    }
    return 0;
}
