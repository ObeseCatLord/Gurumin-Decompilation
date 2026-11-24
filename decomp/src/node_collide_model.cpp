// Recursive collision test between two scene models (their node trees), with a probe offset per test.

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "node_collision.h"

#include "render_vars.h"

#define NODE_FLAG_REGION_MASK 0xe00      // any of the three volume kinds below
#define NODE_FLAG_VOLUME_BOX 0x200
#define NODE_FLAG_VOLUME_SPHERE 0x400
#define NODE_FLAG_VOLUME_CYLINDER 0x800
#define NODE_FLAG_MAP_PROBE 0x100        // on this node: a mapHit request is honoured
#define NODE_FLAG_MAP_KIND 0x4000000     // on the other node: it is map collision
#define NODE_DISTANCE_LIMIT_SQUARED 640000.0f  // nodes further apart than this (squared) cannot touch
#define COLLIDE_MISSED_RETRIED (-2)      // no touch, after a probe already moved on
#define COLLIDE_STOP (-1)                // probes exhausted, or a map hit recorded
#define CONTACT_PASS_EVENT 0x1869b
#define CONTACT_FAIL_EVENT 0x1869f
#define CONTACT_CODE_LIMIT 0x5c          // colour values from here are not contact codes
#define TOUCHED_PUSH_SCALE 0.9f
#define PUSH_STEP 1.0f
#define MODE_COUNT_ONLY (-1)
#define MODE_REPORT 1

// 0x10193140 (Node::CollideModel). Tests this node's tree (placed by `matrix` plus the probe offset) against the
// tree under `other`. Nodes are skipped when a skinned node is involved (unless DAT_11b04510), and nodes without a
// volume kind only recurse into their children. For two nodes with volumes the offsets of both are built from the
// node matrices and the pair is dropped when they are further apart than 800 units. The volumes (box, sphere,
// cylinder) are then tested; on overlap the push direction is kept and the colour classes (m_189b/m_189a) must match
// both ways. `mode` 1 reports the pair through DAT_11b044fc, -1 only applies the class test. A touched model is
// appended to `touchedModels` (with the probe scaled by 0.9 in `touchedPushes`). With `resolve` the probe at
// `*probeIndex` is moved away along the push direction (straight up for map collision) and the same pair is tested
// again; otherwise the next probe is taken. A map hit stores the address of the other node's world matrix in `hitMatrix` (an int slot: LiftProbes keeps it as one), sets
// `*mapHit` and returns -1.
// Returns -1 when all probes are used up or a map hit was found, -2 when the pair was missed after a probe had
// already moved on, otherwise the current probe index.
int Node::CollideModel(Matrix4 *matrix, Node *other, Matrix4 *otherMatrix, Vec4 *probes, int probeCount,
                       int *probeIndex, AnimModel **touchedModels, Vec4 *touchedPushes, int *touchedCount, int mode,
                       int *mapHit, int resolve, int *hitMatrix, unsigned int mask)
{
    int moved = 0;
    if (DAT_11b04510 == 0 && (skinned != 0 || other->skinned != 0))
        return *probeIndex;
    if ((flags & NODE_FLAG_REGION_MASK) == 0)
    {
        for (int i = 0; i < childCount; i++)
        {
            int result = children[i]->CollideModel(matrix, other, otherMatrix, probes, probeCount, probeIndex,
                                                   touchedModels, touchedPushes, touchedCount, mode, mapHit, resolve,
                                                   hitMatrix, mask);
            if (result < 0)
                return result;
        }
        return *probeIndex;
    }
    Vec4 push;
    Vec4 delta;
    Vec4 negatedDelta;
    Vec4 offsetA;
    Vec4 offsetB;
    if ((other->flags & NODE_FLAG_REGION_MASK) != 0 && (mask == 0 || (other->flags & mask) != mask))
    {
        unsigned int mapContact = other->flags & NODE_FLAG_MAP_KIND;
        if (mapHit == 0 || (flags & NODE_FLAG_MAP_PROBE) == 0)
            mapContact = 0;
        Matrix4 *otherWorld = &other->worldMatrix;
        for (;;)
        {
            if (parent != 0 && parent->skinned != 0)
                offsetA.Copy(worldMatrix.GetTranslation() + probes[*probeIndex]);
            else if (frameMatrices != 0)
            {
                Matrix4 placed = *matrix;
                placed._41 += probes[*probeIndex].x;
                placed._42 += probes[*probeIndex].y;
                placed._43 += probes[*probeIndex].z;
                placed = frameMatrices[frame] * placed;
                offsetA.Copy(placed.GetTranslation());
            }
            else
            {
                Matrix4 placed = *matrix;
                placed._41 += probes[*probeIndex].x;
                placed._42 += probes[*probeIndex].y;
                placed._43 += probes[*probeIndex].z;
                placed = this->matrix * placed;
                offsetA.Copy(placed.GetTranslation());
            }
            offsetB.Copy(otherWorld->GetTranslation());
            if (offsetA.DistanceSquared(offsetB) > NODE_DISTANCE_LIMIT_SQUARED)
                goto missed;
            if ((flags & NODE_FLAG_VOLUME_CYLINDER) && (other->flags & NODE_FLAG_VOLUME_CYLINDER))
            {
                if (!CylindersOverlap(&box[NODE_BOX_MIN], &offsetA, &other->box[NODE_BOX_MIN], &offsetB, &delta,
                                      &negatedDelta))
                    goto missed;
                push.Copy(delta);
            }
            else if ((flags & NODE_FLAG_VOLUME_CYLINDER) && (other->flags & NODE_FLAG_VOLUME_BOX))
            {
                if (!BoxCylinderOverlap(&other->box[NODE_BOX_MIN], &offsetB, &box[NODE_BOX_MIN], &offsetA, &delta,
                                        &negatedDelta))
                    goto missed;
                push.Copy(negatedDelta);
            }
            else if ((flags & NODE_FLAG_VOLUME_BOX) && (other->flags & NODE_FLAG_VOLUME_CYLINDER))
            {
                if (!BoxCylinderOverlap(&box[NODE_BOX_MIN], &offsetA, &other->box[NODE_BOX_MIN], &offsetB, &delta,
                                        &negatedDelta))
                    goto missed;
                push.Copy(delta);
            }
            else if ((flags & NODE_FLAG_VOLUME_CYLINDER) && (other->flags & NODE_FLAG_VOLUME_SPHERE))
            {
                if (!CylinderSphereOverlap(&box[NODE_BOX_MIN], &offsetA, &other->bounds, &offsetB, &delta,
                                           &negatedDelta))
                    goto missed;
                push.Copy(delta);
            }
            else if ((flags & NODE_FLAG_VOLUME_SPHERE) && (other->flags & NODE_FLAG_VOLUME_CYLINDER))
            {
                if (!CylinderSphereOverlap(&other->box[NODE_BOX_MIN], &offsetB, &bounds, &offsetA, &delta,
                                           &negatedDelta))
                    goto missed;
                push.Copy(negatedDelta);
            }
            else if ((flags & NODE_FLAG_VOLUME_BOX) && (other->flags & NODE_FLAG_VOLUME_BOX))
            {
                if (!BoxesOverlap(&box[NODE_BOX_MIN], &offsetA, &other->box[NODE_BOX_MIN], &offsetB, &delta,
                                  &negatedDelta))
                    goto missed;
                push.Copy(delta);
            }
            else if ((flags & NODE_FLAG_VOLUME_BOX) && (other->flags & NODE_FLAG_VOLUME_SPHERE))
            {
                if (!BoxSphereOverlap(&box[NODE_BOX_MIN], &offsetA, &other->bounds, &offsetB, &delta, &negatedDelta))
                    goto missed;
                push.Copy(delta);
            }
            else if ((flags & NODE_FLAG_VOLUME_SPHERE) && (other->flags & NODE_FLAG_VOLUME_BOX))
            {
                if (!BoxSphereOverlap(&other->box[NODE_BOX_MIN], &offsetB, &bounds, &offsetA, &delta, &negatedDelta))
                    goto missed;
                push.Copy(negatedDelta);
            }
            else if ((flags & NODE_FLAG_VOLUME_SPHERE) && (other->flags & NODE_FLAG_VOLUME_SPHERE))
            {
                if (!SpheresOverlap(&bounds, &offsetA, &other->bounds, &offsetB, &delta, &negatedDelta))
                    goto missed;
                push.Copy(delta);
            }
            if ((m_189b & (other->m_189b >> 4)) != 0 && (other->m_189b & (m_189b >> 4)) != 0)
            {
                if (mode == MODE_REPORT)
                {
                    if ((m_189a & (other->m_189a >> 4)) == 0 || (other->m_189a & (m_189a >> 4)) == 0)
                    {
                        if (other->m_189a != 0 && m_189a != 0 && DAT_11b044fc != 0)
                        {
                            unsigned char code = other->m_189c;
                            if (code >= 1 && code < CONTACT_CODE_LIMIT)
                                DAT_11b044fc(CONTACT_FAIL_EVENT, code, 1);
                            else
                                DAT_11b044fc(CONTACT_FAIL_EVENT, m_189c, 1);
                        }
                        goto notTouching;
                    }
                    if (DAT_11b044fc != 0)
                    {
                        if (other->m_189c != 0)
                            DAT_11b044fc(CONTACT_PASS_EVENT, other->m_189c, 1);
                        else
                            DAT_11b044fc(CONTACT_PASS_EVENT, m_189c, 1);
                    }
                }
                else if (mode == MODE_COUNT_ONLY)
                {
                    if ((m_189a & (other->m_189a >> 4)) == 0 || (other->m_189a & (m_189a >> 4)) == 0)
                        goto notTouching;
                }
                if (model != 0 && touchedCount != 0)
                {
                    int count = *touchedCount;
                    int i;
                    for (i = 0; i < *touchedCount; i++)
                        if (touchedModels[i] == other->model)
                            break;
                    if (i == count)
                    {
                        touchedModels[*touchedCount] = other->model;
                        touchedPushes[*touchedCount].Copy(probes[*probeIndex] * TOUCHED_PUSH_SCALE);
                        ++*touchedCount;
                    }
                }
                if (resolve != 0)
                {
                    if (other->group == 0)
                    {
                        if (mapContact != 0 && mapHit != 0)
                        {
                            push.x = 0.0f;
                            push.y = 0.0f;
                            push.z = 1.0f;
                            probes[*probeIndex].Copy(probes[*probeIndex] + push * PUSH_STEP);
                            continue;
                        }
                        if (push.x != 0.0f || push.y != 0.0f || push.z != 0.0f)
                        {
                            push.Normalized3();
                            probes[*probeIndex].Copy(probes[*probeIndex] + push * PUSH_STEP);
                            continue;
                        }
                    }
                    else if (other->group == 1)
                    {
                        if (mapContact != 0 && mapHit != 0)
                        {
                            push.x = 0.0f;
                            push.y = 0.0f;
                            push.z = 1.0f;
                            probes[*probeIndex].Copy(probes[*probeIndex] + push * PUSH_STEP);
                            continue;
                        }
                        if (push.x != 0.0f || push.y != 0.0f || push.z != 0.0f)
                        {
                            push.Normalized3();
                            probes[*probeIndex].Copy(probes[*probeIndex] + push * PUSH_STEP);
                            probes[*probeIndex].z = 0;
                            continue;
                        }
                    }
                }
                if (mapContact != 0 && mapHit != 0)
                {
                    *hitMatrix = (int)otherWorld;
                    *mapHit = 1;
                    return COLLIDE_STOP;
                }
                ++*probeIndex;
                if (*probeIndex >= probeCount)
                    return COLLIDE_STOP;
                moved = 1;
                continue;
            }
        notTouching:
            if (mapContact != 0 && mapHit != 0)
            {
                *mapHit = 1;
                return COLLIDE_STOP;
            }
        missed:
            if (moved)
                return COLLIDE_MISSED_RETRIED;
            break;
        }
    }
    for (int i = 0; i < other->childCount; i++)
    {
        int result = CollideModel(matrix, other->children[i], otherMatrix, probes, probeCount, probeIndex,
                                  touchedModels, touchedPushes, touchedCount, mode, mapHit, resolve, hitMatrix, mask);
        if (result < 0)
            return result;
    }
    return *probeIndex;
}
