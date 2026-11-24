// Sphere query against the collision of a scene node tree (node.h): the triangle test of Node::SphereHitsMesh run on
// this node and then on every child (or, on the static-scenery collector, on the nodes it gathered) whose
// two-character tag passes the caller's name filters. Collision regions ("CDLnn" nodes) that are switched off are
// skipped but their children are still visited. Matrix4 has the engine's empty destructor, so the function keeps an
// EH frame for its unused matrix local.

#include "vec4.h"
#include "matrix4.h"
#include "plane.h"
#include "mesh.h"
#include "node.h"
#include "char_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "camera_vars.h"
#include "node_collision_vars.h"

Plane PlaneFromTriangle(const Vec4 &a, const Vec4 &origin, const Vec4 &b);  // 0x101af360 (vec_ops.cpp)

// Node::flags bit: the node has collision geometry.
#define NODE_FLAG_HAS_COLLISION 0x4
// Node::flags bit: the node is tested whatever its tag is (the name filter does not apply).
#define NODE_FLAG_IGNORE_NAME_FILTER 0x4000000
// Node::flags bit: the per-triangle bounding spheres are not used.
#define NODE_FLAG_NO_TRIANGLE_SPHERES 0x10000000
// Collision region names: "CDL" followed by two decimal digits (the region number, 0..99).
#define REGION_DIGIT_TENS 10
// Slack added to the radius when a pack's bounding sphere is tested.
#define PACK_RADIUS_SLACK 2.0f
// Range added to the radius when a triangle's centre is tested (nodes without triangle spheres).
#define TRIANGLE_CENTRE_RANGE 300.0f
// Vertex indices per triangle in an index list.
#define INDICES_PER_TRIANGLE 3
// Corners of a triangle (the centre is their mean).
#define TRIANGLE_CORNER_COUNT 3
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

inline float Squared(float value)
{
    return value * value;
}

// 0x101a1640 (Node::Query). Tests a sphere of `radius` at `point` (offset by the translation of `matrix`) against
// the triangles of this node and of all of its children (or of the gathered static nodes). A node is tested when it
// has collision and packs, its group is 0, and its two-character tag is `name` (0 = any; ignored for nodes with the
// "ignore name filter" flag) and not `excludeName` (0 = none). A node's own test is that of SphereHitsMesh: a skinned
// node ends the whole query at once; otherwise per pack and triangle against the point, finally against the
// triangle's plane, inside and edges. On a hit writes the triangle's plane to `normal` and the depth to `depth`
// and returns -1; otherwise returns 0. Called with the map's collision root (DAT_10255f08) by the character, shot,
// debris and movement code.
int Node::Query(Matrix4 *matrix, Vec4 *point, float radius, Vec4 *normal, float *depth, const char *name,
                const char *excludeName)
{
    Vec4 matrixPosition;  // translation of the node's own matrix
    Vec4 centre;          // the sphere's centre in world space
    Vec4 cornerA;         // the triangle's corners (moved to world space)
    Vec4 cornerB;
    Vec4 cornerC;
    Vec4 worldPosition;   // translation of the node's world matrix
    Vec4 offsetCentre;
    Matrix4 unusedMatrix;  // constructed but never used (the original has it too)
    Plane plane;          // the current triangle's plane
    float distance;       // plane distance, then squared edge distance
    int indexA;           // vertex indices of the triangle's corners
    int indexB;
    int indexC;

    if (skinned != 0)
        return 0;
    if (visibilityCells != 0)
    {
        UpdateVisibleCells();
        DAT_11b04514 = 1;
    }
    int tagOffset = TAG_OFFSET_PLAIN;
    char cell;
    int result;
    if (kind != NODE_KIND_NONE)
    {
        if (DAT_11b002c0 != 0)
        {
            if (DAT_11b04514 != 0)
            {
                if (kind < VISIBILITY_CELL_COUNT)
                {
                    cell = ((char *)DAT_11b002c0)[kind];
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
                cell = DAT_11afffe8[kind];
                tagOffset = TAG_OFFSET_REGION;
                if (cell == 0)
                    goto testChildren;
            }
            if (kind == NODE_KIND_OVF)
                tagOffset = TAG_OFFSET_OVF;
        }
    }
    else if (DAT_11b002c0 != 0)
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

    if ((flags & NODE_FLAG_HAS_COLLISION) != 0 &&
        ((flags & NODE_FLAG_IGNORE_NAME_FILTER) != 0 || name == 0 ||
         (this->name[tagOffset] == name[0] && this->name[tagOffset + 1] == name[1])) &&
        (excludeName == 0 || this->name[tagOffset] != excludeName[0] ||
         this->name[tagOffset + 1] != excludeName[1]) &&
        group == 0 && packCount != 0)
    {
        matrixPosition.Copy(this->matrix.GetTranslation());
        worldPosition.Copy(worldMatrix.GetTranslation());
        offsetCentre.Copy(matrix->GetTranslation() + matrixPosition);
        float radiusSquared = Squared(radius);
        centre.Copy(matrix->GetTranslation() + *point);
        float reach = radius + bounds.w;
        if (!(centre.DistanceSquared(bounds + worldPosition) > reach * reach))
        {
            reach = radius + TRIANGLE_CENTRE_RANGE;
            float wideSquared = reach * reach;
            if ((flags & NODE_FLAG_NO_TRIANGLE_SPHERES) == 0 && triangleSpheres != 0)
            {
                for (int pack = 0; pack < packCount; pack++)
                {
                    reach = radius + packs[pack].radius + PACK_RADIUS_SLACK;
                    if (centre.DistanceSquared(*packs[pack].Sphere() + worldPosition) > reach * reach)
                        continue;
                    for (int index = 0, sphere = 0; index < indexLists[pack].count;
                         index += INDICES_PER_TRIANGLE, sphere++)
                    {
                        DAT_10f65548++;
                        reach = radius + triangleSpheres[pack].spheres[sphere].w;
                        if (centre.DistanceSquared(triangleSpheres[pack].spheres[sphere] + worldPosition) >
                            reach * reach)
                            continue;
                        indexA = indexLists[pack].indices[index + 0];
                        indexB = indexLists[pack].indices[index + 1];
                        indexC = indexLists[pack].indices[index + 2];
                        cornerA.x = packs[pack].vertices[indexA].position[0];
                        cornerA.y = packs[pack].vertices[indexA].position[1];
                        cornerA.z = packs[pack].vertices[indexA].position[2];
                        cornerB.x = packs[pack].vertices[indexB].position[0];
                        cornerB.y = packs[pack].vertices[indexB].position[1];
                        cornerB.z = packs[pack].vertices[indexB].position[2];
                        cornerC.x = packs[pack].vertices[indexC].position[0];
                        cornerC.y = packs[pack].vertices[indexC].position[1];
                        cornerC.z = packs[pack].vertices[indexC].position[2];
                        cornerA.Copy(cornerA + worldPosition);
                        cornerB.Copy(cornerB + worldPosition);
                        cornerC.Copy(cornerC + worldPosition);
                        plane = PlaneFromTriangle(cornerA, cornerB, cornerC);
                        distance = plane.Distance(&centre);
                        if (!(distance >= (double)radius))
                        {
                            DAT_10f6554c++;
                            if (centre.InTriangle(cornerA, cornerB, cornerC) != 0)
                                goto insideHit;
                            distance = centre.DistanceSquaredToSegment(cornerA, cornerB);
                            if (!(distance >= (double)radiusSquared))
                                goto edgeHit;
                            distance = centre.DistanceSquaredToSegment(cornerA, cornerC);
                            if (!(distance >= (double)radiusSquared))
                                goto edgeHit;
                            distance = centre.DistanceSquaredToSegment(cornerC, cornerB);
                            if (!(distance >= (double)radiusSquared))
                                goto edgeHit;
                        }
                    }
                }
            }
            else
            {
                for (int pack = 0; pack < packCount; pack++)
                {
                    reach = radius + packs[pack].radius + PACK_RADIUS_SLACK;
                    if (centre.DistanceSquared(*packs[pack].Sphere() + worldPosition) > reach * reach)
                        continue;
                    for (int index = 0; index < indexLists[pack].count; index += INDICES_PER_TRIANGLE)
                    {
                        DAT_10f65548++;
                        indexA = indexLists[pack].indices[index + 0];
                        indexB = indexLists[pack].indices[index + 1];
                        indexC = indexLists[pack].indices[index + 2];
                        cornerA.x = packs[pack].vertices[indexA].position[0];
                        cornerA.y = packs[pack].vertices[indexA].position[1];
                        cornerA.z = packs[pack].vertices[indexA].position[2];
                        cornerB.x = packs[pack].vertices[indexB].position[0];
                        cornerB.y = packs[pack].vertices[indexB].position[1];
                        cornerB.z = packs[pack].vertices[indexB].position[2];
                        cornerC.x = packs[pack].vertices[indexC].position[0];
                        cornerC.y = packs[pack].vertices[indexC].position[1];
                        cornerC.z = packs[pack].vertices[indexC].position[2];
                        cornerA.Copy(cornerA + worldPosition);
                        cornerB.Copy(cornerB + worldPosition);
                        cornerC.Copy(cornerC + worldPosition);
                        float three = (float)TRIANGLE_CORNER_COUNT;
                        if (!(centre.DistanceSquared(((cornerA + cornerB) + cornerC).Divided(three)) <= wideSquared))
                            continue;
                        plane = PlaneFromTriangle(cornerA, cornerB, cornerC);
                        distance = plane.Distance(&centre);
                        if (!(distance >= (double)radius))
                        {
                            DAT_10f6554c++;
                            if (centre.InTriangle(cornerA, cornerB, cornerC) != 0)
                                goto insideHit;
                            distance = centre.DistanceSquaredToSegment(cornerA, cornerB);
                            if (!(distance >= (double)radiusSquared))
                                goto edgeHit;
                            distance = centre.DistanceSquaredToSegment(cornerA, cornerC);
                            if (!(distance >= (double)radiusSquared))
                                goto edgeHit;
                            distance = centre.DistanceSquaredToSegment(cornerC, cornerB);
                            if (!(distance >= (double)radiusSquared))
                                goto edgeHit;
                        }
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
            if (node->kind == 0)
            {
                if (DAT_11b04514 != 0)
                {
                    if (node->name[0] == 'C' && node->name[1] == 'D' && node->name[2] == 'L' &&
                        (nodeTagOffset = TAG_OFFSET_REGION,
                         DAT_11afffe8[(node->name[3] - '0') * REGION_DIGIT_TENS + (node->name[4] - '0')] == 0))
                        continue;
                    if (node->name[0] == 'O' && node->name[1] == 'v' && node->name[2] == 'F')
                        nodeTagOffset = TAG_OFFSET_OVF;
                }
            }
            else if (DAT_11b04514 != 0)
            {
                if (node->kind >= VISIBILITY_CELL_COUNT ||
                    (nodeTagOffset = TAG_OFFSET_REGION, DAT_11afffe8[node->kind] != 0))
                {
                    if (node->kind == NODE_KIND_OVF)
                        nodeTagOffset = TAG_OFFSET_OVF;
                }
                else
                    continue;
            }
            if ((node->flags & NODE_FLAG_HAS_COLLISION) != 0 &&
                ((node->flags & NODE_FLAG_IGNORE_NAME_FILTER) != 0 || name == 0 ||
                 (node->name[nodeTagOffset] == name[0] && node->name[nodeTagOffset + 1] == name[1])) &&
                (excludeName == 0 || node->name[nodeTagOffset] != excludeName[0] ||
                 node->name[nodeTagOffset + 1] != excludeName[1]) &&
                node->group == 0 && node->packCount != 0)
            {
                result = node->Query(matrix, point, radius, normal, depth, name, excludeName);
                if (result != 0)
                    goto childHit;
            }
        }
    }
    else
    {
        for (int child = 0; child < childCount; child++)
        {
            result = children[child]->Query(matrix, point, radius, normal, depth, name, excludeName);
            if (result != 0)
                goto childHit;
        }
    }
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 0;
childHit:
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return result;
insideHit:
    *depth = distance * distance;
    normal->x = plane.a;
    normal->y = plane.b;
    normal->z = plane.c;
    normal->w = plane.d;
    goto hit;
edgeHit:
    *depth = distance;
    normal->x = plane.a;
    normal->y = plane.b;
    normal->z = plane.c;
    normal->w = plane.d;
hit:
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return -1;
}
