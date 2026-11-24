// Toggle holder, pooled surfaces and frame presentation (EN build). Ported from the matched JP source
// decomp/src/texcache.cpp.
#include "render.h"
#include "texcache.h"
#include "buffer_pool.h"

// Switches the held object off (state 0), when there is one.
void ToggleHolder::SwitchOff()
{
    if (object != 0)
        object->SetState(0);
}

// 1 when the held object reports state 0 (or there is none).
int ToggleHolder::IsOff()
{
    int state = 0;
    if (object != 0)
        object->GetState(&state);
    return state == 0;
}

// While the held object is on, asks the second object for its status and switches the holder off when that fails
// or reports that playback finished.
void ToggleHolder::Poll()
{
    if (poll != 0)
    {
        int state = 0;
        if (object != 0)
        {
            object->GetState(&state);
            if (state != 0)
            {
                long result = poll->Query(0, 0, 0, 0);
                if (result < 0 || result == QUERY_FINISHED)
                    SwitchOff();
            }
        }
    }
}

// TexCache_ReleaseAll: releases every pooled surface but keeps the entries reserved (before a device reset).
extern "C" int TexCache_ReleaseAll(void)
{
    for (int i = 0; i < SURFACE_POOL_SIZE; i++)
    {
        if (g_pooledSurfaces[i] != 0)
        {
            g_pooledSurfaces[i]->Release();
            g_pooledBytesTotal -= g_pooledSurfaceBytes[i];
        }
        g_pooledSurfaceBytes[i] = 0;
        g_pooledSurfaces[i] = 0;
        g_pooledSurfaceReserved[i] = 0;
    }
    return 0;
}

// Render_Present: ends the frame: finishes the 2D overlay, ends the Direct3D scene, presents the back buffer and
// resets the render target for the next frame. Returns 0 once the game is quitting. Called by the main loop.
extern "C" int Render_Present(void)
{
    if (g_quitRequested != 0)
        return 0;
    Render_ComposeFrame();
    Render_CloseScene();
    g_d3dDevice->Present(0, 0, 0, 0);
    if (g_renderToBackBuffer != 0)
        Render_ResetTarget();
    else
        Render_ResetSwapTarget();
    return 1;
}

// Render_PresentSimple: same as Render_Present without the overlay pass (used for loading screens).
extern "C" int Render_PresentSimple(void)
{
    if (g_quitRequested != 0)
        return 0;
    Render_CloseScene();
    g_d3dDevice->Present(0, 0, 0, 0);
    Render_ResetSwapTarget();
    return 1;
}

// SurfacePool_ReleaseAll: releases every pooled surface, clears the pool tables and the byte total. Called when the
// renderer shuts down.
extern "C" int SurfacePool_ReleaseAll(void)
{
    g_pooledBytesTotal = 0;
    for (int i = 0; i < SURFACE_POOL_SIZE; i++)
    {
        if (g_pooledSurfaces[i] != 0)
            g_pooledSurfaces[i]->Release();
        g_pooledSurfaceUsed[i] = 0;
        g_pooledSurfaces[i] = 0;
        g_pooledSurfaceReserved[i] = 0;
    }
    return 0;
}
