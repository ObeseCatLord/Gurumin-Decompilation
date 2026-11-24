// The off-screen DirectDraw surface that holds a decoded video frame (EN build; JP: decomp/include/surface_copy.h).
// This is the one declaration of VideoSurface in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_VIDEO_SURFACE_H
#define GURUMIN_EN_VIDEO_SURFACE_H

#include <windows.h>
#include <ddraw.h>

#include "texture_mgr.h"

// Owner of the off-screen DirectDraw surface of the video frame; only the members EN code has used are named.
struct VideoSurface
{
    char m_0000[0x10];          // 0x00: not yet named
    IDirectDrawSurface *surface;   // 0x10: the surface, 0 = none
    char m_0014[0xc];           // 0x14: not yet named
    int width;                  // 0x20: frame width in pixels
    int height;                 // 0x24: frame height in pixels

    // Copies the surface's pixels into `texture` (16 or 32 bits per pixel). EN 0x7a3f90 (JP 0x1015c6a0).
    void CopyTo(TextureObject *texture);
};

#endif
