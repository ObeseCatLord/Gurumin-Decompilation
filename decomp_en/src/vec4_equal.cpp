// Vec4 equality test (EN build, EN-only: no JP twin found).
#include "vec4.h"

// Vec4::Equals3: 1 when x, y and z equal those of `other`, otherwise 0; w is not compared. No direct caller.
int Vec4::Equals3(const Vec4 &other)
{
    if (x == other.x && y == other.y && z == other.z)
        return 1;
    return 0;
}
