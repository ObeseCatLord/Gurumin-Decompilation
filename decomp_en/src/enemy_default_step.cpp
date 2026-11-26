// The default step of an enemy (EN build). Ported from the JP source (APP.dll 0x100e21a0, open there) and the EN bytes.
#include "anim_model.h"
#include "char.h"
#include "enemy_ai.h"
#include "enemy_moves_long.h"
#include "game_vars.h"
#include "npc_facing.h"
#include "vec4.h"

// Characters with a special step: the one that hides on early maps, the one that slides while talked to, and the one
// that plays a motion on request.
const int STEP_HIDING_CHARACTER = 14;
const int STEP_SLIDING_CHARACTER = 20;
const int STEP_MOTION_CHARACTER = 11;
// Characters that restart their behaviour every step, and the slot from which all characters do.
const int STEP_RESTART_CHARACTER_A = 0x19;
const int STEP_RESTART_CHARACTER_B = 12;
const int STEP_LAST_FREE_SLOT = 0x39;
// The hiding character is hidden on maps below this one.
const int STEP_HIDE_BELOW_MAP = 20;
// Value of g_talkingNpc while the sliding character is talked to.
const int STEP_TALK_NPC = 0xdc;
// The slide runs while the character's motion frame is in this range (exclusive), 2 units a frame.
const int STEP_SLIDE_FIRST_FRAME = 0x82;
const int STEP_SLIDE_END_FRAME = 0x8d;
const float STEP_SLIDE_SPEED = 2.0f;
// Playback layers: the one the sliding character's blend ends in and the one it walks back in, and the blend of the
// walk back (share of the current position and of the resting place).
const int STEP_LAYER_STAND = 4;
const int STEP_LAYER_RETURN = 2;
const float STEP_RETURN_NEW_SHARE = 0.6f;
const float STEP_RETURN_REST_SHARE = 0.4f;
// Motion the motion character plays on request: layer, start, blend frames.
const int STEP_MOTION_LAYER = 10;
const int STEP_MOTION_BLEND = 5;
// Chance per frame (out of 2) that the turn to the player ends.
const int STEP_TURN_END_CHANCE = 2;

// Character 14 clears its alpha on maps below 20; character 20 being talked to (0xdc) slides 2 units a frame ahead of its
// resting place during motion frames 0x82..0x8c, else follows its motion back to it. Busy characters do nothing;
// characters 12, 14, 20, 25 and above 0x39 restart (20 also sets `important`); character 11 plays motion 10 while
// g_scenePlayMotion is set without g_scenePlayMotionStop. Otherwise it turns to the player, idles, and picks an action
// (65/35) once its count passes 0. Returns 0. Called by the enemy behaviour code.
extern "C" int Enemy_DefaultStep(int index)
{
    if (index == STEP_HIDING_CHARACTER)
    {
        if (g_currentMap < STEP_HIDE_BELOW_MAP)
            g_characterModels[index].alpha = 0;
    }
    else if (index == STEP_SLIDING_CHARACTER && g_talkingNpc == STEP_TALK_NPC)
    {
        Vec4 position;
        if (g_characterModels[index].motionFrame < STEP_SLIDE_END_FRAME &&
            g_characterModels[index].motionFrame > STEP_SLIDE_FIRST_FRAME - 1)
        {
            position.y = (float)(g_characterModels[index].motionFrame - STEP_SLIDE_FIRST_FRAME) * STEP_SLIDE_SPEED;
            position.RotateZ(-g_characterModels[index].heading);
            position.Copy(position + g_npcRestingPlace);
        }
        else
        {
            if (g_characterModels[index].playback->blending == 0)
                return 0;
            if (g_characterModels[index].playback->nextLayer == STEP_LAYER_STAND)
            {
                g_npcRestingPlace.Copy(g_characterModels[index].matrix.GetTranslation());
                return 0;
            }
            if (g_characterModels[index].playback->blending == 0 ||
                g_characterModels[index].playback->nextLayer != STEP_LAYER_RETURN)
                return 0;
            position.Copy(g_characterModels[index].matrix.GetTranslation());
            float z = position.z;
            position.Copy(position * STEP_RETURN_NEW_SHARE + g_npcRestingPlace * STEP_RETURN_REST_SHARE);
            position.z = z;
        }
        g_characterModels[index].SetPosition(position.x, position.y, position.z);
        return 0;
    }
    if (Enemy_IsBusy(index) != 0)
        return 0;
    if (index == STEP_RESTART_CHARACTER_A || index == STEP_RESTART_CHARACTER_B || index == STEP_SLIDING_CHARACTER ||
        index == STEP_HIDING_CHARACTER || index > STEP_LAST_FREE_SLOT)
    {
        if (index == STEP_SLIDING_CHARACTER)
            g_characterModels[index].important = 1;
        g_charState[index].state = ENEMY_BEHAVIOUR_FIRST;
    }
    if (index == STEP_MOTION_CHARACTER && g_scenePlayMotion != 0 && g_scenePlayMotionStop == 0)
    {
        g_characterModels[index].PlayMotion(STEP_MOTION_LAYER, 0, STEP_MOTION_BLEND, 1.0f);
        return 0;
    }
    switch (g_charState[index].state)
    {
    case ENEMY_BEHAVIOUR_FIRST:
        if (Enemy_TurnToPlayer(index, STEP_TURN_END_CHANCE) == 0)
            return 0;
        break;
    case ENEMY_BEHAVIOUR_SECOND:
        if (Enemy_IdleMotion(index) == 0)
            return 0;
        break;
    case ENEMY_BEHAVIOUR_THIRD:
        break;
    default:
        if (++g_charState[index].values[ENEMY_VALUE_PICK_DELAY] > ENEMY_PICK_DELAY_FRAMES)
        {
            g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
            Enemy_PickAction_65_35(index);
        }
        return 0;
    }
    g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
    g_charState[index].state = ENEMY_STATE_FINISHED;
    return 0;
}
