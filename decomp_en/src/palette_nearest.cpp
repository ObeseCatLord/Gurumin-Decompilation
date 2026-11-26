// Nearest palette bucket searches of the image converter (EN build, EN-only: no JP twin found).
#include <math.h>
#include <stdlib.h>
#include "palette_table.h"

// Distance value no bucket can reach, the weight of the fourth channel in the four-channel distance and the shift
// applied to the best squared distance before its square root is taken.
const unsigned int PALETTE_DISTANCE_NONE = 0xffffffff;
const int PALETTE_FOURTH_CHANNEL_WEIGHT = 3;
const int PALETTE_DISTANCE_SHIFT = 4;

// Palette_FindNearest3: index of the bucket whose first three channels are nearest to channel0, channel1 and channel2
// (sum of the squared differences); `unused` is not read. The square root of sixteen times the best squared distance
// goes to `*distance` when given (converted as an unsigned value, as the original does). No direct caller.
extern "C" int Palette_FindNearest3(int channel0, int channel1, int channel2, int unused, int *distance)
{
    unsigned int best = PALETTE_DISTANCE_NONE;
    int nearest = 0;
    for (int i = 0; i < g_paletteBucketCount; i++)
    {
        unsigned int squared = g_paletteSquares[abs(g_paletteBuckets[i].channel[0] - channel0)] +
                               g_paletteSquares[abs(g_paletteBuckets[i].channel[2] - channel2)] +
                               g_paletteSquares[abs(g_paletteBuckets[i].channel[1] - channel1)];
        if (squared < best)
        {
            best = squared;
            nearest = i;
        }
    }
    if (distance != 0)
        *distance = (unsigned int)sqrt((double)(best << PALETTE_DISTANCE_SHIFT));
    return nearest;
}

// Palette_FindNearest: index of the bucket nearest to the four channel values (sum of the squared differences, the
// fourth channel counting three times); the square root of sixteen times the best squared distance goes to `*distance`
// when given. Called by Palette_Accumulate.
extern "C" int Palette_FindNearest(int channel0, int channel1, int channel2, int channel3, int *distance)
{
    unsigned int best = PALETTE_DISTANCE_NONE;
    int nearest = 0;
    for (int i = 0; i < g_paletteBucketCount; i++)
    {
        unsigned int squared = g_paletteSquares[abs(g_paletteBuckets[i].channel[3] - channel3)] * PALETTE_FOURTH_CHANNEL_WEIGHT +
                               g_paletteSquares[abs(g_paletteBuckets[i].channel[1] - channel1)] +
                               g_paletteSquares[abs(g_paletteBuckets[i].channel[2] - channel2)] +
                               g_paletteSquares[abs(g_paletteBuckets[i].channel[0] - channel0)];
        if (squared < best)
        {
            best = squared;
            nearest = i;
        }
    }
    if (distance != 0)
        *distance = (unsigned int)sqrt((double)(best << PALETTE_DISTANCE_SHIFT));
    return nearest;
}
