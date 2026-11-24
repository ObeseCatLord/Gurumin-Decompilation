// The UI animation state of the EN build (JP: decomp/include/game_vars.h): the node whose animation the menus play.
#ifndef GURUMIN_EN_UI_ANIM_H
#define GURUMIN_EN_UI_ANIM_H

struct Node;

extern "C" Node *g_uiAnimNode;        // EN 0x00a15284 (JP DAT_104c01dc): the node of the UI animation that plays now
extern "C" int g_uiAnimStep;          // EN 0x00a15288 (JP DAT_104c01e0): frames added per update: 1 forwards, -1 backwards
extern "C" int g_uiAnimFrame;         // EN 0x00a1528c (JP DAT_104c01e4): the frame the animation is at

// Node flag UiAnim_Play sets on the animated node.
const unsigned int NODE_FLAG_UI_ANIM = 0x20;

// Starts playing `node`'s animation forwards, or backwards when `reverse` is non-zero. EN 0x6466f0 (JP 0x100158b0).
extern "C" int UiAnim_Play(Node *node, int reverse);

#endif
