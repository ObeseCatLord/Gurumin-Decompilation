// Playback state init and effect option defaults (EN build). Ported from the matched JP source decomp/src/model_state.cpp.
#include "anim_model.h"
#include "char_move.h"
#include "engine.h"
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

// Chance (out of PLAYBACK_CHANCE_RANGE) is slotOffsetX; the random idle motion waits PLAYBACK_IDLE_COOLDOWN frames.
const int PLAYBACK_CHANCE_RANGE = 1000;
const int PLAYBACK_IDLE_COOLDOWN = 0x82;
// Blend frames of the random idle motion, and of a layer that chains to another (forwards, backwards).
const int PLAYBACK_IDLE_BLEND_FRAMES = 14;
const int PLAYBACK_CHAIN_BLEND_FRAMES = 5;
const int PLAYBACK_CHAIN_BLEND_FRAMES_END = 8;
// Follow-up values of m_076c: stay on the last frame / keep stepping back by one step.
const int PLAYBACK_FOLLOW_HOLD = -2;
const int PLAYBACK_FOLLOW_NONE = -1;

// Playback::Advance: steps the playback by one frame. While blending it counts the frame down and, once the blend is
// over, switches to nextLayer. Otherwise a random idle motion may start first (each slot's chance, out of 1000, is
// slotOffsetX; the motion is slotOffsetY, only when no slot source of that layer is busy). Then the frame moves by the
// layer's speed (layerDirection) in the current direction (m_0024: 0 forwards, 1 backwards). Passing the end of the
// layer plays the layer in m_0e30 (blended), holds (m_076c = -2), loops, or continues on layer m_076c. Returns 1.
// Called every frame for each model with a playback.
int Playback::Advance()
{
    int i = 0;
    if (m_0018 != 0)
        return 1;
    int current = layer;
    if (current == PLAYBACK_NO_LAYER)
        return 1;
    if (blending != 0)
    {
        frame--;
        framePosition = (float)frame;
        if (frame <= -1 - m_0014)
        {
            current = nextLayer;
            blending = 0;
            layer = current;
            frame = m_02ec[current];
            framePosition = (float)frame;
            endState = m_09ac[current];
        }
        return 1;
    }
    if (m_000c > 0)
        m_000c--;
    if (g_idleMotionsSuspended == 0 && m_000c == 0)
    {
        for (i = 0; i < PLAYBACK_SLOT_COUNT; i++)
        {
            if (slotOffsetX[current][i] != 0 && Random() % PLAYBACK_CHANCE_RANGE <= slotOffsetX[current][i])
            {
                int slot = 0;
                while (slotSource[slot] == 0 || slotSource[slot]->busy != 0 ||
                       slotLayer[slot] != slotOffsetY[current][i])
                {
                    slot++;
                    if (slot >= PLAYBACK_SLOT_COUNT)
                    {
                        m_000c = PLAYBACK_IDLE_COOLDOWN;
                        endState = 0;
                        m_7c78 = current;
                        Play(slotOffsetY[current][i], 1, PLAYBACK_IDLE_BLEND_FRAMES, 1.0f);
                        m_7c78 = current;
                        return 1;
                    }
                }
            }
        }
    }
    float step = (float)layerDirection[current];
    if (m_0024 == 0)
    {
        framePosition = step * speed + framePosition;
        frame = (int)framePosition;
        if (layerDirection[current] < 0 && frame < layerLast[current])
        {
            if (m_0e30[current] != -1)
            {
                endState = 0;
                int rewound = frame;
                rewound -= layerDirection[current];
                frame = rewound;
                layer = -1;
                framePosition = (float)rewound;
                Playback *saved = g_lastEnemyPlayback;
                g_lastEnemyPlayback = 0;
                Play(m_0e30[current], 0, PLAYBACK_CHAIN_BLEND_FRAMES, 1.0f);
                g_lastEnemyPlayback = saved;
                return 1;
            }
            int follow = m_076c[current];
            if (follow == PLAYBACK_FOLLOW_HOLD)
            {
                current = layer;
                frame = layerLast[current];
                m_0024 = 1;
                framePosition = (float)frame;
                endState = m_09ac[current];
            }
            else
            {
                m_0024 = 0;
                if (follow == PLAYBACK_FOLLOW_NONE)
                {
                    frame = frame - layerDirection[current];
                    framePosition = (float)frame;
                }
                else
                {
                    current = m_076c[current];
                    layer = current;
                    int first = m_02ec[current];
                    frame = first;
                    framePosition = (float)first;
                    endState = m_09ac[current];
                }
            }
        }
        if (layerDirection[current] > 0 && frame > layerLast[current])
        {
            if (m_0e30[current] != -1)
            {
                endState = 0;
                int rewound = frame;
                rewound -= layerDirection[current];
                frame = rewound;
                layer = -1;
                framePosition = (float)rewound;
                Playback *saved = g_lastEnemyPlayback;
                g_lastEnemyPlayback = 0;
                Play(m_0e30[current], 0, PLAYBACK_CHAIN_BLEND_FRAMES_END, 1.0f);
                g_lastEnemyPlayback = saved;
                return 1;
            }
            int follow = m_076c[current];
            if (follow == PLAYBACK_FOLLOW_HOLD)
            {
                int last = layerLast[layer];
                m_0024 = 1;
                frame = last;
                framePosition = (float)frame;
                endState = m_09ac[layer];
                return 1;
            }
            m_0024 = 0;
            if (follow == PLAYBACK_FOLLOW_NONE)
            {
                frame = frame - layerDirection[current];
                framePosition = (float)frame;
                return 1;
            }
            layer = m_076c[current];
            frame = m_02ec[layer];
            framePosition = (float)frame;
            endState = m_09ac[layer];
        }
    }
    else
    {
        framePosition = framePosition - step * speed;
        frame = (int)framePosition;
        int direction = layerDirection[current];
        if (direction < 0 && frame > m_02ec[current])
        {
            int first = m_02ec[layer];
            m_0024 = 0;
            frame = first;
            framePosition = (float)frame;
            endState = m_09ac[layer];
            return 1;
        }
        if (direction > 0 && frame < m_02ec[current])
        {
            frame = m_02ec[layer];
            m_0024 = 0;
            framePosition = (float)frame;
            endState = m_09ac[layer];
        }
    }
    return 1;
}
