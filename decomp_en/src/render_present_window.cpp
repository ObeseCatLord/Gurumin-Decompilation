// Presenting the composed frame to a window and unbinding every texture stage (EN build, EN-only: no JP twin found).
#include "render.h"

// Render_PresentComposedToWindow: finishes the 2D overlay (Render_ComposeFrame), ends the open Direct3D scene, then
// presents the device back buffer (0, 0, device width, device height) into the client area of `window`. Returns 1.
// No direct caller.
extern "C" int __stdcall Render_PresentComposedToWindow(HWND window)
{
    RECT client;
    RECT source;
    Render_ComposeFrame();
    Render_CloseScene();
    GetClientRect(window, &client);
    source.bottom = g_deviceBackBufferHeight;
    source.right = g_deviceBackBufferWidth;
    source.top = 0;
    source.left = 0;
    g_d3dDevice->Present(&source, &client, window, 0);
    return 1;
}

// Render_UnbindAllTextures: unbinds texture stages 0 and 1 (only when something is bound there, and marks them
// unbound) and clears stages 2 and 3. Returns 0. Called by the 2D scene drawing.
extern "C" int Render_UnbindAllTextures(void)
{
    if (g_stage0Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(0, 0);
        g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    }
    if (g_stage1Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(1, 0);
        g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    }
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    return 0;
}

// Render_PresentCentered: ends the open Direct3D scene, then presents the device back buffer into `window` with the
// destination rectangle centred in the client area. A client area wider or taller than the back buffer is cropped to
// the back buffer size from the origin instead. Returns 1. No direct caller.
extern "C" int __stdcall Render_PresentCentered(HWND window)
{
    RECT client;
    RECT destination;
    Render_CloseScene();
    GetClientRect(window, &client);
    destination.top = (g_deviceBackBufferHeight - client.bottom + client.top) / 2;
    destination.bottom = (destination.top - client.top) + client.bottom;
    destination.left = (g_deviceBackBufferWidth - client.right + client.left) / 2;
    destination.right = (destination.left - client.left) + client.right;
    if (g_deviceBackBufferWidth < client.right)
    {
        destination.right = g_deviceBackBufferWidth;
        destination.left = 0;
    }
    if (g_deviceBackBufferHeight < client.bottom)
    {
        destination.bottom = g_deviceBackBufferHeight;
        destination.top = 0;
    }
    g_d3dDevice->Present(&destination, &client, window, 0);
    return 1;
}
