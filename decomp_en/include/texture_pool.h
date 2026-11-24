// Helper shared by the texture write functions of the EN build: drops the buffer pool copy of a texture's frames.
// The helper is inline in the original (JP: FUN_101484b0 in decomp/src/buffer_pool.cpp).
#ifndef GURUMIN_EN_TEXTURE_POOL_H
#define GURUMIN_EN_TEXTURE_POOL_H

#include "buffer_pool.h"

// Releases the surface of pool entry `index` but keeps the entry reserved, so the next bind rebuilds it from the
// texture. Entries outside the pool or not in use are left alone.
inline void TexturePool_DropSurface(int index)
{
    if (index >= 0 && index < SURFACE_POOL_SIZE && g_pooledSurfaceUsed[index] != 0)
    {
        if (g_pooledSurfaces[index] != 0)
        {
            g_pooledSurfaces[index]->Release();
            g_pooledBytesTotal -= g_pooledSurfaceBytes[index];
        }
        g_pooledSurfaces[index] = 0;
        g_pooledSurfaceUsed[index] = 1;
        g_pooledSurfaceReserved[index] = 0;
    }
}

#endif
