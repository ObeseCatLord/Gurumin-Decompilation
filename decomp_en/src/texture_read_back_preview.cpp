// Reading the first pixels of a render-target texture back as 24-bit pixels (EN build, EN-only: no JP twin found).
#include "texture_mgr.h"
#include "render.h"

// Side in pixels of the square render target that is read back.
const int PREVIEW_READ_SIDE = 256;
// Pixels converted: the first 0x400 pixels of the read-back surface (the top four rows).
const int PREVIEW_PIXEL_COUNT = 0x400;
// Bytes of one 24-bit output pixel.
const int PREVIEW_RGB_PIXEL_SIZE = 3;

// TextureObject::ReadBackRgbPreview: ReadBackRgb for the first 0x400 pixels only. Copies the 256 x 256 render target
// surface into a system memory surface (R5G6B5, or X1R5G5B5 when that format cannot be created) and writes those pixels
// to `rgb` as 24-bit pixels. Returns 1, or 0 when no surface can be created or the copy fails. Called by the screen
// check at EN 0x5c2960.
int TextureObject::ReadBackRgbPreview(unsigned char *rgb)
{
    IDirect3DSurface9 *copy;
    if (g_d3dDevice->CreateOffscreenPlainSurface(PREVIEW_READ_SIDE, PREVIEW_READ_SIDE, D3DFMT_R5G6B5, D3DPOOL_SYSTEMMEM, &copy, 0) != 0 &&
        g_d3dDevice->CreateOffscreenPlainSurface(PREVIEW_READ_SIDE, PREVIEW_READ_SIDE, D3DFMT_X1R5G5B5, D3DPOOL_SYSTEMMEM, &copy, 0) != 0)
        return 0;
    if (g_d3dDevice->GetRenderTargetData(surface, copy) != 0)
    {
        copy->Release();
        return 0;
    }
    D3DLOCKED_RECT locked;
    copy->LockRect(&locked, 0, D3DLOCK_READONLY);
    unsigned char *blue = rgb + 2;
    const unsigned char *bytes = (const unsigned char *)locked.pBits;
    const unsigned short *words = (const unsigned short *)locked.pBits;
    for (int i = 0; i < PREVIEW_PIXEL_COUNT; i++)
    {
        blue[-2] = bytes[i * 2 + 1] & 0xf8;
        blue[-1] = (unsigned char)(words[i] >> 5) << 2;
        blue[0] = bytes[i * 2] << 3;
        blue += PREVIEW_RGB_PIXEL_SIZE;
    }
    copy->UnlockRect();
    copy->Release();
    return 1;
}
