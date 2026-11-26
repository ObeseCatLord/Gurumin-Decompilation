// Video-memory copies of pooled textures (EN build). Ported from the JP source decomp/src/buffer_pool.cpp (APP.dll
// 0x10148590, unmatched) and the EN bytes.
#include "buffer_pool.h"
#include "render.h"
#include "texture_mgr.h"

// Render_Command flush argument unused values.
const int RENDER_COMMAND_UNUSED = 0;
// Bytes per pixel of the 16-bit pooled textures.
const int POOLED_BYTES_PER_PIXEL = 2;
// Creation attempts after the first one before TexturePool_Get gives up.
const int POOL_MAX_RETRIES = 8;
// Pooled texture mode (the `slot` argument) that selects the opaque 16-bit format.
const int POOL_MODE_OPAQUE = 1;

// Returns the video-memory copy of pooled texture `poolIndex`, creating it from `texture` (width x height, 16 bit:
// R5G6B5 for slot 1, else A4R4G4B4) when it has none. While the pool would exceed its budget, or after a failed
// creation, the least recently used copy is released first; creation is tried up to nine times. Every pooled
// texture ages by one and the new one starts at 0. Returns 0 for a bad index or when creation fails. Called by the
// texture binding code (BindStage0 and BindStage1).
extern "C" IDirect3DBaseTexture9 *TexturePool_Get(int poolIndex, IDirect3DTexture9 *texture, int width, int height, int slot)
{
    if (poolIndex < 0)
        return 0;
    if (poolIndex >= SURFACE_POOL_SIZE || g_pooledSurfaceUsed[poolIndex] == 0)
        return 0;
    IDirect3DTexture9 *existing = g_pooledSurfaces[poolIndex];
    if (existing != 0)
        return existing;
    int evictPending = 0;
    int attempts = 0;
    D3DFORMAT format = D3DFMT_A4R4G4B4;
    int bytes = width * height * POOLED_BYTES_PER_PIXEL;
    if (slot == POOL_MODE_OPAQUE)
        format = D3DFMT_R5G6B5;
    for (;;)
    {
        if (g_pooledBytesTotal + bytes > g_pooledBytesBudget || evictPending)
        {
            int oldest = -1;
            int oldestAge = 0;
            evictPending = 0;
            for (int i = 0; i < SURFACE_POOL_SIZE; i++)
            {
                if (g_pooledSurfaces[i] != 0 && (!evictPending || g_pooledSurfaceReserved[i] > oldestAge))
                {
                    oldestAge = g_pooledSurfaceReserved[i];
                    oldest = i;
                    evictPending = 1;
                }
            }
            if (oldest >= 0)
            {
                Render_Command(RENDER_FLUSH_SCENE, RENDER_COMMAND_UNUSED, RENDER_COMMAND_UNUSED);
                if (g_pooledSurfaces[oldest] != 0)
                {
                    g_pooledSurfaces[oldest]->Release();
                    g_pooledBytesTotal -= g_pooledSurfaceBytes[oldest];
                }
                g_pooledSurfaces[oldest] = 0;
                g_pooledSurfaceReserved[oldest] = 0;
                g_pooledSurfaceUsed[oldest] = 1;
                evictPending = 0;
                continue;
            }
        }
        if (g_d3dDevice->CreateTexture(width, height, 1, 0, format, D3DPOOL_DEFAULT, &g_pooledSurfaces[poolIndex], 0) == 0)
        {
            texture->AddDirtyRect(0);
            if (g_d3dDevice->UpdateTexture(texture, g_pooledSurfaces[poolIndex]) != 0)
            {
                g_pooledSurfaces[poolIndex]->Release();
                g_pooledSurfaces[poolIndex] = 0;
                return 0;
            }
            g_pooledSurfaceUsed[poolIndex] = 1;
            for (int j = 0; j < SURFACE_POOL_SIZE; j++)
                g_pooledSurfaceReserved[j]++;
            g_pooledBytesTotal += bytes;
            g_pooledSurfaceReserved[poolIndex] = 0;
            g_pooledSurfaceBytes[poolIndex] = bytes;
            return g_pooledSurfaces[poolIndex];
        }
        if (++attempts > POOL_MAX_RETRIES)
        {
            g_pooledSurfaces[poolIndex] = 0;
            return 0;
        }
        evictPending = 1;
    }
}
