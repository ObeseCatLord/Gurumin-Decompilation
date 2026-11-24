// Nearest-hit segment cast (Node::RayCast, with an excluded-flags mask and the "CT" name filter) against the collision meshes of a scene node tree (node.h). Unlike Node::Collide, which
// stops at the first triangle it finds, RayCast tests every node and keeps the hit nearest to the segment's
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
// Name filter "CT" only matches "CT" tags; a tag ending in "S" matches every other name filter.
#define TAG_FILTER_FIRST 'C'
#define TAG_FILTER_SECOND 'T'
#define TAG_ALWAYS_SECOND 'S'
#define NO_HIT_DISTANCE 999999.0f

// 0x101a9670 (Node::RayCast). Casts the segment between `to` and `from` against the triangles of this node and all of
// its children (or, on the static-scenery collector, of the nodes it gathered). A node takes part when its flags
// contain every bit of `requiredFlags` (0 = any) and none of the bits of `excludedFlags`, and it has a mesh and is not a group. `name`
// (0 = any) is compared with the node's two-character tag; the filter "CT" only matches "CT" tags and a tag ending in
// "S" matches every other filter. A node's own triangles are the opaque packs' triangles, tested from both sides; a
// hit counts when it lies closer than the segment's length to both ends. The hit nearest to `from` is kept: its
// point goes to `hit` and its distance to `t` (when given; children are cast without one). A switched-off collision
// region or visibility cell (while "only enabled regions", DAT_11b04514, is set) skips the node's own triangles but
// still visits its children. Counts the cast in DAT_11b044d8. Returns 1 on a hit.
int Node::RayCast(const char *name, unsigned requiredFlags, unsigned excludedFlags, const Vec4 *toEnd, const Vec4 *fromEnd, Vec4 *hit, float *t)
{
    Vec4 *to = const_cast<Vec4 *>(toEnd);        // the vector helpers are not declared const
    Vec4 *from = const_cast<Vec4 *>(fromEnd);
    float ownDistance = NO_HIT_DISTANCE;

    DAT_11b044d8 = DAT_11b044d8 + 1;
    if (t == 0)
        t = &ownDistance;
    Vec4 direction;   // to - from: the first test's ray direction
    Vec4 origin;      // copy of `to`: the rays start here
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
    Vec4 reverse;     // from - to: the second test's ray direction (the triangle's other side)
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

    if (!(requiredFlags != 0 && (flags & requiredFlags) != requiredFlags) && (flags & excludedFlags) == 0 &&
        indexLists != 0 && group == 0 &&
        (name == 0 ||
         (name[0] == TAG_FILTER_FIRST && name[1] == TAG_FILTER_SECOND
              ? this->name[tagOffset] == TAG_FILTER_FIRST && this->name[tagOffset + 1] == TAG_FILTER_SECOND
              : this->name[tagOffset] == name[0] &&
                    (this->name[tagOffset + 1] == TAG_ALWAYS_SECOND || this->name[tagOffset + 1] == name[1]))))
    {
        float range = to->Distance(*from);
        direction.Copy(*to - *from);
        origin.Copy(*to);
        reverse.Copy(direction * -1.0f);
        for (int pack = 0; pack < packCount; pack++)
        {
            if (materials[packs[pack].material].alpha != OPAQUE_ALPHA)
                continue;
            center.x = packs[pack].center[0];
            center.y = packs[pack].center[1];
            center.z = packs[pack].center[2];
            if (!(to->Distance(center) <= range + packs[pack].radius))
                continue;
            if (!(center.DistanceToLine(*to, *from) <= range + packs[pack].radius))
                continue;
            for (int triangle = 0; triangle < indexLists[pack].count / INDICES_PER_TRIANGLE; triangle++)
            {
                if (triangleSpheres != 0 &&
                    triangleSpheres[pack].spheres[triangle].DistanceSquaredToLine(*to, *from) >
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
                    worldA.Distance(*to) < range)
                {
                    float toDistance = worldA.Distance(*from);
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
            if ((requiredFlags != 0 && (node->flags & requiredFlags) != requiredFlags) ||
                (node->flags & excludedFlags) != 0 || node->indexLists == 0 || node->group != 0)
                continue;
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
            if (name == 0 ||
                (name[0] == TAG_FILTER_FIRST && name[1] == TAG_FILTER_SECOND
                     ? node->name[nodeTagOffset] == TAG_FILTER_FIRST && node->name[nodeTagOffset + 1] == TAG_FILTER_SECOND
                     : node->name[nodeTagOffset] == name[0] &&
                           (node->name[nodeTagOffset + 1] == TAG_ALWAYS_SECOND || node->name[nodeTagOffset + 1] == name[1])))
            {
                if (node->RayCast(name, requiredFlags, excludedFlags, to, from, hit, 0) != 0)
                    found = 1;
            }
        }
    }
    else
    {
        for (int child = 0; child < childCount; child++)
        {
            if (children[child]->RayCast(name, requiredFlags, excludedFlags, to, from, hit, 0) != 0)
                found = 1;
        }
    }
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return found;
}
