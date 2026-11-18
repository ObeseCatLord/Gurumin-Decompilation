// Intersection of two circles in the plane (used by the scene node code at 0x10194f40). Circles are (x, y, radius)
// triples of doubles.

#include <math.h>

// 0x10194a10 (Circle_Intersect). Intersects circle `a` with circle `b`: returns 2 with both points in out[0..3]
// (x1, y1, x2, y2), 1 with the single touching point in out[0..1], or -1 when the circles do not meet (or have the
// same centre). `unused` is not read.
extern "C" int __stdcall FUN_10194a10(const double *a, const double *b, int unused, double *out)
{
    double ra2 = pow(a[2], 2.0);
    double rb2 = pow(b[2], 2.0);
    double dx = b[0] - a[0];
    double dy = b[1] - a[1];
    double d2 = pow(dx, 2.0) + pow(dy, 2.0);
    if (d2 < 1e-15)
        return -1;
    double difference = rb2 - ra2;
    double k = (rb2 + ra2) * d2;
    double disc = k + k - pow(d2, 2.0) - pow(difference, 2.0);
    if (disc < -1e-15)
        return -1;
    double t = 0.5 - 0.5 / d2 * difference;
    double px = t * dx + a[0];
    double py = t * dy + a[1];
    if (disc < 1e-15)
    {
        out[0] = px;
        out[1] = py;
        return 1;
    }
    double h = sqrt(disc) * (0.5 / d2);
    double ox = h * dx;
    double oy = h * dy;
    out[0] = px - oy;
    out[1] = ox + py;
    out[2] = oy + px;
    out[3] = py - ox;
    return 2;
}
