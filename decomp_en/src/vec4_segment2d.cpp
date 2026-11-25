// Distance of a point to a segment in the x/y plane (EN build; JP twin: 0x101acc60).
#include "vec4.h"

// Squared distance in the x/y plane from this point to the segment (x1, y1)-(x2, y2): the point is projected onto the
// line and the projection is clamped to the ends of the segment. A segment of zero length counts as the point
// (x1, y1). Called by the node sphere and cylinder overlap tests. EN 0x7734d0 (JP 0x101acc60).
float Vec4::DistanceSquaredToSegment2D(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2)
        return (x - x1) * (x - x1) + (y - y1) * (y - y1);
    float dx = x2 - x1;
    float dy = y2 - y1;
    float t = ((x * dx - (dx * x1 + dy * y1)) + y * dy) / (dy * dy + dx * dx);
    if (t < 0.0f)
        return (x - x1) * (x - x1) + (y - y1) * (y - y1);
    if (t > 1.0f)
        return (x - x2) * (x - x2) + (y - y2) * (y - y2);
    float nearestY = y - (t * dy + y1);
    float nearestX = x - (t * dx + x1);
    return nearestX * nearestX + nearestY * nearestY;
}
