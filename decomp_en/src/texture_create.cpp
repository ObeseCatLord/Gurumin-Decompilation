// Creating named textures from pixel data in memory (EN build; JP: decomp/src/texture_create.cpp).

#include <string.h>

#include "engine_memory.h"
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
