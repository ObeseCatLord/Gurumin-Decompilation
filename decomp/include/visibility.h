// VisCell, VisData: declared here once (first used by src/visibility.cpp).
#ifndef GURUMIN_VISIBILITY_H
#define GURUMIN_VISIBILITY_H

#include "vec4.h"

struct VisCell
{
    unsigned char count;
    unsigned char neighbours[0x3f];
};
struct VisData
{
    VisCell cells[0x40];
    Box2 boxes[0x40];
};

#endif
