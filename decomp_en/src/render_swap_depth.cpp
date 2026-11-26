// Swap-target depth buffer and presenting to a window (EN build, EN-only: no JP twin found).
#include "render.h"

// Replaces the depth buffer used with the swap targets by a square D3DFMT_D16 surface `size` pixels on a side (the old one
// is released first). Returns 0. Called when the swap targets are (re)created.
extern "C" int Render_CreateSwapDepth(int size)
{
    g_swapDepthSize = size;
    if (g_swapDepthSurface != 0)
        g_swapDepthSurface->Release();
    g_swapDepthSurface = 0;
    g_d3dDevice->CreateDepthStencilSurface(g_swapDepthSize, g_swapDepthSize, SWAP_DEPTH_FORMAT, D3DMULTISAMPLE_NONE, 0, TRUE,
                                           &g_swapDepthSurface, 0);
    return 0;
}

// Ends the open Direct3D scene (when there is one) and presents the back buffer to `window`. Returns 1.
extern "C" int __stdcall Render_PresentToWindow(HWND window)
{
    RECT client;
    if (g_sceneOpen != 0)
        g_d3dDevice->EndScene();
    g_sceneOpen = 0;
    GetClientRect(window, &client);
    g_d3dDevice->Present(0, 0, window, 0);
    return 1;
}
