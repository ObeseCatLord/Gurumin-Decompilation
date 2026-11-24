// Effect models (EN build). Ported from the matched JP source decomp/src/effect_model.cpp.
#include "effect_model.h"
#include "engine_memory.h"
#include "node_data.h"

// Constructor of one effect slot: the colour, rotation, matrix and key bank members construct themselves.
EffectModel::EffectModel()
{
}

// EffectModel::Free: stops the effect: forgets the frame counter, releases and frees the model node and makes the slot
// free again. Returns 1. Called when an effect model ends.
int EffectModel::Free()
{
    visible = 0;
    frame = 0;
    if (node != 0)
    {
        node->Release();
        g_engineFree(node);
    }
    node = 0;
    return 1;
}
