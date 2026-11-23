// Copying a DirectDraw surface (DDSURFACEDESC, 0x6c bytes) into a Direct3D texture. The texture's writers
// (0x1014f060 for 16-bit, 0x1014ef20 for 32-bit pixels) lock the texture and copy rows.

#include "ddraw_min.h"

#include "surface_copy.h"


// Locks the surface (DDLOCK_WAIT | DDLOCK_NOSYSLOCK) and copies its pixels into `texture`; the pixel size
// (pitch / width) picks the 16-bit or 32-bit writer. Other sizes are not copied.
void VideoSurface::CopyTo(Texture *texture)
{
    if (surface != 0)
    {
        SurfaceDesc desc;
        desc.size = sizeof(desc);
        surface->Lock(0, &desc, 0x801, 0);
        int pitch = desc.pitch;
        if (pitch / width == 2)
            texture->Write16(0, 0, width, height, pitch, desc.bits);
        else if (pitch / width == 4)
            texture->M_1014ef20(0, 0, width, height, pitch, desc.bits);
        surface->Unlock(0);
    }
}
