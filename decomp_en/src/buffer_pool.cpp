// EN Direct3D surface pools (JP twin: buffer_pool.cpp).

#include "buffer_pool.h"

// Clears all texture slot tables and the VRAM total. Called when the renderer is reset.
extern "C" void BufferPool_Clear(void)
{
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotTextureA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotTextureB[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesB[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotBytesC[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotBytesD[i] = 0;
    g_vramInUse = 0;
}

// Releases the surface of pool entry `index` and frees the entry; `width` and `height` are ignored.
extern "C" int TexCache_Free(int index, int width, int height)
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

// Releases the textures and surfaces of texture slot `slot`, takes the first surface's bytes off the VRAM total and
// clears the slot's entries. Called by SlotTable::Release and the mesh buffer builder when a mesh drops its buffers.
extern "C" int Texture_ReleaseSlot(int slot)
{
    if (slot < 0)
        return 0;
    if (slot >= SURFACE_SLOT_COUNT)
        return 0;
    if (g_slotTextureA[slot] != 0)
        g_slotTextureA[slot]->Release();
    if (g_slotTextureB[slot] != 0)
        g_slotTextureB[slot]->Release();
    if (g_slotSurfacesA[slot] != 0)
    {
        g_slotSurfacesA[slot]->Release();
        g_vramInUse -= g_slotBytesD[slot] + g_slotBytesC[slot];
    }
    if (g_slotSurfacesB[slot] != 0)
        g_slotSurfacesB[slot]->Release();
    g_slotTextureA[slot] = 0;
    g_slotTextureB[slot] = 0;
    g_slotSurfacesA[slot] = 0;
    g_slotSurfacesB[slot] = 0;
    return 1;
}
