// Moves a model's probe points out of whatever blocks them (AnimModel::LiftProbes).

#include <math.h>
#include <stdlib.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "char_vars.h"
#include "render_vars.h"

#define LIFT_NEARBY_DISTANCE_SQUARED 562500.0f  // models farther than 750 units are not tested
#define LIFT_HIDDEN_X -50000.0f                 // a node placed below this is not in the scene
#define LIFT_NO_FLOOR -30000.0f
#define LIFT_NO_HIT_DISTANCE 10000.0f
#define LIFT_MAX_PROBES 32
#define LIFT_MAX_MAP_STEPS 500
#define LIFT_MAX_RESTARTS 16

enum LiftMode
{
    LIFT_UNTIL_CLEAR = 1,   // every blocked probe is raised and the lifts are counted in the result
    LIFT_REPORT_HITS = 2    // like 1, and the hit's data is reported through outHit and outFlag
};

enum ContactState
{
    CONTACT_MAP = 2,
    CONTACT_MODEL = 3
};

// 0x101643c0 (AnimModel::LiftProbes). Raises the probe points `probes` (probeCount of them, z is raised) by the
// step of m_2390 (4, 2 or 1) until they no longer touch the other loaded models and then the map collision. The
// distance to the nearest map hit is lowered into `nearest`. Returns the result of the last collision test (-1 =
// blocked for good, otherwise the probe that was last raised), 0 when the model has no map collision.
int AnimModel::LiftProbes(float *nearest, Vec4 *probes, int probeCount, AnimModel **hitIds, Vec4 *hits, int *hitOther,
                          int unused1, unsigned int flags, int unused2, int mode, int extra, int *outHit,
                          int *outFlag)
{
    int result = 0;
    int probeIndex;
    float stepSize;
    int step = 4;
    int liftCount;
    int mapSteps;
    float nearestHit;
    int restarts;
    int lastResult;
    if (m_2390 == 0)
        step = 2;
    if (m_2390 == 1)
        step = 1;
    lifted = 0;
    nearestHit = LIFT_NO_HIT_DISTANCE;
    if (collider == 0 || m_231c == 0)
        return 0;
    Vec4 hitPoint;
    Vec4 hitNormal;
    float floors[LIFT_MAX_PROBES];
    probeIndex = 0;
    mapSteps = 0;
    for (int i = 0; i < probeCount; i++)
        floors[i] = LIFT_NO_FLOOR;
    *hitIds = 0;
    liftCount = 0;
    *hitOther = 0;
    lastResult = 0;
    restarts = 0;
    for (;;)
    {
        int hitData;
        int hit;
        Vec4 position = matrix.GetTranslation();
        for (int i = 0; i < 0x100; i++)
        {
            if (DAT_10faaa88[i] == 0 || root == DAT_10faaa88[i] || DAT_10faaa88[i]->hasCollision == 0 ||
                !(DAT_10faaa88[i]->matrix._41 > LIFT_HIDDEN_X))
                continue;
            if (!(position.DistanceSquared(DAT_10faaa88[i]->worldMatrix.GetTranslation()) < LIFT_NEARBY_DISTANCE_SQUARED))
                continue;
            if (m_2474 == 0 && liftCount != 0 && DAT_10faaa88[i]->model->m_2418 == this)
            {
                int otherOwner = *hitOther;
                hitData = probeIndex;
                int count = root->CollideModel(&matrix, DAT_10faaa88[i], DAT_10ff19e8[i], probes, probeCount, &hitData, hitIds,
                                         hits, &otherOwner, 1, &hit, extra, 0, 0);
                if (count != hitData)
                    DAT_10faaa88[i]->model->m_0290 = (float)(liftCount + 2);
                continue;
            }
            for (;;)
            {
                hit = 0;
                hitData = 0;
                result = root->CollideModel(&matrix, DAT_10faaa88[i], DAT_10ff19e8[i], probes, probeCount, &probeIndex, hitIds, hits,
                                      hitOther, 1, &hit, extra, &hitData, 0);
                if ((result == -1 || result == -2 || hit != 0) && contactState != 0)
                    contactState = CONTACT_MODEL;
                if (mode == LIFT_UNTIL_CLEAR)
                {
                    if (hit == 0)
                        break;
                    liftCount += step;
                    lifted = 1;
                    stepSize = (float)step;
                    probes[probeIndex].z += stepSize;
                    if (outHit != 0)
                        *outHit = hitData;
                }
                else
                {
                    if (mode != LIFT_REPORT_HITS || hit == 0)
                        break;
                    if (outHit != 0)
                        *outHit = hitData;
                    stepSize = (float)step;
                    probes[probeIndex].z += stepSize;
                    if (outFlag != 0)
                        *outFlag = 1;
                }
            }
            if (m_0075 != 0 && restarts < LIFT_MAX_RESTARTS && (result == -1 || result == -2))
            {
                stepSize = (float)step;
                restarts++;
                probeIndex = 0;
                probes[0].z += stepSize;
                goto restart;
            }
            if (result == -1)
                break;
            if (result == -2)
                goto restart;
        }
        DAT_11b002c0 = collisionFilter;
        if (result == -1 || (result >= 0 && lastResult != 0 && lastResult - 1 == result))
            break;
        lastResult = result + 1;
        do
        {
            hit = 0;
            hitData = 0;
            if (mapTested == 0)
                DAT_11b002c8 = 1;
            result = collider->M_1019f340(-1, &matrix, m_231c, colliderReach, &nearestHit, probes, probeCount,
                                          &probeIndex, &hitPoint, &hitNormal, m_023c | flags, &hit, extra,
                                          &hitData, 0);
            if (mapTested == 0)
            {
                DAT_11b002c8 = 0;
                mapTested = 1;
            }
            if ((result == -1 || result == -2 || hit != 0) && contactState != 0)
            {
                m_0000.Copy(hitNormal);
                contactState = CONTACT_MAP;
            }
            if (mode == LIFT_UNTIL_CLEAR)
            {
                if (hit == 0)
                    break;
                lifted = 1;
                liftCount += step;
                if (probes[probeIndex].z < floors[probeIndex])
                    probes[probeIndex].z = floors[probeIndex];
                stepSize = (float)step;
                probes[probeIndex].z += stepSize;
                floors[probeIndex] = probes[probeIndex].z;
            }
            else
            {
                if (mode != LIFT_REPORT_HITS || hit == 0)
                    break;
                if (outHit != 0)
                    *outHit = hitData;
                if (probes[probeIndex].z < floors[probeIndex])
                    probes[probeIndex].z = floors[probeIndex];
                stepSize = (float)step;
                probes[probeIndex].z += stepSize;
                floors[probeIndex] = probes[probeIndex].z;
                if (outFlag != 0)
                    *outFlag = 1;
            }
            mapSteps++;
        } while (mapSteps < LIFT_MAX_MAP_STEPS);
        if (result == -2)
            result = probeIndex;
        else if (liftCount == 0 || result == -1)
            break;
    restart:;
    }
    if (nearestHit < *nearest)
        *nearest = nearestHit;
    DAT_11b002c0 = 0;
    return result;
}
