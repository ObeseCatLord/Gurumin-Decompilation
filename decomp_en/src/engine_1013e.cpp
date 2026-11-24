// Message pump, frame blend start and rectangle present (EN build). Ported from the matched JP source
// decomp/src/engine_1013e.cpp.
#include <windows.h>
#include <mmsystem.h>

#include "engine.h"
#include "texture_mgr.h"
#include "render.h"
#include "window.h"

// TextureManager::PumpMessages: handles at most MESSAGES_PER_PUMP pending window messages per call so a flood of input cannot stall
// the frame; F10 (which would open the system menu) is dropped. Returns 0 (and sets the quit flag) when WM_QUIT arrives, else 1. Called by the main loop.
int TextureManager::PumpMessages()
{
    MSG msg;
    int count = 0;
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (count >= MESSAGES_PER_PUMP)
            break;
        count++;
        if (msg.message == WM_QUIT)
        {
            g_quitRequested = 1;
            return 0;
        }
        if (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_F10)
            continue;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 1;
}

// FrameBlend_Start: starts the frame blend (after-image effect): `active` turns it on, and the strength moves
// towards `target` over `frames` updates (at least 1). The limit is the higher of the current strength and the
// target. Without blending support (setting off or a single frame buffer) everything stays at 0. Also restarts the
// frame timer. Returns 0.
extern "C" int FrameBlend_Start(int active, int target, int frames)
{
    g_frameBlendActive = active;
    float goal = (float)target;
    g_frameBlendLimit = g_frameBlendStrength < goal ? goal : g_frameBlendStrength;
    if (frames < 1)
        frames = 1;
    g_frameBlendStep = (goal - g_frameBlendStrength) / (float)frames;
    g_nextFrameDeadline = timeGetTime() + FRAME_MS;
    if (g_frameBlendSupported == 0 || g_singleFrameBuffer == 1)
    {
        g_frameBlendStrength = 0;
        g_frameBlendLimit = 0;
        g_frameBlendStep = 0;
    }
    return 0;
}

// TextureManager::Present: ends the scene and presents the back-buffer rectangle `source` (in the game's 640x480
// coordinates), scaled to the same place in the window's client area. Returns 1. Called for partial presents.
int TextureManager::Present(const RECT *source)
{
    RECT dest;
    Render_CloseScene();
    GetClientRect(g_gameWindow.window, &dest);
    float scaleX = (float)dest.right / (float)GAME_SCREEN_WIDTH;
    float scaleY = (float)dest.bottom / (float)GAME_SCREEN_HEIGHT;
    dest = *source;
    dest.left = (long)((float)dest.left * scaleX);
    dest.bottom = (long)((float)dest.bottom * scaleY);
    dest.right = (long)((float)dest.right * scaleX);
    dest.top = (long)((float)dest.top * scaleY);
    g_d3dDevice->Present(source, &dest, 0, 0);
    return 1;
}

// Milliseconds to sleep while the game waits for the device.
const int RECOVER_SLEEP_MS = 1;

// TextureManager::PumpAndRecover: handles the pending window messages and waits (still handling messages) while the
// game is inactive or the device is lost. When the device can be reset again, all default-pool resources are
// released, the device is reset (a message box when that fails) and everything is recreated. Then a new scene is
// begun. Returns 1, or 0 once WM_QUIT has been received. Called by the main loop.
int TextureManager::PumpAndRecover()
{
    MSG message;
    if (g_quitRequested != 0)
        return 0;
    long status;
    for (;;)
    {
        while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                g_quitRequested = 1;
                return 0;
            }
            if (message.message == WM_SYSKEYDOWN && message.wParam == VK_F10)
                continue;
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        if (g_windowActive == 0 && g_gameWindow.active == 0)
        {
            Sleep(RECOVER_SLEEP_MS);
            continue;
        }
        status = g_d3dDevice->TestCooperativeLevel();
        if (status == D3DERR_DEVICELOST)
        {
            Sleep(RECOVER_SLEEP_MS);
            continue;
        }
        break;
    }
    if (status == D3DERR_DEVICENOTRESET)
    {
        OnLostDevice();
        Textures_ReleaseGenerated();
        if (g_d3dDevice->Reset(&g_presentParameters) != 0)
            MessageBoxA(0, "Restore failed", "err", 0);
        OnResetDevice();
        Textures_RestoreGenerated();
    }
    g_stage0Texture = 0;
    Render_RestartScene();
    return 1;
}
