// Direct3D 9 device creation (a translation unit compiled without optimisation, see names_od.cpp).
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <string.h>

#include "d3d9_min.h"
#include "name_paths.h"
#include "render_device.h"
#include "render_vars.h"
#include "render_window.h"
#include "texture_manager.h"
#include "ui_vars.h"
#include "win_vars.h"

extern "C" void FUN_101b4948(unsigned long seed);
extern "C" int FUN_101b2670(void);                                 // Packs_Load
extern "C" void FUN_10147700(void);                                // BufferPool_Clear
extern "C" void FUN_10148310(void);                                // TexCache_Clear
extern "C" int FUN_10169720(void);
extern "C" void FUN_1013fa10(int index, int frames, float z, float y, float x, float w);
extern "C" IDirect3D9Like *__stdcall FUN_101c37c0(unsigned int sdkVersion);   // Direct3DCreate9
extern "C" int FUN_1013e6d0(void);
extern "C" void FUN_101b2260(void);

#define ADAPTER_DEFAULT 0
#define BEHAVIOR_NONE 0
#define SWP_SIZE_ONLY (SWP_NOMOVE | SWP_NOOWNERZORDER)
#define BLEND_WHITE_INDEX 0
#define BLEND_BLACK_INDEX 1
#define D3DFMT_D16_ 80
#define TRIDENT_ADAPTER_NAME "Trident Video Accelerator Cyber-XP4"

// 0x10148b00 (Render_CreateDevice). Starts the renderer: loads the packs, clears the caches and tables, creates the
// IDirect3D9 object, reads the display mode and adapter name, resizes the window so its client area is the back-buffer
// size, fills the present parameters (DAT_10f0ff84) and creates the device (hardware vertex processing on the HAL
// device, else software on HAL, else the reference device), then calls OnResetDevice. Returns 1 on success, 0 on
// failure, or E_FAIL when Direct3D is missing.
int TextureManager::CreateDevice()
{
    D3DCAPS9_ createdCaps;
    unsigned long behaviorFlags;
    D3DDISPLAYMODE_ displayMode;
    RECT clientRect;
    HWND window;
    D3DADAPTER_IDENTIFIER9_ identifier;
    D3DCAPS9_ hardwareCaps;
    int result;

    FUN_101b4948(GetTickCount());
    if (FUN_101b2670() == 0)
        return 0;
    FUN_10147700();
    FUN_10148310();
    FUN_10169720();
    FUN_1013fa10(BLEND_WHITE_INDEX, 0, 1.0f, 1.0f, 1.0f, 1.0f);
    FUN_1013fa10(BLEND_BLACK_INDEX, 0, 0, 0, 0, 0);
    DAT_10f6552c = 0;
    DAT_10f0ec60[0][0] = 0;
    DAT_10f0ec60[1][0] = 0;
    DAT_10f0ec60[2][0] = 0;
    DAT_10f0ec60[3][0] = 0;
    DAT_10f0d548[0][0] = 0;
    DAT_10f0d548[1][0] = 0;
    DAT_10f0d548[2][0] = 0;
    DAT_10f0d548[3][0] = 0;
    DAT_10f1fd28[0] = 0;
    DAT_10f1fd2c = 0;
    DAT_10f1fcfc[0] = 0;
    DAT_10f1fd00 = 0;
    DAT_10f65538 = 0;
    memset(DAT_10f0ffc0, 0, sizeof(DAT_10f0ffc0));
    DAT_10f654d8 = 0;
    DAT_10f65504 = 0;
    DAT_10f65698.Create(DAT_10f140c8);
    DAT_10f141d8 = FUN_101c37c0(D3D_SDK_VERSION_);
    if (DAT_10f141d8 == 0)
        return 0x80004005;
    FUN_1013e6d0();
    FUN_101b2260();
    if (DAT_10f141d8->GetAdapterDisplayMode(ADAPTER_DEFAULT, &displayMode) != 0)
    {
        DAT_10f141d8->Release();
        return 0;
    }
    if (displayMode.Format == D3DFMT_R8G8B8_)
        DAT_10f65540 = 1;
    if (displayMode.Format == D3DFMT_X1R5G5B5_)
        DAT_10f65540 = 1;
    if (DAT_10f654a4 != 0)
    {
        if (DAT_10f65514 != 0 || displayMode.Format == D3DFMT_X8R8G8B8_ || displayMode.Format == D3DFMT_R8G8B8_)
        {
            // the desktop mode already is a 32-bit one (or the game is fullscreen): keep the colour depth
        }
        else
            DAT_10f654a4 = 0;
    }
    else if (DAT_10f65514 != 0 || displayMode.Format == D3DFMT_R5G6B5_ || displayMode.Format == D3DFMT_X1R5G5B5_)
    {
        // the desktop mode already is a 16-bit one (or the game is fullscreen): keep the colour depth
    }
    else
        DAT_10f654a4 = 1;
    if (DAT_10f141d8->GetAdapterIdentifier(ADAPTER_DEFAULT, 0, &identifier) != 0)
    {
        DAT_10f141d8->Release();
        return 0;
    }
    strcpy(DAT_10f140d0, identifier.Description);
    if (_strnicmp(DAT_10f140d0, TRIDENT_ADAPTER_NAME, strlen(TRIDENT_ADAPTER_NAME)) == 0)
        DAT_10f65550 = 0;
    window = DAT_10f6569c;
    GetClientRect(window, &clientRect);
    SetWindowPos(window, 0, 0, 0, DAT_10202b24 + (DAT_10202b24 - clientRect.right),
                 DAT_10202b28 + (DAT_10202b28 - clientRect.bottom), SWP_SIZE_ONLY);
    memset(&DAT_10f0ff84, 0, sizeof(DAT_10f0ff84));
    DAT_10f0ff84.Windowed = DAT_10f65514 == 0;
    DAT_10f0ff84.BackBufferWidth = DAT_10202b24;
    DAT_10f0ff84.BackBufferHeight = DAT_10202b28;
    DAT_10202b2c = DAT_10202b24;
    DAT_10202b30 = DAT_10202b28;
    if (DAT_10f654c8 != 0)
        DAT_10f0ff84.SwapEffect = D3DSWAPEFFECT_COPY_;
    else
        DAT_10f0ff84.SwapEffect = D3DSWAPEFFECT_COPY_;
    if (DAT_10f65514 != 0)
        DAT_10f0ff84.SwapEffect = D3DSWAPEFFECT_COPY_;
    DAT_10f0ff84.BackBufferFormat = displayMode.Format;
    if (DAT_10f65514 != 0)
    {
        if (DAT_10f654a4 != 0)
            DAT_10f0ff84.BackBufferFormat = D3DFMT_X8R8G8B8_;
        else
            DAT_10f0ff84.BackBufferFormat = D3DFMT_R5G6B5_;
    }
    DAT_10f0ff84.BackBufferCount = 1;
    DAT_10f0ff84.EnableAutoDepthStencil = 1;
    DAT_10f0ff84.AutoDepthStencilFormat = D3DFMT_D16_;
    if (DAT_10f654a8 != 0)
        DAT_10f0ff84.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE_;
    else
        DAT_10f0ff84.PresentationInterval = D3DPRESENT_INTERVAL_ONE_;
    behaviorFlags = BEHAVIOR_NONE;
    if (DAT_10f65488 != 0)
        behaviorFlags |= D3DCREATE_MULTITHREADED_;
    DAT_10f655c0 = 1;
    DAT_10f141d0 = 2;
    DAT_10f65528 = 0x10;
    DAT_10f65524 = 0;
    if (DAT_10f65550 != 0 &&
        DAT_10f141d8->CreateDevice(ADAPTER_DEFAULT, D3DDEVTYPE_HAL_, window,
                                   behaviorFlags | D3DCREATE_HARDWARE_VERTEXPROCESSING_, &DAT_10f0ff84,
                                   &DAT_10f1fcf0) == 0)
    {
        DAT_10f1fcf0->GetDeviceCaps(&hardwareCaps);
        if (hardwareCaps.MaxVertexShaderConst >= MIN_HARDWARE_SHADER_CONSTANTS)
        {
            DAT_10f141d0 = 0;
            DAT_10f65528 = 0;
            DAT_10f6557c = 1;
            DAT_10f655c0 = 0;
        }
        else
            DAT_10f1fcf0->Release();
    }
    if (DAT_10f141d0 == 2)
    {
        result = DAT_10f141d8->CreateDevice(ADAPTER_DEFAULT, D3DDEVTYPE_HAL_, window,
                                            behaviorFlags | D3DCREATE_SOFTWARE_VERTEXPROCESSING_, &DAT_10f0ff84,
                                            &DAT_10f1fcf0);
        if (result != 0 &&
            DAT_10f141d8->CreateDevice(ADAPTER_DEFAULT, D3DDEVTYPE_REF_, window,
                                       behaviorFlags | D3DCREATE_SOFTWARE_VERTEXPROCESSING_, &DAT_10f0ff84,
                                       &DAT_10f1fcf0) != 0)
            return 0;
    }
    timeBeginPeriod(1);
    DAT_10f1fcf0->GetDeviceCaps(&createdCaps);
    if (createdCaps.PixelShaderVersion >= MIN_PIXEL_SHADER_VERSION)
        DAT_10f65524 = 1;
    else
        DAT_10f654e4 = 0;
    OnResetDevice();
    return 1;
}
