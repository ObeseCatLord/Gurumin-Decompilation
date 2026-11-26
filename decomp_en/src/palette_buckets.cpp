// Loading the palette buckets and gathering pixels into them (EN build, EN-only: no JP twin found).
#include "palette_table.h"

// Value g_paletteLastMatch gets when a palette is loaded.
const int PALETTE_NO_MATCH = -1;

// Loads the buckets from the PALETTE_SIZE entries of `palette`: the first three channels are copied, the fourth is 0, the
// counters are cleared and g_paletteSquares gets the squares 0..255. Called by the image converter at EN 0x748e40.
extern "C" void Palette_LoadOpaque(const PaletteColour *palette)
{
    g_paletteLastMatch = PALETTE_NO_MATCH;
    g_paletteBucketCount = PALETTE_SIZE;
    for (int i = 0; i < PALETTE_SIZE; i++)
    {
        g_paletteBuckets[i].channel[0] = palette[i].channel[0];
        g_paletteBuckets[i].channel[1] = palette[i].channel[1];
        g_paletteBuckets[i].channel[2] = palette[i].channel[2];
        g_paletteBuckets[i].channel[3] = 0;
        g_paletteBuckets[i].sumChannel2 = 0;
        g_paletteBuckets[i].sumChannel1 = 0;
        g_paletteBuckets[i].sumChannel0 = 0;
        g_paletteBuckets[i].sumChannel3 = 0;
        g_paletteBuckets[i].pixelCount = 0;
        g_paletteBuckets[i].sumDistance = 0;
        g_paletteSquares[i] = i * i;
    }
}

// Same as Palette_LoadOpaque, but the fourth channel of each bucket comes from the palette too. Called by the image
// converter at EN 0x748fa0.
extern "C" void Palette_LoadWithAlpha(const PaletteColour *palette)
{
    g_paletteLastMatch = PALETTE_NO_MATCH;
    g_paletteBucketCount = PALETTE_SIZE;
    for (int i = 0; i < PALETTE_SIZE; i++)
    {
        g_paletteBuckets[i].channel[0] = palette[i].channel[0];
        g_paletteBuckets[i].channel[1] = palette[i].channel[1];
        g_paletteBuckets[i].channel[2] = palette[i].channel[2];
        g_paletteBuckets[i].channel[3] = palette[i].channel[3];
        g_paletteBuckets[i].sumChannel2 = 0;
        g_paletteBuckets[i].sumChannel1 = 0;
        g_paletteBuckets[i].sumChannel0 = 0;
        g_paletteBuckets[i].sumChannel3 = 0;
        g_paletteBuckets[i].pixelCount = 0;
        g_paletteBuckets[i].sumDistance = 0;
        g_paletteSquares[i] = i * i;
    }
}

// Finds the bucket nearest to each of the `count` pixels and adds the pixel's channels, one pixel and the match distance
// to that bucket's counters. The bucket's channels run in the opposite order to the pixel's first three. Called by the
// image converter at EN 0x748fa0.
extern "C" void Palette_Accumulate(const PaletteColour *pixels, int count)
{
    for (; count > 0; count--, pixels++)
    {
        int first = pixels->channel[0];
        int second = pixels->channel[1];
        int third = pixels->channel[2];
        int fourth = pixels->channel[3];
        int distance;
        int index = Palette_FindNearest(third, second, first, fourth, &distance);
        g_paletteBuckets[index].sumChannel2 += first;
        g_paletteBuckets[index].sumChannel0 += third;
        g_paletteBuckets[index].sumChannel1 += second;
        g_paletteBuckets[index].sumChannel3 += fourth;
        g_paletteBuckets[index].pixelCount++;
        g_paletteBuckets[index].sumDistance += distance;
    }
}
