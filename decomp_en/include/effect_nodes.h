// The effect node pool (EN build): 0x47e nodes, each loaded when its flag is set.
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_EFFECT_NODES_H
#define GURUMIN_EN_EFFECT_NODES_H

#include "node.h"

// Entries of the effect node pool.
const int EFFECT_NODE_COUNT = 0x47e;

extern "C" int g_effectNodeLoaded[EFFECT_NODE_COUNT];   // EN 0x01673458 (JP DAT_10ff1df8): non-zero when the node is in use
extern "C" Node g_effectNodes[EFFECT_NODE_COUNT];       // EN 0x016a57d0 (JP DAT_111be9c8): the nodes (0x18a0 bytes each)

// Releases every used node of the effect node pool. EN 0x740d90 (JP 0x1016fab0).
extern "C" int EffectNodes_ReleaseAll(void);

#endif
