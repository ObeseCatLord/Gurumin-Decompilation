// Reference counts for shared model and texture files, and texture slot objects.
// The TextureObject methods here keep a copy of the source image so the texture can be rebuilt after a device reset.

#include "texture_object.h"

#include "mesh_ref.h"

#include "texture_ref.h"

#include "anim_model.h"

#include "camera_vars.h"
#include "map_vars.h"
#include "render_vars.h"

extern "C" int __stdcall FUN_10143870(int, int, int);

// Releases one reference to a shared model file; the name entry is freed when the count reaches 0.
// Returns the remaining count.
extern "C" int FUN_10169990(const MeshRef *mesh)
{
    int index = mesh->index;
    if (index < 0)
        return 0;
    if (--DAT_10feb3e0[index] <= 0)
    {
        DAT_10fada90[index][0] = 0;
        DAT_10feb3e0[index] = 0;
    }
    return DAT_10feb3e0[index];
}

// 0 when the shared texture name table (0x400 entries) still has a free entry, -1 when it is full.
extern "C" int FUN_101699d0(void)
{
    for (int i = 0; i < 0x400; i++)
    {
        if (DAT_10ff65f8[i][0] == 0)
            return 0;
    }
    return -1;
}

// Adds a reference to a shared texture file and returns the new count.
extern "C" int FUN_10169be0(const TextureRef *texture)
{
    int index = texture->slot;
    return ++DAT_10faae88[index];
}

// Releases one reference to a shared texture file (freeing the name at 0). Returns the remaining count.
extern "C" int FUN_10169c00(const TextureRef *texture)
{
    int index = texture->slot;
    if (index < 0)
        return 0;
    if (--DAT_10faae88[index] <= 0)
    {
        DAT_10ff65f8[index][0] = 0;
        DAT_10faae88[index] = 0;
    }
    return DAT_10faae88[index];
}
#include "engine_memory.h"
extern "C" int FUN_1013f880(void);                // the game's random number

// 0x10169c60. Creates the texture from `width` x `height` RGB `pixels` (at most TEXTURE_MAX_SIDE on each side).
// `alpha` is ALPHA_NONE, ALPHA_GENERATED_1 / ALPHA_GENERATED_2 or a buffer of alpha pixels (one per RGB triple).
// Unless DAT_1101d878 is set (or the alpha is generated), the pixels and alpha pixels are kept in copies for
// re-creating the texture after a device reset. With `flags` set the alpha is speckled: one in four pixels at random
// keeps it (a new buffer when there was no alpha). Returns 0 for a bad size, else the result of Upload. Called by
// Texture_CreateFromPixels and Texture_CreateCopy (texture_create.cpp).
int TextureObject::Create(int width, int height, const void *pixels, int alpha, int flags)
{
    if (width > TEXTURE_MAX_SIDE)
        return 0;
    if (height > TEXTURE_MAX_SIDE)
        return 0;
    if (alpha == ALPHA_GENERATED_1 || alpha == ALPHA_GENERATED_2)
        generatedAlpha = 1;
    else
        generatedAlpha = 0;
    hasOwnAlpha = 0;
    if (generatedAlpha == 0 && alpha != ALPHA_NONE)
        hasOwnAlpha = 1;
    sourcePixels = 0;
    sourceAlpha = 0;
    sourceWidth = width;
    sourceHeight = height;
    if (width <= 0 || height <= 0)
        return 0;
    if (DAT_1101d878 == 0 || generatedAlpha != 0)
    {
        if (alpha > ALPHA_GENERATED_2 || alpha == ALPHA_NONE)
        {
            int size = width * height * 3;
            sourcePixels = (unsigned char *)DAT_11b0b938(size, "m_rgb");
            DAT_11b07530(sourcePixels, pixels, size);
            if (alpha != ALPHA_NONE && alpha != ALPHA_GENERATED_1)
            {
                sourceAlpha = (unsigned char *)DAT_11b0b938(size, "m_al");
                DAT_11b07530(sourceAlpha, (const void *)alpha, size);
            }
        }
        // The generated kinds are stored as their marker value instead of a buffer.
        if (alpha == ALPHA_GENERATED_1)
            sourceAlpha = (unsigned char *)ALPHA_GENERATED_1;
        if (alpha == ALPHA_GENERATED_2)
            sourceAlpha = (unsigned char *)ALPHA_GENERATED_2;
    }
    if (flags != 0 && alpha != ALPHA_GENERATED_1 && alpha != ALPHA_GENERATED_2)
    {
        if (alpha == ALPHA_NONE)
        {
            int size = width * height * 3;
            unsigned char *speckles = (unsigned char *)DAT_11b0b938(size, "m_al");
            alpha = (int)speckles;
            sourceAlpha = speckles;
            for (int i = 0; i < size; i += 3)
                speckles[i] = (FUN_1013f880() & 7) < 2 ? 0xff : 0;
        }
        else
        {
            for (int i = 0; i < width * height * 3; i += 3)
            {
                if ((FUN_1013f880() & 7) >= 2)
                    ((unsigned char *)alpha)[i] = 0;
            }
        }
    }
    return Upload(width, height, (unsigned char *)pixels, (const unsigned char *)alpha);
}

// 0x10169e40. Frees the kept image and releases the Direct3D texture. The texture memory counter drops by the size
// of a 16-bit texture with five mip levels below it. Called by the texture manager when a slot is unloaded.
void TextureObject::Unload()
{
    DAT_1101d874 -= sourceWidth * sourceHeight * 2;
    DAT_1101d874 -= (sourceWidth / 2) * (sourceHeight / 2) * 2;
    DAT_1101d874 -= (sourceWidth / 4) * (sourceHeight / 4) * 2;
    DAT_1101d874 -= (sourceWidth / 8) * (sourceHeight / 8) * 2;
    DAT_1101d874 -= (sourceWidth / 16) * (sourceHeight / 16) * 2;
    DAT_1101d874 -= (sourceWidth / 32) * (sourceHeight / 32) * 2;
    // The generated-alpha markers are not buffers.
    if (sourceAlpha == (unsigned char *)ALPHA_GENERATED_1)
        sourceAlpha = 0;
    if (sourceAlpha == (unsigned char *)ALPHA_GENERATED_2)
        sourceAlpha = 0;
    if (sourcePixels != 0)
        DAT_11b1ba40(sourcePixels);
    if (sourceAlpha != 0)
        DAT_11b1ba40(sourceAlpha);
    sourceAlpha = 0;
    sourcePixels = 0;
    sourceWidth = 0;
    sourceHeight = 0;
    Release();
}

// 0x10169c40. Constructor (runs for each of the texture slots DAT_118d0ba8): no kept image yet.
TextureObject::TextureObject()
{
    sourcePixels = 0;
    sourceAlpha = 0;
}

// 0x10169df0. Releases the Direct3D texture of a generated-alpha texture before a device reset (texture manager).
int TextureObject::Recreate()
{
    if (generatedAlpha != 0)
        return Release();
    return 1;
}

// 0x10169e10. Re-uploads the kept image of a generated-alpha texture after a device reset (texture manager).
int TextureObject::Restore()
{
    if (generatedAlpha != 0)
        return Upload(sourceWidth, sourceHeight, sourcePixels, sourceAlpha);
    return 1;
}

// 0x10169f70. Calls FUN_10143870 with the image size and `value`.
int TextureObject::UpdateImage(int value)
{
    return FUN_10143870(sourceWidth, sourceHeight, value);
}

// 0x1016a220. Sets the source and layer of motion slot `slot` (16 slots, layers up to 0x8f; a source is required).
int Playback::SetSlotSource(void *source, int slot, int layer)
{
    if (layer > 0x8f)
        return 0;
    if (slot > 15)
        return 0;
    if (source == 0)
        return 0;
    slotLayer[slot] = layer;
    slotSource[slot] = (MotionSource *)source;
    return 1;
}

// 0x1016a260. Sets the (x, y) offset of slot `slot` in layer `layer`.
int Playback::SetSlotOffset(int layer, int slot, int x, int y)
{
    if (layer > 0x8f)
        return 0;
    if (slot > 15)
        return 0;
    slotOffsetX[layer][slot] = x;
    slotOffsetY[layer][slot] = y;
    return 1;
}

// 0x1016a2a0. Sets parameter `slot` (0..7) of layer `layer`.
int Playback::SetLayerParam(int layer, int slot, int value)
{
    if (layer > 0x8f)
        return 0;
    if (slot > 7)
        return 0;
    layerParams[layer][slot] = value;
    return 1;
}

// Sets the model the camera follows (DAT_1101d884) and cancels any camera cut blend still running. Returns 0.
extern "C" int FUN_1016a8a0(CameraTarget *target)
{
    DAT_1101d884 = target;
    DAT_1101d88c = 0;
    return 0;
}
