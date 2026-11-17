// Copying a DirectDraw surface (DDSURFACEDESC, 0x6c bytes) into a Direct3D texture. The texture's writers
// (0x1014f060 for 16-bit, 0x1014ef20 for 32-bit pixels) lock the texture and copy rows.

// DDSURFACEDESC (DirectDraw 1-3).
struct SurfaceDesc
{
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    long pitch;               // 0x10: bytes per row
    unsigned long backBufferCount;
    unsigned long refreshRate;
    unsigned long alphaBitDepth;
    unsigned long reserved;
    void *bits;               // 0x24: first pixel
    unsigned long colourKeys[8];
    unsigned long pixelFormat[8];
    unsigned long caps;
};

// IDirectDrawSurface (only Lock at slot 25 and Unlock at slot 32 are used).
struct DrawSurface
{
    virtual void __stdcall Slot00() = 0;
    virtual void __stdcall Slot01() = 0;
    virtual void __stdcall Slot02() = 0;
    virtual void __stdcall Slot03() = 0;
    virtual void __stdcall Slot04() = 0;
    virtual void __stdcall Slot05() = 0;
    virtual void __stdcall Slot06() = 0;
    virtual void __stdcall Slot07() = 0;
    virtual void __stdcall Slot08() = 0;
    virtual void __stdcall Slot09() = 0;
    virtual void __stdcall Slot10() = 0;
    virtual void __stdcall Slot11() = 0;
    virtual void __stdcall Slot12() = 0;
    virtual void __stdcall Slot13() = 0;
    virtual void __stdcall Slot14() = 0;
    virtual void __stdcall Slot15() = 0;
    virtual void __stdcall Slot16() = 0;
    virtual void __stdcall Slot17() = 0;
    virtual void __stdcall Slot18() = 0;
    virtual void __stdcall Slot19() = 0;
    virtual void __stdcall Slot20() = 0;
    virtual void __stdcall Slot21() = 0;
    virtual void __stdcall Slot22() = 0;
    virtual void __stdcall Slot23() = 0;
    virtual void __stdcall Slot24() = 0;
    virtual long __stdcall Lock(void *rect, SurfaceDesc *desc, unsigned long flags, void *event) = 0;
    virtual void __stdcall Slot26() = 0;
    virtual void __stdcall Slot27() = 0;
    virtual void __stdcall Slot28() = 0;
    virtual void __stdcall Slot29() = 0;
    virtual void __stdcall Slot30() = 0;
    virtual void __stdcall Slot31() = 0;
    virtual long __stdcall Unlock(void *rect) = 0;
};

struct Texture
{
    int M_1014f060(int x, int y, int width, int height, int pitch, void *bits);  // copies 16-bit pixels
    int M_1014ef20(int x, int y, int width, int height, int pitch, void *bits);  // copies 32-bit pixels
};

// Owner of an off-screen DirectDraw surface (the video frame).
struct VideoSurface
{
    char pad_0000[0x10];
    DrawSurface *surface;
    char pad_0014[0xc];
    int width;
    int height;
    void CopyTo(Texture *texture);
};

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
            texture->M_1014f060(0, 0, width, height, pitch, desc.bits);
        else if (pitch / width == 4)
            texture->M_1014ef20(0, 0, width, height, pitch, desc.bits);
        surface->Unlock(0);
    }
}
