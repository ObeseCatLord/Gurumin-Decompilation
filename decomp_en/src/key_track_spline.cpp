// Spline interpolation of a growable key track (EN build). The code is byte-identical to KeyInterpolator::Spline
// (key_spline.cpp, EN 0x771b60) and has no direct caller: the exe holds one copy per key container class.
#include "containers.h"

// Coefficients of the cubic Hermite basis.
const float TRACK_SPLINE_HALF = 0.5f;
const float TRACK_SPLINE_TWO = 2.0f;
const float TRACK_SPLINE_THREE = 3.0f;

// Cubic Hermite interpolation between `b` (t = 0) and `c` (t = 1) with neighbours `a` and `d`; `this` is not read.
float KeyTrack::Spline(float t, float a, float b, float c, float d)
{
    float bc = c - b;
    float tangentB = (b - a) * TRACK_SPLINE_HALF + bc;
    float tangentC = (d - c) + bc * TRACK_SPLINE_HALF;
    float t2 = t * t;
    float t3 = t2 * t;
    float t2x3 = t2 * TRACK_SPLINE_THREE;
    return (t3 - t2 * TRACK_SPLINE_TWO + t) * tangentB + (t3 * TRACK_SPLINE_TWO - t2x3 + 1.0f) * b + (t3 * -TRACK_SPLINE_TWO + t2x3) * c +
           (t3 - t2) * tangentC;
}
