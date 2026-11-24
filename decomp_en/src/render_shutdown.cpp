// Renderer shutdown (EN build). Ported from the matched JP source decomp/src/render_shutdown.cpp.
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "buffer_pool.h"
#include "render.h"
#include "texture_mgr.h"
#include "window.h"

// Timer resolution in milliseconds that was requested at start-up and is given back here.
const int TIMER_RESOLUTION_MS = 1;

// TextureManager::Shutdown: destroys the game window, releases every Direct3D resource, the device and the Direct3D
// object, and writes their remaining reference counts to the debugger ("Release Perfect!!" when none are left). The
// count stays uninitialised when the first objects are already gone, as in the original. Returns 1. Called when the
// game exits.
int TextureManager::Shutdown()
{
    char message[256];
    int references;
    DestroyWindow(g_gameWindow.window);
    UnregisterClassA(g_gameWindow.className, g_instance);
    OnLostDevice();
    SurfacePool_ReleaseAll();
    BufferPool_ReleaseAll();
    FileBuffers_Free();
    if (g_backBufferSurface != 0)
        references = g_backBufferSurface->Release();
    if (g_d3dDevice != 0)
        references = g_d3dDevice->Release();
    sprintf(message, "Release D3D Device Ref=%d\n", references);
    OutputDebugStringA(message);
    if (references == 0)
        OutputDebugStringA("Release Perfect!!\n");
    if (g_direct3D != 0)
        references = g_direct3D->Release();
    sprintf(message, "Release D3D Ref=%d\n", references);
    OutputDebugStringA(message);
    if (references == 0)
        OutputDebugStringA("Release Perfect!!   Grate!!\n");
    timeEndPeriod(TIMER_RESOLUTION_MS);
    g_direct3D = 0;
    g_d3dDevice = 0;
    g_backBufferSurface = 0;
    return 1;
}
