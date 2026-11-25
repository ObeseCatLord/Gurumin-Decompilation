// Creating named textures from pixel data in memory (EN build; JP: decomp/src/texture_create.cpp).

#include <string.h>

#include "engine.h"
#include "engine_memory.h"
#include "model_bank.h"
#include "render.h"
#include "texture_mgr.h"

// Bytes of an all-white RGB image per pixel, and the value of a white byte.
const int RGB_BYTES_PER_PIXEL = 3;
const int WHITE_BYTE = 0xff;

// Like Texture_CreateFromPixels, with the size, pixels and alpha taken from `source` (all white when it kept no
// pixels): returns the slot of the texture named `name` (the existing one when that name is loaded), or -1 for an
// empty name, when no slot is free or when the texture cannot be created. EN 0x71c740 (JP 0x10175a90).
extern "C" int Texture_CreateCopy(const char *name, const TextureObject *source, int flags)
{
    const unsigned char *pixels = source->sourcePixels;
    int alpha = source->sourceAlpha;
    int width = source->sourceWidth;
    int height = source->sourceHeight;
    if (name[0] == 0)
        return -1;
    int slot = Texture_FindByName(name);
    if (slot != -1)
        return slot;
    for (slot = 0; slot < TEXTURE_SLOT_COUNT; slot++)
    {
        if (g_textureNames[slot][0] == 0)
            break;
    }
    if (slot != TEXTURE_SLOT_COUNT)
    {
        if (pixels == 0)
        {
            int size = width * height * RGB_BYTES_PER_PIXEL;
            unsigned char *white = (unsigned char *)g_engineAlloc(size, "rgb");
            memset(white, WHITE_BYTE, size);
            if (g_textures[slot].Create(width, height, white, alpha, flags))
            {
                strcpy(g_textureNames[slot], name);
                g_engineFree(white);
                g_textureRefCount[slot] = 1;
                return slot;
            }
            g_engineFree(white);
        }
        else if (g_textures[slot].Create(width, height, pixels, alpha, flags))
        {
            strcpy(g_textureNames[slot], name);
            g_textureRefCount[slot] = 1;
            return slot;
        }
    }
    return -1;
}

// Creates the texture from width x height RGB `pixels` (at most TEXTURE_MAX_SIDE on each side). `alpha` is
// TEXTURE_ALPHA_NONE, a generated kind or a pointer to alpha pixels (one per RGB triple). Unless the shared textures are
// loading (or the alpha is generated) the pixels and the alpha pixels are kept in copies so the texture can be rebuilt
// after a device reset. With `flags` set the alpha is speckled: one pixel in four at random keeps its alpha (a new
// alpha buffer when there was none). Returns 0 for a bad size, else the result of Upload. Called by
// Texture_CreateFromPixels and Texture_CreateCopy. EN 0x71c410 (JP 0x10169c60).
int TextureObject::Create(int width, int height, const void *pixels, int alpha, int flags)
{
    if (width > TEXTURE_MAX_SIDE)
        return 0;
    if (height > TEXTURE_MAX_SIDE)
        return 0;
    if (alpha == TEXTURE_ALPHA_GENERATED_1 || alpha == TEXTURE_ALPHA_GENERATED_2)
        generatedAlpha = 1;
    else
        generatedAlpha = 0;
    hasOwnAlpha = 0;
    if (generatedAlpha == 0 && alpha != TEXTURE_ALPHA_NONE)
        hasOwnAlpha = 1;
    sourcePixels = 0;
    sourceAlpha = 0;
    sourceWidth = width;
    sourceHeight = height;
    if (width <= 0 || height <= 0)
        return 0;
    if (g_sharedTextureLoading == 0 || generatedAlpha != 0)
    {
        if (alpha > TEXTURE_ALPHA_GENERATED_2 || alpha == TEXTURE_ALPHA_NONE)
        {
            int size = width * height * TEXTURE_SOURCE_BYTES;
            sourcePixels = (unsigned char *)g_engineAlloc(size, "m_rgb");
            g_engineCopy((void *)sourcePixels, pixels, size);
            if (alpha != TEXTURE_ALPHA_NONE && alpha != TEXTURE_ALPHA_GENERATED_1)
            {
                sourceAlpha = (int)g_engineAlloc(size, "m_al");
                g_engineCopy((void *)sourceAlpha, (const void *)alpha, size);
            }
        }
        if (alpha == TEXTURE_ALPHA_GENERATED_1)
            sourceAlpha = TEXTURE_ALPHA_GENERATED_1;
        if (alpha == TEXTURE_ALPHA_GENERATED_2)
            sourceAlpha = TEXTURE_ALPHA_GENERATED_2;
    }
    int size;
    if (flags != 0 && alpha != TEXTURE_ALPHA_GENERATED_1 && alpha != TEXTURE_ALPHA_GENERATED_2)
    {
        if (alpha == TEXTURE_ALPHA_NONE)
        {
            unsigned char *speckles = (unsigned char *)g_engineAlloc(size = width * height * TEXTURE_SOURCE_BYTES, "m_al");
            alpha = (int)speckles;
            sourceAlpha = alpha;
            for (int i = 0; i < size; i += TEXTURE_SOURCE_BYTES)
                speckles[i] = (Random() & TEXTURE_SPECKLE_MASK) >= TEXTURE_SPECKLE_KEEP ? 0 : -1;
        }
        else
        {
            size = width * height * TEXTURE_SOURCE_BYTES;
            for (int i = 0; i < size; i += TEXTURE_SOURCE_BYTES)
            {
                if ((Random() & TEXTURE_SPECKLE_MASK) >= TEXTURE_SPECKLE_KEEP)
                    ((unsigned char *)alpha)[i] = 0;
            }
        }
    }
    return Upload(width, height, (unsigned char *)pixels, (const unsigned char *)alpha);
}
