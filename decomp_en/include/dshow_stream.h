// A DirectShow video stream (EN build). Declared once here (JP: decomp/include/dshow_stream.h).
#ifndef GURUMIN_EN_DSHOW_STREAM_H
#define GURUMIN_EN_DSHOW_STREAM_H

#include "ddraw_min.h"

// CLSID of the multimedia stream the player creates, its interface id, the id of the primary video stream and the id of
// the DirectDraw interface of that stream. EN 0x008bd7fc, 0x008c06bc, 0x008bd79c, 0x008c076c.
extern "C" const GUID g_clsidMultiMediaStream;
extern "C" const GUID g_iidMultiMediaStream;
extern "C" const GUID g_purposeVideoStream;
extern "C" const GUID g_iidDirectDrawMediaStream;

// Largest movie the player keeps open (larger ones are closed again).
const int DSHOW_MAX_WIDTH = 0x400;
const int DSHOW_MAX_HEIGHT = 0x200;
// Wide characters in the buffer the movie path is converted into.
const int DSHOW_PATH_CHARS = 0x104;

struct DShowStream
{
    IAMMultiMediaStreamLike *stream;         // 0x00: the multimedia stream
    IMediaStreamLike *video;                 // 0x04: the primary video stream
    IDirectDrawMediaStreamLike *ddStream;    // 0x08: its DirectDraw stream
    IUnknown *sample;                        // 0x0c: the stream sample drawing into `surface`
    IUnknown *surface;                       // 0x10: the surface the video is drawn on
    int pad_0014;                            // 0x14: not used
    LONGLONG frameTime;                      // 0x18: time per frame
    int width;                               // 0x20: movie width
    int height;                              // 0x24: movie height

    void Close();         // EN 0x77cfc0 (JP 0x101488b0): releases the five interfaces, last to first
    int EndStreams();     // EN 0x7a3dd0 (EN-only, dshow_stream.cpp): ends and releases the two media streams and the multimedia stream, clears the movie size; returns 0
    long Open(const char *path);   // EN 0x7a2510 (JP 0x1015c450): opens the movie file and its stream
};

#endif
