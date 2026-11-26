// An idle NPC that reacts when the player comes near (EN build). Ported from the JP draft decomp/src/char_move.cpp
// (Npc_GreetWhenNear, 0x100b5550), which is not matched there.
#include <math.h>

#include "anim_model.h"
#include "camera.h"
#include "char_move.h"
#include "dialogs.h"
#include "engine.h"
#include "npc_facing.h"
#include "npc_greet.h"
#include "sound_effects.h"

// CharState::values entries of the NPC: the talk code (line number in the tens and units, trigger radius in hundreds) and
// the frames left before the NPC may speak again.
const int NEAR_VALUE_TALK = 1;
const int NEAR_VALUE_WAIT = 0;
// The talk code is radius * NEAR_TALK_RADIUS_SCALE + line; the player is "near" within radius * NEAR_RADIUS_UNIT units.
const int NEAR_TALK_RADIUS_SCALE = 100;
const int NEAR_RADIUS_UNIT = 100;
// NPC states: not started, waiting for the player, reacting (turning to the player).
const int NEAR_STATE_START = 0;
const int NEAR_STATE_WAIT = 1;
const int NEAR_STATE_REACT = 2;
// Radius of the enemies woken when the NPC reacts, and the sound it plays at its position.
const float NEAR_WAKE_RADIUS = 800.0f;
const int NEAR_REACT_SOUND = 0x20c;
// The reaction sound is only played within this squared distance of the camera target, or always for slots below
// NEAR_ALWAYS_SOUND_SLOT.
const float NEAR_SOUND_RANGE_SQUARED = 6250000.0f;
const int NEAR_ALWAYS_SOUND_SLOT = 0x3a;
// Key prompt the NPC shows when it reacts, and for how many frames; frames it keeps turning to the player.
const int NEAR_REACT_PROMPT = 15;
const int NEAR_REACT_PROMPT_FRAMES = 60;
const int NEAR_TURN_FRAMES = 30;
// Chances per frame out of NEAR_CHANCE_RANGE: the idle-motion voice and the talk line.
const int NEAR_CHANCE_RANGE = 10000;
const int NEAR_IDLE_VOICE_CHANCE = 100;
const int NEAR_TALK_CHANCE = 200;
// Voice played while the idle motion runs, with the motion speed it adds.
const int NEAR_IDLE_VOICE = 0x2a3;
const float NEAR_IDLE_SPEED = 24.0f;
// Talk line numbers above NEAR_LINE_SPLIT use prompt keys numbered from NEAR_LINE_KEY_BASE instead of line - 1.
const int NEAR_LINE_SPLIT = 20;
const int NEAR_LINE_KEY_BASE = 1979;
// Frames the NPC waits after speaking.
const int NEAR_TALK_WAIT_FRAMES = 75;

// An idling NPC that reacts to the player: it plays voice 0x2a3 (speeding up its motion) now and then while its idle
// motion is enabled, and voice 0x209..0x20b with its talk prompt every 75 frames or more. While it has not reacted yet it
// reacts when told to, when it already saw the player or when the player comes within the radius of its talk code: it
// wakes the enemies within 800 units, plays sound 0x20c at its position (within 2500 units of the camera target, or always
// for slots below 0x3a), shows prompt 15 for 60 frames and turns to face the player for 30 frames before its state is
// reset. Returns 0. Called by the NPC behaviour dispatcher.
extern "C" int Npc_GreetWhenNear(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distanceSquared = (player - self).LengthSquared3();
    if (g_charState[index].state < NEAR_STATE_REACT &&
        (g_charToldToReact[index] != 0 || g_charState[index].sawPlayer != 0 ||
         distanceSquared < (float)(g_charState[index].values[NEAR_VALUE_TALK] / NEAR_TALK_RADIUS_SCALE *
                                   (g_charState[index].values[NEAR_VALUE_TALK] / NEAR_TALK_RADIUS_SCALE) *
                                   (NEAR_RADIUS_UNIT * NEAR_RADIUS_UNIT))))
    {
        Enemies_WakeNearby(index, NEAR_WAKE_RADIUS, 0);
        g_charState[index].state = NEAR_STATE_REACT;
        g_charToldToReact[index] = 0;
        g_charState[index].values[NEAR_VALUE_WAIT] = 0;
        float cameraSquared = (g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
        if (cameraSquared < NEAR_SOUND_RANGE_SQUARED || index < NEAR_ALWAYS_SOUND_SLOT)
        {
            Vec4 position;
            position.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                         g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                         g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
            Sound_PlayEffect(NEAR_REACT_SOUND, &position, 1.0f);
        }
        if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != NEAR_REACT_PROMPT)
        {
            g_keyPromptShown[index] = 1;
            g_keyPromptKey[index] = NEAR_REACT_PROMPT;
        }
        g_keyPromptFrames[index] = NEAR_REACT_PROMPT_FRAMES;
    }
    if (g_charState[index].state == NEAR_STATE_START)
    {
        g_charState[index].state = NEAR_STATE_WAIT;
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
        return 0;
    }
    if (g_charState[index].state == NEAR_STATE_WAIT)
    {
        if (g_characterModels[index].idleMotionEnabled != 0 && Random() % NEAR_CHANCE_RANGE < NEAR_IDLE_VOICE_CHANCE)
        {
            Character_PlaySound(index, NEAR_IDLE_VOICE, 0, 0);
            g_characterModels[index].AddMotionSpeed(NEAR_IDLE_SPEED);
        }
        if (g_charState[index].values[NEAR_VALUE_TALK] % NEAR_TALK_RADIUS_SCALE > 0 &&
            g_charState[index].values[NEAR_VALUE_WAIT] == 0 && Random() % NEAR_CHANCE_RANGE < NEAR_TALK_CHANCE)
        {
            Character_PlaySound(index, Random() % NPC_TALK_SOUND_COUNT + NPC_TALK_SOUND_FIRST, 0, 0);
            int line = g_charState[index].values[NEAR_VALUE_TALK] % NEAR_TALK_RADIUS_SCALE;
            if (line > NEAR_LINE_SPLIT)
                KeyPrompt_Show(index, line + NEAR_LINE_KEY_BASE, NEAR_REACT_PROMPT_FRAMES);
            else
                KeyPrompt_Show(index, line - 1, NEAR_REACT_PROMPT_FRAMES);
            g_charState[index].values[NEAR_VALUE_WAIT] = NEAR_TALK_WAIT_FRAMES;
        }
        if (g_charState[index].values[NEAR_VALUE_WAIT] > 0)
        {
            g_charState[index].values[NEAR_VALUE_WAIT]--;
            return 0;
        }
    }
    else if (g_charState[index].state == NEAR_STATE_REACT)
    {
        g_characterModels[index].targetHeading =
            (float)atan2(player.x - self.x, player.y - self.y) + NPC_TURN_HALF;
        if (++g_charState[index].values[NEAR_VALUE_WAIT] > NEAR_TURN_FRAMES)
            Char_ResetState(index);
    }
    return 0;
}
