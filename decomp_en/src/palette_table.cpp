// Clearing palette buckets of the image converter (EN build, EN-only: no JP twin found).
#include "palette_table.h"

// Zeroes the six counters of bucket `index`, keeping its colour channels. No direct caller found (reached through a
// function pointer).
extern "C" void Palette_ClearBucket(int index)
{
    g_paletteBuckets[index].m_000c = 0;
    g_paletteBuckets[index].m_0008 = 0;
    g_paletteBuckets[index].m_0004 = 0;
    g_paletteBuckets[index].m_0010 = 0;
    g_paletteBuckets[index].m_0014 = 0;
    g_paletteBuckets[index].m_0018 = 0;
}

// Second copy of Palette_ClearBucket with the same body (the original has both). No direct caller found.
extern "C" void Palette_ClearBucketTwin(int index)
{
    g_paletteBuckets[index].m_000c = 0;
    g_paletteBuckets[index].m_0008 = 0;
    g_paletteBuckets[index].m_0004 = 0;
    g_paletteBuckets[index].m_0010 = 0;
    g_paletteBuckets[index].m_0014 = 0;
    g_paletteBuckets[index].m_0018 = 0;
}
