// Calls of the character attachment and shot code (char_attach.cpp, char_shots.cpp).
#ifndef GURUMIN_CHAR_ATTACH_H
#define GURUMIN_CHAR_ATTACH_H

#include "vec4.h"

// 0x1001a760 (char_attach.cpp): attach 1 gives character `index`'s left hand the child model `name` (removing a "scop_l"
// that is already there); anything else removes the "scop_l" child. Returns 0.
extern "C" int FUN_1001a760(int index, int attach, const char *name);

// 0x1001a680 (Char_FireShot, char_shots.cpp): starts a shot of character `owner` from `position`. Returns 0.
extern "C" int FUN_1001a680(int owner, const Vec4 *position, int spread);

#endif
