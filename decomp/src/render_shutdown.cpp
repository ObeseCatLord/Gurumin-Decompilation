// The renderer's message pump with device-lost recovery, and its shutdown at the end of the game (TextureManager::M_10175e70
// calls Shutdown after unloading the textures).
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "d3d9_min.h"

#include "texture_manager.h"

#define D3DERR_DEVICELOST ((long)0x88760868)
#define D3DERR_DEVICENOTRESET ((long)0x88760869)

#include "render_vars.h"

extern "C" int DAT_10f6550c;                  // texture bound to stage 0 (0 = forget the binding)
extern "C" int FUN_101734d0(void);            // Texture_OnLostDevice: releases every registered texture
extern "C" int FUN_10173510(void);            // Texture_OnResetDevice: recreates them
extern "C" int FUN_1013ed20(void);            // Render_BeginScene
#include "win_vars.h"
extern "C" IUnknownLike *DAT_10f141dc;
extern "C" int FUN_101483a0(void);            // BufferPool_ReleaseAll (texcache.cpp)
extern "C" int FUN_10148280(void);            // releases the four buffer pools
extern "C" void FUN_101b22b0(void);

// 0x1015c730. Destroys the game window, releases every Direct3D resource, the device and the Direct3D object, and
// writes their remaining reference counts to the debugger ("Release Perfect!!" when none are left). The count stays
// uninitialised when the first objects are already gone, as in the original. Returns 1.
int TextureManager::Shutdown()
{
    char message[256];
    int references;
    DestroyWindow(DAT_10f6569c);
    UnregisterClassA(DAT_10f65754, DAT_10f140c8);
    OnLostDevice();
    FUN_101483a0();
    FUN_10148280();
    FUN_101b22b0();
    if (DAT_10f141dc != 0)
        references = DAT_10f141dc->Release();
    if (DAT_10f1fcf0 != 0)
        references = DAT_10f1fcf0->Release();
    sprintf(message, "Release D3D Device Ref=%d\n", references);
    OutputDebugStringA(message);
    if (references == 0)
        OutputDebugStringA("Release Perfect!!\n");
    if (DAT_10f141d8 != 0)
        references = DAT_10f141d8->Release();
    sprintf(message, "Release D3D Ref=%d\n", references);
    OutputDebugStringA(message);
    if (references == 0)
        OutputDebugStringA("Release Perfect!!   Grate!!\n");
    timeEndPeriod(1);
    DAT_10f141d8 = 0;
    DAT_10f1fcf0 = 0;
    DAT_10f141dc = 0;
    return 1;
}

// Frame_PumpAndRecover (0x1015c850). Handles the pending window messages and waits (still handling messages) while
// the game is inactive or the device is lost. When the device can be reset again, all default-pool resources are
// released, the device is reset (a "restore failed" message box when that fails) and everything is recreated.
// Then a new scene is begun. Returns 1, or 0 once WM_QUIT has been received.
int TextureManager::PumpAndRecover()
{
    MSG message;
    if (DAT_10f6552c != 0)
        return 0;
    long status;
    do
    {
        while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                DAT_10f6552c = 1;
                return 0;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    } while ((DAT_10f65530 == 0 && DAT_10f656a0 == 0) ||
             (status = DAT_10f1fcf0->TestCooperativeLevel()) == D3DERR_DEVICELOST);
    if (status == D3DERR_DEVICENOTRESET)
    {
        OnLostDevice();
        FUN_101734d0();
        if (DAT_10f1fcf0->Reset(DAT_10f0ff84) != 0)
            MessageBoxA(0, "\x83\x8a\x83X\x83g\x83" "A" "\x8e\xb8\x94s", "err", 0);  // "restore failed"
        OnResetDevice();
        FUN_10173510();
    }
    DAT_10f6550c = 0;
    FUN_1013ed20();
    return 1;
}
