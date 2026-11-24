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

// Value passed as the last frame of a model that plays to its end.
const int EFFECT_PLAY_TO_END = -1;
// Slot number returned when no slot is free.
const int EFFECT_NO_SLOT = -1;

// Index of the first free slot of the effect model table, or -1 when all 64 are in use.
static int FindFreeEffectModel()
{
    for (int i = 0; i < EFFECT_MODEL_COUNT; i++)
    {
        if (g_effectModels[i].node == 0)
            return i;
    }
    return EFFECT_NO_SLOT;
}

// Places effect model `path` at (x, y, z) in a free slot, starting at frame `start` and playing to the end, rotated
// by (ox, oy, oz). Returns the slot, or -1 when all 64 are in use. Called through the script command table.
extern "C" int EffectModel_Place(const char *path, float x, float y, float z, int start, float ox, float oy, float oz)
{
    int slot = FindFreeEffectModel();
    if (slot >= 0)
    {
        g_effectModels[slot].Setup(path, 0, start, EFFECT_PLAY_TO_END);
        g_effectModels[slot].AddKey(0, x, y, z);
        g_effectModels[slot].SetRotation(ox, oy, oz);
        g_effectModels[slot].visible = 1;
        return slot;
    }
    return EFFECT_NO_SLOT;
}

// Same as EffectModel_Place with a colour (r, g, b, a) for the effect.
extern "C" int EffectModel_PlaceColoured(const char *path, float x, float y, float z, int start, float ox, float oy,
                                         float oz, const Vec4 &colour)
{
    int slot = FindFreeEffectModel();
    if (slot >= 0)
    {
        g_effectModels[slot].Setup(path, 0, start, EFFECT_PLAY_TO_END);
        g_effectModels[slot].color.Copy(colour);
        g_effectModels[slot].AddKey(0, x, y, z);
        g_effectModels[slot].SetRotation(ox, oy, oz);
        g_effectModels[slot].visible = 1;
        return slot;
    }
    return EFFECT_NO_SLOT;
}
