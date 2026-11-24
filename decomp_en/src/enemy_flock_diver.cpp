// EN out-of-line float atan2 wrapper (JP twin: enemy_flock_diver.cpp; the JP map called it cosf). The compiler's
// atan2f(float y, float x) inline calls the double atan2; this copy was emitted out of line in the flock-diver object.

#include <math.h>

// Heading of the vector (x, y) as a float. Called by the flock-diver ring positioning.
extern "C" float Flock_Atan2(float y, float x)
{
    return atan2f(y, x);
}
