// Sweeping a move through the map to find where it first runs into the world.

#include "vec4.h"

#include "matrix4.h"
#include "node.h"
#include "map_vars.h"

// Size of the sphere the sweep tests against the world, and of the margin kept from the map limits.
#define SWEEP_RADIUS 30.0f
// Length of one sweep step.
#define SWEEP_STEP 5.0f
// Highest z a swept sphere may reach.
#define SWEEP_CEILING 2000.0f

// 0x10034210 (Move_SweepToWorld). Walks from `start` along `step` in SWEEP_STEP increments (the last one clipped to
// the length of `step`), testing a sphere at each stop against the world collision and then against the map's
// limits. On the first contact `normal` gets the surface normal, `depth` the penetration, `reached` the displacement
// walked so far and, when given, `hitDirection` and a copy of the normal both get their components rounded after
// normalising. Returns 0 when the whole move is free, 1 on a hit (or a map limit), 2 when the hit was one of the
// "@ami" nodes.
int FUN_10034210(const Vec4 *start, Vec4 *step, float unusedRadius, Vec4 *normal, float *depth,
                            Vec4 *reached, Vec4 *hitDirection)
{
    Vec4 position;
    Matrix4 matrix;
    Vec4 contactNormal;
    Vec4 nodeNormal;
    Vec4 rounded;
    float travelled = 0.0f;
    int arrived = 0;
    float penetration;
    float nodeDepth;
    matrix.SetIdentity();
    position.Copy(*start);
    while ((position - *start).Length() < step->Length())
    {
        if (arrived != 0)
            break;
        travelled += SWEEP_STEP;
        if (step->Length() < travelled)
        {
            travelled = step->Length();
            arrived = 1;
        }
        position.Copy(*step);
        position.Normalized3();
        position.Copy(position * travelled);
        reached->Copy(position);
        position.Copy(position + *start);
        if (DAT_10255f08 != 0 && DAT_10255f08->Query(&matrix, &position, SWEEP_RADIUS, &contactNormal, &penetration, 0, 0) != 0)
        {
            int index = 0;
            int onNode = 0;
            Node *node = DAT_10255f08->Find("@ami", 0);
            while (node != 0)
            {
                if (node->Query(&matrix, &position, SWEEP_RADIUS, &nodeNormal, &nodeDepth, 0, 0) != 0)
                {
                    onNode = 1;
                    contactNormal.Copy(nodeNormal);
                    penetration = nodeDepth;
                    break;
                }
                index++;
                node = DAT_10255f08->Find("@ami", index);
            }
            if (hitDirection != 0)
            {
                rounded.Copy(contactNormal);
                hitDirection->Normalized3();
                rounded.Normalized3();
                hitDirection->x = (float)(int)(hitDirection->x + 0.5f);
                hitDirection->y = (float)(int)(hitDirection->y + 0.5f);
                hitDirection->z = (float)(int)(hitDirection->z + 0.5f);
                rounded.x = (float)(int)(rounded.x + 0.5f);
                rounded.y = (float)(int)(rounded.y + 0.5f);
                rounded.z = (float)(int)(rounded.z + 0.5f);
            }
            normal->Copy(contactNormal);
            *depth = penetration;
            if (onNode != 0)
                return 2;
            return 1;
        }
        contactNormal.Fill(0.0f);
        penetration = 0.0f;
        int outside = 0;
        float low = position.x - SWEEP_RADIUS;
        float limit = (float)DAT_10210ff0;
        if (low < limit)
        {
            contactNormal.x = 1.0f;
            penetration = limit - low;
            outside = 1;
        }
        float high = position.x + SWEEP_RADIUS;
        limit = (float)DAT_10210ff4;
        if (high > limit)
        {
            contactNormal.x = -1.0f;
            limit = limit - high;
            outside = 1;
            if (penetration < limit)
                penetration = limit;
        }
        low = position.y - SWEEP_RADIUS;
        limit = (float)DAT_10210ffc;
        if (low < limit)
        {
            contactNormal.y = 1.0f;
            limit = limit - low;
            outside = 1;
            if (penetration < limit)
                penetration = limit;
        }
        high = position.y + SWEEP_RADIUS;
        limit = (float)DAT_10210ff8;
        if (high > limit)
        {
            contactNormal.y = -1.0f;
            limit = limit - high;
            outside = 1;
            if (penetration < limit)
                penetration = limit;
        }
        low = position.z - SWEEP_RADIUS;
        if (low < 0.0f)
        {
            contactNormal.z = 1.0f;
            low = -low;
            outside = 1;
            if (penetration < low)
                penetration = low;
        }
        if (position.z + SWEEP_RADIUS > SWEEP_CEILING)
        {
            contactNormal.z = -1.0f;
            limit = SWEEP_CEILING - (position.y + SWEEP_RADIUS);
            if (penetration < limit)
                penetration = limit;
            outside = 1;
        }
        if (outside != 0)
        {
            normal->Copy(contactNormal);
            *depth = penetration;
            return 1;
        }
    }
    return 0;
}
