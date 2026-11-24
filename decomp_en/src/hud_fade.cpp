// The fading HUD panel (JP twin: hud_fade.cpp).

#include "dialogs.h"
#include "hud_fade.h"
#include "texture_mgr.h"

// Position and size of the panel, and of the texture frame drawn inside it (640x480 coordinates).
const int PANEL_LEFT = 0x19d;
const int PANEL_TOP = 0x74;
const int PANEL_WIDTH = 200;
const int PANEL_HEIGHT = 0xcc;
const int FRAME_LEFT = 0x19f;
const int FRAME_TOP = 0x76;
const int FRAME_RIGHT = 0x263;
const int FRAME_BOTTOM = 0x13e;

// Draws the dark panel (and the texture g_fadePanelTexture when one is set) with an alpha of level / 15 while the
// overlay flag is set or the game is in mode 0 (playing the map). Called once per frame by the HUD pass.
extern "C" void Hud_DrawFadePanel(void)
{
    if (g_overlayFlag != 0 || g_gameMode == 0)
    {
        Render_Command(RENDER_DEPTH_WRITE, 0, 0);
        if (g_fadePanelEnabled != 0 || g_fadePanelLevel != 0)
        {
            g_dialogGame.DrawPanelAlt(PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT, 0, g_fadePanelLevel / FADE_PANEL_LEVEL_MAX);
            if (g_fadePanelTexture > 0)
                g_textures[g_fadePanelTexture].DrawFrameAltStretched(FRAME_LEFT, FRAME_TOP, FRAME_RIGHT, FRAME_BOTTOM, 0,
                                                                     g_fadePanelLevel / FADE_PANEL_LEVEL_MAX, 1, 0);
        }
        Render_Command(RENDER_DEPTH_WRITE, 1, 0);
    }
}
