// Collision node search near a segment without the closest point (EN build, EN-only: no JP twin found).
#include "node.h"
#include "vec4.h"

// Node::FindNearSegment: as CollectNearSegment, but the closest point of the segment is not reported: finds the
// collision nodes (NODE_FLAG_SHAPE_SPHERE, with collision active) of the subtree that lie within `range` plus
// NODE_SEGMENT_RADIUS_FACTOR of their bounds radius of the segment from-to. Skinned nodes are skipped unless
// g_testSkinnedNodes is set. Each hit is appended to `found` (when given) and counted in *foundCount; without a list the
// search stops at the first hit. Returns 1 when a node was found. No direct caller recovered.
int Node::FindNearSegment(int unused, const Vec4 *from, const Vec4 *to, float range, Node **found, int *foundCount)
{
    if (g_testSkinnedNodes == 0 && skinned != 0)
        return 0;
    if ((flags & NODE_FLAG_SHAPE_SPHERE) == 0)
    {
        for (int i = 0; i < childCount; i++)
        {
            int hit = children[i]->FindNearSegment(unused, from, to, range, found, foundCount);
            if (hit != 0 && found == 0)
                return hit;
        }
        return 0;
    }
    if (hasCollision == 0)
        return 0;
    if (worldMatrix.GetTranslation().DistanceToSegment(*from, *to) < bounds.w * NODE_SEGMENT_RADIUS_FACTOR + range)
    {
        if (found != 0)
        {
            found[*foundCount] = this;
            (*foundCount)++;
        }
        return 1;
    }
    return 0;
}
