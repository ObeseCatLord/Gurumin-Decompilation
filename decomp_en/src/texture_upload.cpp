// Creating the Direct3D texture of a TextureObject from 24-bit images (EN build; JP twin: texture_bind.cpp).
#include <d3d9.h>

#include "buffer_pool.h"
#include "render.h"
#include "texture_mgr.h"

// Flag for LockRect: do not take the system lock (D3DLOCK_NOSYSLOCK).
const DWORD UPLOAD_LOCK_FLAGS = 0x800;
// Values of the `alpha` argument that ask for an empty render target instead of a filled texture.
const int UPLOAD_TARGET_565 = 1;
const int UPLOAD_TARGET_8888 = 2;
// TextureObject::poolSlot values: how the frames of the texture live in the buffer pool.
const int POOL_SLOT_RGB565 = 1;
const int POOL_SLOT_ARGB4444 = 2;
// Bytes of a 24-bit pixel.
const int UPLOAD_SOURCE_PIXEL_BYTES = 3;
// Mip levels of the system memory texture.
const int UPLOAD_LEVELS = 1;

// TextureObject::Upload: see the declaration. System memory textures also get a buffer pool index. Called by
// TextureObject::Create and the texture reloaders.
int TextureObject::Upload(int width, int height, unsigned char *pixels, const unsigned char *alpha)
{
    if (g_stage0Texture == this)
        g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    if (g_stage1Texture == this)
        g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    if (g_d3dDevice == 0)
        return 1;
    poolSlot = 0;
    target = 0;
    this->width = width;
    this->height = height;
    if (alpha == 0)
    {
        if (g_d3dDevice->CreateTexture(width, height, 1, 0, D3DFMT_R5G6B5, D3DPOOL_SYSTEMMEM, &texture, 0) != 0)
            return 0;
        m_0018 = 0;
        poolSlot = POOL_SLOT_RGB565;
        surface = 0;
        for (int level = 0; level < UPLOAD_LEVELS; level++)
        {
            D3DLOCKED_RECT locked;
            if (texture->LockRect(level, &locked, 0, UPLOAD_LOCK_FLAGS) != 0)
                return 0;
            unsigned char *source = pixels;
            for (int y = 0; y < height; y++)
            {
                unsigned short *row = (unsigned short *)((char *)locked.pBits + locked.Pitch * y);
                unsigned char *pixel = source;
                for (int x = 0; x < width; x++)
                {
                    unsigned short red = pixel[2];
                    unsigned short green = pixel[1];
                    row[x] = (unsigned short)((((red & 0xfff8) << 5 | green) & 0xfffc) << 3 | pixel[0] >> 3);
                    pixel += UPLOAD_SOURCE_PIXEL_BYTES;
                }
                source += width * UPLOAD_SOURCE_PIXEL_BYTES;
            }
            texture->UnlockRect(level);
            if (level < UPLOAD_LEVELS - 1)
            {
                HalveRgb24(pixels, width, height);
                width /= 2;
                height /= 2;
            }
        }
    }
    else if (alpha == (const unsigned char *)UPLOAD_TARGET_8888)
    {
        target = UPLOAD_TARGET_8888;
        if (g_d3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, 0) != 0)
            return 0;
        return texture->GetSurfaceLevel(0, &surface) == 0;
    }
    else if (alpha == (const unsigned char *)UPLOAD_TARGET_565)
    {
        target = UPLOAD_TARGET_565;
        if (g_d3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &texture, 0) != 0)
            return 0;
        return texture->GetSurfaceLevel(0, &surface) == 0;
    }
    else
    {
        if (g_d3dDevice->CreateTexture(width, height, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_SYSTEMMEM, &texture, 0) != 0)
            return 0;
        m_0018 = 0;
        poolSlot = POOL_SLOT_ARGB4444;
        D3DLOCKED_RECT locked;
        texture->LockRect(0, &locked, 0, UPLOAD_LOCK_FLAGS);
        surface = 0;
        unsigned char *source = pixels;
        const unsigned char *alphaSource = alpha;
        for (int y = 0; y < height; y++)
        {
            unsigned short *row = (unsigned short *)((char *)locked.pBits + locked.Pitch * y);
            unsigned char *pixel = source;
            const unsigned char *alphaPixel = alphaSource;
            for (int x = 0; x < width; x++)
            {
                unsigned short red = pixel[2];
                unsigned short green = pixel[1];
                row[x] = (unsigned short)((((red & 0xfff0) << 4 | green) & 0xfff0) |
                                          (unsigned short)(alphaPixel[0] >> 4) << 12 | pixel[0] >> 4);
                pixel += UPLOAD_SOURCE_PIXEL_BYTES;
                alphaPixel += UPLOAD_SOURCE_PIXEL_BYTES;
            }
            source += width * UPLOAD_SOURCE_PIXEL_BYTES;
            alphaSource += width * UPLOAD_SOURCE_PIXEL_BYTES;
        }
        texture->UnlockRect(0);
    }
    poolIndex = BufferPool_Alloc();
    return 1;
}
