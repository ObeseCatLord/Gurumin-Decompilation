// Segment tests against the triangles of one scene node's mesh (EN build). Ported from the matched JP source
// decomp/src/node_mesh_hits.cpp: each opaque vertex pack whose bounding sphere the segment can reach has its triangles
// moved to world space (through the first skin matrix when the node is skinned) and tested from both sides with
// Node::IntersectTriangle. Collision regions ("CDLnn" nodes) that are switched off are skipped.
#include "anim_model.h"
#include "containers.h"
#include "index_list.h"
#include "node.h"
#include "node_data.h"
#include "vec4.h"
#include "visibility.h"

// Material alpha of the packs that collide (packs with any other alpha are see-through and skipped).
#define OPAQUE_ALPHA 1.0f
// Lift added to the third corner's z before the test, so a triangle lying exactly in the segment's plane still hits.
#define THIRD_CORNER_LIFT 0.01f
// Vertex indices per triangle in an index list.
#define INDICES_PER_TRIANGLE 3
// Collision region names: "CDL" followed by two decimal digits (the region number, 0..99).
#define REGION_DIGIT_TENS 10
// Node::SegmentHitsMesh (EN 0x75ca20, JP 0x101a8780). Tests the segment `from`-`to` against this node's own triangles (children are not visited); called by
// the segment tests of the collision code. Returns 0 at once when the node is a collision region that is switched off (in the current test's
// cell list when g_collisionFilter is set, else in the visible-cell table g_visibleCells). A pack is tested only when its material is opaque and
// its bounding sphere is within the segment's length of `from` and of the segment's line; a triangle only when its
// own bounding sphere (sphere lists, when the node has them) touches the line. Returns 1 when a triangle is hit, from
// either side, at a point closer than the segment's length to both ends. A node with visibility cells updates them
// first and keeps "only enabled regions" (g_regionsOnlyEnabled) set during the test.
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
        g_regionsOnlyEnabled = 1;
    }
    if (g_collisionFilter != 0)
    {
        if (g_regionsOnlyEnabled != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
            ((char *)g_collisionFilter)[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
            goto switchedOff;
    }
    else if (g_regionsOnlyEnabled != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
             g_visibleCells[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
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
                    g_regionsOnlyEnabled = 0;
                return 1;
            }
        }
    }
    if (visibilityCells != 0)
        g_regionsOnlyEnabled = 0;
    return 0;
switchedOff:
    if (visibilityCells != 0)
        g_regionsOnlyEnabled = 0;
    return 0;
}
