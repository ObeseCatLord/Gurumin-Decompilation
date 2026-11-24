#include <math.h>
extern "C" float FUN_006e2150(float x)
{
    return (float)sin(x);
}
extern "C" float DAT_008625d8, DAT_008625dc;
extern "C" int FUN_007cd180(float *value)
{
    int level = (int)(*value * DAT_008625d8 + DAT_008625dc);
    if (level > 1023) return 1023;
    if (level < 0) level = 0;
    return level;
}
