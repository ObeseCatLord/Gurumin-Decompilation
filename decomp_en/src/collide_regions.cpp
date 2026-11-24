// Collision against the world's region nodes (EN build). JP twin: collide_regions.cpp.
#include "collide_regions.h"
#include "key_bank.h"
#include "node_data.h"

// Moves `position` by `velocity` against the visible nodes of `root` whose x/y bounds contain the position (strictly
// inside). Stops at the first node that reports a collision (Node::Collide) and returns 1; 0 if none. Called by the
// marker test and by the movement code.
extern "C" int Regions_Collide(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit)
{
    int i = 0;
    for (Node *node = root->FindByFlags(NODE_MASK_REGION, 0, 0); node != 0; node = root->FindByFlags(NODE_MASK_REGION, ++i, 0))
    {
        if (node->box[NODE_BOX_MIN].x < position->x && node->box[NODE_BOX_MAX].x > position->x &&
            node->box[NODE_BOX_MIN].y < position->y && node->box[NODE_BOX_MAX].y > position->y &&
            node->Collide(tag, mask, position, velocity, hit) != 0)
            return 1;
    }
    return 0;
}

// For every marker point: sets its key flag to 4 and casts a ray from 10000 units above it to 10000 units below it
// against the world's "testatari" ("test hit") regions; the point's byte in g_markerFree becomes 1 when nothing was
// hit (or there is no "testatari" node), else 0. Does nothing while no world is loaded. Returns 0.
extern "C" int Markers_TestRegions(void)
{
    Vec4 from;
    Vec4 to;
    Vec4 hit;
    if (g_worldRoot == 0)
        return 0;
    for (int i = 0; i < g_markerNode->extra->tracks[NODE_TRACK_TRANSLATION].count; i++)
    {
        g_markerNode->extra->tracks[NODE_TRACK_TRANSLATION].flags[i] = KEY_FLAG_MOTION;
        from.Copy(g_markerNode->extra->tracks[NODE_TRACK_TRANSLATION].values[i]);
        to.Copy(from);
        from.z += REGION_RAY_REACH;
        to.z -= REGION_RAY_REACH;
        Node *regions = g_worldRoot->Find("testatari", 0);
        if (regions != 0 && Regions_Collide(regions, 0, NODE_MASK_REGION, &from, &to, &hit) != 0)
            g_markerFree[i] = 0;
        else
            g_markerFree[i] = 1;
    }
    return 0;
}
