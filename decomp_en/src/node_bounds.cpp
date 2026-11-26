// Bounding volumes of a node's mesh (EN build): the per-pack boxes and spheres and the node's own box and sphere.
// Ported from the Ghidra output and the EN bytes (the JP twin, APP.dll 0x1019d8e0, is not matched).
#include "containers.h"
#include "node.h"

// Start values of a min/max scan: any real coordinate lies between them.
const float BOUNDS_SCAN_MIN_START = 999999.0f;
const float BOUNDS_SCAN_MAX_START = -999999.0f;
// The w of the corner points of a box.
const float BOUNDS_CORNER_W = 1.0f;
// A sphere around a box has half the length of the box diagonal as its radius.
const float BOUNDS_HALF = 0.5f;
// Offset of the node-name characters that give the kind of the node: "CDL<nn>-" (collision region) and "OvF<n>-" (overlay).
const int BOUNDS_REGION_PREFIX = NODE_COLLISION_PREFIX_LENGTH;
const int BOUNDS_OVERLAY_PREFIX = NODE_OVERLAY_PREFIX_LENGTH;

// Recomputes the bounds of the node's mesh: every vertex pack gets its box (boundsMin/boundsMax, w = 1), its centre and
// its radius, then the node gets the box around all packs and the sphere around that box (bounds, radius in w). A pack
// radius is only computed when it was already non-zero, when the node is flagged NODE_FLAG_KEEP_MESH or when the
// name after the "CDL<nn>-" / "OvF<n>-" prefix starts with "CS", "CA" or "CT" (the collision spheres). Nodes without a
// mesh (packCount = -1) are left alone. Returns 1. Called after a node's mesh is built or changed.
int Node::ComputeBounds()
{
    int namePrefix = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        namePrefix = BOUNDS_REGION_PREFIX;
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
        namePrefix = BOUNDS_OVERLAY_PREFIX;
    if (packCount == -1)
        return 1;

    float boxMinX = BOUNDS_SCAN_MIN_START;
    float boxMinY = BOUNDS_SCAN_MIN_START;
    float boxMinZ = BOUNDS_SCAN_MIN_START;
    float boxMaxX = BOUNDS_SCAN_MAX_START;
    float boxMaxY = BOUNDS_SCAN_MAX_START;
    float boxMaxZ = BOUNDS_SCAN_MAX_START;
    int i;
    float packMaxX;
    float packMinX;
    float packMaxY;
    float packMinY;
    float packMaxZ;
    float packMinZ;
    for (i = 0; i < packCount; i++)
    {
        packMaxX = BOUNDS_SCAN_MAX_START;
        packMinX = BOUNDS_SCAN_MIN_START;
        packMaxY = BOUNDS_SCAN_MAX_START;
        packMinY = BOUNDS_SCAN_MIN_START;
        packMaxZ = BOUNDS_SCAN_MAX_START;
        packMinZ = BOUNDS_SCAN_MIN_START;
        const Vertex *vertex = packs[i].vertices;
        for (int v = 0; v < packs[i].vertexCount; v++)
        {
            if (vertex[v].position[0] < packMinX)
                packMinX = vertex[v].position[0];
            if (packMaxX < vertex[v].position[0])
                packMaxX = vertex[v].position[0];
            if (vertex[v].position[1] < packMinY)
                packMinY = vertex[v].position[1];
            if (packMaxY < vertex[v].position[1])
                packMaxY = vertex[v].position[1];
            if (vertex[v].position[2] < packMinZ)
                packMinZ = vertex[v].position[2];
            if (packMaxZ < vertex[v].position[2])
                packMaxZ = vertex[v].position[2];
        }
        if (packMinX < boxMinX)
            boxMinX = packMinX;
        if (boxMaxX < packMaxX)
            boxMaxX = packMaxX;
        if (packMinY < boxMinY)
            boxMinY = packMinY;
        if (boxMaxY < packMaxY)
            boxMaxY = packMaxY;
        if (packMinZ < boxMinZ)
            boxMinZ = packMinZ;
        if (boxMaxZ < packMaxZ)
            boxMaxZ = packMaxZ;
        packs[i].boundsMin.x = packMinX;
        packs[i].boundsMin.y = packMinY;
        packs[i].boundsMin.z = packMinZ;
        packs[i].boundsMin.w = BOUNDS_CORNER_W;
        packs[i].boundsMax.x = packMaxX;
        packs[i].boundsMax.y = packMaxY;
        packs[i].boundsMax.z = packMaxZ;
        packs[i].boundsMax.w = BOUNDS_CORNER_W;
        packs[i].center[0] = (packMinX + packMaxX) * BOUNDS_HALF;
        packs[i].center[1] = (packMinY + packMaxY) * BOUNDS_HALF;
        packs[i].center[2] = (packMinZ + packMaxZ) * BOUNDS_HALF;
        if (packs[i].radius != 0.0f || (flags & NODE_FLAG_KEEP_MESH) != 0 ||
            (name[namePrefix] == 'C' &&
             (name[namePrefix + 1] == 'S' || name[namePrefix + 1] == 'A' || name[namePrefix + 1] == 'T')))
        {
            Vec4 extent;
            extent.x = packMaxX - packMinX;
            extent.y = packMaxY - packMinY;
            extent.z = packMaxZ - packMinZ;
            packs[i].radius = extent.Length() * BOUNDS_HALF;
        }
    }

    Vec4 extent;
    extent.x = boxMaxX - boxMinX;
    extent.y = boxMaxY - boxMinY;
    extent.z = boxMaxZ - boxMinZ;
    bounds.x = (boxMinX + boxMaxX) * BOUNDS_HALF;
    bounds.y = (boxMinY + boxMaxY) * BOUNDS_HALF;
    bounds.z = (boxMinZ + boxMaxZ) * BOUNDS_HALF;
    bounds.w = extent.Length() * BOUNDS_HALF;
    box[NODE_BOX_MIN].x = boxMinX;
    box[NODE_BOX_MIN].y = boxMinY;
    box[NODE_BOX_MIN].z = boxMinZ;
    box[NODE_BOX_MIN].w = BOUNDS_CORNER_W;
    box[NODE_BOX_MAX].x = boxMaxX;
    box[NODE_BOX_MAX].y = boxMaxY;
    box[NODE_BOX_MAX].z = boxMaxZ;
    box[NODE_BOX_MAX].w = BOUNDS_CORNER_W;
    return 1;
}
