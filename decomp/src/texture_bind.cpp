// Texture objects: binding to the Direct3D texture stages and releasing.
//
// A TextureObject (0x38 bytes, table of 1600 at 0x118d0ba8) owns a Direct3D texture. Animated or streamed
// textures keep their frames in the buffer pool (buffer_pool.cpp); for those `poolSlot` != 0 and the texture to
// bind is fetched from the pool when the object is bound.
#include "d3d9_min.h"

// D3DLOCKED_RECT.
struct LockedRect
{
    int pitch;
    void *bits;
};

// IDirect3DTexture9 (only LockRect at slot 19 and UnlockRect at slot 20 are used here).
struct D3DTexture : IUnknownLike
{
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
    virtual long __stdcall LockRect(unsigned int level, LockedRect *locked, const void *rect, unsigned long flags) = 0;
    virtual long __stdcall UnlockRect(unsigned int level) = 0;
};

struct TextureObject
{
    D3DTexture *texture;     // 0x00: Direct3D texture
    IUnknownLike *texture2;  // 0x04: second surface (e.g. the alpha or palette part), may be 0
    int width;               // 0x08: texture width in pixels
    int height;              // 0x0c: texture height in pixels
    int pad_0010;
    int poolIndex;           // 0x14: buffer pool index of the frames
    int pad_0018;
    int poolSlot;            // 0x1c: non-zero when the frames live in the buffer pool
    char pad_0020[0x18];
    int BindStage0();
    int BindStage1();
    int Write16(int x, int y, int rowPixels, int rows, int pitch, const void *bits);
    int Release();
};

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;      // the Direct3D device
extern "C" TextureObject *DAT_10f6550c;         // texture bound to stage 0 (1 = none/unknown)
extern "C" TextureObject *DAT_10f65510;         // texture bound to stage 1 (1 = none/unknown)
extern "C" int DAT_10f141d4;                    // number of texture switches this frame (statistics)
extern "C" void *FUN_10148590(int poolIndex, IUnknownLike *texture, int width, int height, int slot);
extern "C" int FUN_10148520(int poolIndex, int width, int height);
extern "C" int FUN_101484b0(int poolIndex, int width, int height);  // drops the pooled copy (buffer_pool.cpp)
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);  // engine memcpy

// Binds this texture to stage 0 unless it is bound there already. Returns 1.
int TextureObject::BindStage0()
{
    if (DAT_10f6550c != this)
    {
        DAT_10f141d4++;
        if (poolSlot != 0)
        {
            void *pooled = FUN_10148590(poolIndex, texture, width, height, poolSlot);
            DAT_10f1fcf0->SetTexture(0, pooled);
            DAT_10f6550c = this;
            return 1;
        }
        DAT_10f1fcf0->SetTexture(0, texture);
        DAT_10f6550c = this;
    }
    return 1;
}

// Binds this texture to stage 1 (multi-texturing) unless it is bound there already. Returns 1.
int TextureObject::BindStage1()
{
    if (DAT_10f65510 != this)
    {
        DAT_10f141d4++;
        if (poolSlot != 0)
        {
            void *pooled = FUN_10148590(poolIndex, texture, width, height, poolSlot);
            DAT_10f1fcf0->SetTexture(1, pooled);
            DAT_10f65510 = this;
            return 1;
        }
        DAT_10f1fcf0->SetTexture(1, texture);
        DAT_10f65510 = this;
    }
    return 1;
}

// Releases the Direct3D texture(s) (returning pooled frames to the buffer pool first) and forgets any stage
// binding that still points at this object. Returns 0.
int TextureObject::Release()
{
    if (texture != 0)
    {
        if (poolSlot != 0)
            FUN_10148520(poolIndex, width, height);
        texture->Release();
    }
    if (texture2 != 0)
        texture2->Release();
    if (DAT_10f6550c == this)
        DAT_10f6550c = (TextureObject *)1;
    if (DAT_10f65510 == this)
        DAT_10f65510 = (TextureObject *)1;
    width = 0;
    height = 0;
    poolSlot = 0;
    texture = 0;
    texture2 = 0;
    return 0;
}

// Copies 16-bit pixels into the texture: rows y..rows-1 of `rowPixels` pixels each from `bits` (`pitch` bytes per
// source row; `x` is not used). Drops the pooled copy and forgets stage bindings of this texture so the new
// pixels are used. Returns 0 when the texture cannot be locked, else 1.
int TextureObject::Write16(int x, int y, int rowPixels, int rows, int pitch, const void *bits)
{
    LockedRect locked;
    if (texture == 0)
        return 0;
    if (texture->LockRect(0, &locked, 0, 0x800) != 0)
        return 0;
    char *dst = (char *)locked.bits;
    const char *src = (const char *)bits;
    for (int row = y; row < rows; row++)
    {
        DAT_11b07530(dst, src, rowPixels * 2);
        dst += locked.pitch;
        src += pitch;
    }
    texture->UnlockRect(0);
    FUN_101484b0(poolIndex, width, height);
    if (DAT_10f6550c == this)
        DAT_10f6550c = (TextureObject *)1;
    if (DAT_10f65510 == this)
        DAT_10f65510 = (TextureObject *)1;
    return 1;
}
