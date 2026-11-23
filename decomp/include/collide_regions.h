// MarkerSet, MarkerOwner: declared here once (first used by src/collide_regions.cpp).
#ifndef GURUMIN_COLLIDE_REGIONS_H
#define GURUMIN_COLLIDE_REGIONS_H

#include "node.h"

// A set of marker points of a model (reached through the model at DAT_104c03ac, field 0x2a8).
struct MarkerSet
{
    char pad_0000[8];
    Vec4 *points;             // 0x08
    char *flags;              // 0x0c: one byte per point
    int count;                // 0x10
};
struct MarkerOwner
{
    char pad_0000[0x2a8];
    MarkerSet *markers;       // 0x2a8
};

#endif
