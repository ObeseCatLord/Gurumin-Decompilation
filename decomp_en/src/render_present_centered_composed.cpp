// Presenting the composed frame centred in a window (EN build, EN-only: no JP twin found).
#include "render.h"

// Render_PresentCenteredComposed: finishes the 2D overlay (Render_ComposeFrame), ends the open Direct3D scene, then
// presents the device back buffer into `window` with the destination rectangle centred in the client area (a client area
// wider or taller than the back buffer is cropped to the back buffer size from the origin instead). Returns 1. No direct
// caller.
extern "C" int __stdcall Render_PresentCenteredComposed(HWND window)
{
    RECT client;
    RECT destination;
    Render_ComposeFrame();
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
