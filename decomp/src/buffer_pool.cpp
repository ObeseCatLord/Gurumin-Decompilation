// Direct3D surface pools: 0x5a0 texture slots with several surface tables, plus a 0x200-entry pool of
// reusable buffers (the frames of animated textures). DAT_10f1a86c tracks the VRAM in use.

#include <string.h>
#include "d3d9_min.h"

struct Bounds
{
    float x;
    float y;
    float z;
    float w;
};

struct SurfaceSet
{
    IUnknownLike *surfaces[5];
    void Release();
};

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;
extern "C" IUnknownLike *DAT_10f0d5e0[0x5a0];
extern "C" IUnknownLike *DAT_10f1c7f0[0x5a0];
extern "C" IUnknownLike *DAT_10f60d38[0x5a0];
extern "C" IUnknownLike *DAT_10f1a870[0x5a0];
extern "C" int DAT_10f1e670[0x5a0];
extern "C" int DAT_10f628e8[0x5a0];
extern "C" int DAT_10f1a86c;
extern "C" IUnknownLike *DAT_10f1bff0[0x200];
extern "C" int DAT_10f1fd90[0x200];
extern "C" int DAT_10f0cd40[0x200];
extern "C" int DAT_10f1de70[0x200];
extern "C" int DAT_10f0ff80;

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

// Releases the five surfaces of a set, last to first.
void SurfaceSet::Release()
{
    if (surfaces[4] != 0)
        surfaces[4]->Release();
    surfaces[4] = 0;
    if (surfaces[3] != 0)
        surfaces[3]->Release();
    surfaces[3] = 0;
    if (surfaces[2] != 0)
        surfaces[2]->Release();
    surfaces[2] = 0;
    if (surfaces[1] != 0)
        surfaces[1]->Release();
    surfaces[1] = 0;
    if (surfaces[0] != 0)
        surfaces[0]->Release();
    surfaces[0] = 0;
}
