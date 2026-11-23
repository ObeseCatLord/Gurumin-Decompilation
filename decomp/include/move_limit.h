// MoveLimit: declared here once (first used by src/char_leash.cpp).
#ifndef GURUMIN_MOVE_LIMIT_H
#define GURUMIN_MOVE_LIMIT_H

// Per-character movement limits, 0x100 bytes per slot (set up by the spawn code at 0x1002ae70).
struct MoveLimit
{
    int rangeX;               // allowed distance from the start position along x (0 = unlimited)
    int rangeY;               // the same along y
    int limitX;               // 1 = the x range applies
    int limitY;               // 1 = the y range applies
    char pad_0010[0xf0];
};

#endif
