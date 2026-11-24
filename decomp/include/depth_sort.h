// Sort helper for draw lists (src/model_misc.cpp).
#ifndef GURUMIN_DEPTH_SORT_H
#define GURUMIN_DEPTH_SORT_H

// 0x10165020: sorts `count` keys ascending (bubble sort) and applies every swap to the parallel arrays a, b (either may
// be 0) and c.
extern "C" int FUN_10165020(float *keys, float *a, float *b, int *c, int count);

#endif
