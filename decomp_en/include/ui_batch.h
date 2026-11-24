// The 2D triangle batch the menus and overlays draw with, and the full-screen overlay texture (JP: DAT_1021cffc,
// DAT_104c0140). This is the one declaration of these globals in decomp_en: include this header.
#ifndef GURUMIN_EN_UI_BATCH_H
#define GURUMIN_EN_UI_BATCH_H

#include "texture_mgr.h"

extern "C" TriangleBatch g_batch2D;      // EN 0x009e3500 (JP DAT_1021cffc): the 2D batch (falling drops, menus, overlays)
extern "C" int g_overlayTexture;         // EN 0x00a13920 (JP DAT_104c0140): texture slot of the full-screen overlay

// Draws the full-screen overlay quad with additive blending. EN 0x5e0560 (JP 0x1000c340).
extern "C" int Overlay_DrawFullScreen(void);

#endif
