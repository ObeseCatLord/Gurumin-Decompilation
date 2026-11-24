// EN out-of-line float atan2 wrapper (JP twin: attach_slots.cpp; the JP map called it acosf). The compiler's
// atan2f(float y, float x) inline calls the double atan2; this copy was emitted out of line in the attach-slots object.

#include <math.h>

// Heading of the vector (x, y) as a float. Called by the weapon attach pitch/heading code.
extern "C" float Attach_Atan2(float y, float x)
{
    return atan2f(y, x);
}
