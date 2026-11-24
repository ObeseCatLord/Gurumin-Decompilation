// The effect node pool (EN build): 0x47e nodes, each loaded when its flag is set.
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_EFFECT_NODES_H
#define GURUMIN_EN_EFFECT_NODES_H

#include "node.h"

// Entries of the effect node pool.
const int EFFECT_NODE_COUNT = 0x47e;

extern "C" int g_effectNodeLoaded[EFFECT_NODE_COUNT];   // EN 0x01673458 (JP DAT_10ff1df8): non-zero when the node is in use
extern "C" Node g_effectNodes[EFFECT_NODE_COUNT];       // EN 0x016a57d0 (JP DAT_111be9c8): the nodes (0x18a0 bytes each)

// Bytes of an EffectNameEntry and the offset of its name.
const int EFFECT_NAME_ENTRY_SIZE = 0x3d8;
const int EFFECT_NAME_OFFSET = 0x23b;
// EffectNameEntry::kind values whose model Effect_LoadModel loads.
const int EFFECT_KIND_MODEL_A = 4;
const int EFFECT_KIND_MODEL_B = 10;
const int EFFECT_KIND_MODEL_C = 20;

// One entry of the effect table (kind and model file name; the rest is not recovered yet).
struct EffectNameEntry
{
    int kind;                                                      // 0x000: effect kind
    char m_0004[EFFECT_NAME_OFFSET - 4];                           // 0x004: not recovered
    char name[EFFECT_NAME_ENTRY_SIZE - EFFECT_NAME_OFFSET];        // 0x23b: model file name
};

extern "C" EffectNameEntry g_effectNames[EFFECT_NODE_COUNT];       // EN 0x01dd0358 (JP DAT_1101dde0): kind and model name of each effect

// Loads the model of effect `index` when its kind is 4, 10 or 20, builds its mesh and marks the node used. EN 0x739090 (JP 0x1016fa40).
extern "C" int __stdcall Effect_LoadModel(int index);

// Releases every used node of the effect node pool. EN 0x740d90 (JP 0x1016fab0).
extern "C" int EffectNodes_ReleaseAll(void);

#endif
