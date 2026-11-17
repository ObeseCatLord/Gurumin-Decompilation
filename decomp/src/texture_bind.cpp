// Texture objects: binding to the Direct3D texture stages and releasing.
//
// A TextureObject (0x38 bytes, table of 1600 at 0x118d0ba8) owns a Direct3D texture. Animated or streamed
// textures keep their frames in the buffer pool (buffer_pool.cpp); for those `poolSlot` != 0 and the texture to
// bind is fetched from the pool when the object is bound.
#include "d3d9_min.h"

struct TextureObject
{
    IUnknownLike *texture;   // 0x00: Direct3D texture
    IUnknownLike *texture2;  // 0x04: second surface (e.g. the alpha or palette part), may be 0
    int frame;               // 0x08
    int frameCount;          // 0x0c
    int pad_0010;
    int poolIndex;           // 0x14: buffer pool index of the frames
    int pad_0018;
    int poolSlot;            // 0x1c: non-zero when the frames live in the buffer pool
    char pad_0020[0x18];
    int BindStage0();
    int BindStage1();
    int Release();
};

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;      // the Direct3D device
extern "C" TextureObject *DAT_10f6550c;         // texture bound to stage 0 (1 = none/unknown)
extern "C" TextureObject *DAT_10f65510;         // texture bound to stage 1 (1 = none/unknown)
extern "C" int DAT_10f141d4;                    // number of texture switches this frame (statistics)
extern "C" void *FUN_10148590(int poolIndex, IUnknownLike *texture, int frame, int frameCount, int slot);
extern "C" int FUN_10148520(int poolIndex, int frame, int frameCount);

// Binds this texture to stage 0 unless it is bound there already. Returns 1.
int TextureObject::BindStage0()
{
    if (DAT_10f6550c != this)
    {
        DAT_10f141d4++;
        if (poolSlot != 0)
        {
            void *pooled = FUN_10148590(poolIndex, texture, frame, frameCount, poolSlot);
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
            void *pooled = FUN_10148590(poolIndex, texture, frame, frameCount, poolSlot);
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
            FUN_10148520(poolIndex, frame, frameCount);
        texture->Release();
    }
    if (texture2 != 0)
        texture2->Release();
    if (DAT_10f6550c == this)
        DAT_10f6550c = (TextureObject *)1;
    if (DAT_10f65510 == this)
        DAT_10f65510 = (TextureObject *)1;
    frame = 0;
    frameCount = 0;
    poolSlot = 0;
    texture = 0;
    texture2 = 0;
    return 0;
}
