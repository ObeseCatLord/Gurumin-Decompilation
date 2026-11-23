// Nearest-hit segment cast against the collision meshes of a scene node tree (node.h). Unlike Node::Collide, which
// stops at the first triangle it finds, CastSegment2 tests every node and keeps the hit nearest to the segment's
// end. Matrix4 has the engine's empty destructor, so the skinned path keeps an EH frame for its matrix temporaries.

#include "vec4.h"
#include "matrix4.h"
#include "mesh.h"
#include "node.h"
#include "node_data.h"
#include "char_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "frame_vars.h"
#include "camera_vars.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);  // 0x101ae060: point times matrix, divided by w

// Material alpha of the packs that collide (packs with any other alpha are see-through and skipped).
#define OPAQUE_ALPHA 1.0f
// Lift added to the third corner's z before the test, so a triangle lying exactly in the segment's plane still hits.
#define THIRD_CORNER_LIFT 0.01f
// Vertex indices per triangle in an index list.
#define INDICES_PER_TRIANGLE 3
// Collision region names: "CDL" followed by two decimal digits (the region number, 0..99).
#define REGION_DIGIT_TENS 10
// Node kinds below this are visibility cell numbers (DAT_11afffe8 / the test's cell list have one entry per cell).
#define VISIBILITY_CELL_COUNT 0x40
// Node kind 0xff = none (the name decides); 0xfd = an "OvF" node.
#define NODE_KIND_NONE 0xff
#define NODE_KIND_OVF 0xfd
// Where a node's two-character tag sits in its name: at the start for plain nodes, after "CDLnn_" for collision
// regions and region-cell nodes, after "OvF" and four more characters for "OvF" nodes.
#define TAG_OFFSET_PLAIN 0
#define TAG_OFFSET_REGION 6
#define TAG_OFFSET_OVF 7
// Distance kept when the caller did not ask for the position along the segment.
#define NO_HIT_DISTANCE 999999.0f

// 0x101aa080 (Node::CastSegment2). Casts the segment `from`-`to` against the triangles of this node and all of its
// children (or, on the static-scenery collector, of the nodes it gathered) whose flags contain every bit of `mask` (0
// = any) and whose two-character tag is `name` (0 = any). A node's own triangles are the opaque packs' triangles,
// tested from both sides; a hit counts when it lies closer than the segment's length to both ends. The hit nearest to
// `to` is kept: its point goes to `hit` and its distance from `to` to `t` (when given; the nearest value is shared
// with the recursive calls through it). A switched-off collision region or visibility cell (while "only enabled
// regions", DAT_11b04514, is set) skips its own triangles but still visits its children. Counts the cast in
// DAT_11b044d8. Returns 1 when any triangle was hit.
int Node::CastSegment2(const char *name, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t)
{
    float ownDistance = NO_HIT_DISTANCE;

    DAT_11b044d8 = DAT_11b044d8 + 1;
    if (t == 0)
        t = &ownDistance;
    Vec4 direction;   // from - to: the first test's ray direction
    Vec4 origin;      // copy of `from`: the rays start here
    int found = 0;
    Vec4 center;      // the pack's bounding sphere centre
    Vec4 cornerA;     // the triangle's corners in node space
    Vec4 cornerB;
    Vec4 cornerC;
    Vec4 worldA;      // the corners in world space (the hit point is written into worldA)
    Vec4 worldB;
    Vec4 worldC;
    Vec4 uvA;         // texture coordinates of the corners (x, y)
    Vec4 uvB;
    Vec4 uvC;
    Vec4 reverse;     // to - from: the second test's ray direction (the triangle's other side)
    float hitT;       // IntersectTriangle's outputs (not used here)
    float texU;
    float texV;
    if (visibilityCells != 0)
    {
        UpdateVisibleCells();
        DAT_11b04514 = 1;
    }
    int tagOffset = TAG_OFFSET_PLAIN;
    if (kind != NODE_KIND_NONE)
    {
        if (DAT_11b002c0 != 0)
        {
            if (DAT_11b04514 != 0)
            {
                if (kind < VISIBILITY_CELL_COUNT)
                {
                    char cell = ((char *)DAT_11b002c0)[kind];
                    tagOffset = TAG_OFFSET_REGION;
                    if (cell == 0)
                        goto testChildren;
                }
                if (kind == NODE_KIND_OVF)
                    tagOffset = TAG_OFFSET_OVF;
            }
        }
        else if (DAT_11b04514 != 0)
        {
            if (kind < VISIBILITY_CELL_COUNT)
            {
                char cell = DAT_11afffe8[kind];
                tagOffset = TAG_OFFSET_REGION;
                if (cell == 0)
                    goto testChildren;
            }
            if (kind == NODE_KIND_OVF)
                tagOffset = TAG_OFFSET_OVF;
        }
    }
    else
    {
        if (DAT_11b002c0 != 0)
        {
            if (DAT_11b04514 != 0)
            {
                if (this->name[0] == 'C' && this->name[1] == 'D' && this->name[2] == 'L')
                {
                    tagOffset = TAG_OFFSET_REGION;
                    if (((char *)DAT_11b002c0)[(this->name[3] - '0') * REGION_DIGIT_TENS + (this->name[4] - '0')] == 0)
                        goto testChildren;
                }
                if (this->name[0] == 'O' && this->name[1] == 'v' && this->name[2] == 'F')
                    tagOffset = TAG_OFFSET_OVF;
            }
        }
        else if (DAT_11b04514 != 0)
        {
            if (this->name[0] == 'C' && this->name[1] == 'D' && this->name[2] == 'L')
            {
                tagOffset = TAG_OFFSET_REGION;
                if (DAT_11afffe8[(this->name[3] - '0') * REGION_DIGIT_TENS + (this->name[4] - '0')] == 0)
                    goto testChildren;
            }
            if (this->name[0] == 'O' && this->name[1] == 'v' && this->name[2] == 'F')
                tagOffset = TAG_OFFSET_OVF;
        }
    }

    if ((mask == 0 || (flags & mask) == mask) &&
        (name == 0 || (this->name[tagOffset] == name[0] && this->name[tagOffset + 1] == name[1])) &&
        indexLists != 0 && group == 0)
    {
        float range = from->Distance(*to);
        direction.Copy(*from - *to);
        origin.Copy(*from);
        reverse.Copy(direction * -1.0f);
        for (int pack = 0; pack < packCount; pack++)
        {
            if (materials[packs[pack].material].alpha != OPAQUE_ALPHA)
                continue;
            center.x = packs[pack].center[0];
            center.y = packs[pack].center[1];
            center.z = packs[pack].center[2];
            if (!(from->Distance(center) <= range + packs[pack].radius))
                continue;
            if (!(center.DistanceToLine(*from, *to) <= range + packs[pack].radius))
                continue;
            for (int triangle = 0; triangle < indexLists[pack].count / INDICES_PER_TRIANGLE; triangle++)
            {
                if (triangleSpheres != 0 &&
                    triangleSpheres[pack].spheres[triangle].DistanceSquaredToLine(*from, *to) >
                        triangleSpheres[pack].spheres[triangle].w * triangleSpheres[pack].spheres[triangle].w)
                    continue;
                int vertex = indexLists[pack].indices[triangle * INDICES_PER_TRIANGLE + 0];
                cornerA.x = packs[pack].vertices[vertex].position[0];
                cornerA.y = packs[pack].vertices[vertex].position[1];
                cornerA.z = packs[pack].vertices[vertex].position[2];
                uvA.x = packs[pack].vertices[vertex].uv[0];
                uvA.y = packs[pack].vertices[vertex].uv[1];
                vertex = indexLists[pack].indices[triangle * INDICES_PER_TRIANGLE + 1];
                cornerB.x = packs[pack].vertices[vertex].position[0];
                cornerB.y = packs[pack].vertices[vertex].position[1];
                cornerB.z = packs[pack].vertices[vertex].position[2];
                uvB.x = packs[pack].vertices[vertex].uv[0];
                uvB.y = packs[pack].vertices[vertex].uv[1];
                vertex = indexLists[pack].indices[triangle * INDICES_PER_TRIANGLE + 2];
                cornerC.x = packs[pack].vertices[vertex].position[0];
                cornerC.y = packs[pack].vertices[vertex].position[1];
                cornerC.z = packs[pack].vertices[vertex].position[2] + THIRD_CORNER_LIFT;
                uvC.x = packs[pack].vertices[vertex].uv[0];
                uvC.y = packs[pack].vertices[vertex].uv[1];
                if (skin != 0 && skin->boneCount != 0)
                {
                    worldA.Copy(Project(cornerA, skin->skinMatrices[0] * worldMatrix));
                    worldB.Copy(Project(cornerB, skin->skinMatrices[0] * worldMatrix));
                    worldC.Copy(Project(cornerC, skin->skinMatrices[0] * worldMatrix));
                }
                else
                {
                    worldA.Copy(Project(cornerA, worldMatrix));
                    worldB.Copy(Project(cornerB, worldMatrix));
                    worldC.Copy(Project(cornerC, worldMatrix));
                }
                if ((IntersectTriangle(&origin, &direction, &worldA, &worldB, &worldC, &hitT, &texU, &texV, &uvA,
                                       &uvB, &uvC) != 0 ||
                     IntersectTriangle(&origin, &reverse, &worldA, &worldB, &worldC, &hitT, &texU, &texV, &uvA,
                                       &uvB, &uvC) != 0) &&
                    worldA.Distance(*from) < range)
                {
                    float toDistance = worldA.Distance(*to);
                    if (toDistance < range)
                    {
                        if (toDistance < *t)
                        {
                            *t = toDistance;
                            hit->Copy(worldA);
                        }
                        found = 1;
                    }
                }
            }
        }
    }

testChildren:
    if (isStatic != 0)
    {
        for (int i = 0; i < staticCount; i++)
        {
            Node *node = staticNodes[i];
            int nodeTagOffset = TAG_OFFSET_PLAIN;
            if (node->indexLists != 0 && node->group == 0 && (mask == 0 || (node->flags & mask) == mask))
            {
                if (node->kind != NODE_KIND_NONE)
                {
                    if (DAT_11b04514 != 0)
                    {
                        if (node->kind < VISIBILITY_CELL_COUNT)
                        {
                            nodeTagOffset = TAG_OFFSET_REGION;
                            if (DAT_11afffe8[node->kind] == 0)
                                continue;
                        }
                        if (node->kind == NODE_KIND_OVF)
                            nodeTagOffset = TAG_OFFSET_OVF;
                    }
                }
                else
                {
                    if (DAT_11b04514 != 0)
                    {
                        if (node->name[0] == 'C' && node->name[1] == 'D' && node->name[2] == 'L')
                        {
                            nodeTagOffset = TAG_OFFSET_REGION;
                            if (DAT_11afffe8[(node->name[3] - '0') * REGION_DIGIT_TENS + (node->name[4] - '0')] == 0)
                                continue;
                        }
                        if (node->name[0] == 'O' && node->name[1] == 'v' && node->name[2] == 'F')
                            nodeTagOffset = TAG_OFFSET_OVF;
                    }
                }
                if ((name == 0 || (node->name[nodeTagOffset] == name[0] && node->name[nodeTagOffset + 1] == name[1])) &&
                    node->CastSegment2(name, mask, from, to, hit, t) != 0)
                    found = 1;
            }
        }
    }
    else
    {
        for (int child = 0; child < childCount; child++)
        {
            if (children[child]->CastSegment2(name, mask, from, to, hit, t) != 0)
                found = 1;
        }
    }
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return found;
}
