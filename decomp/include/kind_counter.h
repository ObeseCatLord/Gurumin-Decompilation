// KindCounter: declared here once (first used by src/app_0012.cpp).
#ifndef GURUMIN_KIND_COUNTER_H
#define GURUMIN_KIND_COUNTER_H

// One int every 0x40 bytes (laid over the CharState table at 0x1020b430, see world.h); used for the per-kind
// cooldown counters below.
struct KindCounter
{
    int value;
    char pad_0004[0x3c];
};

#endif
