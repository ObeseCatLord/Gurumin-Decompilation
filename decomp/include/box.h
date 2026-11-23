// Box: declared here once (first used by src/box_overlap.cpp).
#ifndef GURUMIN_BOX_H
#define GURUMIN_BOX_H

#include "vec4.h"

struct Box
{
    Vec4 min;
    Vec4 max;
};

#endif
