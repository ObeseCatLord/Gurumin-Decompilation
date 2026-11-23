// Reference counts for shared model and texture files, and texture slot objects.
// MeshInstance is the same 0x38-byte texture slot object as TextureObject (texture_bind.cpp).

struct IntPair
{
    int first;
    int second;
    IntPair();
};

struct MeshInstance : IntPair
{
    char pad_0008[0x18];
    int m_0020;               // 0x20: image pixels (1 and 2 mark built-in images that are not freed)
    int m_0024;               // 0x24: second image buffer
    char m_0028;              // 0x28: 1 when the image has its own alpha pixels
    char pad_0029[3];
    int m_002c;               // 0x2c: image width
    int m_0030;               // 0x30: image height
    int m_0034;
    int Release();            // 0x1014f520 (TextureObject::Release)
    // 0x1014f1f0: creates the Direct3D texture from `width` x `height` RGB pixels and alpha (0, 1, 2 or pixels).
    int Upload(int width, int height, int pixels, int alpha);
    int Create(int width, int height, int pixels, int alpha, int flags);
    void FreeImage();

    MeshInstance();
    int M_10169df0();
    int M_10169e10();
    int M_10169f70(int value);
};

struct MeshRef
{
    char pad_0000[0x4c];
    int index;
};

struct TextureRef
{
    int index;
};

#include "anim_model.h"

extern "C" int DAT_10feb3e0[0xc80];
extern "C" char DAT_10fada90[0xc80][0x20];
extern "C" int DAT_10faae88[0x400];
extern "C" char DAT_10ff65f8[0x400][0x20];
extern "C" int DAT_1101d884;
extern "C" int DAT_1101d88c;
extern "C" int FUN_1014f520(void);
extern "C" int __stdcall FUN_1014f1f0(int, int, int, int);
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
    int index = texture->index;
    return ++DAT_10faae88[index];
}

// Releases one reference to a shared texture file (freeing the name at 0). Returns the remaining count.
extern "C" int FUN_10169c00(const TextureRef *texture)
{
    int index = texture->index;
    if (index < 0)
        return 0;
    if (--DAT_10faae88[index] <= 0)
    {
        DAT_10ff65f8[index][0] = 0;
        DAT_10faae88[index] = 0;
    }
    return DAT_10faae88[index];
}

extern "C" int DAT_1101d874;                     // texture memory in use (bytes)
extern "C" void (*DAT_11b1ba40)(void *);          // engine free
extern "C" int DAT_1101d878;                     // non-zero = images without generated alpha are not kept
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);              // engine allocator
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);   // engine memcpy
extern "C" int FUN_1013f880(void);                // the game's random number

// 0x10169c60 (TextureObject::Create). Creates the texture from `width` x `height` RGB `pixels` (at most 2048 x 2048).
// `alpha` is 0 (opaque), 1 or 2 (generated alpha) or a buffer of alpha pixels (one per RGB triple). Unless
// DAT_1101d878 is set, the pixels and alpha pixels are kept in copies (for re-creating the texture after a device
// reset). With `flags` set the alpha is speckled: one in four pixels at random keeps it (a new buffer when there
// was no alpha). Returns 0 for a bad size, else the result of Upload.
int MeshInstance::Create(int width, int height, int pixels, int alpha, int flags)
{
    if (width > 0x800 || height > 0x800)
        return 0;
    if (alpha == 1 || alpha == 2)
        m_0034 = 1;
    else
        m_0034 = 0;
    m_0028 = 0;
    if (m_0034 == 0 && alpha != 0)
        m_0028 = 1;
    m_0024 = 0;
    m_0020 = 0;
    m_002c = width;
    m_0030 = height;
    if (width <= 0 || height <= 0)
        return 0;
    if (DAT_1101d878 == 0 || m_0034 != 0)
    {
        if (alpha > 2 || alpha == 0)
        {
            int size = width * height * 3;
            m_0024 = (int)DAT_11b0b938(size, "m_rgb");
            DAT_11b07530((void *)m_0024, (const void *)pixels, size);
            if (alpha != 0 && alpha != 1)
            {
                m_0020 = (int)DAT_11b0b938(size, "m_al");
                DAT_11b07530((void *)m_0020, (const void *)alpha, size);
            }
        }
        if (alpha == 1)
            m_0020 = 1;
        if (alpha == 2)
            m_0020 = 2;
    }
    if (flags != 0 && alpha != 1 && alpha != 2)
    {
        if (alpha == 0)
        {
            int size = width * height * 3;
            unsigned char *speckles = (unsigned char *)DAT_11b0b938(size, "m_al");
            alpha = (int)speckles;
            m_0020 = alpha;
            for (int i = 0; i < size; i += 3)
                speckles[i] = (FUN_1013f880() & 7) < 2 ? 0xff : 0;
        }
        else
        {
            int size = width * height * 3;
            for (int i = 0; i < size; i += 3)
            {
                if ((FUN_1013f880() & 7) >= 2)
                    ((unsigned char *)alpha)[i] = 0;
            }
        }
    }
    return Upload(width, height, pixels, alpha);
}

// Frees the stored image and releases the Direct3D texture. The texture memory counter drops by the size of a
// 16-bit texture with five mip levels below it.
void MeshInstance::FreeImage()
{
    DAT_1101d874 -= m_002c * m_0030 * 2;
    DAT_1101d874 -= (m_002c / 2) * (m_0030 / 2) * 2;
    DAT_1101d874 -= (m_002c / 4) * (m_0030 / 4) * 2;
    DAT_1101d874 -= (m_002c / 8) * (m_0030 / 8) * 2;
    DAT_1101d874 -= (m_002c / 16) * (m_0030 / 16) * 2;
    DAT_1101d874 -= (m_002c / 32) * (m_0030 / 32) * 2;
    if (m_0020 == 1)
        m_0020 = 0;
    if (m_0020 == 2)
        m_0020 = 0;
    if (m_0024 != 0)
        DAT_11b1ba40((void *)m_0024);
    if (m_0020 != 0)
        DAT_11b1ba40((void *)m_0020);
    m_0020 = 0;
    m_0024 = 0;
    m_002c = 0;
    m_0030 = 0;
    Release();
}

// Constructor: clears the two size fields (the rest is set when a texture is loaded).
MeshInstance::MeshInstance()
{
    m_0024 = 0;
    m_0020 = 0;
}

// Releases the Direct3D texture when one was created (device lost).
int MeshInstance::M_10169df0()
{
    if (m_0034 != 0)
        return FUN_1014f520();
    return 1;
}

// Re-creates the Direct3D texture from the stored image (after a device reset).
int MeshInstance::M_10169e10()
{
    if (m_0034 != 0)
        return Upload(m_002c, m_0030, m_0024, m_0020);
    return 1;
}

// Updates the texture with `value` (FUN_10143870 on the stored image).
int MeshInstance::M_10169f70(int value)
{
    return FUN_10143870(m_002c, m_0030, value);
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
    slotSource[slot] = source;
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

// Sets the scene parameter DAT_1101d884 and resets its timer.
extern "C" int FUN_1016a8a0(int value)
{
    DAT_1101d884 = value;
    DAT_1101d88c = 0;
    return 0;
}
