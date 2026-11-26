// Clearing palette buckets of the image converter (EN build, EN-only: no JP twin found).
#include "palette_table.h"

// Zeroes the six counters of bucket `index`, keeping its colour channels. No direct caller found (reached through a
// function pointer).
extern "C" void Palette_ClearBucket(int index)
{
    g_paletteBuckets[index].sumChannel2 = 0;
    g_paletteBuckets[index].sumChannel1 = 0;
    g_paletteBuckets[index].sumChannel0 = 0;
    g_paletteBuckets[index].sumChannel3 = 0;
    g_paletteBuckets[index].pixelCount = 0;
    g_paletteBuckets[index].sumDistance = 0;
}

// Second copy of Palette_ClearBucket with the same body (the original has both). No direct caller found.
extern "C" void Palette_ClearBucketTwin(int index)
{
    g_paletteBuckets[index].sumChannel2 = 0;
    g_paletteBuckets[index].sumChannel1 = 0;
    g_paletteBuckets[index].sumChannel0 = 0;
    g_paletteBuckets[index].sumChannel3 = 0;
    g_paletteBuckets[index].pixelCount = 0;
    g_paletteBuckets[index].sumDistance = 0;
}
