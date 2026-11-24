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

// Releases all four surfaces of every texture slot and clears the tables. Called when the renderer is shut down.
extern "C" void BufferPool_ReleaseAll(void)
{
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
    {
        if (g_slotTextureA[i] != 0)
            g_slotTextureA[i]->Release();
        if (g_slotTextureB[i] != 0)
            g_slotTextureB[i]->Release();
        if (g_slotSurfacesA[i] != 0)
            g_slotSurfacesA[i]->Release();
        if (g_slotSurfacesB[i] != 0)
            g_slotSurfacesB[i]->Release();
    }
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotTextureA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotTextureB[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesA[i] = 0;
    for (int i = 0; i < SURFACE_SLOT_COUNT; i++)
        g_slotSurfacesB[i] = 0;
}

// Value of every axis of an empty bounding box.
const float BOUNDS_EMPTY = -30000.0f;
// Number of boxes Bounds_ResetFour resets.
const int BOUNDS_RESET_COUNT = 4;

// Resets four bounding boxes to the "empty" value and their counts to 0. Returns 0. Called when the light boxes of a
// lit shader are rebuilt.
extern "C" int Bounds_ResetFour(void *unused, Vec4 *bounds, int *counts)
{
    bounds[0].x = BOUNDS_EMPTY;
    bounds[0].y = BOUNDS_EMPTY;
    bounds[0].z = BOUNDS_EMPTY;
    bounds[0].w = 1.0f;
    counts[0] = 0;
    bounds[1].x = BOUNDS_EMPTY;
    bounds[1].y = BOUNDS_EMPTY;
    bounds[1].z = BOUNDS_EMPTY;
    bounds[1].w = 1.0f;
    counts[1] = 0;
    bounds[2].x = BOUNDS_EMPTY;
    bounds[2].y = BOUNDS_EMPTY;
    bounds[2].z = BOUNDS_EMPTY;
    bounds[2].w = 1.0f;
    counts[2] = 0;
    bounds[3].x = BOUNDS_EMPTY;
    bounds[3].y = BOUNDS_EMPTY;
    bounds[3].z = BOUNDS_EMPTY;
    bounds[3].w = 1.0f;
    counts[3] = 0;
    return 0;
}

// Creates a mesh buffer pair in the first free slot (a system-memory vertex buffer and a 16-bit index buffer, each
// rounded up to the next multiple of MESH_BUFFER_GRANULARITY bytes), records their sizes and returns them in
// `vertexBuffer` and `indexBuffer`. Returns the slot, or -1 when no slot is free or a buffer cannot be created.
extern "C" int MeshBuffers_Create(IDirect3DVertexBuffer9 **vertexBuffer, int vertexBytes, IDirect3DIndexBuffer9 **indexBuffer, int indexBytes)
{
    int indexSize = (indexBytes / MESH_BUFFER_GRANULARITY + 1) * MESH_BUFFER_GRANULARITY;
    vertexBytes = (vertexBytes / MESH_BUFFER_GRANULARITY + 1) * MESH_BUFFER_GRANULARITY;
    int slot;
    for (slot = 0; slot < SURFACE_SLOT_COUNT; slot++)
    {
        if (g_slotTextureA[slot] == 0)
            break;
    }
    if (slot == SURFACE_SLOT_COUNT)
        return -1;
    if (g_d3dDevice->CreateVertexBuffer(vertexBytes, 0, 0, D3DPOOL_SYSTEMMEM, &g_slotTextureA[slot], 0) != 0)
        return -1;
    if (g_d3dDevice->CreateIndexBuffer(indexSize, 0, D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &g_slotTextureB[slot], 0) != 0)
    {
        g_slotTextureA[slot]->Release();
        g_slotTextureA[slot] = 0;
        return -1;
    }
    g_slotBytesC[slot] = vertexBytes;
    g_slotBytesD[slot] = indexSize;
    g_slotMarked[slot] = 0;
    *vertexBuffer = g_slotTextureA[slot];
    *indexBuffer = g_slotTextureB[slot];
    return slot;
}
