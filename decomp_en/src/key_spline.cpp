// Spline interpolation of key containers (EN build). JP twin: decomp/src/vec4.cpp (KeyInterpolator::Spline).
#include "key_bank.h"

// Coefficients of the cubic Hermite basis.
const float SPLINE_HALF = 0.5f;
const float SPLINE_TWO = 2.0f;
const float SPLINE_THREE = 3.0f;

// Cubic Hermite interpolation between `b` (t = 0) and `c` (t = 1) with neighbours `a` and `d`. The tangents are
// (b - a) / 2 + (c - b) at b and (c - b) / 2 + (d - c) at c. Called by KeyBank::Sample and the track sampler.
float KeyInterpolator::Spline(float t, float a, float b, float c, float d)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return ((t3 - t2 * SPLINE_TWO) + t) * ((b - a) * SPLINE_HALF + (c - b)) +
           ((t3 * SPLINE_TWO - t2 * SPLINE_THREE) + 1.0f) * b +
           ((d - c) + (c - b) * SPLINE_HALF) * (t3 - t2) + (t3 * -SPLINE_TWO + t2 * SPLINE_THREE) * c;
}
