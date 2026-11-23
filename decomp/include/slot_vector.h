// SlotVector: declared here once (first used by src/collide_query.cpp).
#ifndef GURUMIN_SLOT_VECTOR_H
#define GURUMIN_SLOT_VECTOR_H

#include "anim_model.h"

// Per-slot direction record, 0x40 bytes (the first vector of DAT_10802bf8; see char_shots.cpp).
struct SlotVector
{
    Vec4 vector;
    char pad_0010[0x30];
};

#endif
