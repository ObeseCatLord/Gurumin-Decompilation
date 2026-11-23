// Texture, VideoSurface: declared here once (first used by src/surface_copy.cpp).
#ifndef GURUMIN_SURFACE_COPY_H
#define GURUMIN_SURFACE_COPY_H

#include "ddraw_min.h"

struct Texture
{
    int Write16(int x, int y, int width, int height, int pitch, void *bits);  // 0x1014f060: copies 16-bit pixels
    int M_1014ef20(int x, int y, int width, int height, int pitch, void *bits);  // copies 32-bit pixels
};
// Owner of an off-screen DirectDraw surface (the video frame).
struct VideoSurface
{
    char pad_0000[0x10];
    IDirectDrawSurfaceLike *surface;
    char pad_0014[0xc];
    int width;
    int height;
    void CopyTo(Texture *texture);
};

#endif
