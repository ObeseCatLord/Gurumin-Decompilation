// Playback state init and effect option defaults (EN build). Ported from the matched JP source decomp/src/model_state.cpp.
#include "anim_model.h"
#include "effect_options.h"
#include "node.h"
#include <string.h>

// EffectOptions constructor: no owner, default colour and scale, no flags, only settings[2] on. Called by every
// character behaviour that starts an effect.
EffectOptions::EffectOptions() throw()
{
    owner = 0;
    settings[7] = 0;
    colour = EFFECT_OPTIONS_DEFAULT_COLOUR;
    settings[4] = 0;
    settings[5] = 0;
    settings[0] = 0;
    scale = EFFECT_OPTIONS_DEFAULT_SCALE;
    settings[1] = 0;
    settings[3] = 0;
    boxMin = 0;
    boxMax = 0;
    m_003c = 0;
    m_0040 = 0;
    m_0000 = 0;
    settings[EFFECT_SETTING_DEFAULT_ON] = 1;
    m_000e = 0;
    flags = 0;
    list = 0;
    m_0010 = 0;
    m_0014 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
}

// Playback::Init: resets a newly allocated playback object: speed 1, no layer, all tables cleared and the 16 slot ids
// at -1. Called when a model first needs a playback (AnimModel::GetPlayback).
void Playback::Init()
{
    m_000c = 0;
    m_0018 = 0;
    layer = PLAYBACK_NO_LAYER;
    frame = 0;
    framePosition = 0;
    m_0024 = 0;
    speed = 1.0f;
    blending = 0;
    endState = 0;
    m_7c78 = PLAYBACK_NO_LAYER;
    headerFlag = 0;
    memset(layerActive, 0, sizeof(layerActive));
    memset(layerLast, 0, sizeof(layerLast));
    for (int i = 0; i < PLAYBACK_SLOT_COUNT; i++)
    {
        slotSource[i] = 0;
        slotLayer[i] = PLAYBACK_NO_LAYER;
    }
    m_0010 = 0;
    m_0014 = 0;
    m_0018 = 0;
    layer = PLAYBACK_NO_LAYER;
    frame = 0;
    m_0028 = 0;
}

// Playback::BindNode: binds the playback to `node` and resets it to the start; the frame count is taken from the node
// when it has more than 8 frames (shorter or missing animations count as 0). Returns 1. Called when a model's node
// tree is attached.
int Playback::BindNode(Node *node)
{
    m_0018 = 0;
    layer = PLAYBACK_NO_LAYER;
    frame = 0;
    m_0024 = 0;
    blending = 0;
    endState = 0;
    m_7c78 = PLAYBACK_NO_LAYER;
    boundNode = node;
    int frames = 0;
    if (node != 0)
    {
        int last = node->GetLastFrame();
        if (last > PLAYBACK_MIN_FRAMES)
            frames = last;
    }
    frameCount = frames;
    return 1;
}

// Playback::BindModel: BindNode for the model's own node tree (its root); the same reset. Returns 1. Called when a
// model is loaded or its playback is created.
int Playback::BindModel(AnimModel *owner)
{
    m_0018 = 0;
    layer = PLAYBACK_NO_LAYER;
    frame = 0;
    m_0024 = 0;
    blending = 0;
    endState = 0;
    m_7c78 = PLAYBACK_NO_LAYER;
    boundNode = owner->root;
    int frames = 0;
    if (owner->root != 0)
    {
        int last = owner->root->GetLastFrame();
        if (last > PLAYBACK_MIN_FRAMES)
            frames = last;
    }
    frameCount = frames;
    return 1;
}

// Playback::SetLayer: defines motion layer `layer` (first to last frame, direction from their order) and clears its
// slot offsets and follow-up layers. Returns 0 for a layer out of range. Called when a model's motion table is set up.
int Playback::SetLayer(int layerIndex, int a, int first, int last, int b, int c)
{
    if (layerIndex > PLAYBACK_LAYER_COUNT - 1)
        return 0;
    m_0e30[layerIndex] = b;
    m_076c[layerIndex] = a;
    m_09ac[layerIndex] = c;
    layerLast[layerIndex] = last;
    layerActive[layerIndex] = 1;
    layerDirection[layerIndex] = (first <= last) * 2 - 1;
    m_02ec[layerIndex] = first;
    for (int slot = 0; slot < PLAYBACK_SLOT_COUNT; slot++)
        slotOffsetX[layerIndex][slot] = 0;
    layerParams[layerIndex][0] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][1] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][2] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][3] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][4] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][5] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][6] = PLAYBACK_NO_FOLLOWUP;
    layerParams[layerIndex][7] = PLAYBACK_NO_FOLLOWUP;
    return 1;
}

// Playback::Play: starts motion layer `layerIndex` at `playSpeed`. Does nothing (returns 0) for an inactive or
// out-of-range layer, or, unless `restart` is set, when the layer already plays, is being blended into, or is the
// layer the current one chains to. A layer in the current layer's follow-up list comes up by itself (returns 1).
// Without blend frames the layer starts at once from its first frame; otherwise the playback blends into it over
// blendFrames + 1 frames (at most PLAYBACK_BLEND_MAX + 1). Returns 1. Called whenever a character changes motion.
int Playback::Play(int layerIndex, int restart, int blendFrames, float playSpeed)
{
    unsigned int requested = layerIndex;
    if (requested > PLAYBACK_LAYER_COUNT - 1 || layerActive[layerIndex] == 0)
        return 0;
    unsigned int current = layer;
    if ((current == requested && restart == 0 && m_076c[layerIndex] != -1) ||
        (endState == PLAYBACK_END_HELD && m_7c78 == layerIndex))
        return 0;
    if (current != PLAYBACK_NO_LAYER)
    {
        if (blending != 0 && nextLayer == layerIndex && restart == 0)
            return 0;
        if (current != PLAYBACK_NO_LAYER)
        {
            if (restart == 0 && current == m_076c[layerIndex])
                return 0;
            if (current != PLAYBACK_NO_LAYER && restart == 0)
            {
                for (int i = 0; i < PLAYBACK_FOLLOWUP_COUNT; i++)
                {
                    unsigned int from = current;
                    if (from == PLAYBACK_BLEND_LAYER)
                        from = nextLayer;
                    if (layerParams[from][i] == layerIndex)
                        return 1;
                }
            }
        }
    }
    m_7c78 = PLAYBACK_NO_LAYER;
    endState = m_09ac[layerIndex];
    m_0024 = 0;
    speed = playSpeed;
    if (blendFrames == 0)
    {
        nextLayer = 0;
        blending = 0;
        layer = layerIndex;
        frame = m_02ec[layerIndex];
        framePosition = (float)m_02ec[layerIndex];
        endState = m_09ac[layerIndex];
        return 1;
    }
    blending = 1;
    nextLayer = layerIndex;
    if (boundNode != 0)
    {
        int steps = PLAYBACK_BLEND_MAX;
        if (blendFrames < PLAYBACK_BLEND_MAX)
            steps = blendFrames;
        g_motionBlendCount++;
        boundNode->BlendFrames(frame, m_02ec[layerIndex], steps + 1);
        m_0014 = steps;
    }
    layer = PLAYBACK_BLEND_LAYER;
    frame = -1;
    framePosition = -1.0f;
    return 1;
}
