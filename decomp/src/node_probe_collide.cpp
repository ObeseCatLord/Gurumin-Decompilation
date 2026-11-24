// Probe-sphere collision against the triangle meshes of a scene node tree (node.h). The probes are the sample points
// of a moving model (AnimModel::LiftProbes, IsClearOfMap): each is a sphere of one radius, moved by the model's matrix.
// The function is the multi-probe, recursive relative of Node::SphereHitsMesh (node_sphere_hits.cpp): same bounding
// sphere, pack and triangle tests, but a touched probe can be pushed out of the triangle and the test goes on with
// the next probe. Matrix4 has the engine's empty destructor, so the function keeps an EH frame for its unused matrix.

#include "vec4.h"
#include "matrix4.h"
#include "plane.h"
#include "mesh.h"
#include "node.h"
#include "char_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "camera_vars.h"
#include "frame_vars.h"
#include "node_collision.h"
#include "node_collision_vars.h"

Plane PlaneFromTriangle(const Vec4 &a, const Vec4 &origin, const Vec4 &b);   // 0x101af360 (vec_ops.cpp)
Vec4 TriangleNormal(const Vec4 &a, const Vec4 &origin, const Vec4 &b);       // 0x101ae8b0 (vec_ops.cpp)

// Node::flags bit: the node has collision geometry.
#define NODE_FLAG_HAS_COLLISION 0x4
// Node::flags bit: a probe touching the node ends the test at once (after the contact callback).
#define NODE_FLAG_CONTACT_ENDS_TEST 0x10
// Node::flags bit: the per-triangle bounding spheres are not used.
#define NODE_FLAG_NO_TRIANGLE_SPHERES 0x10000000
// Node::flags bit: the node is map collision; a touch is reported to the caller instead of pushed out.
#define NODE_FLAG_MAP_KIND 0x4000000
// Probe flags: the low two bits are the test kind, bit 2 lets the test touch nodes that belong to a group.
#define PROBE_KIND_MASK 3
#define PROBE_KIND_COUNTED 1
#define PROBE_FLAG_TOUCH_GROUPS 4
// Node::group values: 0 = plain map, the others are special collision groups.
#define GROUP_PLAIN 0
#define GROUP_OPEN 0x13881
#define GROUP_COUNTED 80000
#define GROUP_QUIET 0x3c
// Group numbers 50000..50099: the test below is written as it was compiled and is never true.
#define GROUP_RANGE_START 50000
#define GROUP_RANGE_END 50099
// Touches of GROUP_COUNTED nodes are counted in DAT_11b0450c up to this value; from the next one on they are ignored.
#define COUNTED_TOUCH_LIMIT 0x4c
// Node kinds below this are visibility cell numbers; kind 0xff = none (the name decides), 0xfd = an "OvF" node.
#define VISIBILITY_CELL_COUNT 0x40
#define NODE_KIND_NONE 0xff
#define NODE_KIND_OVF 0xfd
#define TAG_OFFSET_PLAIN 0
#define TAG_OFFSET_REGION 6
#define TAG_OFFSET_OVF 7
// Collision regions: "CDL" followed by two decimal digits.
#define REGION_DIGIT_TENS 10
// Range added to a node's bounding radius before its sphere is tested.
#define NODE_BOUNDS_SLACK 60.0f
// Slack added to a pack's radius.
#define PACK_RADIUS_SLACK 2.0f
// Range added to the probe radius when a triangle's centre is tested (nodes without triangle spheres).
#define TRIANGLE_CENTRE_RANGE 300.0f
// Vertex indices per triangle in an index list.
#define INDICES_PER_TRIANGLE 3
// Corners of a triangle (the centre is their mean).
#define TRIANGLE_CORNER_COUNT 3
// A touched probe is pushed out along the triangle's normal by this much, at most this many times in one call.
#define PROBE_PUSH_DISTANCE 4.0f
#define PROBE_PUSH_LIMIT 500
// Results: all probes used up or a map touch reported, and a probe that moved on while triangles were still tested.
#define PROBES_DONE (-1)
#define PROBE_MOVED_ON (-2)

inline float Squared(float value)
{
    return value * value;
}

// Reports a touch of a node of group `group` to DAT_11b044fc; returns non-zero when the node ends the test.
inline int ReportContact(Node *node, int type)
{
    if (DAT_11b044fc != 0)
    {
        DAT_11b044fc(node->group, (int)node, type);
        return (node->flags & NODE_FLAG_CONTACT_ENDS_TEST) != 0;
    }
    return 0;
}

// 0x1019f340 (Node::CollideProbesWithMesh). Tests the probe spheres `probes` (radius `reach`, the current one is
// probes[*probeIndex], moved by the translation of `world` and of this node's matrix) against the triangles of `node`
// and then of its children (or, on a static-scenery collector, of the nodes it gathered). `this` is the root of the
// tree: its matrix places the probes, and its kind and visibility cells steer the "only enabled regions" checks.
// `flags` = test kind (low two bits, PROBE_KIND_COUNTED counts GROUP_COUNTED touches) and PROBE_FLAG_TOUCH_GROUPS.
// Nodes need the bits of `mask`, the collision flag and packs; skinned nodes, disabled collision regions and nodes of
// the wrong group are skipped (but their children are visited). A node whose bounding sphere is out of range is
// skipped; the others are marked in `skipped` (when the caller asked for it and DAT_11b002c8 is set, in which case a
// static node that was found out of range is also skipped by the next test).
// A touched triangle: the touching node's contact callback runs for grouped nodes (kind 0 is exempt); map collision
// (NODE_FLAG_MAP_KIND, when `hitFlag` is given) sets *hitFlag, stores the address of the node's world matrix in
// *hitMatrix and ends the test with -1. Otherwise, with `resolve`, the probe is pushed out along the triangle normal
// and the same triangle is tested again (up to 500 pushes, then the probe is zeroed); without it the touch is counted
// and the next probe becomes current. The plane's normal goes to `hitNormal`, the squared depth to `nearest`.
// Returns the current probe index when all nodes were tested, -1 when all probes are used up (`hitPoint` then gets the
// last triangle sphere's centre), a map touch was reported or a contact callback ended the test, and -2 when a
// probe moved on and more triangles remain (the caller tests again), and 0 for a disabled collision region.
int Node::CollideProbesWithMesh(unsigned int mask, Matrix4 *world, Node *node, float reach, float *nearest,
                                Vec4 *probes, int probeCount, int *probeIndex, Vec4 *hitPoint, Vec4 *hitNormal,
                                unsigned int flags, int *hitFlag, int resolve, int *hitMatrix, int *skipped)
{
    DAT_11b044d4 = DAT_11b044d4 + 1;
    int moved = 0;
    Vec4 matrixPosition;  // translation of this node's matrix
    Vec4 centre;          // the current probe in world space
    Vec4 cornerA;         // the triangle's corners (moved to world space)
    Vec4 cornerB;
    Vec4 cornerC;
    Vec4 worldPosition;   // translation of the tested node's world matrix
    Vec4 offsetCentre;    // translation of `world` plus this node's: the probes are offsets from it
    Matrix4 unusedMatrix;  // constructed but never used (the original has it too)
    Plane plane;          // the current triangle's plane
    float distance;       // plane distance, then squared edge distance
    float range;          // distance within which a sphere test is made
    int indexA;           // vertex indices of the triangle's corners
    int indexB;
    int indexC;
    int type = flags & PROBE_KIND_MASK;
    int pushes = 0;
    int isCamera;
    unsigned int touchGroups;
    if (node->visibilityCells != 0)
    {
        node->UpdateVisibleCells();
        DAT_11b04514 = 1;
    }
    int tagOffset = TAG_OFFSET_PLAIN;
    if (node->kind != NODE_KIND_NONE)
    {
        if (DAT_11b002c0 != 0)
        {
            if (DAT_11b04514 != 0)
            {
                if (node->kind < VISIBILITY_CELL_COUNT)
                {
                    if (((char *)DAT_11b002c0)[node->kind] == 0)
                        goto regionOff;
                    tagOffset = TAG_OFFSET_REGION;
                }
                if (kind == NODE_KIND_OVF)
                    tagOffset = TAG_OFFSET_OVF;
            }
        }
        else if (DAT_11b04514 != 0)
        {
            if (node->kind < VISIBILITY_CELL_COUNT)
            {
                if (DAT_11afffe8[node->kind] == 0)
                    goto regionOff;
                tagOffset = TAG_OFFSET_REGION;
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
                if (node->name[0] == 'C' && node->name[1] == 'D' && node->name[2] == 'L')
                {
                    if (((char *)DAT_11b002c0)[(node->name[3] - '0') * REGION_DIGIT_TENS + (node->name[4] - '0')] == 0)
                        goto regionOff;
                    tagOffset = TAG_OFFSET_REGION;
                }
                else if (node->name[0] == 'O' && node->name[1] == 'v' && node->name[2] == 'F')
                    tagOffset = TAG_OFFSET_OVF;
            }
        }
        else if (DAT_11b04514 != 0)
        {
            if (node->name[0] == 'C' && node->name[1] == 'D' && node->name[2] == 'L')
            {
                if (DAT_11afffe8[(node->name[3] - '0') * REGION_DIGIT_TENS + (node->name[4] - '0')] == 0)
                    goto regionOff;
                tagOffset = TAG_OFFSET_REGION;
            }
            else if (node->name[0] == 'O' && node->name[1] == 'v' && node->name[2] == 'F')
                tagOffset = TAG_OFFSET_OVF;
        }
    }
    isCamera = 0;
    if (node->name[tagOffset] == 'C' && node->name[tagOffset + 1] == 'A')
        isCamera = 1;
    if (node->skinned != 0)
    {
        if (visibilityCells != 0)
            DAT_11b04514 = 0;
        return *probeIndex;
    }
    touchGroups = flags & PROBE_FLAG_TOUCH_GROUPS;
    if ((touchGroups == 0 && node->group != GROUP_OPEN && node->group != GROUP_PLAIN) ||
        (type != PROBE_KIND_COUNTED && node->group != GROUP_PLAIN && node->group != GROUP_OPEN &&
         node->group < GROUP_RANGE_START && node->group > GROUP_RANGE_END))
    {
        if (DAT_11b002c8 != 0 && skipped != 0 && node->group != GROUP_QUIET)
            *skipped = 1;
    }
    else if ((mask & node->flags) == 0)
    {
        if (DAT_11b002c8 != 0 && skipped != 0 && node->group != GROUP_QUIET)
            *skipped = 1;
    }
    else if ((node->flags & NODE_FLAG_HAS_COLLISION) == 0 || node->packCount == 0)
    {
    }
    else
    {
        unsigned int mapKind = node->flags & NODE_FLAG_MAP_KIND;
        if (hitFlag == 0)
            mapKind = 0;
        matrixPosition.Copy(matrix.GetTranslation());
        worldPosition.Copy(node->worldMatrix.GetTranslation());
        offsetCentre.Copy(world->GetTranslation() + matrixPosition);
        float radiusSquared = Squared(reach);
        centre.Copy(offsetCentre + probes[*probeIndex]);
        range = reach + node->bounds.w + NODE_BOUNDS_SLACK;
        if (centre.DistanceSquared(node->bounds + worldPosition) > range * range)
            goto scanSub;
        if (DAT_11b002c8 != 0 && skipped != 0)
            *skipped = 1;
        *hitFlag = 0;
        range = reach + TRIANGLE_CENTRE_RANGE;
        float wideSquared = range * range;
        if ((node->flags & NODE_FLAG_NO_TRIANGLE_SPHERES) == 0 && node->triangleSpheres != 0)
        {
            for (int pack = 0; pack < node->packCount; pack++)
            {
                range = reach + node->packs[pack].radius + PACK_RADIUS_SLACK;
                if (centre.DistanceSquared(*node->packs[pack].Sphere() + worldPosition) > range * range)
                    continue;
                for (int index = 0, sphere = 0; index < node->indexLists[pack].count; index += INDICES_PER_TRIANGLE, sphere++)
                {
                    DAT_10f65548++;
                    if (moved != 0)
                        return PROBE_MOVED_ON;
                    range = reach + node->triangleSpheres[pack].spheres[sphere].w;
                    if (centre.DistanceSquared(node->triangleSpheres[pack].spheres[sphere] + worldPosition) > range * range)
                        continue;
                    indexA = node->indexLists[pack].indices[index + 0];
                    indexB = node->indexLists[pack].indices[index + 1];
                    indexC = node->indexLists[pack].indices[index + 2];
                    cornerA.x = node->packs[pack].vertices[indexA].position[0];
                    cornerA.y = node->packs[pack].vertices[indexA].position[1];
                    cornerA.z = node->packs[pack].vertices[indexA].position[2];
                    cornerB.x = node->packs[pack].vertices[indexB].position[0];
                    cornerB.y = node->packs[pack].vertices[indexB].position[1];
                    cornerB.z = node->packs[pack].vertices[indexB].position[2];
                    cornerC.x = node->packs[pack].vertices[indexC].position[0];
                    cornerC.y = node->packs[pack].vertices[indexC].position[1];
                    cornerC.z = node->packs[pack].vertices[indexC].position[2];
                    cornerA.Copy(cornerA + worldPosition);
                    cornerB.Copy(cornerB + worldPosition);
                    cornerC.Copy(cornerC + worldPosition);
                    plane = PlaneFromTriangle(cornerA, cornerB, cornerC);
                    if (plane.a == 0.0f && plane.b == 0.0f && plane.c == 0.0f && plane.d == 0.0f)
                        continue;
                    for (distance = plane.Distance(&centre); !(distance >= (double)reach); distance = plane.Distance(&centre))
                    {
                        if (hitNormal != 0)
                        {
                            hitNormal->x = plane.a;
                            hitNormal->y = plane.b;
                            hitNormal->z = plane.c;
                        }
                        DAT_10f6554c++;
                        if (centre.InTriangle(cornerA, cornerB, cornerC) != 0)
                        {
                            *nearest = distance * distance;
                            if (node->group != GROUP_PLAIN)
                            {
                                if (type == PROBE_KIND_COUNTED)
                                {
                                    if (node->group != GROUP_COUNTED)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                        break;
                                    }
                                    if (DAT_11b0450c < COUNTED_TOUCH_LIMIT)
                                        DAT_11b0450c++;
                                    if (DAT_11b0450c > COUNTED_TOUCH_LIMIT - 1)
                                        break;
                                }
                                else if (node->group != GROUP_OPEN)
                                {
                                    if (type != 0)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                    }
                                    break;
                                }
                            }
                            if (mapKind != 0 && node->group == GROUP_PLAIN)
                            {
                                *hitFlag = 1;
                                if (hitMatrix != 0)
                                    *hitMatrix = (int)&node->worldMatrix;
                                return PROBES_DONE;
                            }
                            *hitFlag = 0;
                            if (resolve != 0)
                            {
                                Vec4 push;
                                push.Copy(TriangleNormal(cornerA, cornerB, cornerC));
                                push.Normalized3();
                                probes[*probeIndex].Copy(probes[*probeIndex] + push * PROBE_PUSH_DISTANCE);
                                centre.Copy(offsetCentre + probes[*probeIndex]);
                                pushes++;
                                if (pushes <= PROBE_PUSH_LIMIT)
                                    continue;
                                probes[*probeIndex].Fill(0);
                            }
                            *probeIndex = *probeIndex + 1;
                            DAT_11b002b4 = isCamera;
                            if (*probeIndex >= probeCount)
                            {
                                hitPoint->Copy(node->triangleSpheres[pack].spheres[sphere] + worldPosition);
                                return PROBES_DONE;
                            }
                            centre.Copy(offsetCentre + probes[*probeIndex]);
                            moved = 1;
                        }
                        else
                        {
                            distance = centre.DistanceSquaredToSegment(cornerA, cornerB);
                            if (distance >= (double)radiusSquared)
                            {
                                distance = centre.DistanceSquaredToSegment(cornerA, cornerC);
                                if (distance >= (double)radiusSquared)
                                {
                                    distance = centre.DistanceSquaredToSegment(cornerC, cornerB);
                                    if (distance >= (double)radiusSquared)
                                        break;
                                }
                            }
                            *nearest = distance;
                            if (node->group != GROUP_PLAIN)
                            {
                                if (type == PROBE_KIND_COUNTED)
                                {
                                    if (node->group != GROUP_COUNTED)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                        break;
                                    }
                                    if (DAT_11b0450c < COUNTED_TOUCH_LIMIT)
                                        DAT_11b0450c++;
                                    if (DAT_11b0450c > COUNTED_TOUCH_LIMIT - 1)
                                        break;
                                }
                                else if (node->group != GROUP_OPEN)
                                {
                                    if (type != 0)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                    }
                                    break;
                                }
                            }
                            if (mapKind != 0)
                            {
                                *hitFlag = 1;
                                if (hitMatrix != 0)
                                    *hitMatrix = (int)&node->worldMatrix;
                                return PROBES_DONE;
                            }
                            *hitFlag = 0;
                            if (resolve != 0)
                            {
                                Vec4 push;
                                push.Copy(TriangleNormal(cornerA, cornerB, cornerC));
                                push.Normalized3();
                                probes[*probeIndex].Copy(probes[*probeIndex] + push * PROBE_PUSH_DISTANCE);
                                centre.Copy(offsetCentre + probes[*probeIndex]);
                                pushes++;
                                if (pushes <= PROBE_PUSH_LIMIT)
                                    continue;
                                probes[*probeIndex].Fill(0);
                            }
                            *probeIndex = *probeIndex + 1;
                            DAT_11b002b4 = isCamera;
                            if (*probeIndex >= probeCount)
                            {
                                hitPoint->Copy(node->triangleSpheres[pack].spheres[sphere] + worldPosition);
                                return PROBES_DONE;
                            }
                            centre.Copy(offsetCentre + probes[*probeIndex]);
                            moved = 1;
                        }
                    }
                }
            }
        }
        else
        {
            for (int pack = 0; pack < node->packCount; pack++)
            {
                range = reach + node->packs[pack].radius + PACK_RADIUS_SLACK;
                if (centre.DistanceSquared(*node->packs[pack].Sphere() + worldPosition) > range * range)
                    continue;
                for (int index = 0; index < node->indexLists[pack].count; index += INDICES_PER_TRIANGLE)
                {
                    DAT_10f65548++;
                    if (moved != 0)
                        return PROBE_MOVED_ON;
                    indexA = node->indexLists[pack].indices[index + 0];
                    indexB = node->indexLists[pack].indices[index + 1];
                    indexC = node->indexLists[pack].indices[index + 2];
                    cornerA.x = node->packs[pack].vertices[indexA].position[0];
                    cornerA.y = node->packs[pack].vertices[indexA].position[1];
                    cornerA.z = node->packs[pack].vertices[indexA].position[2];
                    cornerB.x = node->packs[pack].vertices[indexB].position[0];
                    cornerB.y = node->packs[pack].vertices[indexB].position[1];
                    cornerB.z = node->packs[pack].vertices[indexB].position[2];
                    cornerC.x = node->packs[pack].vertices[indexC].position[0];
                    cornerC.y = node->packs[pack].vertices[indexC].position[1];
                    cornerC.z = node->packs[pack].vertices[indexC].position[2];
                    cornerA.Copy(cornerA + worldPosition);
                    cornerB.Copy(cornerB + worldPosition);
                    cornerC.Copy(cornerC + worldPosition);
                    float three = (float)TRIANGLE_CORNER_COUNT;
                    while (centre.DistanceSquared(((cornerA + cornerB) + cornerC).Divided(three)) <= wideSquared)
                    {
                        plane = PlaneFromTriangle(cornerA, cornerB, cornerC);
                        if (plane.a == 0.0f && plane.b == 0.0f && plane.c == 0.0f && plane.d == 0.0f)
                            break;
                        distance = plane.Distance(&centre);
                        if (distance >= (double)reach)
                            break;
                        if (hitNormal != 0)
                        {
                            hitNormal->x = plane.a;
                            hitNormal->y = plane.b;
                            hitNormal->z = plane.c;
                        }
                        DAT_10f6554c++;
                        if (centre.InTriangle(cornerA, cornerB, cornerC) != 0)
                        {
                            *nearest = distance * distance;
                            if (node->group != GROUP_PLAIN)
                            {
                                if (type == PROBE_KIND_COUNTED)
                                {
                                    if (node->group != GROUP_COUNTED)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                        break;
                                    }
                                    if (DAT_11b0450c < COUNTED_TOUCH_LIMIT)
                                        DAT_11b0450c++;
                                    if (DAT_11b0450c > COUNTED_TOUCH_LIMIT - 1)
                                        break;
                                }
                                else if (node->group != GROUP_OPEN)
                                {
                                    if (type != 0)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                    }
                                    break;
                                }
                            }
                            if (mapKind != 0 && node->group == GROUP_PLAIN)
                            {
                                *hitFlag = 1;
                                if (hitMatrix != 0)
                                    *hitMatrix = (int)&node->worldMatrix;
                                return PROBES_DONE;
                            }
                            *hitFlag = 0;
                            if (resolve != 0)
                            {
                                Vec4 push;
                                push.Copy(TriangleNormal(cornerA, cornerB, cornerC));
                                push.Normalized3();
                                probes[*probeIndex].Copy(probes[*probeIndex] + push * PROBE_PUSH_DISTANCE);
                                centre.Copy(offsetCentre + probes[*probeIndex]);
                                pushes++;
                                if (pushes <= PROBE_PUSH_LIMIT)
                                    continue;
                                probes[*probeIndex].Fill(0);
                            }
                            *probeIndex = *probeIndex + 1;
                            DAT_11b002b4 = isCamera;
                            if (*probeIndex >= probeCount)
                                return PROBES_DONE;
                            centre.Copy(offsetCentre + probes[*probeIndex]);
                            moved = 1;
                        }
                        else
                        {
                            distance = centre.DistanceSquaredToSegment(cornerA, cornerB);
                            if (distance >= (double)radiusSquared)
                            {
                                distance = centre.DistanceSquaredToSegment(cornerA, cornerC);
                                if (distance >= (double)radiusSquared)
                                {
                                    distance = centre.DistanceSquaredToSegment(cornerC, cornerB);
                                    if (distance >= (double)radiusSquared)
                                        break;
                                }
                            }
                            *nearest = distance;
                            if (node->group != GROUP_PLAIN)
                            {
                                if (type == PROBE_KIND_COUNTED)
                                {
                                    if (node->group != GROUP_COUNTED)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                        break;
                                    }
                                    if (DAT_11b0450c < COUNTED_TOUCH_LIMIT)
                                        DAT_11b0450c++;
                                    if (DAT_11b0450c > COUNTED_TOUCH_LIMIT - 1)
                                        break;
                                }
                                else if (node->group != GROUP_OPEN)
                                {
                                    if (type != 0)
                                    {
                                        if (ReportContact(node, type))
                                            return PROBES_DONE;
                                    }
                                    break;
                                }
                            }
                            if (mapKind != 0)
                            {
                                *hitFlag = 1;
                                if (hitMatrix != 0)
                                    *hitMatrix = (int)&node->worldMatrix;
                                return PROBES_DONE;
                            }
                            *hitFlag = 0;
                            if (resolve != 0)
                            {
                                Vec4 push;
                                push.Copy(TriangleNormal(cornerA, cornerB, cornerC));
                                push.Normalized3();
                                probes[*probeIndex].Copy(probes[*probeIndex] + push * PROBE_PUSH_DISTANCE);
                                centre.Copy(offsetCentre + probes[*probeIndex]);
                                pushes++;
                                if (pushes <= PROBE_PUSH_LIMIT)
                                    continue;
                                probes[*probeIndex].Fill(0);
                            }
                            *probeIndex = *probeIndex + 1;
                            DAT_11b002b4 = isCamera;
                            if (*probeIndex >= probeCount)
                                return PROBES_DONE;
                            centre.Copy(offsetCentre + probes[*probeIndex]);
                            moved = 1;
                        }
                    }
                }
            }
        }
    }
passDown:
    if (node->isStatic != 0)
    {
        matrixPosition.Copy(matrix.GetTranslation());
        offsetCentre.Copy(world->GetTranslation() + matrixPosition);
        centre.Copy(offsetCentre + probes[*probeIndex]);
    }
scanSub:
    int result;
    if (node->isStatic != 0)
    {
        for (int i = 0; i < node->staticCount; i++)
        {
            Node *sub = node->staticNodes[i];
            result = *probeIndex;
            if (DAT_11b002c8 == 0 && sub->m_0088 != 0)
                ;
            else if (sub->kind == NODE_KIND_NONE
                         ? (DAT_11b002c0 != 0 && DAT_11b04514 != 0 && sub->name[0] == 'C' && sub->name[1] == 'D' &&
                            sub->name[2] == 'L' &&
                            ((char *)DAT_11b002c0)[(sub->name[3] - '0') * REGION_DIGIT_TENS + (sub->name[4] - '0')] == 0)
                         : (DAT_11b002c0 != 0 && DAT_11b04514 != 0 && kind < VISIBILITY_CELL_COUNT &&
                            ((char *)DAT_11b002c0)[kind] == 0))
                ;
            else if ((sub->flags & mask) == 0)
                ;
            else if (touchGroups == 0 && sub->group != GROUP_OPEN && sub->group != GROUP_PLAIN)
                ;
            else if (type != PROBE_KIND_COUNTED && sub->group != GROUP_PLAIN && sub->group != GROUP_OPEN &&
                     sub->group < GROUP_RANGE_START && sub->group > GROUP_RANGE_END)
                ;
            else
            {
                worldPosition.Copy(sub->worldMatrix.GetTranslation());
                if (sub->group != GROUP_QUIET)
                {
                    range = reach + sub->bounds.w + NODE_BOUNDS_SLACK;
                    if (!(centre.DistanceSquared(sub->bounds + worldPosition) > range * range))
                    {
                        int skippedFlag = 0;
                        result = CollideProbesWithMesh(mask, world, sub, reach, nearest, probes, probeCount, probeIndex,
                                                       hitPoint, hitNormal, flags, hitFlag, resolve, hitMatrix,
                                                       &skippedFlag);
                        if (DAT_11b002c8 != 0)
                        {
                            if (skippedFlag == 0)
                                sub->m_0088 = 1;
                            else
                                sub->m_0088 = 0;
                        }
                    }
                }
            }
            if (result < 0)
                goto failed;
        }
    }
    else
    {
        for (int i = 0; i < node->childCount; i++)
        {
            result = CollideProbesWithMesh(mask, world, node->children[i], reach, nearest, probes, probeCount,
                                           probeIndex, hitPoint, hitNormal, flags, hitFlag, resolve, hitMatrix,
                                           skipped);
            if (result < 0)
                goto failed;
        }
    }
    if (node->visibilityCells != 0)
        DAT_11b04514 = 0;
    return *probeIndex;
failed:
    if (node->visibilityCells != 0)
        DAT_11b04514 = 0;
    return result;
regionOff:
    if (DAT_11b002c8 != 0 && skipped != 0)
        *skipped = 1;
    return 0;
}
