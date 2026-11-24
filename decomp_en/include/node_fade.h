// Nodes whose transparency follows the player's distance and the region they lie in (node_fade.cpp; JP:
// render_vars.h DAT_1021a1e0, DAT_10254ac0, DAT_104c03b0). This is the one declaration of these globals in decomp_en.
#ifndef GURUMIN_EN_NODE_FADE_H
#define GURUMIN_EN_NODE_FADE_H

#include "node.h"
#include "vec4.h"

// Fading nodes the game tracks at most.
const int FADE_NODE_COUNT = 0x60;

extern "C" Node *g_fadeNodes[FADE_NODE_COUNT];        // EN 0x0091da48 (JP DAT_1021a1e0): the fading nodes (0 = unused entry)
extern "C" int g_fadeNodeCount;                       // EN 0x0091dbc8 (JP DAT_104c03b0): entries of g_fadeNodes in use
extern "C" Node *g_fadeNodeOwners[FADE_NODE_COUNT];   // EN 0x0091dbd0 (JP DAT_10254ac0): map region node each one lies in (0 = none)
extern "C" Matrix4 g_viewerMatrix;                    // EN 0x015b2d60 (JP DAT_1086df38): the viewer's matrix; its translation is where the player stands

// Updates every fading node's alpha and its parent's glow. EN 0x5b4120 (JP 0x10017fc0).
extern "C" int FadeNodes_Update(void);

#endif
