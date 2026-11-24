// A DirectShow video stream (EN build). Declared once here (JP: decomp/include/dshow_stream.h).
#ifndef GURUMIN_EN_DSHOW_STREAM_H
#define GURUMIN_EN_DSHOW_STREAM_H

#include <unknwn.h>

struct DShowStream
{
    IUnknown *stream;     // 0x00: the multimedia stream
    IUnknown *video;      // 0x04: the primary video stream
    IUnknown *ddStream;   // 0x08: its DirectDraw stream
    IUnknown *sample;     // 0x0c: the stream sample drawing into `surface`
    IUnknown *surface;    // 0x10: the surface the video is drawn on

    void Close();         // EN 0x77cfc0 (JP 0x101488b0): releases the five interfaces, last to first
};

#endif
