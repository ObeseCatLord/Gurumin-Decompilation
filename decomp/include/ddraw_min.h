// DirectDraw types used by the movie player (dshow_stream.cpp) and the frame copy (surface_copy.cpp); this compiler's
// SDK has no ddraw.h. This is the one declaration of each: include this header, never redeclare them.
#ifndef GURUMIN_DDRAW_MIN_H
#define GURUMIN_DDRAW_MIN_H

#include "com_min.h"

// DDSURFACEDESC (DirectDraw 1-3, 0x6c bytes).
struct SurfaceDesc
{
    unsigned long size;           // 0x00: sizeof(SurfaceDesc)
    unsigned long flags;          // 0x04: which fields are valid
    unsigned long height;         // 0x08
    unsigned long width;          // 0x0c
    long pitch;                   // 0x10: bytes per row
    unsigned long backBufferCount;
    unsigned long refreshRate;
    unsigned long alphaBitDepth;
    unsigned long reserved;
    void *bits;                   // 0x24: first pixel
    unsigned long colourKeys[8];  // 0x28: four DDCOLORKEY pairs
    unsigned long pixelFormat[8]; // 0x48: DDPIXELFORMAT
    unsigned long caps;           // 0x68: DDSCAPS
};

// IDirectDrawSurface in vtable order (parameters only for the methods used: Lock at slot 25, Unlock at slot 32).
struct IDirectDrawSurfaceLike : IUnknownLike
{
    virtual long __stdcall AddAttachedSurface() = 0;
    virtual long __stdcall AddOverlayDirtyRect() = 0;
    virtual long __stdcall Blt() = 0;
    virtual long __stdcall BltBatch() = 0;
    virtual long __stdcall BltFast() = 0;
    virtual long __stdcall DeleteAttachedSurface() = 0;
    virtual long __stdcall EnumAttachedSurfaces() = 0;
    virtual long __stdcall EnumOverlayZOrders() = 0;
    virtual long __stdcall Flip() = 0;
    virtual long __stdcall GetAttachedSurface() = 0;
    virtual long __stdcall GetBltStatus() = 0;
    virtual long __stdcall GetCaps() = 0;
    virtual long __stdcall GetClipper() = 0;
    virtual long __stdcall GetColorKey() = 0;
    virtual long __stdcall GetDC() = 0;
    virtual long __stdcall GetFlipStatus() = 0;
    virtual long __stdcall GetOverlayPosition() = 0;
    virtual long __stdcall GetPalette() = 0;
    virtual long __stdcall GetPixelFormat() = 0;
    virtual long __stdcall GetSurfaceDesc() = 0;
    virtual long __stdcall Initialize() = 0;
    virtual long __stdcall IsLost() = 0;
    virtual long __stdcall Lock(void *rect, SurfaceDesc *desc, unsigned long flags, void *event) = 0;  // slot 25
    virtual long __stdcall ReleaseDC() = 0;
    virtual long __stdcall Restore() = 0;
    virtual long __stdcall SetClipper() = 0;
    virtual long __stdcall SetColorKey() = 0;
    virtual long __stdcall SetOverlayPosition() = 0;
    virtual long __stdcall SetPalette() = 0;
    virtual long __stdcall Unlock(void *rect) = 0;                                                  // slot 32
};

#endif
