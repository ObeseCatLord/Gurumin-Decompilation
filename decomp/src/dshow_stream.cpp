// Movie playback through a DirectShow multimedia stream: the file's primary video stream is decoded into a
// DirectDraw surface (DShowStream::Open; the class name comes from the game's own debug messages).

#include <windows.h>
#include <string.h>

// The DirectDraw surface description (DDSURFACEDESC, 0x6c bytes; the SDK of this compiler has no ddraw.h).
struct SurfaceDesc
{
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    char rest[0x5c];
};

// COM interfaces in vtable order (amstream.h / ddstream.h), with parameters for the methods used here.
struct IUnknownLike
{
    virtual long __stdcall QueryInterface(const GUID &iid, void **object) = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
};

struct IDirectDrawLike : IUnknownLike
{
    virtual long __stdcall Compact() = 0;
    virtual long __stdcall CreateClipper() = 0;
    virtual long __stdcall CreatePalette() = 0;
    virtual long __stdcall CreateSurface(SurfaceDesc *desc, IUnknownLike **surface, IUnknownLike *outer) = 0;
};

struct IMediaStreamLike : IUnknownLike
{
};

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
    virtual long __stdcall CreateSample(IUnknownLike *surface, const RECT *rect, unsigned long flags,
                                        IUnknownLike **sample) = 0;
    virtual long __stdcall GetTimePerFrame(LONGLONG *time) = 0;
};

struct IAMMultiMediaStreamLike : IUnknownLike
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
    virtual long __stdcall AddMediaStream(IUnknownLike *stream, const GUID *purpose, unsigned long flags,
                                          IMediaStreamLike **added) = 0;
    virtual long __stdcall OpenFile(const WCHAR *name, unsigned long flags) = 0;
};

extern "C" const GUID DAT_101fccdc;              // CLSID_AMMultiMediaStream
extern "C" const GUID DAT_101fccec;              // IID_IAMMultiMediaStream
extern "C" const GUID DAT_101fccfc;              // MSPID_PrimaryVideo
extern "C" const GUID DAT_101fcccc;              // IID_IDirectDrawMediaStream

struct DShowStream
{
    IAMMultiMediaStreamLike *stream;         // 0x00
    IMediaStreamLike *video;                 // 0x04: the primary video stream
    IDirectDrawMediaStreamLike *ddStream;    // 0x08
    IUnknownLike *sample;                    // 0x0c: the stream sample drawing into `surface`
    IUnknownLike *surface;                   // 0x10
    int pad_0014;
    LONGLONG frameTime;                      // 0x18: time per frame
    int width;                               // 0x20
    int height;                              // 0x24
    void Close();
    long Open(const char *path);
};

// 0x101488b0 (DShowStream::Close). Releases the five interfaces, last to first.
void DShowStream::Close()
{
    if (surface != 0)
        surface->Release();
    surface = 0;
    if (sample != 0)
        sample->Release();
    sample = 0;
    if (ddStream != 0)
        ddStream->Release();
    ddStream = 0;
    if (video != 0)
        video->Release();
    video = 0;
    if (stream != 0)
        stream->Release();
    stream = 0;
}

// 0x1015c450 (DShowStream::Open). Opens movie file `path`: creates the multimedia stream, adds its primary video
// stream, opens the file, gets the DirectDraw media stream and its time per frame, and creates a surface in the
// stream's format with a stream sample drawing into it. Each failing step prints "failed: DShowStream::Open(n)"
// to the debugger and returns its error. Movies larger than 1024 x 512 are closed again (returns 1).
long DShowStream::Open(const char *path)
{
    WCHAR name[0x104];
    SurfaceDesc format;
    SurfaceDesc desired;
    unsigned long flags;
    IDirectDrawLike *directDraw;
    IAMMultiMediaStreamLike *created;
    Close();
    created = 0;
    long result = CoCreateInstance(DAT_101fccdc, 0, CLSCTX_INPROC_SERVER, DAT_101fccec, (void **)&created);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(1)\r\n");
        return result;
    }
    stream = created;
    result = created->Initialize(0, 0, 0);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(2)\r\n");
        return result;
    }
    result = created->AddMediaStream(0, &DAT_101fccfc, 0, 0);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(3)\r\n");
        return result;
    }
    MultiByteToWideChar(CP_ACP, 0, path, -1, name, 0x208);
    result = stream->OpenFile(name, 0);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(5)\r\n");
        return result;
    }
    result = stream->GetMediaStream(DAT_101fccfc, &video);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(6)\r\n");
        return result;
    }
    result = video->QueryInterface(DAT_101fcccc, (void **)&ddStream);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(7)\r\n");
        return result;
    }
    ddStream->GetTimePerFrame(&frameTime);
    memset(&format, 0, sizeof(format));
    memset(&desired, 0, sizeof(desired));
    format.size = sizeof(format);
    desired.size = sizeof(desired);
    result = ddStream->GetFormat(&format, 0, &desired, &flags);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(8)\r\n");
        return result;
    }
    result = ddStream->GetDirectDraw(&directDraw);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(9)\r\n");
        return result;
    }
    result = directDraw->CreateSurface(&desired, &surface, 0);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(10)\r\n");
        return result;
    }
    directDraw->Release();
    result = ddStream->CreateSample(surface, 0, 0, &sample);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(11)\r\n");
        return result;
    }
    width = format.width;
    height = format.height;
    if (height > 0x200 || width > 0x400)
    {
        Close();
        return 1;
    }
    return result;
}
