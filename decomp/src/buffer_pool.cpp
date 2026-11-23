// Direct3D surface pools: 0x5a0 texture slots with several surface tables, plus a 0x200-entry pool of
// reusable buffers (the frames of animated textures). DAT_10f1a86c tracks the VRAM in use.

#include <string.h>
#include "d3d9_min.h"

#include "bounds.h"

#include "render_vars.h"

// Resets four bounding boxes to the 'empty' value (-30000 in each axis, w = 1) and their counts to 0.
extern "C" int FUN_10147620(void *, Bounds *bounds, int *counts)
{
    bounds[0].w = 1.0f;
    bounds[0].x = -30000.0f;
    bounds[0].y = -30000.0f;
    bounds[0].z = -30000.0f;
    counts[0] = 0;
    bounds[1].w = 1.0f;
    bounds[1].x = -30000.0f;
    bounds[1].y = -30000.0f;
    bounds[1].z = -30000.0f;
    counts[1] = 0;
    bounds[2].w = 1.0f;
    bounds[2].x = -30000.0f;
    bounds[2].y = -30000.0f;
    bounds[2].z = -30000.0f;
    counts[2] = 0;
    bounds[3].w = 1.0f;
    bounds[3].x = -30000.0f;
    bounds[3].y = -30000.0f;
    bounds[3].z = -30000.0f;
    counts[3] = 0;
    return 0;
}

// Clears all texture slot tables and the VRAM total.
extern "C" void FUN_10147700(void)
{
    memset(DAT_10f1e670, 0, sizeof(DAT_10f1e670));
    memset(DAT_10f628e8, 0, sizeof(DAT_10f628e8));
    memset(DAT_10f1c7f0, 0, sizeof(DAT_10f1c7f0));
    memset(DAT_10f0d5e0, 0, sizeof(DAT_10f0d5e0));
    memset(DAT_10f60d38, 0, sizeof(DAT_10f60d38));
    memset(DAT_10f1a870, 0, sizeof(DAT_10f1a870));
    DAT_10f1a86c = 0;
}

// Releases the two main surfaces of every texture slot (after a device reset) and clears their tables.
extern "C" void FUN_10147760(void)
{
    DAT_10f1fcf0->GetAvailableTextureMem();
    for (int i = 0; i < 0x5a0; i++)
    {
        if (DAT_10f0d5e0[i] != 0)
            DAT_10f0d5e0[i]->Release();
        if (DAT_10f1c7f0[i] != 0)
            DAT_10f1c7f0[i]->Release();
    }
    memset(DAT_10f1c7f0, 0, sizeof(DAT_10f1c7f0));
    memset(DAT_10f0d5e0, 0, sizeof(DAT_10f0d5e0));
    DAT_10f1a86c = 0;
}

// Releases all four surfaces of every texture slot and clears the tables.
extern "C" void FUN_10148280(void)
{
    for (int i = 0; i < 0x5a0; i++)
    {
        if (DAT_10f1a870[i] != 0)
            DAT_10f1a870[i]->Release();
        if (DAT_10f60d38[i] != 0)
            DAT_10f60d38[i]->Release();
        if (DAT_10f0d5e0[i] != 0)
            DAT_10f0d5e0[i]->Release();
        if (DAT_10f1c7f0[i] != 0)
            DAT_10f1c7f0[i]->Release();
    }
    memset(DAT_10f1c7f0, 0, sizeof(DAT_10f1c7f0));
    memset(DAT_10f0d5e0, 0, sizeof(DAT_10f0d5e0));
    memset(DAT_10f60d38, 0, sizeof(DAT_10f60d38));
    memset(DAT_10f1a870, 0, sizeof(DAT_10f1a870));
}

// Takes the first free buffer pool entry and returns its index, or -1 when all 0x200 are in use.
extern "C" int FUN_10148410(void)
{
    int i;
    for (i = 0; i < 0x200; i++)
    {
        if (DAT_10f1fd90[i] == 0)
            break;
    }
    if (i == 0x200)
        return -1;
    DAT_10f1fd90[i] = 1;
    return i;
}

// Releases the surface of pool entry `index` but keeps the entry reserved.
extern "C" int FUN_101484b0(int index)
{
    if (index >= 0 && index < 0x200 && DAT_10f1fd90[index] != 0)
    {
        if (DAT_10f1bff0[index] != 0)
        {
            DAT_10f1bff0[index]->Release();
            DAT_10f0ff80 -= DAT_10f1de70[index];
        }
        DAT_10f1bff0[index] = 0;
        DAT_10f1fd90[index] = 1;
        DAT_10f0cd40[index] = 0;
    }
    return 0;
}

// Releases the surface of pool entry `index` and frees the entry.
extern "C" int FUN_10148520(int index)
{
    if (index >= 0 && index < 0x200 && DAT_10f1fd90[index] != 0)
    {
        if (DAT_10f1bff0[index] != 0)
        {
            DAT_10f1bff0[index]->Release();
            DAT_10f0ff80 -= DAT_10f1de70[index];
        }
        DAT_10f1bff0[index] = 0;
        DAT_10f1fd90[index] = 0;
        DAT_10f0cd40[index] = 0;
    }
    return 0;
}


// 0x10147880. Creates a mesh buffer pair in the first free slot: a vertex buffer and a 16-bit index buffer in system
// memory, each rounded up to the next multiple of 256 bytes. Their sizes are recorded and the buffers returned in
// `vertexBuffer` and `indexBuffer`. Returns the slot, or -1 when no slot is free or a buffer cannot be created.
extern "C" int FUN_10147880(IUnknownLike **vertexBuffer, int vertexBytes, IUnknownLike **indexBuffer, int indexBytes)
{
    int indexSize = (indexBytes / 256 + 1) * 256;
    vertexBytes = (vertexBytes / 256 + 1) * 256;  // rounded up in place
    int slot;
    for (slot = 0; slot < 0x5a0; slot++)
    {
        if (DAT_10f1a870[slot] == 0)
            break;
    }
    if (slot == 0x5a0)
        return -1;
    if (DAT_10f1fcf0->CreateVertexBuffer(vertexBytes, 0, 0, 2 /* D3DPOOL_SYSTEMMEM */, &DAT_10f1a870[slot], 0) != 0)
        return -1;
    if (DAT_10f1fcf0->CreateIndexBuffer(indexSize, 0, 0x65 /* D3DFMT_INDEX16 */, 2, &DAT_10f60d38[slot], 0) != 0)
    {
        DAT_10f1a870[slot]->Release();
        DAT_10f1a870[slot] = 0;
        return -1;
    }
    DAT_10f628e8[slot] = vertexBytes;
    DAT_10f1e670[slot] = indexSize;
    DAT_10f60690[slot] = 0;
    *vertexBuffer = DAT_10f1a870[slot];
    *indexBuffer = DAT_10f60d38[slot];
    return slot;
}

extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// 0x10148590. Returns the video-memory copy of pooled buffer `index`, creating it from `source` (width x height,
// 16-bit: R5G6B5 for mode 1, else A4R4G4B4) when it has none. While the pool would exceed its budget, or after a
// failed creation, the least recently used copy is released first; creation is tried up to nine times. Every
// pooled buffer ages by one and the new one starts at 0. Returns 0 for a bad index or when creation fails.
extern "C" IUnknownLike *FUN_10148590(int index, IDirect3DTexture9Like *source, int width, int height, int mode)
{
    if (index < 0 || index >= 0x200 || DAT_10f1fd90[index] == 0)
        return 0;
    IUnknownLike **slot = &DAT_10f1bff0[index];
    if (*slot != 0)
        return *slot;
    int size = width * height * 2;
    int format = mode != 1 ? 0x1a : 0x17;
    int evict = 0;
    int attempts = 0;
    for (;;)
    {
        if (DAT_1020340c < size + DAT_10f0ff80 || evict)
        {
            int oldest = -1;
            int age = 0;
            int found = 0;
            for (int i = 0; i < 0x200; i++)
            {
                if (DAT_10f1bff0[i] != 0 && (!found || DAT_10f0cd40[i] > age))
                {
                    age = DAT_10f0cd40[i];
                    oldest = i;
                    found = 1;
                }
            }
            if (oldest >= 0)
            {
                FUN_1013f070(6, 0, 0);
                if (DAT_10f1bff0[oldest] != 0)
                {
                    DAT_10f1bff0[oldest]->Release();
                    DAT_10f0ff80 -= DAT_10f1de70[oldest];
                }
                DAT_10f1bff0[oldest] = 0;
                DAT_10f0cd40[oldest] = 0;
                DAT_10f1fd90[oldest] = 1;
                evict = 0;
                continue;
            }
        }
        if (DAT_10f1fcf0->CreateTexture(width, height, 1, 0, format, 0, slot, 0) == 0)
        {
            source->AddDirtyRect(0);
            if (DAT_10f1fcf0->UpdateTexture(source, *slot) != 0)
            {
                (*slot)->Release();
                *slot = 0;
                return 0;
            }
            DAT_10f1fd90[index] = 1;
            for (int j = 0; j < 0x200; j++)
                DAT_10f0cd40[j]++;
            DAT_10f0ff80 += size;
            DAT_10f0cd40[index] = 0;
            DAT_10f1de70[index] = size;
            return *slot;
        }
        if (++attempts > 8)
        {
            *slot = 0;
            return 0;
        }
        evict = 1;
    }
}
