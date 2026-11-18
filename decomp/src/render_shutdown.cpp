// Renderer shutdown at the end of the game (TextureManager::M_10175e70 calls it after unloading the textures).

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "d3d9_min.h"

struct TextureManager
{
    int OnLostDevice();  // 0x1014b2a0 (render_reset.cpp Render_OnLostDevice; it does not use `this`)
    int Shutdown();
};

extern "C" HWND DAT_10f6569c;                 // the game window
extern "C" char DAT_10f65754[];               // its window class name
extern "C" HINSTANCE DAT_10f140c8;            // the module instance
extern "C" IDirect3DDevice9 *DAT_10f1fcf0;    // the Direct3D device
extern "C" IUnknownLike *DAT_10f141dc;
extern "C" IUnknownLike *DAT_10f141d8;        // the IDirect3D9 object
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
