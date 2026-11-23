// Direct3D 9 device creation (TextureManager::CreateDevice, src_od/render_device.cpp): the IDirect3D9 object, the
// structures it fills, and the globals of the device settings. One declaration each: include this header.
#ifndef GURUMIN_RENDER_DEVICE_H
#define GURUMIN_RENDER_DEVICE_H

#include "com_min.h"

// D3DFORMAT values the device creation tests.
#define D3DFMT_R8G8B8_ 20
#define D3DFMT_A8R8G8B8_ 21
#define D3DFMT_X8R8G8B8_ 22
#define D3DFMT_R5G6B5_ 23
#define D3DFMT_X1R5G5B5_ 24

#define D3D_SDK_VERSION_ 0x20
#define D3DDEVTYPE_HAL_ 1
#define D3DDEVTYPE_REF_ 2
#define D3DCREATE_MULTITHREADED_ 0x04
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING_ 0x20
#define D3DCREATE_HARDWARE_VERTEXPROCESSING_ 0x40
#define D3DSWAPEFFECT_COPY_ 3
#define D3DPRESENT_INTERVAL_ONE_ 1
#define D3DPRESENT_INTERVAL_IMMEDIATE_ 0x80000000

// The vertex shader constant count that tells a card with real hardware vertex processing.
#define MIN_HARDWARE_SHADER_CONSTANTS 0x60

// Pixel shader version below this (as a number) = no usable pixel shaders.
#define MIN_PIXEL_SHADER_VERSION 1.0

struct D3DPRESENT_PARAMETERS_
{
    unsigned int BackBufferWidth;
    unsigned int BackBufferHeight;
    int BackBufferFormat;
    unsigned int BackBufferCount;
    int MultiSampleType;
    unsigned long MultiSampleQuality;
    int SwapEffect;
    void *hDeviceWindow;
    int Windowed;
    int EnableAutoDepthStencil;
    int AutoDepthStencilFormat;
    unsigned long Flags;
    unsigned int FullScreen_RefreshRateInHz;
    unsigned int PresentationInterval;
};

struct D3DDISPLAYMODE_
{
    unsigned int Width;
    unsigned int Height;
    unsigned int RefreshRate;
    int Format;
};

struct D3DADAPTER_IDENTIFIER9_
{
    char Driver[512];
    char Description[512];
    char rest[0x450 - 1024];
};

// D3DCAPS9 (0x130 bytes) up to the fields that are read.
struct D3DCAPS9_
{
    unsigned long pad_0000[0xc8 / 4];
    unsigned long MaxVertexShaderConst;   // 0xc8
    unsigned long PixelShaderVersion;     // 0xcc
    unsigned long pad_00d0[(0x130 - 0xd0) / 4];
};

// IDirect3D9 up to CreateDevice.
struct IDirect3D9Like : IUnknownLike
{
    virtual long __stdcall RegisterSoftwareDevice() = 0;
    virtual unsigned int __stdcall GetAdapterCount() = 0;
    virtual long __stdcall GetAdapterIdentifier(unsigned int adapter, unsigned long flags,
                                                D3DADAPTER_IDENTIFIER9_ *identifier) = 0;
    virtual unsigned int __stdcall GetAdapterModeCount() = 0;
    virtual long __stdcall EnumAdapterModes() = 0;
    virtual long __stdcall GetAdapterDisplayMode(unsigned int adapter, D3DDISPLAYMODE_ *mode) = 0;
    virtual long __stdcall CheckDeviceType() = 0;
    virtual long __stdcall CheckDeviceFormat() = 0;
    virtual long __stdcall CheckDeviceMultiSampleType() = 0;
    virtual long __stdcall CheckDepthStencilMatch() = 0;
    virtual long __stdcall CheckDeviceFormatConversion() = 0;
    virtual long __stdcall GetDeviceCaps() = 0;
    virtual void *__stdcall GetAdapterMonitor() = 0;
    virtual long __stdcall CreateDevice(unsigned int adapter, int deviceType, void *focusWindow,
                                        unsigned long behaviorFlags, D3DPRESENT_PARAMETERS_ *parameters,
                                        struct IDirect3DDevice9 **device) = 0;
};

extern "C" D3DPRESENT_PARAMETERS_ DAT_10f0ff84;           // the present parameters of the device (Reset uses them too)
extern "C" int DAT_10f0ffc0[0x1000];                      // table cleared at device creation
extern "C" int DAT_10202b2c;                              // copy of the back-buffer width set at device creation
extern "C" int DAT_10202b30;                              // copy of the back-buffer height
extern "C" IDirect3D9Like *DAT_10f141d8;                  // the IDirect3D9 object
extern "C" int DAT_10f141d0;                              // device state: 2 = not created yet, 0 = hardware vertex processing
extern "C" char DAT_10f140d0[];                           // description of the display adapter
extern "C" int DAT_10f654c8;                              // (both branches choose the copy swap effect)
extern "C" int DAT_10f654a8;                              // 1 = do not wait for the vertical blank
extern "C" int DAT_10f65488;                              // 1 = create a multithreaded device
extern "C" int DAT_10f65550;                              // 1 = try a hardware device first
extern "C" int DAT_10f6557c;                              // 1 = hardware vertex processing in use
extern "C" int DAT_10f65528;                              // 0x10 = software vertex processing, 0 = hardware
extern "C" int DAT_10f65524;                              // 1 = the card has usable pixel shaders

#endif
