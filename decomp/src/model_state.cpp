// Model playback state.

#include <string.h>

#include "node.h"
#include "anim_model.h"

#include "effect_manager.h"

// Clears the collision flag of every loaded model in the 0x100-entry model table DAT_10faaa88.
extern "C" int FUN_101619a0(void)
{
    for (int i = 0; i < 0x100; i++)
    {
        if (DAT_10faaa88[i] != 0)
            DAT_10faaa88[i]->hasCollision = 0;
    }
    return 0;
}

// Constructor (see anim_channels.cpp for why callers treat it as throw()).
EffectOptions::EffectOptions() throw()
{
    owner = 0;
    settings[7] = 0;
    m_0004 = 0xa2c2a;
    settings[4] = 0;
    settings[5] = 0;
    settings[0] = 0;
    m_0008 = 1.0f;
    settings[1] = 0;
    settings[3] = 0;
    m_0034 = 0;
    m_0038 = 0;
    m_003c = 0;
    m_0040 = 0;
    m_0000 = 0;
    settings[2] = 1;
    m_000e = 0;
    flags = 0;
    list = 0;
    m_0010 = 0;
    m_0014 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
}

// Resets a newly allocated playback object: speed 1, no node, all tables cleared and the 16 slot ids at -1.
void Playback::Init()
{
    m_000c = 0;
    m_0018 = 0;
    layer = -1;
    frame = 0;
    m_0004 = 0;
    m_0024 = 0;
    m_0008 = 1.0f;
    blending = 0;
    m_0bec = 0;
    m_7c78 = -1;
    m_0000 = 0;
    memset(m_00ac, 0, sizeof(m_00ac));
    memset(m_052c, 0, sizeof(m_052c));
    for (int i = 0; i < 16; i++)
    {
        slotSource[i] = 0;
        slotLayer[i] = -1;
    }
    m_0010 = 0;
    m_0014 = 0;
    m_0018 = 0;
    frame = 0;
    m_0028 = 0;
    layer = -1;
}

// 0x1016a100 (Playback::SetLayer). Sets up animation layer `layer` (0..0x8f) to play from frame `first` to `last` (backwards when last < first) with
// the values a, b, c, and resets its per-layer tables. Returns 1, or 0 for a layer number out of range.
// The 16 slot offsets and the 8 used layer parameters are cleared with one store each, as the original does.
int Playback::SetLayer(int layer, int a, int first, int last, int b, int c)
{
    if (layer > 0x8f)
        return 0;
    m_0e30[layer] = b;
    m_076c[layer] = a;
    m_09ac[layer] = c;
    m_02ec[layer] = first;
    m_052c[layer] = last;
    m_00ac[layer] = 1;
    m_0bf0[layer] = first <= last ? 1 : -1;
    slotOffsetX[layer][0] = 0;
    slotOffsetX[layer][1] = 0;
    slotOffsetX[layer][2] = 0;
    slotOffsetX[layer][3] = 0;
    slotOffsetX[layer][4] = 0;
    slotOffsetX[layer][5] = 0;
    slotOffsetX[layer][6] = 0;
    slotOffsetX[layer][7] = 0;
    slotOffsetX[layer][8] = 0;
    slotOffsetX[layer][9] = 0;
    slotOffsetX[layer][10] = 0;
    slotOffsetX[layer][11] = 0;
    slotOffsetX[layer][12] = 0;
    slotOffsetX[layer][13] = 0;
    slotOffsetX[layer][14] = 0;
    slotOffsetX[layer][15] = 0;
    layerParams[layer][0] = -2;
    layerParams[layer][1] = -2;
    layerParams[layer][2] = -2;
    layerParams[layer][3] = -2;
    layerParams[layer][4] = -2;
    layerParams[layer][5] = -2;
    layerParams[layer][6] = -2;
    layerParams[layer][7] = -2;
    return 1;
}

// Binds the playback to `node` and resets it to the start; the frame count is taken from the node when it has
// more than 8 frames (shorter or missing animations play as 0 frames). Returns 1.
int Playback::BindNode(Node *node)
{
    layer = -1;
    m_7c78 = -1;
    m_0018 = 0;
    frame = 0;
    m_0024 = 0;
    blending = 0;
    m_0bec = 0;
    m_7c80 = node;
    if (node != 0)
    {
        int frames = node->M_10199340();
        if (frames > 8)
        {
            frameCount = frames;
            return 1;
        }
    }
    frameCount = 0;
    return 1;
}

// Same as BindNode, binding the playback to `owner`'s root node.
int Playback::BindModel(AnimModel *owner)
{
    layer = -1;
    m_7c78 = -1;
    m_0018 = 0;
    frame = 0;
    m_0024 = 0;
    blending = 0;
    m_0bec = 0;
    m_7c80 = owner->root;
    if (owner->root != 0)
    {
        int frames = owner->root->M_10199340();
        if (frames > 8)
        {
            frameCount = frames;
            return 1;
        }
    }
    frameCount = 0;
    return 1;
}

#include "render_vars.h"

// 0x1016a2e0 (Playback::Play). Starts layer `layer` at `speed`. Nothing happens (returns 0) for an inactive or
// out-of-range layer, or, unless `restart` is set, when the layer is already playing, being blended into or is the
// one the current layer chains to (m_076c); a layer listed in the current layer's follow-up table (m_5870) is
// left to come up by itself (returns 1). Without `blendFrames` the layer starts at once from its first frame;
// otherwise the playback blends into it over `blendFrames` + 1 frames (at most 15). Returns 1.
int Playback::Play(int newLayer, int restart, int blendFrames, float speed)
{
    if (newLayer < 0 || newLayer > 0x8f || m_00ac[newLayer] == 0
        || (layer == newLayer && restart == 0 && m_076c[newLayer] != -1)
        || (m_0bec == 2 && m_7c78 == newLayer))
        return 0;
    if (layer != -1 && blending != 0 && nextLayer == newLayer && restart == 0)
        return 0;
    if (layer != -1 && restart == 0 && layer == m_076c[newLayer])
        return 0;
    if (layer != -1 && restart == 0)
    {
        for (int i = 0; i < 8; i++)
        {
            int from = layer;
            if (layer == 0x90)
                from = nextLayer;
            if (layerParams[from][i] == newLayer)
                return 1;
        }
    }
    m_7c78 = -1;
    m_0bec = m_09ac[newLayer];
    m_0024 = 0;
    m_0008 = speed;
    if (blendFrames == 0)
    {
        nextLayer = 0;
        blending = 0;
        layer = newLayer;
        frame = m_02ec[newLayer];
        m_0004 = (float)m_02ec[newLayer];
        m_0bec = m_09ac[newLayer];
        return 1;
    }
    blending = 1;
    nextLayer = newLayer;
    if (m_7c80 != 0)
    {
        if (blendFrames > 13)
            blendFrames = 14;
        DAT_110175f8++;
        m_7c80->BlendFrames(frame, m_02ec[newLayer], blendFrames + 1);
        m_0014 = blendFrames;
    }
    layer = 0x90;
    frame = -1;
    m_0004 = -1.0f;
    return 1;
}
