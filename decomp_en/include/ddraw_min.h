// DirectDraw and DirectShow interface shapes used by the movie player (src/dshow_stream.cpp), declared in vtable order
// with parameters only for the methods the game calls. The movie code calls the slots through these types. This is the
// one declaration of each in decomp_en.
#ifndef GURUMIN_EN_DDRAW_MIN_H
#define GURUMIN_EN_DDRAW_MIN_H

#include <windows.h>
#include <unknwn.h>

// DDSURFACEDESC (DirectDraw 1-3, 0x6c bytes).
struct SurfaceDesc
{
    unsigned long size;           // 0x00: sizeof(SurfaceDesc)
    unsigned long flags;          // 0x04: which fields are valid
    unsigned long height;         // 0x08: surface height
    unsigned long width;          // 0x0c: surface width
    long pitch;                   // 0x10: bytes per row
    unsigned long backBufferCount;   // 0x14: back buffers
    unsigned long refreshRate;    // 0x18: refresh rate
    unsigned long alphaBitDepth;  // 0x1c: alpha bits
    unsigned long reserved;       // 0x20: reserved
    void *bits;                   // 0x24: first pixel
    unsigned long colourKeys[8];  // 0x28: four DDCOLORKEY pairs
    unsigned long pixelFormat[8]; // 0x48: DDPIXELFORMAT
    unsigned long caps;           // 0x68: DDSCAPS
};

// IDirectDraw up to CreateSurface.
struct IDirectDrawLike : IUnknown
{
    virtual long __stdcall Compact() = 0;
    virtual long __stdcall CreateClipper() = 0;
    virtual long __stdcall CreatePalette() = 0;
    virtual long __stdcall CreateSurface(SurfaceDesc *desc, IUnknown **surface, IUnknown *outer) = 0;
};

// IMediaStream (nothing called besides IUnknown).
struct IMediaStreamLike : IUnknown
{
};

// IDirectDrawMediaStream.
struct IDirectDrawMediaStreamLike : IMediaStreamLike
{
    virtual long __stdcall GetMultiMediaStream() = 0;
    virtual long __stdcall GetInformation() = 0;
    virtual long __stdcall SetSameFormat() = 0;
    virtual long __stdcall AllocateSample() = 0;
    virtual long __stdcall CreateSharedSample() = 0;
    virtual long __stdcall SendEndOfStream() = 0;
    virtual long __stdcall GetFormat(SurfaceDesc *current, void **palette, SurfaceDesc *desired,
                                     unsigned long *flags) = 0;
    virtual long __stdcall SetFormat() = 0;
    virtual long __stdcall GetDirectDraw(IDirectDrawLike **directDraw) = 0;
    virtual long __stdcall SetDirectDraw() = 0;
    virtual long __stdcall CreateSample(IUnknown *surface, const RECT *rect, unsigned long flags,
                                        IUnknown **sample) = 0;
    virtual long __stdcall GetTimePerFrame(LONGLONG *time) = 0;
};

// IAMMultiMediaStream.
struct IAMMultiMediaStreamLike : IUnknown
{
    virtual long __stdcall GetInformation() = 0;
    virtual long __stdcall GetMediaStream(const GUID &purpose, IMediaStreamLike **stream) = 0;
    virtual long __stdcall EnumMediaStreams() = 0;
    virtual long __stdcall GetState() = 0;
    virtual long __stdcall SetState() = 0;
    virtual long __stdcall GetTime() = 0;
    virtual long __stdcall GetDuration() = 0;
    virtual long __stdcall Seek() = 0;
    virtual long __stdcall GetEndOfStreamEventHandle() = 0;
    virtual long __stdcall Initialize(int type, unsigned long flags, void *graph) = 0;
    virtual long __stdcall GetFilterGraph() = 0;
    virtual long __stdcall GetFilter() = 0;
    virtual long __stdcall AddMediaStream(IUnknown *stream, const GUID *purpose, unsigned long flags,
                                          IMediaStreamLike **added) = 0;
    virtual long __stdcall OpenFile(const WCHAR *name, unsigned long flags) = 0;
};

#endif
