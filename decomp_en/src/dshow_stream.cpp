// EN DirectShow video stream (JP twin: dshow_stream.cpp).

#include <string.h>

#include "dshow_stream.h"

// Releases the five interfaces, last to first. Called when a movie ends.
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

// Opens movie file `path`: creates the multimedia stream, adds its primary video stream, opens the file, gets the
// DirectDraw media stream and its time per frame, and creates a surface in the stream's format with a stream sample
// drawing into it. Each failing step prints "failed: DShowStream::Open(n)" to the debugger and returns its error.
// Movies larger than 1024 x 512 are closed again (returns 1). Called when a scene movie starts.
long DShowStream::Open(const char *path)
{
    WCHAR name[DSHOW_PATH_CHARS];
    SurfaceDesc format;
    SurfaceDesc desired;
    unsigned long flags;
    IDirectDrawLike *directDraw;
    IAMMultiMediaStreamLike *created;
    Close();
    created = 0;
    long result = CoCreateInstance(g_clsidMultiMediaStream, 0, CLSCTX_INPROC_SERVER, g_iidMultiMediaStream,
                                   (void **)&created);
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
    result = created->AddMediaStream(0, &g_purposeVideoStream, 0, 0);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(3)\r\n");
        return result;
    }
    MultiByteToWideChar(CP_ACP, 0, path, -1, name, DSHOW_PATH_CHARS * 2);
    result = stream->OpenFile(name, 0);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(5)\r\n");
        return result;
    }
    result = stream->GetMediaStream(g_purposeVideoStream, &video);
    if (result < 0)
    {
        OutputDebugStringA("failed: DShowStream::Open(6)\r\n");
        return result;
    }
    result = video->QueryInterface(g_iidDirectDrawMediaStream, (void **)&ddStream);
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
    }
    else
    {
        width = format.width;
        height = format.height;
        if (height > DSHOW_MAX_HEIGHT || width > DSHOW_MAX_WIDTH)
        {
            Close();
            return 1;
        }
    }
    return result;
}

// Ends the video stream and its DirectDraw stream (SendEndOfStream), releases both and the multimedia stream, then
// forgets them and the movie size. The sample and the surface are left alone. Returns 0. Called when the game shuts
// down (EN 0x609f70).
int DShowStream::EndStreams()
{
    if (video != 0)
    {
        ((IDirectDrawMediaStreamLike *)video)->SendEndOfStream();
        video->Release();
    }
    if (ddStream != 0)
    {
        ddStream->SendEndOfStream();
        ddStream->Release();
    }
    if (stream != 0)
        stream->Release();
    stream = 0;
    video = 0;
    ddStream = 0;
    width = 0;
    height = 0;
    return 0;
}
