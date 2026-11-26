// qsort comparators of the palette buckets (EN build, EN-only: no JP twin found).
#include "palette_table.h"

// Orders buckets by their summed match distance, largest first. Passed to qsort by the palette builder at EN 0x7492d0.
extern "C" int PaletteBucket_CompareByDistanceDescending(const PaletteBucket *a, const PaletteBucket *b)
{
    unsigned int distanceA = a->sumDistance;
    unsigned int distanceB = b->sumDistance;
    if (distanceB > distanceA)
        return 1;
    if (distanceB < distanceA)
        return -1;
    return 0;
}

// Identical twin of PaletteBucket_CompareByDistanceDescending, passed to qsort by the second palette builder.
extern "C" int PaletteBucket_CompareByDistanceDescendingTwin(const PaletteBucket *a, const PaletteBucket *b)
{
    unsigned int distanceA = a->sumDistance;
    unsigned int distanceB = b->sumDistance;
    if (distanceB > distanceA)
        return 1;
    if (distanceB < distanceA)
        return -1;
    return 0;
}
