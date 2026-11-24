// Per-character movement limits (EN build; JP: decomp/include/move_limit.h). This is the one declaration of
// MoveLimit and its table in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_MOVE_LIMIT_H
#define GURUMIN_EN_MOVE_LIMIT_H

#include "char.h"

// Bytes of a MoveLimit.
const int MOVE_LIMIT_SIZE = 0x100;
// MoveLimit::limitX / limitY value that turns the range check on.
const int MOVE_LIMIT_ENABLED = 1;

// The leash of a character: how far it may stray from its start position (set up by the spawn code).
struct MoveLimit
{
    int rangeX;               // 0x00: allowed distance from the start position along x (0 = unlimited)
    int rangeY;               // 0x04: the same along y
    int limitX;               // 0x08: 1 = the x range applies
    int limitY;               // 0x0c: 1 = the y range applies
    char m_0010[MOVE_LIMIT_SIZE - 0x10];   // 0x10: not used by EN code yet
};

extern "C" MoveLimit g_moveLimits[CHAR_SLOT_COUNT];   // EN 0x00927300 (JP DAT_102110b0): the leash of each character slot

#endif
