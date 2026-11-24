// Sort helper for draw lists (EN build). JP twin: decomp/src/model_misc.cpp.
#include "depth_sort.h"

// SortByDepth: sorts `count` keys into ascending order (bubble sort) and applies every swap to the parallel arrays
// `a` and `b` (either may be 0) and `c`. Used to sort draw lists by depth. Returns 1.
extern "C" int SortByDepth(float *keys, float *a, float *b, int *c, int count)
{
    int swapped;
    do
    {
        swapped = 0;
        for (int i = 0; i < count - 1; i++)
        {
            if (keys[i + 1] < keys[i])
            {
                float key = keys[i];
                keys[i] = keys[i + 1];
                keys[i + 1] = key;
                if (a != 0)
                {
                    float t = a[i];
                    a[i] = a[i + 1];
                    a[i + 1] = t;
                }
                if (b != 0)
                {
                    float t = b[i];
                    b[i] = b[i + 1];
                    b[i + 1] = t;
                }
                int t = c[i];
                c[i] = c[i + 1];
                c[i + 1] = t;
                swapped = 1;
            }
        }
    } while (swapped);
    return 1;
}
