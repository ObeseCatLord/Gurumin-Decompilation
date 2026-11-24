// GroundObject, GroundOrigin: declared here once (first used by src/ground_height.cpp).
#ifndef GURUMIN_GROUND_HEIGHT_H
#define GURUMIN_GROUND_HEIGHT_H

#include "matrix4.h"

// A ground object with a height map (only the fields used here).
struct GroundObject
{
    char pad_0000[0x1cc];
    Matrix4 matrix;                                  // 0x1cc: world matrix
};
// The same object seen from its height map origin.
struct GroundOrigin
{
    char pad_0000[0x1fc];
    float x;                                         // 0x1fc: corner of the height map
    float y;                                         // 0x200
    float base;                                      // 0x204: height added to the samples
};

// Number of ground object slots.
#define GROUND_OBJECT_SLOTS 0x60

extern "C" GroundOrigin *DAT_1020d860[GROUND_OBJECT_SLOTS];   // the ground objects (0 = free); cleared by game_reset

extern "C" void FUN_1003bc60(int x, int y, int mode, int radius);   // Ground_Dig: digs the ground around the world point (x, y)

#endif
