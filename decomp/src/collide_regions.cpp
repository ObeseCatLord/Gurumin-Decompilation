// Collision against the flagged regions of a model: every node with flag 0x4 (visible) whose x/y bounds contain
// the position is tested in turn.

#include "vec4.h"

#include "node.h"

// Moves `position` by `velocity` against the visible nodes of `root` whose x/y bounds contain the position
// (strictly inside). Stops at the first node that reports a collision (Node::Collide) and returns 1; 0 if none.
extern "C" int FUN_1001fff0(Node *root, int a, int b, Vec4 *position, Vec4 *velocity, Vec4 *hit)
{
    int i = 0;
    for (Node *node = root->FindByFlags(4, 0, 0); node != 0; node = root->FindByFlags(4, ++i, 0))
    {
        if (node->boundsMin.x < position->x && node->boundsMax.x > position->x && node->boundsMin.y < position->y &&
            node->boundsMax.y > position->y && node->Collide(a, b, position, velocity, hit) != 0)
            return 1;
    }
    return 0;
}

// A set of marker points of a model (reached through the model at DAT_104c03ac, field 0x2a8).
struct MarkerSet
{
    char pad_0000[8];
    Vec4 *points;             // 0x08
    char *flags;              // 0x0c: one byte per point
    int count;                // 0x10
};

struct MarkerOwner
{
    char pad_0000[0x2a8];
    MarkerSet *markers;       // 0x2a8
};

extern "C" Node *DAT_10255f08;          // the world's root node
extern "C" MarkerOwner *DAT_104c03ac;
extern "C" char *DAT_104c0394;          // one byte per marker: 1 = no ground below it

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
    for (int i = 0; i < DAT_104c03ac->markers->count; i++)
    {
        DAT_104c03ac->markers->flags[i] = 4;
        from.Copy(DAT_104c03ac->markers->points[i]);
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
