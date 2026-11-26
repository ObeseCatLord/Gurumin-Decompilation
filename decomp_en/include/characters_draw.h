// Drawing every character (EN build): the draw list of the character slots and the outline settings.
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_CHARACTERS_DRAW_H
#define GURUMIN_EN_CHARACTERS_DRAW_H

#include "char.h"
#include "vec4.h"

// Slots of the characters to draw this frame, in draw order, and how many there are. EN 0x009e1610, 0x009e13d0 (JP DAT_10315140, DAT_104c0054).
extern "C" int g_charDrawList[CHAR_SLOT_COUNT];
extern "C" int g_charDrawCount;
// Per character slot: the fade value g_drawFade takes while the slot is drawn (used from slot 0x3a). EN 0x00a35ce8 (JP DAT_10447bd0).
extern "C" float g_charDrawFade[CHAR_SLOT_COUNT];
// Per character slot: non-zero = drawn without the area fade (g_drawFade stays 0). EN 0x00bc87d8 (JP DAT_1020d438).
extern "C" char g_charDrawPlain[CHAR_SLOT_COUNT];
// Outline settings: the texture pair selector (odd/even), the character slot that gets an outline (0 = none), 0 = the
// outline is added to the scene otherwise darkened into it, and its opacity in percent.
// EN 0x00a3638d, 0x00a3638f, 0x00b493f1, 0x00b493f2 (JP DAT_104c02c4 to DAT_104c02c7).
extern "C" unsigned char g_outlineParity;
extern "C" unsigned char g_outlineSlot;
extern "C" unsigned char g_outlineDarken;
extern "C" unsigned char g_outlinePercent;
// Texture slot of each outline texture (the first entry of each pair). EN 0x00da7260 (JP DAT_10836100).
extern "C" int g_outlineTextures[2][2];

// Queues the model of every character slot in the draw list under `view` * `projection`, with the outline pass for the
// selected slot. Returns 1. EN 0x5d5190 (JP 0x1000c720).
int Characters_Draw(Matrix4 *view, const Matrix4 &projection, Vec4 *defaultLight);

#endif
