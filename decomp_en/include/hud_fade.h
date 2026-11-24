// The dark HUD panel that fades in over the game (hud_fade.cpp; JP: DAT_104bf8c0, DAT_104bff00, DAT_104bff0c,
// DAT_102024a0). This is the one declaration of these globals in decomp_en: include this header.
#ifndef GURUMIN_EN_HUD_FADE_H
#define GURUMIN_EN_HUD_FADE_H

// Highest value of g_fadePanelLevel (the panel is fully opaque).
const float FADE_PANEL_LEVEL_MAX = 15.0f;

extern "C" int g_gameMode;           // EN 0x00a20814 (JP DAT_104bf8c0): mode the game is in; 0 = playing the map
extern "C" int g_fadePanelEnabled;   // EN 0x0092f270 (JP DAT_104bff00): non-zero = the panel is in use
extern "C" int g_fadePanelLevel;     // EN 0x0092f27c (JP DAT_104bff0c): 0..15, the panel's alpha in fifteenths
extern "C" int g_fadePanelTexture;   // EN 0x0090839c (JP DAT_102024a0): texture slot drawn inside the panel, 0 or less = none

// Draws the dark panel (and the texture g_fadePanelTexture when one is set) with an alpha of level / 15. EN 0x5cf1a0 (JP 0x1000f4d0).
extern "C" void Hud_DrawFadePanel(void);

#endif
