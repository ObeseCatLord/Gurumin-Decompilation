// The intersection of two circles in the plane (circle_intersect.cpp).
#ifndef GURUMIN_CIRCLE_INTERSECT_H
#define GURUMIN_CIRCLE_INTERSECT_H

// 0x10194a10: intersects circle `a` with circle `b` (each x, y, radius as doubles): returns 2 with both points in
// out[0..3] (x1, y1, x2, y2), 1 with the touching point in out[0..1], -1 when they do not meet.
extern "C" int __stdcall FUN_10194a10(const double *a, const double *b, int unused, double *out);

#endif
