// Reading a render-target texture back as 24-bit pixels (EN build, EN-only: no JP twin found).
#include "texture_mgr.h"
#include "render.h"

// Side in pixels of the square render target that is read back.
const int READ_BACK_SIDE = 256;
// Pixels of the read-back image.
const int READ_BACK_PIXEL_COUNT = READ_BACK_SIDE * READ_BACK_SIDE;
// Bytes of one 24-bit output pixel.
const int RGB_PIXEL_SIZE = 3;

// TextureObject::ReadBackRgb: copies the 256 x 256 R5G6B5 render target surface into a system memory surface and writes
// it to `rgb` as 24-bit pixels (blue, green, red per pixel, each channel widened by shifting). Returns 1, or 0 when the
// copy fails.
int TextureObject::ReadBackRgb(unsigned char *rgb)
{
    IDirect3DSurface9 *copy;
    g_d3dDevice->CreateOffscreenPlainSurface(READ_BACK_SIDE, READ_BACK_SIDE, D3DFMT_R5G6B5, D3DPOOL_SYSTEMMEM, &copy, 0);
    if (g_d3dDevice->GetRenderTargetData(surface, copy) != 0)
    {
        copy->Release();
        return 0;
    }
    D3DLOCKED_RECT locked;
    copy->LockRect(&locked, 0, D3DLOCK_READONLY);
    unsigned char *green = rgb + 1;
    const unsigned char *bytes = (const unsigned char *)locked.pBits;
    const unsigned short *words = (const unsigned short *)locked.pBits;
    for (int i = 0; i < READ_BACK_PIXEL_COUNT; i++)
    {
        green[1] = bytes[i * 2 + 1] & 0xf8;
        green[0] = (unsigned char)(words[i] >> 5) << 2;
        green[-1] = bytes[i * 2] << 3;
        green += RGB_PIXEL_SIZE;
    }
    copy->UnlockRect();
    copy->Release();
    return 1;
}
