// EN Direct3D surface pools (JP twin: buffer_pool.cpp).

#include "buffer_pool.h"

// Clears all texture slot tables and the VRAM total. Called when the renderer is reset.
extern "C" void BufferPool_Clear(void)
{
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotTableA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotTableB[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesB[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesC[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesD[i] = 0;
    g_vramInUse = 0;
}

// Releases the surface of pool entry `index` and frees the entry.
extern "C" int TexCache_Free(int index)
{
    if (index < 0)
        return 0;
    if (index >= SURFACE_POOL_SIZE)
        return 0;
    if (g_pooledSurfaceUsed[index] == 0)
        return 0;
    if (g_pooledSurfaces[index] != 0)
    {
        g_pooledSurfaces[index]->Release();
        g_pooledBytesTotal -= g_pooledSurfaceBytes[index];
    }
    g_pooledSurfaces[index] = 0;
    g_pooledSurfaceUsed[index] = 0;
    g_pooledSurfaceReserved[index] = 0;
    return 0;
}

// Releases the two main surfaces of every texture slot (after a device reset) and clears their tables.
extern "C" void BufferPool_ReleaseDynamic(void)
{
    g_d3dDevice->GetAvailableTextureMem();
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
    {
        if (g_slotSurfacesA[i] != 0)
            g_slotSurfacesA[i]->Release();
        if (g_slotSurfacesB[i] != 0)
            g_slotSurfacesB[i]->Release();
    }
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesB[i] = 0;
    g_vramInUse = 0;
}
