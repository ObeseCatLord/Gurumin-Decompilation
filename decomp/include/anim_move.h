// Helpers of AnimModel::MoveWithCollisions (anim_move.cpp): the probe rings it tests a move with and the engine
// functions it calls that no other file declares.
#ifndef GURUMIN_ANIM_MOVE_H
#define GURUMIN_ANIM_MOVE_H

#include "vec4.h"
#include "matrix4.h"

extern "C" int DAT_1020342c;   // zero = the world has no collision data: models move freely

// One probe of a ring: the direction (x, 0, z) in the frame of the move, and the factor it is scaled by after the
// horizontal speed.
struct RingProbe
{
    float x;
    float z;
    float scale;
};

Matrix4 UnscaledMatrix(const Matrix4 *matrix);                         // 0x101b17a0: the matrix with its scale divided out
Vec4 RotateAboutAxis(const Vec4 *axis, float angle, const Vec4 *point);   // 0x101b16f0: `point` turned by `angle` about `axis`

#endif
