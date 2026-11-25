// Restoring the saved render state (EN build).
#include <string.h>

#include "render.h"
#include "map_view.h"

// Unbinds the textures of stages 0 to 3, then (when render targets work) puts the saved render target, depth buffer,
// back buffer size and viewport back. Called when a frame's offscreen passes are finished. EN 0x7a4160 (JP 0x1014b390).
extern "C" void Render_RestoreTarget(void)
{
    g_stageBindingsReset = 0;
    g_d3dDevice->SetTexture(0, 0);
    g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    g_d3dDevice->SetTexture(1, 0);
    g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    if (g_stage0Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(0, 0);
        g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    }
    if (g_renderTargetsWork != 0)
    {
        g_d3dDevice->SetRenderTarget(0, g_savedRenderTarget);
        g_d3dDevice->SetDepthStencilSurface(g_savedDepthSurface);
        memset(&g_viewport, 0, sizeof(g_viewport));
        g_backBufferWidth = g_savedWidth;
        g_backBufferHeight = g_savedHeight;
        g_viewport = g_savedViewport;
        g_d3dDevice->SetViewport(&g_savedViewport);
    }
}
