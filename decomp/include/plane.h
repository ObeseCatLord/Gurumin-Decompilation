// Plane: declared here once (methods in src/vec_math.cpp).
#ifndef GURUMIN_PLANE_H
#define GURUMIN_PLANE_H

#include "vec4.h"

// Plane a*x + b*y + c*z + d = 0, stored in a Vec4 (x = a, y = b, z = c, w = d).
struct Plane
{
    float a;
    float b;
    float c;
    float d;
    int IntersectRay(Vec4 *out, const Vec4 *origin, const Vec4 *direction);
    float Distance(const Vec4 *point);
};

#endif
