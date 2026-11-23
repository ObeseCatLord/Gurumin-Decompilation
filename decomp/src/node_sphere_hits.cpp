// Sphere test against the triangles of one scene node's mesh (node.h): the sphere's centre is compared with each
// vertex pack's bounding sphere, then with each triangle (its own bounding sphere when the node has them), and finally
// with the triangle's plane, its inside and its three edges. Triangle corners are moved to world space by the node's
// translation only. Collision regions ("CDLnn" nodes) that are switched off are skipped. Matrix4 has the engine's empty
// destructor, so the function keeps an EH frame for its unused matrix local.

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

inline float Squared(float value)
{
    return value * value;
}

// 0x101a0cf0 (Node::SphereHitsMesh). Tests a sphere of `radius` at `point` (offset by the translation of `matrix`)
// against this node's own triangles (children are not visited); called by FUN_10093cc0. Returns 0 at once for a
// skinned node and for a collision region that is switched off (in the current test's cell list when DAT_11b002c0 is
// set, else in the visible-cell table DAT_11afffe8). A node needs the collision flag and packs. A pack is tested only
// when the point is within radius + node radius of the node's sphere and within radius + pack radius + 2 of the pack's
// sphere; a triangle only when the point is within radius + its sphere's radius of that sphere (nodes with triangle
// spheres) or within radius + 300 of its centre (the others). A triangle hits when the point is closer to its plane
// than `radius` and either inside it (depth = squared plane distance) or closer than `radius` to one of its edges
// (depth = squared edge distance). On a hit writes the triangle's plane to `normal` and the depth to `depth`, and
// returns -1; otherwise returns 0. A node with visibility cells updates them first and keeps "only enabled regions"
// (DAT_11b04514) set during the test.
int Node::SphereHitsMesh(Matrix4 *matrix, Vec4 *point, float radius, Vec4 *normal, float *depth)
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
    if (DAT_11b002c0 != 0)
    {
        if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
            ((char *)DAT_11b002c0)[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
            goto done;
    }
    else if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
             DAT_11afffe8[(name[3] - '0') * REGION_DIGIT_TENS + (name[4] - '0')] == 0)
        goto done;

    if ((flags & NODE_FLAG_HAS_COLLISION) != 0 && packCount != 0)
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
    goto done;
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
done:
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 0;
}
