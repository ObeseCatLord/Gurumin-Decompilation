// SwitchObject: declared here once (first used by src/switch_effects.cpp).
#ifndef GURUMIN_SWITCH_OBJECT_H
#define GURUMIN_SWITCH_OBJECT_H

#include "matrix4.h"

// A map object that can be held as a switch.
struct SwitchObject
{
    char pad_0000[0xac];
    unsigned char flags;          // 0xac: 0x20 = no effect, the holder is released instead
    char pad_00ad[0x19f];
    Matrix4 matrix;               // 0x24c: world matrix
};

#endif
