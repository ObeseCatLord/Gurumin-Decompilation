// SlotPosition: declared here once (first used by src/char_leash.cpp).
#ifndef GURUMIN_SLOT_POSITION_H
#define GURUMIN_SLOT_POSITION_H

#include "anim_model.h"

// Per-slot position record, 0x40 bytes (char_shots.cpp); here the start position of each character.
struct SlotPosition
{
    Vec4 position;
    char pad_0010[0x30];
};

#endif
