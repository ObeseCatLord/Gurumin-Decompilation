// SlotMotion: declared here once (first used by src/char_shots.cpp).
#ifndef GURUMIN_SLOT_MOTION_H
#define GURUMIN_SLOT_MOTION_H

// Per-slot flight direction and speed.
struct SlotMotion
{
    float angle;              // rotation about the vertical axis, copied from the owner (see FUN_1001a590)
    float heading;            // radians
    float speed;
    int pad_000c;
};

#endif
