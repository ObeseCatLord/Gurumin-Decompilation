// Model definition commands (EN build). No matched JP twin: ported from the JP draft and the EN bytes.
#include <string.h>

#include "anim_model.h"
#include "char.h"
#include "model_def.h"

// "SET" command layer whose first two values are remembered in the definition, and the "DAT" value that selects the
// alternate player motion set.
const int MODEL_SET_LAYER_REMEMBERED = 2;
const int MODEL_DAT_PLAYER_ALT = 4;

// Model_ApplyCommands: see the declaration in model_def.h. Called when a character's model is loaded and when the
// scene scripts reset a slot.
extern "C" void Model_ApplyCommands(int model, int slot)
{
    int modelNumber;
    if (slot >= 0)
    {
        modelNumber = model;
    }
    else
    {
        slot = model;
        modelNumber = g_charRecords[slot].params[PARAM_MODEL];
    }
    for (int i = 0; i < g_modelDefs[modelNumber].commandCount; i++)
    {
        if (strncmp(g_modelDefs[modelNumber].commands[i], "SET", MODEL_COMMAND_NAME_LENGTH) == 0)
        {
            g_characterModels[slot].SetLayer(g_modelDefs[modelNumber].params[i][0], g_modelDefs[modelNumber].params[i][1], g_modelDefs[modelNumber].params[i][2], g_modelDefs[modelNumber].params[i][3], g_modelDefs[modelNumber].params[i][4], g_modelDefs[modelNumber].params[i][5]);
            if (g_modelDefs[modelNumber].params[i][0] == MODEL_SET_LAYER_REMEMBERED)
            {
                g_modelDefs[modelNumber].m_0024 = g_modelDefs[modelNumber].params[i][2];
                g_modelDefs[modelNumber].m_0028 = g_modelDefs[modelNumber].params[i][3];
            }
        }
        else if (strncmp(g_modelDefs[modelNumber].commands[i], "DIS", MODEL_COMMAND_NAME_LENGTH) == 0)
        {
            g_characterModels[slot].SetFollowUp(g_modelDefs[modelNumber].params[i][0], g_modelDefs[modelNumber].params[i][1], g_modelDefs[modelNumber].params[i][2]);
        }
        else if (strncmp(g_modelDefs[modelNumber].commands[i], "RND", MODEL_COMMAND_NAME_LENGTH) == 0)
        {
            g_characterModels[slot].SetSlotOffset(g_modelDefs[modelNumber].params[i][0], g_modelDefs[modelNumber].params[i][1], g_modelDefs[modelNumber].params[i][2], g_modelDefs[modelNumber].params[i][3]);
        }
        else if (strncmp(g_modelDefs[modelNumber].commands[i], "DAT", MODEL_COMMAND_NAME_LENGTH) == 0)
        {
            if (slot == 0)
            {
                if (g_modelDefs[modelNumber].params[i][0] == MODEL_DAT_PLAYER_ALT)
                    g_characterModels[slot].SetSlotSource(g_motionSourcePlayerAlt, g_modelDefs[modelNumber].params[i][0], g_modelDefs[modelNumber].params[i][1]);
                else
                    g_characterModels[slot].SetSlotSource(g_motionSourcePlayer, g_modelDefs[modelNumber].params[i][0], g_modelDefs[modelNumber].params[i][1]);
            }
            else
            {
                g_characterModels[slot].SetSlotSource(g_motionSourceCharacter, g_modelDefs[modelNumber].params[i][0], g_modelDefs[modelNumber].params[i][1]);
            }
        }
    }
}
