// Per-frame ageing of the pooled surfaces (EN build, EN-only: no JP twin found).
#include "frame_pacer.h"
#include "render.h"

// TextureCache_Age: adds one frame to the age counter of every entry of the pooled surface table (the array declared as
// g_pooledSurfaceReserved: its entries are counted up once per frame, not flags). Returns 0. Called once per drawn
// frame by Frame_Pace (0x7a8460).
extern "C" int TextureCache_Age(void)
{
    for (int i = 0; i < SURFACE_POOL_SIZE; i++)
        g_pooledSurfaceReserved[i]++;
    return 0;
}
