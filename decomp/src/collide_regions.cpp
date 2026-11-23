// Collision against the flagged regions of a model: every node with flag 0x4 (visible) whose x/y bounds contain
// the position is tested in turn.

#include "vec4.h"

#include "node.h"

// Moves `position` by `velocity` against the visible nodes of `root` whose x/y bounds contain the position
// (strictly inside). Stops at the first node that reports a collision (Node::Collide) and returns 1; 0 if none.
extern "C" int FUN_1001fff0(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit)
{
    int i = 0;
    for (Node *node = root->FindByFlags(4, 0, 0); node != 0; node = root->FindByFlags(4, ++i, 0))
    {
        if (node->box[NODE_BOX_MIN].x < position->x && node->box[NODE_BOX_MAX].x > position->x && node->box[NODE_BOX_MIN].y < position->y &&
            node->box[NODE_BOX_MAX].y > position->y && node->Collide(tag, mask, position, velocity, hit) != 0)
            return 1;
    }
    return 0;
}

#define RAY_LENGTH 1700.0f      // the probe's reach along y (back) and z (up)
#define RAY_STRETCH 10.0f       // how far the probe is stretched before it is turned into the far end of the ray
#define REGION_MARGIN 150.0f    // slack around a node's x/y box in the overlap test
#define RAY_DROP 350.0f         // how far the ray's start and end are lowered in y

// 0x10020090 (Collide_RayThroughRegions). Casts a ray from `position` along the probe direction (0, -1700, 1700)
// against the visible nodes (flag 0x4) of `root`. A node is tested (Node::Collide with `tag`, `mask`, `hit`) when the
// x/y box of the ray (both ends of the probe scaled to the node's z extent, lowered by 350) overlaps the node's x/y
// box widened by 150 on every side. Returns 1 at the first node that reports a collision, else 0. `velocity` is unused.
extern "C" int FUN_10020090(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit)
{
    int i = 0;
    Vec4 lowCorner;
    Vec4 highCorner;
    Vec4 nodeMin;
    Vec4 nodeMax;
    Vec4 ray;
    Vec4 nearPoint;
    Vec4 farPoint;
    for (Node *node = root->FindByFlags(4, 0, 0); node != 0; node = root->FindByFlags(4, ++i, 0))
    {
        ray.x = 0.0f;
        ray.y = -RAY_LENGTH;
        ray.z = RAY_LENGTH;
        float zLow = node->box[NODE_BOX_MIN].z > node->box[NODE_BOX_MAX].z ? node->box[NODE_BOX_MAX].z : node->box[NODE_BOX_MIN].z;
        float depthLow = position->z - zLow;
        float zHigh = node->box[NODE_BOX_MIN].z < node->box[NODE_BOX_MAX].z ? node->box[NODE_BOX_MAX].z : node->box[NODE_BOX_MIN].z;
        float depthHigh = position->z - zHigh;
        nearPoint.Copy(ray.Divided(ray.z) * depthLow);
        farPoint.Copy(ray.Divided(ray.z) * depthHigh);
        ray.x = nearPoint.x > farPoint.x ? farPoint.x : nearPoint.x;
        ray.y = nearPoint.y > farPoint.y ? farPoint.y : nearPoint.y;
        if (!(nearPoint.x < farPoint.x))
            farPoint.x = nearPoint.x;
        if (!(nearPoint.y < farPoint.y))
            farPoint.y = nearPoint.y;
        nearPoint.Copy(ray);
        lowCorner.Copy(*position - nearPoint);
        highCorner.Copy(*position - farPoint);
        nodeMin.x = node->box[NODE_BOX_MIN].x > node->box[NODE_BOX_MAX].x ? node->box[NODE_BOX_MAX].x : node->box[NODE_BOX_MIN].x;
        nodeMin.y = node->box[NODE_BOX_MIN].y > node->box[NODE_BOX_MAX].y ? node->box[NODE_BOX_MAX].y : node->box[NODE_BOX_MIN].y;
        nodeMax.x = node->box[NODE_BOX_MIN].x < node->box[NODE_BOX_MAX].x ? node->box[NODE_BOX_MAX].x : node->box[NODE_BOX_MIN].x;
        nodeMax.y = node->box[NODE_BOX_MIN].y < node->box[NODE_BOX_MAX].y ? node->box[NODE_BOX_MAX].y : node->box[NODE_BOX_MIN].y;
        ray.x = lowCorner.x > highCorner.x ? highCorner.x : lowCorner.x;
        ray.y = lowCorner.y > highCorner.y ? highCorner.y : lowCorner.y;
        if (!(lowCorner.x < highCorner.x))
            highCorner.x = lowCorner.x;
        if (!(lowCorner.y < highCorner.y))
            highCorner.y = lowCorner.y;
        lowCorner.Copy(ray);
        lowCorner.y -= RAY_DROP;
        highCorner.y -= RAY_DROP;
        if (lowCorner.x - REGION_MARGIN < nodeMax.x + REGION_MARGIN && lowCorner.y - REGION_MARGIN < nodeMax.y + REGION_MARGIN &&
            highCorner.x + REGION_MARGIN > nodeMin.x - REGION_MARGIN && highCorner.y + REGION_MARGIN > nodeMin.y - REGION_MARGIN)
        {
            ray.x = 0.0f;
            ray.y = -RAY_LENGTH;
            ray.z = RAY_LENGTH;
            float stretch = RAY_STRETCH;
            ray.Copy(ray * stretch);
            ray.Copy(*position - ray);
            lowCorner.Copy(*position);
            lowCorner.y -= RAY_DROP;
            ray.y -= RAY_DROP;
            if (node->Collide(tag, mask, &lowCorner, &ray, hit) != 0)
                return 1;
        }
    }
    return 0;
}

#include "node_data.h"


#include "map_vars.h"

// 0x10020f60. For every marker point: sets its flag to 4 and casts a ray from 10000 units above it to 10000 units
// below it against the world's "testatari" ("test hit") regions; the result byte in DAT_104c0394 becomes 1 when
// nothing was hit (or there is no "testatari" node), else 0. Does nothing while no world is loaded. Returns 0.
extern "C" int FUN_10020f60(void)
{
    Vec4 from;
    Vec4 to;
    Vec4 hit;
    if (DAT_10255f08 == 0)
        return 0;
    for (int i = 0; i < DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].count; i++)
    {
        DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].flags[i] = 4;
        from.Copy(DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].values[i]);
        to.Copy(from);
        from.z += 10000.0f;
        to.z -= 10000.0f;
        Node *regions = DAT_10255f08->Find("testatari", 0);
        if (regions != 0 && FUN_1001fff0(regions, 0, 4, &from, &to, &hit) != 0)
            DAT_104c0394[i] = 0;
        else
            DAT_104c0394[i] = 1;
    }
    return 0;
}
