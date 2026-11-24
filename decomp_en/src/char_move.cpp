// Character movement (EN build). Ported from the matched JP source decomp/src/char_move.cpp.
#include "char_move.h"
#include "anim_model.h"
#include "char.h"
#include "dialogs.h"
#include "engine.h"

// Character_SetMove: sets the movement impulse of character `index` from (x, y, z): half of it in the walking mode
// unless the idle motion is active, otherwise 0.7 of it, raised to 1.4 while the character is nearly standing still
// (speed shorter than 1). Called by the enemy and player movement code. Returns 0.
extern "C" int Character_SetMove(int index, float x, float y, float z)
{
    if (g_characterModels[index].idleMotionEnabled != IDLE_MOTION_ACTIVE && g_characterModels[index].mode == MODEL_MODE_WALKING)
    {
        g_characterModels[index].move.x = x * MOVE_SCALE_WALKING;
        g_characterModels[index].move.y = y * MOVE_SCALE_WALKING;
        g_characterModels[index].move.z = z * MOVE_SCALE_WALKING;
        return 0;
    }
    g_characterModels[index].move.x = x * MOVE_SCALE_NORMAL;
    g_characterModels[index].move.y = y * MOVE_SCALE_NORMAL;
    g_characterModels[index].move.z = z * MOVE_SCALE_NORMAL;
    if (g_characterModels[index].velocity.LengthSquared3() < STANDING_SPEED_SQUARED)
    {
        g_characterModels[index].move.x = x * MOVE_SCALE_STANDING;
        g_characterModels[index].move.y = y * MOVE_SCALE_STANDING;
        g_characterModels[index].move.z = z * MOVE_SCALE_STANDING;
    }
    return 0;
}

// Npc_Idle: keeps NPC `index` standing still in idle motion 2. Now and then (a 3 % chance per frame, then at least 75
// frames apart) an NPC with a talk line plays one of the voice sounds 0x209..0x20b and shows its talk prompt for 60
// frames. Called by the NPC behaviour update. Returns 0.
extern "C" int Npc_Idle(int index)
{
    if (g_charToldToReact[index] != 0 || g_charState[index].sawPlayer != 0)
    {
        g_charToldToReact[index] = 0;
        g_charState[index].sawPlayer = 0;
    }
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_charState[index].state = NPC_STATE_IDLE;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        g_characterModels[index].idling = 1;
        g_characterModels[index].moveStep.Fill(0);
        g_characterModels[index].move.Fill(0);
        g_characterModels[index].velocity.Fill(0);
        g_charTalkPending[index] = 0;
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] > 0)
        {
            g_charState[index].values[CHARSTATE_NPC_WAIT]--;
            return 0;
        }
        if (g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE != 0 &&
            Random() % NPC_TALK_CHANCE_RANGE < NPC_TALK_CHANCE)
        {
            Character_PlaySound(index, Random() % NPC_TALK_SOUND_COUNT + NPC_TALK_SOUND_FIRST, 0, 0);
            KeyPrompt_Show(index,
                           g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE * NPC_PROMPT_KEY_SCALE - 1 +
                               g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE,
                           NPC_PROMPT_FRAMES);
            g_charState[index].values[CHARSTATE_NPC_WAIT] = NPC_TALK_GAP_FRAMES;
        }
    }
    return 0;
}
