// Creating named textures from pixel data in memory (font pages, model textures read from .it3 files and the like).

#include <string.h>

// A texture slot (0x38 bytes, texture_mgr.cpp).
struct TextureObject
{
    char data[0x38];
    // 0x10169c60: creates the texture from `width` x `height` RGB pixels; `alpha` is 0 (none), 1 or 2 (generated)
    // or the alpha pixels. Returns 0 on failure.
    int Create(int width, int height, const void *pixels, int alpha, int flags);
};

extern "C" int FUN_10161920(const char *name);   // Texture_FindByName: slot holding `name`, or -1
extern "C" TextureObject DAT_118d0ba8[1600];       // the texture slots
extern "C" char DAT_10ffe5f8[1600][0x40];          // their names (empty = free)
extern "C" char DAT_10fdeb90[1600];                // reference count per slot
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);   // the engine's allocator
extern "C" void (*DAT_11b1ba40)(void *);                      // the engine's free

// 0x10175c90 (Texture_CreateFromPixels). Returns the slot of the texture named `name`: the existing one when that
// name is loaded, else a free slot that gets a new texture from `pixels` (all white when `pixels` is 0) with one
// reference. Returns -1 for an empty name, when no slot is free or when the texture cannot be created.
extern "C" int FUN_10175c90(const char *name, int width, int height, const unsigned char *pixels, int alpha,
                            int flags)
{
    if (name[0] == 0)
        return -1;
    int slot = FUN_10161920(name);
    if (slot != -1)
        return slot;
    for (slot = 0; slot < 1600; slot++)
    {
        if (DAT_10ffe5f8[slot][0] == 0)
            break;
    }
    if (slot != 1600)
    {
        if (pixels == 0)
        {
            int size = width * height * 3;
            unsigned char *white = (unsigned char *)DAT_11b0b938(size, "rgb");
            memset(white, 0xff, size);
            if (DAT_118d0ba8[slot].Create(width, height, white, alpha, flags))
            {
                strcpy(DAT_10ffe5f8[slot], name);
                DAT_11b1ba40(white);
                DAT_10fdeb90[slot] = 1;
                return slot;
            }
            DAT_11b1ba40(white);
        }
        else if (DAT_118d0ba8[slot].Create(width, height, pixels, alpha, flags))
        {
            strcpy(DAT_10ffe5f8[slot], name);
            DAT_10fdeb90[slot] = 1;
            return slot;
        }
    }
    return -1;
}
