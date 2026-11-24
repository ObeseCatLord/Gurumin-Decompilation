// Direct3D device-loss release and render target reset (EN build). Ported from the matched JP source
// decomp/src/render_reset.cpp.
#include <string.h>

#include "render.h"
#include "buffer_pool.h"
#include "texture_mgr.h"

// Releases a COM object when the pointer is set. Used for every resource below.
static inline void ReleaseIfSet(IUnknown *object)
{
    if (object != 0)
        object->Release();
}

// TextureManager::OnLostDevice: releases every default-pool resource before IDirect3DDevice9::Reset: the draw list buffers,
// the render targets, the 2D vertex buffer, the dynamic buffer pool, the texture cache and the single objects
// below. Some pointers are cleared afterwards and some are left dangling until they are recreated, as in the
// original. Returns 1. Called when the device is lost.
int TextureManager::OnLostDevice()
{
    for (int list = 0; list < DRAW_LIST_COUNT; list++)
    {
        for (int i = 0; i < DRAW_LIST_BUFFER_COUNT; i++)
            ReleaseIfSet(g_drawListBuffers[list][i]);
    }
    for (int target = 0; target < RENDER_TARGET_SURFACE_COUNT; target++)
    {
        ReleaseIfSet(g_renderTargetSurfaces[target]);
        g_renderTargetSurfaces[target] = 0;
    }
    g_dynamicVertexBuffer2D->Release();
    BufferPool_ReleaseDynamic();
    TexCache_ReleaseAll();
    ReleaseIfSet(g_backBufferSurface);
    g_backBufferSurface = 0;
    ReleaseIfSet(g_depthSurface);
    ReleaseIfSet(g_swapTargets[0]);
    ReleaseIfSet(g_swapTargets[1]);
    ReleaseIfSet(g_swapTextures[0]);
    ReleaseIfSet(g_swapTextures[1]);
    g_depthSurface = 0;
    ReleaseIfSet(g_swapDepthSurface);
    g_swapDepthSurface = 0;
    return 1;
}

// Sets the viewport width and left edge for the current split-screen mode (whole screen, left half or right half).
static inline void SetViewportSplit(int width)
{
    if (g_splitScreen == 0)
    {
        g_viewport.X = 0;
        g_viewport.Width = width;
    }
    else if (g_splitScreen == SPLIT_SCREEN_LEFT)
    {
        g_viewport.X = 0;
        g_viewport.Width = width / 2;
    }
    else if (g_splitScreen == SPLIT_SCREEN_RIGHT)
    {
        g_viewport.X = width / 2;
        g_viewport.Width = width / 2;
    }
}

// Render_ResetTarget: unbinds the textures of stages 0..3, makes the back buffer and depth buffer the render
// targets again and sets the viewport to the whole back buffer or one half of it (split screen). Unless an
// off-screen target is set up, this is also saved as the state the state reset returns to. Called after Present
// when rendering to the back buffer.
extern "C" void Render_ResetTarget(void)
{
    g_stageBindingsReset = 0;
    g_d3dDevice->SetTexture(0, 0);
    g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    g_d3dDevice->SetTexture(1, 0);
    g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    g_d3dDevice->SetRenderTarget(0, g_backBufferSurface);
    g_d3dDevice->SetDepthStencilSurface(g_depthSurface);
    int width = g_deviceBackBufferWidth;
    int height = g_deviceBackBufferHeight;
    memset(&g_viewport, 0, sizeof(g_viewport));
    g_viewport.X = 0;
    g_viewport.Y = 0;
    g_viewport.Width = width;
    g_viewport.Height = height;
    SetViewportSplit(width);
    g_viewport.MinZ = 0;
    g_viewport.MaxZ = 1.0f;
    g_backBufferWidth = width;
    g_backBufferHeight = height;
    if (g_offscreenTargetActive == 0)
    {
        g_savedDepthSurface = g_depthSurface;
        g_savedRenderTarget = g_backBufferSurface;
        g_savedViewport = g_viewport;
        g_savedWidth = width;
        g_savedHeight = height;
    }
    g_d3dDevice->SetViewport(&g_viewport);
}

// Render_ResetSwapTarget: like Render_ResetTarget, but the render target is the swap target in use and the depth
// buffer the swap depth buffer, and the viewport is at most g_maxViewportSize wide and high. Called after Present
// when rendering to a swap target.
extern "C" void Render_ResetSwapTarget(void)
{
    g_stageBindingsReset = 0;
    g_d3dDevice->SetTexture(0, 0);
    g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    g_d3dDevice->SetTexture(1, 0);
    g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    Stage0_Unbind();
    g_d3dDevice->SetRenderTarget(0, g_swapTargets[g_swapTargetInUse]);
    g_d3dDevice->SetDepthStencilSurface(g_swapDepthSurface);
    memset(&g_viewport, 0, sizeof(g_viewport));
    g_viewport.Y = 0;
    float limit = g_maxViewportSize;
    float widthLimit = (float)g_deviceBackBufferWidth;
    int width = (int)(limit > widthLimit ? widthLimit : limit);
    float heightLimit = (float)g_deviceBackBufferHeight;
    int height = (int)(limit > heightLimit ? heightLimit : limit);
    SetViewportSplit(width);
    g_viewport.Height = height;
    g_viewport.MinZ = 0;
    g_viewport.MaxZ = 1.0f;
    if (g_offscreenTargetActive == 0)
    {
        g_savedWidth = width;
        g_savedHeight = height;
        g_savedRenderTarget = g_swapTargets[g_swapTargetInUse];
        g_savedDepthSurface = g_swapDepthSurface;
        g_savedViewport = g_viewport;
    }
    g_backBufferWidth = width;
    g_backBufferHeight = height;
    g_d3dDevice->SetViewport(&g_viewport);
}
