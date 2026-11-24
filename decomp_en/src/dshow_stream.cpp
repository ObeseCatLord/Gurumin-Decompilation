// EN DirectShow video stream (JP twin: dshow_stream.cpp).

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
