// Surface pool reset (EN build, EN-only: no JP twin found).
#include "buffer_pool.h"

// SurfacePool_Reset: forgets every pooled surface without releasing it (the device was just created, so the pool starts
// empty): zeroes the byte total and head value, then the in-use flags, the surface pointers and the reservation
// counters of all SURFACE_POOL_SIZE entries. Called by Render_CreateDevice (EN 0x7a13e0).
extern "C" void SurfacePool_Reset(void)
{
    g_surfacePoolHead = 0;
    g_pooledBytesTotal = 0;
    for (int entry = 0; entry < SURFACE_POOL_SIZE; entry++)
        g_pooledSurfaceUsed[entry] = 0;
    for (int entry = 0; entry < SURFACE_POOL_SIZE; entry++)
        g_pooledSurfaces[entry] = 0;
    for (int entry = 0; entry < SURFACE_POOL_SIZE; entry++)
        g_pooledSurfaceReserved[entry] = 0;
}
