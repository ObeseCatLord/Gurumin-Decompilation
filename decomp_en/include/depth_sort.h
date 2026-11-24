// Sort helper for draw lists (EN build).
#ifndef GURUMIN_EN_DEPTH_SORT_H
#define GURUMIN_EN_DEPTH_SORT_H

// Sorts `count` keys ascending (bubble sort) and applies every swap to the parallel arrays a and b (either may be 0)
// and c. Returns 1. EN 0x71cf80 (JP 0x10165020).
extern "C" int SortByDepth(float *keys, float *a, float *b, int *c, int count);

#endif
