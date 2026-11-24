// Overlay drawing (JP twin: app_0007.cpp).

#include "texture_mgr.h"
#include "ui_batch.h"

// Draws the full-screen overlay quad (texture slot g_overlayTexture) with additive blending. Called by the scene
// frame that fades the overlay in.
extern "C" int Overlay_DrawFullScreen(void)
{
    Render_Command(RENDER_DEPTH_WRITE, 0, 0);
    Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
    g_batch2D.DrawWith(&g_textures[g_overlayTexture]);
    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    return 1;
}
