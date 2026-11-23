#ifndef GURUMIN_START_VECTORS_H
#define GURUMIN_START_VECTORS_H

#include "vec4.h"

// Per-character start vectors, 0x40 bytes (char_shots.cpp, char_leash.cpp).
struct StartVectors
{
    Vec4 position;            // start position
    Vec4 m_0010;
    char pad_0020[0x20];
};

#endif
