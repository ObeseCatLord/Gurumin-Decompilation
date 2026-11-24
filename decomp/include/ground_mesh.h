// Ground object mesh building (src/ground_mesh.cpp): the height-map mesh of a ground object and its shading.
#ifndef GURUMIN_GROUND_MESH_H
#define GURUMIN_GROUND_MESH_H

#include "vec4.h"

// A rectangle of ints (left, top, right, bottom), like the Windows RECT.
struct IntRect
{
    int left;
    int top;
    int right;
    int bottom;
};

extern "C" float DAT_1031568c[][16][16];   // brightness per height sample of each ground object, 0.3 (dark) to 1.0

// 0x101b1200: unit normal of the triangle (a, b, c): the cross product of (a - b) and (c - b), normalised.
Vec4 Triangle_Normal(const Vec4 &a, const Vec4 &b, const Vec4 &c);

// 0x10016ec0 (ground_mesh.cpp): builds the meshes of the ground objects (one slot, or all when `slot` < 0).
extern "C" void FUN_10016ec0(IntRect *area, int slot, int shadeOnly);

#endif
