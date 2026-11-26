// An NPC walking up to the player (EN build). Ported from the JP draft decomp/src/npc_approach.cpp.
#include <math.h>

#include "anim_model.h"
#include "npc_facing.h"

// Npc_ApproachPlayer: see the declaration in npc_facing.h. Called by the NPC behaviour dispatcher.
extern "C" int Npc_ApproachPlayer(int index)
{
    Vec4 offset = g_characterModels[0].matrix.GetTranslation() - g_characterModels[index].matrix.GetTranslation();
    offset.z *= APPROACH_HEIGHT_WEIGHT;
    float distance = offset.Length();
    if (g_charState[index].state == 0)
    {
        g_charState[index].state = 1;
        g_characterModels[index].mode = 0;
        g_charState[index].values[APPROACH_VALUE_STOP_DISTANCE] =
            g_charState[index].values[APPROACH_VALUE_CODE] % APPROACH_CODE_RANGE * APPROACH_STOP_UNIT;
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
        return 0;
    }
    float turn = g_characterModels[index].heading - g_characterModels[index].targetHeading;
    while (turn > NPC_TURN_HALF)
        turn -= ANGLE_FULL_TURN;
    while (turn < -NPC_TURN_HALF)
        turn += ANGLE_FULL_TURN;
    g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
    g_characterModels[index].targetHeading =
        (float)atan2(g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                     g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y]) +
        NPC_TURN_HALF;
    if (distance < (float)g_charState[index].values[APPROACH_VALUE_STOP_DISTANCE] ||
        g_charState[index].sawPlayer != 0 && g_charState[index].values[APPROACH_VALUE_CODE] / APPROACH_CODE_RANGE != 0)
        Char_ResetState(index);
    return 0;
}
