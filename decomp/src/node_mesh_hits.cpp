// Segment tests against the triangles of one scene node's mesh (node.h): each opaque vertex pack whose bounding
// sphere the segment can reach has its triangles moved to world space (through the first skin matrix when the node
// is skinned) and tested from both sides with Node::IntersectTriangle. Collision regions ("CDLnn" nodes) that are
// switched off are skipped. Matrix4 has the engine's empty destructor, so the skinned path keeps an EH frame for its
// matrix temporaries.

#include "vec4.h"
#include "matrix4.h"
#include "mesh.h"
#include "node.h"
#include "node_data.h"
#include "char_vars.h"
#include "map_vars.h"
#include "render_vars.h"
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
// Node kind whose two-character tag sits where an "OvF" node's does.
#define NODE_KIND_OVF 0xfd
// Where Collide finds a node's two-character tag in its name: at the start for plain nodes, after "CDLnn_" for
// collision regions and region-cell nodes, after "OvF" and four more characters for "OvF" nodes.
#define TAG_OFFSET_PLAIN 0
#define TAG_OFFSET_REGION 6
#define TAG_OFFSET_OVF 7

// 0x101a8780. Tests the segment `from`-`to` against this node's own triangles (children are not visited); called by
// FUN_1011d810. Returns 0 at once when the node is a collision region that is switched off (in the current test's
// cell list when DAT_11b002c0 is set, else in the visible-cell table DAT_11afffe8). A pack is tested only when its material is opaque and
// its bounding sphere is within the segment's length of `from` and of the segment's line; a triangle only when its
// own bounding sphere (sphere lists, when the node has them) touches the line. Returns 1 when a triangle is hit, from
// either side, at a point closer than the segment's length to both ends. A node with visibility cells updates them
// first and keeps "only enabled regions" (DAT_11b04514) set during the test.
int Node::SegmentHitsMesh(Vec4 *from, Vec4 *to)
{
    Vec4 direction;   // from - to: the first test's ray direction
    Vec4 origin;      // copy of `from`: the rays start here
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
    if (DAT_11b002c0 != 0)
    {
        if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
            ((char *)DAT_11b002c0)[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
            goto switchedOff;
    }
    else if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
             DAT_11afffe8[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
        goto switchedOff;

    float range = from->Distance(*to);
    if (indexLists == 0)
        goto switchedOff;
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
            if (triangleSpheres != 0 && triangleSpheres[pack].spheres[triangle].DistanceSquaredToLine(*from, *to) >
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
            if ((IntersectTriangle(&origin, &direction, &worldA, &worldB, &worldC, &hitT, &texU, &texV, &uvA, &uvB,
                                   &uvC) != 0 ||
                 IntersectTriangle(&origin, &reverse, &worldA, &worldB, &worldC, &hitT, &texU, &texV, &uvA, &uvB,
                                   &uvC) != 0) &&
                worldA.Distance(*from) < range && worldA.Distance(*to) < range)
            {
                if (visibilityCells != 0)
                    DAT_11b04514 = 0;
                return 1;
            }
        }
    }
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 0;
switchedOff:
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 0;
}

// 0x101a8e60 (Node::Collide). Tests the segment `from`-`to` against the triangles of this node and then of each child
// in turn (depth first); the first hit point goes to `hit` and the function returns 1 (or the child's result).
// A node's own triangles are tested like SegmentHitsMesh's (opaque packs, both sides, closer than the segment's
// length to both ends) when its flags share a bit with `mask` (0 = any), its two-character tag matches `tag` (0 =
// any), it has a mesh and its group is 0. A switched-off collision region or visibility cell (when "only enabled
// regions", DAT_11b04514, is set) skips its own triangles but still visits its children. Called by the character,
// enemy, camera and shot code with the map's collision root (DAT_10255f08) and by FUN_1001fff0 per region node.
int Node::Collide(const char *tag, unsigned int mask, Vec4 *from, Vec4 *to, Vec4 *hit)
{
    Vec4 direction;   // from - to: the first test's ray direction
    Vec4 origin;      // copy of `from`: the rays start here
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
    if (kind != 0)
    {
        // A visibility cell node: switched off when its cell is not in the test's cell list (or not visible).
        if (DAT_11b002c0 != 0)
        {
            if (DAT_11b04514 != 0)
            {
                if (kind < VISIBILITY_CELL_COUNT)
                {
                    tagOffset = TAG_OFFSET_REGION;
                    if (((char *)DAT_11b002c0)[kind] == 0)
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
                tagOffset = TAG_OFFSET_REGION;
                if (DAT_11afffe8[kind] == 0)
                    goto testChildren;
            }
            if (kind == NODE_KIND_OVF)
                tagOffset = TAG_OFFSET_OVF;
        }
    }
    else if (DAT_11b002c0 != 0)
    {
        // A collision region "CDLnn": switched off when its number is not in the test's cell list (or disabled).
        if (DAT_11b04514 != 0)
        {
            if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
            {
                tagOffset = TAG_OFFSET_REGION;
                if (((char *)DAT_11b002c0)[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
                    goto testChildren;
            }
            if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
                tagOffset = TAG_OFFSET_OVF;
        }
    }
    else if (DAT_11b04514 != 0)
    {
        if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        {
            tagOffset = TAG_OFFSET_REGION;
            if (DAT_11afffe8[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
                goto testChildren;
        }
        if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
            tagOffset = TAG_OFFSET_OVF;
    }

    if ((mask == 0 || (flags & mask) != 0) &&
        (tag == 0 || (name[tagOffset] == tag[0] && name[tagOffset + 1] == tag[1])) && indexLists != 0 && group == 0)
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
                        hit->Copy(worldA);
                        if (visibilityCells != 0)
                            DAT_11b04514 = 0;
                        return 1;
                    }
                }
            }
        }
    }

testChildren:
    for (int child = 0; child < childCount; child++)
    {
        int result = children[child]->Collide(tag, mask, from, to, hit);
        if (result != 0)
        {
            if (visibilityCells != 0)
                DAT_11b04514 = 0;
            return result;
        }
    }
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 0;
}
