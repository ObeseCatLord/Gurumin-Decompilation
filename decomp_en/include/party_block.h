// Party members blocking each other (EN build; JP: decomp/src/party_block.cpp).
#ifndef GURUMIN_EN_PARTY_BLOCK_H
#define GURUMIN_EN_PARTY_BLOCK_H

#include "vec4.h"

// 1 when another party member would be in the way of character `self` after it moves by `step` within `range`. EN 0x5c0ca0 (JP 0x10034cc0).
extern "C" int Party_BlocksMove(int self, const Vec4 &step, int range);

#endif
