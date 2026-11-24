// Playback state init and effect option defaults (EN build). Ported from the matched JP source decomp/src/model_state.cpp.
#include "anim_model.h"
#include "effect_options.h"
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
