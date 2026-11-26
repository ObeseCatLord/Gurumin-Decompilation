// Sampling the per-world key banks at the world frame (EN build, EN-only: no JP twin found).
#include "world_renderer.h"

// Value of the first key bank of world slot `index` at the world frame goes to `*first`, that of the second key bank to
// `*second`. Returns 0. No direct caller.
int World_SampleFrameKeys(int index, Vec4 *first, Vec4 *second)
{
    Vec4 value;
    g_frameKeysA[index].Sample(g_worldFrame[index], &value.x, &value.y, &value.z, &value.w, KEY_SAMPLE_POSITION);
    first->Copy(value);
    g_frameKeysB[index].Sample(g_worldFrame[index], &value.x, &value.y, &value.z, &value.w, KEY_SAMPLE_POSITION);
    second->Copy(value);
    return 0;
}
