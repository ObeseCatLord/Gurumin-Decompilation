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

// IDirect3DSurface9 (LockRect at slot 13 and UnlockRect at slot 14).
struct D3DSurface : IUnknownLike
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
    virtual long __stdcall LockRect(LockedRect *locked, const void *rect, unsigned long flags) = 0;
    virtual long __stdcall UnlockRect() = 0;
};

struct TextureObject
{
    D3DTexture *texture;     // 0x00: Direct3D texture
    D3DSurface *surface;     // 0x04: surface of a render-target texture (or a second part), may be 0
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
    int CaptureRgb24(unsigned char *out);
    int CaptureRgbRows(unsigned char *out);
    int DrawTriangles(const void *vertices, int triangles);
    int DrawStrip(struct ScreenVertex *vertices, int count);
    int WriteRgb32(int left, int top, int right, int bottom, int pitch, const unsigned char *bits);
    int WritePaletted(int left, int top, int right, int bottom, int pitch, const unsigned char *bits,
                      const unsigned short *palette);
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
    if (surface != 0)
        surface->Release();
    if (DAT_10f6550c == this)
        DAT_10f6550c = (TextureObject *)1;
    if (DAT_10f65510 == this)
        DAT_10f65510 = (TextureObject *)1;
    width = 0;
    height = 0;
    poolSlot = 0;
    texture = 0;
    surface = 0;
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

// 0x1014f120. Writes 8-bit palette indexes into the 16-bit texture: the rectangle left..right-1, top..bottom-1 of
// the texture gets palette[index] for each source byte (`bits` is the rectangle's first row, `pitch` bytes per
// row). Like Write16, it drops the pooled copy and forgets the stage bindings afterwards. Returns 0 when the
// texture cannot be locked, else 1.
int TextureObject::WritePaletted(int left, int top, int right, int bottom, int pitch, const unsigned char *bits,
                                 const unsigned short *palette)
{
    LockedRect locked;
    if (texture == 0)
        return 0;
    if (texture->LockRect(0, &locked, 0, 0x800) != 0)
        return 0;
    char *dst = (char *)locked.bits;
    const unsigned char *src = bits;
    for (int y = top; y < bottom; y++)
    {
        for (int x = left; x < right; x++)
            ((unsigned short *)dst)[x] = palette[src[x - left]];
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

// 0x1014ef20. Writes 32-bit pixels (B, G, R, unused bytes) into the 16-bit (R5G6B5) texture, rows top..bottom-1,
// two pixels per 32-bit store: the row of `pitch` source bytes becomes pitch / 8 pairs (`left` and `right` are not
// used). Like Write16, it drops the pooled copy and forgets the stage bindings afterwards. Returns 0 when the
// texture cannot be locked, else 1.
int TextureObject::WriteRgb32(int left, int top, int right, int bottom, int pitch, const unsigned char *bits)
{
    LockedRect locked;
    if (texture == 0)
        return 0;
    if (texture->LockRect(0, &locked, 0, 0x800) != 0)
        return 0;
    const unsigned char *src = bits;
    int pairs = pitch / 8;
    char *dst = (char *)locked.bits;
    for (int y = top; y < bottom; y++)
    {
        for (int i = 0; i < pairs; i++)
        {
            // second pixel in the high half, first pixel in the low half
            *(unsigned int *)dst = ((((((src[6] & 0xf8) << 5 | (src[5] & 0xfc)) << 6 | (src[4] & 0xf8)) << 5 |
                                     (src[2] & 0xf8)) << 5 | (src[1] & 0xfc)) << 3) | (src[0] >> 3);
            dst += 4;
            src += 8;
        }
        dst += locked.pitch - pairs * 4;
        src += pitch - pairs * 8;
    }
    texture->UnlockRect(0);
    FUN_101484b0(poolIndex, width, height);
    if (DAT_10f6550c == this)
        DAT_10f6550c = (TextureObject *)1;
    if (DAT_10f65510 == this)
        DAT_10f65510 = (TextureObject *)1;
    return 1;
}

// 0x10145060. Copies this 256x256 render target (R5G6B5) into `out` as 24-bit pixels in B, G, R byte order: the
// thumbnail Save_Write stores in the save file. The surface is read back through a system-memory copy. Returns 1,
// or 0 when the read-back fails.
int TextureObject::CaptureRgb24(unsigned char *out)
{
    D3DSurface *copy;
    DAT_10f1fcf0->CreateOffscreenPlainSurface(256, 256, 0x17 /* D3DFMT_R5G6B5 */, 2 /* D3DPOOL_SYSTEMMEM */,
                                              (void **)&copy, 0);
    if (DAT_10f1fcf0->GetRenderTargetData(surface, copy) != 0)
    {
        copy->Release();
        return 0;
    }
    LockedRect locked;
    copy->LockRect(&locked, 0, 0x10 /* D3DLOCK_READONLY */);
    const unsigned short *pixels = (const unsigned short *)locked.bits;
    for (int i = 0; i < 256 * 256; i++)
    {
        out[i * 3 + 2] = (pixels[i] >> 8) & 0xf8;               // red: top 5 bits
        out[i * 3 + 1] = (unsigned char)(pixels[i] >> 5) << 2;  // green: middle 6 bits
        out[i * 3 + 0] = pixels[i] << 3;                        // blue: low 5 bits
    }
    copy->UnlockRect();
    copy->Release();
    return 1;
}

// 0x10145120. Like CaptureRgb24, but reads back only the first 0x400 pixels (four rows of 256) and writes them in
// R, G, B byte order. When an R5G6B5 copy cannot be created an X1R5G5B5 one is tried (its pixels are still
// converted as R5G6B5). Called by FUN_10017d30. Returns 1, or 0 when the copy or the read-back fails.
int TextureObject::CaptureRgbRows(unsigned char *out)
{
    D3DSurface *copy;
    if (DAT_10f1fcf0->CreateOffscreenPlainSurface(256, 256, 0x17 /* D3DFMT_R5G6B5 */, 2 /* D3DPOOL_SYSTEMMEM */,
                                                  (void **)&copy, 0) != 0 &&
        DAT_10f1fcf0->CreateOffscreenPlainSurface(256, 256, 0x18 /* D3DFMT_X1R5G5B5 */, 2, (void **)&copy, 0) != 0)
        return 0;
    if (DAT_10f1fcf0->GetRenderTargetData(surface, copy) != 0)
    {
        copy->Release();
        return 0;
    }
    LockedRect locked;
    copy->LockRect(&locked, 0, 0x10 /* D3DLOCK_READONLY */);
    const unsigned short *pixels = (const unsigned short *)locked.bits;
    for (int i = 0; i < 0x400; i++)
    {
        out[i * 3 + 0] = (pixels[i] >> 8) & 0xf8;               // red: top 5 bits
        out[i * 3 + 1] = (unsigned char)(pixels[i] >> 5) << 2;  // green: middle 6 bits
        out[i * 3 + 2] = pixels[i] << 3;                        // blue: low 5 bits
    }
    copy->UnlockRect();
    copy->Release();
    return 1;
}

extern "C" void FUN_1013ef30(int state, int value);                   // Render_SetRenderState (cached)
extern "C" void FUN_1013ef90(int stage, int type, int value);         // Render_SetTextureStageState (cached)
extern "C" void FUN_10143b80(void);                                   // Render_UnbindTexture1
extern "C" int DAT_10f0d5c8;                                          // draw calls this frame (statistics)

// 0x1014bb70. Draws `triangles` triangles from `vertices` (FVF 0x1c4: position, diffuse colour, one texture
// coordinate; 0x20 bytes per vertex) with this texture on stage 0: texture times vertex colour for both colour and
// alpha, stages 1 to 3 off. Returns 0 when there is nothing to draw, else 1. Used by the triangle batches
// (tri_batch.cpp).
int TextureObject::DrawTriangles(const void *vertices, int triangles)
{
    if (triangles < 1)
        return 0;
    BindStage0();
    FUN_1013ef30(0x8d, 1);                  // D3DRS_COLORVERTEX
    DAT_10f1fcf0->SetFVF(0x1c4);
    DAT_10f1fcf0->SetVertexShader(0);
    FUN_1013ef30(0x91, 1);                  // D3DRS_DIFFUSEMATERIALSOURCE = D3DMCS_COLOR1
    FUN_1013ef90(0, 4, 4);                  // alpha: modulate
    FUN_1013ef90(0, 5, 2);                  //   texture
    FUN_1013ef90(0, 6, 0);                  //   diffuse
    FUN_1013ef90(0, 1, 4);                  // colour: modulate
    FUN_1013ef90(0, 2, 2);                  //   texture
    FUN_1013ef90(0, 3, 0);                  //   diffuse
    FUN_10143b80();
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    DAT_10f0d5c8++;
    DAT_10f1fcf0->DrawPrimitiveUP(4 /* D3DPT_TRIANGLELIST */, triangles, vertices, 0x20);
    return 1;
}

extern "C" int DAT_10202b34;  // back-buffer width
extern "C" int DAT_10202b38;  // back-buffer height

// Pre-transformed vertex (FVF 0x1c4), 0x20 bytes.
struct ScreenVertex
{
    float x;
    float y;
    char pad_0008[0x18];
};

// 0x1014ba20. Draws a triangle strip of `count` vertices given in 640x480 screen coordinates: the positions are
// scaled to the back-buffer size in place, then drawn like DrawTriangles. Returns 0 when there are fewer than 3
// vertices, else 1.
int TextureObject::DrawStrip(ScreenVertex *vertices, int count)
{
    if (count < 3)
        return 0;
    BindStage0();
    FUN_10143b80();
    FUN_1013ef30(0x8d, 1);                  // D3DRS_COLORVERTEX
    DAT_10f1fcf0->SetFVF(0x1c4);
    DAT_10f1fcf0->SetVertexShader(0);
    FUN_1013ef30(0x91, 1);                  // D3DRS_DIFFUSEMATERIALSOURCE = D3DMCS_COLOR1
    FUN_1013ef90(0, 4, 4);                  // alpha: modulate
    FUN_1013ef90(0, 5, 2);                  //   texture
    FUN_1013ef90(0, 6, 0);                  //   diffuse
    FUN_1013ef90(0, 1, 4);                  // colour: modulate
    FUN_1013ef90(0, 2, 2);                  //   texture
    FUN_1013ef90(0, 3, 0);                  //   diffuse
    float scaleX = (float)DAT_10202b34 / 640.0f;
    float scaleY = (float)DAT_10202b38 / 480.0f;
    FUN_10143b80();
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    for (int i = 0; i < count; i++)
    {
        vertices[i].x = scaleX * vertices[i].x;
        vertices[i].y = scaleY * vertices[i].y;
    }
    DAT_10f0d5c8++;
    DAT_10f1fcf0->DrawPrimitiveUP(5 /* D3DPT_TRIANGLESTRIP */, count - 2, vertices, 0x20);
    return 1;
}
