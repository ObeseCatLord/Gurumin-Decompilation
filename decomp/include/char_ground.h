// Putting characters on the ground (char_ground.cpp).
#ifndef GURUMIN_CHAR_GROUND_H
#define GURUMIN_CHAR_GROUND_H

// 0x1001c090 (Char_PlaceOnGround): casts down for character `index` (when DAT_102022b8 or `force` is set) and moves
// its shadow matrix to the hit point. Returns 0.
extern "C" int FUN_1001c090(int index, int force);

#endif
