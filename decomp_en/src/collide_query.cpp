// World collision queries (EN build). Ported from the matched JP source decomp/src/collide_query.cpp.
#include "collide_query.h"
#include "anim_model.h"
#include "char.h"
#include "char_shots.h"
#include "node.h"
#include "visibility.h"

// Collision_GroundAhead: ledge test: 1 when there is world collision below the point `distance` units along
// `direction` from `position`. The ray runs from 90 units above that point to `depth` units below it. Enemy AI uses
// it (depth 300, distance 15) to stop at edges. Returns 0 when nothing is hit or no world is loaded.
extern "C" int Collision_GroundAhead(const Vec4 *position, Vec4 *direction, float depth, int distance)
{
    Vec4 from;
    Vec4 to;
    Vec4 hit;
    from.Copy(*direction * (float)distance);
    from.Copy(from + *position);
    to.Copy(from);
    from.z += LEDGE_PROBE_HEIGHT;
    to.z -= depth;
    if (g_mapRootNode != 0 && g_mapRootNode->Collide(0, COLLISION_MASK_WORLD, &from, &to, &hit) != 0)
        return 1;
    return 0;
}

// Collision_ProbeDown: casts a ray down from `height` units above `position` to `position` against the world
// collision and stores the result in `ground`. Returns 1 on a hit, 0 when nothing was hit (`ground` then has
// x = -200000) or no world is loaded. On a hit the code copies `unused` (a zero vector) over the hit point and
// computes a length it throws away, so callers only learn whether something was hit.
extern "C" int Collision_ProbeDown(Vec4 position, Vec4 *ground, float height)
{
    Vec4 from;
    Vec4 unused;
    Vec4 hit;
    float zero = 0;
    hit.Fill(zero);
    hit.x = PROBE_NO_HIT;
    from.Copy(position);
    from.z += height;
    if (g_mapRootNode != 0 && g_mapRootNode->CastSegment(0, COLLISION_MASK_WORLD, &from, &position, &hit, 0) != 0)
    {
        hit.Copy(unused);
        (hit - position).Length();
    }
    ground->Copy(hit);
    if (hit.x == PROBE_NO_HIT)
        return 0;
    return 1;
}

// Char_CastRayDown: casts a ray down `depth` units from character `index` (from its head node when `fromHead` is set
// and it has one, else from its position, 30 units up when `fromHead` is set), moved by `offset` when given, against
// the world collision (or the small objects with `smallObjects`). With `testOnly` only whether something is hit is
// returned; otherwise the hit point goes to `ground` (x = -200000 when there is none). Returns 1 on a hit, else 0.
extern "C" int Char_CastRayDown(int index, Vec4 *ground, float depth, int fromHead, const Vec4 *offset, int testOnly,
                                int smallObjects)
{
    Vec4 from;
    Vec4 to;
    Vec4 unused;
    Vec4 hit;
    float zero = 0;
    hit.Fill(zero);
    unsigned int mask = smallObjects != 0 ? COLLISION_MASK_SMALL_OBJECTS : COLLISION_MASK_WORLD;
    hit.x = PROBE_NO_HIT;
    if (fromHead != 0 && g_characterModels[index].headNode != 0)
    {
        from.Copy(g_characterModels[index].headNode->worldMatrix.GetTranslation());
    }
    else
    {
        from.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        from.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
        from.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z];
        from.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
        if (fromHead != 0)
            from.z += HEAD_PROBE_LIFT;
    }
    if (offset != 0)
        from.Copy(from + *offset);
    to.Copy(from);
    to.z -= depth;
    if (testOnly != 0)
    {
        if (g_mapRootNode != 0 && g_mapRootNode->Collide(0, mask, &to, &from, &hit) != 0)
            return 1;
        return 0;
    }
    if (g_mapRootNode != 0)
        g_mapRootNode->CastSegment(0, mask, &to, &from, &hit, 0);
    if (ground != 0)
        ground->Copy(hit);
    if (hit.x == PROBE_NO_HIT)
        return 0;
    return 1;
}

// Collision_ProbePoint: 1 when a sphere of `radius` at the point (x, y, z) touches the world collision. C++ linkage
// on purpose: the Matrix4 local gives the function its exception frame. Called by the effect and projectile code.
int Collision_ProbePoint(float x, float y, float z, float radius)
{
    Matrix4 matrix;
    matrix.SetIdentity();
    Vec4 point;
    Vec4 hit;
    float depth;
    point.Set(x, y, z, 0);
    return g_mapRootNode->Query(&matrix, &point, radius, &hit, &depth, 0, 0);
}

// Slot_HasNoGround: every tenth frame (spread over the slots by slot number) casts a ray at the point 15 units along
// slot `slot`'s vector from `point`, from 300 below to 50 above it, against the world collision (filtered by the
// slot's collision filter), and records the hit point in g_slotGroundPoint or that there is no ground. Returns 1 when
// the slot has no ground below it (from this or an earlier test), else 0. Called by the ledge-aware movers.
extern "C" int Slot_HasNoGround(int slot, const Vec4 *point)
{
    if (g_frameCounter % GROUND_TEST_PERIOD == slot % GROUND_TEST_PERIOD)
    {
        Vec4 low;
        Vec4 high;
        Vec4 hit;
        low.Copy(g_shotSlots[slot].position * 15.0f);  // 15 = GROUND_TEST_AHEAD; a named constant would be passed by address
        low.Copy(low + *point);
        high.Copy(low);
        high.z += GROUND_TEST_UP;
        low.z -= GROUND_TEST_DOWN;
        g_collisionFilter = g_characterModels[slot].collisionFilter;
        if (g_mapRootNode != 0 && g_mapRootNode->CastSegment(0, COLLISION_MASK_WORLD, &low, &high, &hit, 0) != 0)
        {
            g_slotGroundPoint[slot].Copy(hit);
            g_slotNoGround[slot] = 0;
        }
        else
        {
            g_slotNoGround[slot] = 1;
        }
        g_collisionFilter = 0;
    }
    return g_slotNoGround[slot] != 0;
}
