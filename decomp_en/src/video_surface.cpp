// Copying the video frame surface into a texture (EN build). Ported from the matched JP source
// decomp/src/surface_copy.cpp.
#include "video_surface.h"

// DDLOCK_WAIT | DDLOCK_NOSYSLOCK: wait for the surface and do not take the system lock.
const DWORD VIDEO_LOCK_FLAGS = 0x801;
// Bytes per pixel of the two surface formats that are copied.
const int BYTES_PER_PIXEL_16 = 2;
const int BYTES_PER_PIXEL_32 = 4;

// Locks the surface and copies its pixels into `texture`; the pixel size (pitch / width) picks the 16-bit or 32-bit
// writer. Other sizes are not copied. Called by the movie player after each decoded frame.
void VideoSurface::CopyTo(TextureObject *texture)
{
    if (surface != 0)
    {
        DDSURFACEDESC desc;
        desc.dwSize = sizeof(desc);
        surface->Lock(0, &desc, VIDEO_LOCK_FLAGS, 0);
        int pitch = desc.lPitch;
        if (pitch / width == BYTES_PER_PIXEL_16)
            texture->Write16(0, 0, width, height, pitch, desc.lpSurface);
        else if (pitch / width == BYTES_PER_PIXEL_32)
            texture->WriteRgb32(0, 0, width, height, pitch, (const unsigned char *)desc.lpSurface);
        surface->Unlock(0);
    }
}
