// Triangle spheres of a node's collision mesh (EN build). Ported from the JP draft decomp/src/scene_node.cpp
// (Node::BuildCollisionSpheres, 0x1019e120, not matched there) and the EN bytes.
#include "containers.h"
#include "engine_memory.h"
#include "index_list.h"
#include "node.h"

// The larger and smaller of two values; like the original they evaluate their arguments twice.
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// Offsets of the node-name characters that name the collision kind: "CDL<nn>-" (collision region) and "OvF<n>-" (overlay).
const int SPHERES_REGION_PREFIX = NODE_COLLISION_PREFIX_LENGTH;
const int SPHERES_OVERLAY_PREFIX = NODE_OVERLAY_PREFIX_LENGTH;
// Vertex indices of a triangle in an index list.
const int TRIANGLE_INDEX_COUNT = 3;
// A triangle sphere's radius is the farthest corner's distance from the box centre, with a little margin.
const float TRIANGLE_SPHERE_MARGIN = 1.1f;
// The centre of a triangle's box is the middle between its extremes.
const float SPHERE_HALF = 2.0f;

// Frees the buffers of `count` sphere lists in a row (inline in the original).
static void SphereLists_Free(IndexList *lists, int count)
{
    if (lists != 0 && count > 0)
    {
        do
        {
            lists->FreeSpheres();
            lists++;
            count--;
        } while (count != 0);
    }
}

// Builds the sphere list of every vertex pack of a collision node: one sphere per triangle, centred in the triangle's
// bounding box, with TRIANGLE_SPHERE_MARGIN times the distance to its farthest corner as radius. A node is a collision
// node when its name (after a "CDL" or "OvF" prefix) starts with "CA", "CS" or "CT", or it has the flag
// NODE_FLAG_KEEP_MESH or one of the debug pass flags. The old lists are freed first. Then the children are built the
// same way. Returns 1. Called after a map or model is loaded.
int Node::BuildCollisionSpheres()
{
    Vec4 a;
    Vec4 b;
    Vec4 c;
    int prefix = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        prefix = SPHERES_REGION_PREFIX;
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
        prefix = SPHERES_OVERLAY_PREFIX;
    if (((name[prefix] == 'C' && name[prefix + 1] == 'A') || (name[prefix] == 'C' && name[prefix + 1] == 'S') ||
         (name[prefix] == 'C' && name[prefix + 1] == 'T') || (flags & NODE_FLAG_KEEP_MESH) != 0 ||
         (flags & NODE_FLAG_SPECIAL_PASSES) != 0) &&
        packCount > 0)
    {
        SphereLists_Free(triangleSpheres, packCount);
        if (triangleSpheres != 0)
            g_engineFree(triangleSpheres);
        triangleSpheres = (IndexList *)g_engineAlloc(packCount * sizeof(IndexList), "m_vpackindexsp");
        for (int p = 0; p < packCount; p++)
        {
            triangleSpheres[p].InitSpheres();
            triangleSpheres[p].ResizeSpans(indexLists[p].count / TRIANGLE_INDEX_COUNT);
            for (int i = 0; i < indexLists[p].count; i += TRIANGLE_INDEX_COUNT)
            {
                int first = indexLists[p].indices[i];
                int second = indexLists[p].indices[i + 1];
                int third = indexLists[p].indices[i + 2];
                a.x = packs[p].vertices[first].position[0];
                a.y = packs[p].vertices[first].position[1];
                a.z = packs[p].vertices[first].position[2];
                b.x = packs[p].vertices[second].position[0];
                b.y = packs[p].vertices[second].position[1];
                b.z = packs[p].vertices[second].position[2];
                c.x = packs[p].vertices[third].position[0];
                c.y = packs[p].vertices[third].position[1];
                c.z = packs[p].vertices[third].position[2];
                triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].x =
                    (MAX(MAX(a.x, b.x), c.x) + MIN(MIN(a.x, b.x), c.x)) / SPHERE_HALF;
                triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].y =
                    (MAX(MAX(a.y, b.y), c.y) + MIN(MIN(a.y, b.y), c.y)) / SPHERE_HALF;
                triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].z =
                    (MAX(MAX(a.z, b.z), c.z) + MIN(MIN(a.z, b.z), c.z)) / SPHERE_HALF;
                float da = triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].Distance(a);
                float db = triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].Distance(b);
                float dc = triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].Distance(c);
                triangleSpheres[p].spheres[i / TRIANGLE_INDEX_COUNT].w = MAX(MAX(da, db), dc) * TRIANGLE_SPHERE_MARGIN;
            }
        }
    }
    for (int child = 0; child < childCount; child++)
        children[child]->BuildCollisionSpheres();
    return 1;
}
