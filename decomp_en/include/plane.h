// Plane of the EN build: a x + b y + c z + d = 0 (JP: decomp/include/plane.h).
#ifndef GURUMIN_EN_PLANE_H
#define GURUMIN_EN_PLANE_H

#include "vec4.h"

// Plane a*x + b*y + c*z + d = 0.
struct Plane
{
    float a;   // x coefficient of the normal
    float b;   // y coefficient of the normal
    float c;   // z coefficient of the normal
    float d;   // offset

    // Intersects the line origin + t * direction with the plane and stores the point in `out`. Returns 0 (leaving
    // `out` alone) when the line is parallel to the plane, else 1. EN 0x7775a0 (JP 0x101ad2c0).
    int IntersectRay(Vec4 *out, const Vec4 *origin, const Vec4 *direction);

    // Unsigned distance of `point` from the plane (the normal is divided by its length). EN 0x7730a0
    // (plane_distance.cpp, EN-only).
    float Distance(const Vec4 *point);
};

// Strength 1..0 for a displacement `v`: 1 within 100 units, then linear to 0 over 500 more. EN 0x775670
// (plane_distance.cpp, EN-only).
float Vec3_FalloffStrength(const Vec4 *v);

// The normal of the triangle (origin, a, b): the cross product of the edges a - origin and b - origin, not
// normalised. EN 0x773e60.
Vec4 TriangleNormal(const Vec4 &a, const Vec4 &origin, const Vec4 &b);

// The plane through the triangle (origin, a, b): the normal is (a - origin) x (b - origin), not normalised, and d
// puts `origin` on the plane. EN 0x7720a0.
Plane PlaneFromTriangle(const Vec4 &a, const Vec4 &origin, const Vec4 &b);

#endif
