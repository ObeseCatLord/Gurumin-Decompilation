// Looking for a ledge or wall beside a character (0x10050df0, called by 0x1006be40): the ground below it, a wall to
// its side and the ground beyond that wall.

#include <math.h>

#include "vec4.h"

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct CharRecord
{
    char pad_0000[0x2358];
    Vec4 position;            // 0x2358: the world matrix's translation row
    char pad_2368[0x114];
    float heading;            // 0x247c: facing angle in radians
    char pad_2480[0x1c];
};

typedef char check_charrecord[sizeof(CharRecord) == 0x249c ? 1 : -1];

struct CollisionWorld
{
    int RayCast(const char *name, unsigned kind, unsigned mask, const Vec4 *to, const Vec4 *from, Vec4 *hit,
                float *t);                                              // 0x101a9670
    int CastSegment(const char *name, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t);   // 0x101aaa20
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" CollisionWorld *DAT_10255f08;
extern "C" int FUN_100248e0(Vec4 position, Vec4 *ground, float height);     // Collision_ProbeDown

// 0x10050df0 (Char_ProbeSideLedge). Casts down from character `index` for the ground (*ground; *drop = its depth
// below the character, -1 without ground), then from 10 units above that ground, pulled 10 units back, a ray
// `reach` units to the character's left. With a wall hit (*wall, *wallDistance) it looks for ground 1000 units below
// a point 100 units further left: returns 2 with *height = the floored height of that ground above the wall hit,
// or 1 without ground there. Returns 0 without ground or wall (*wallDistance = -1 when the ground was found).
extern "C" int FUN_10050df0(int index, Vec4 *wall, Vec4 *ground, float *wallDistance, float *height, float *drop,
                            float reach)
{
    Vec4 from;
    Vec4 side;
    Vec4 forward;
    Vec4 wallHit;
    from.x = DAT_10db6ee8[index].position.x;
    from.y = DAT_10db6ee8[index].position.y;
    from.z = DAT_10db6ee8[index].position.z + 10.0f;
    from.w = DAT_10db6ee8[index].position.w;
    side.Fill(0.0f);
    side.y = reach;
    side.RotateZ(-DAT_10db6ee8[index].heading - 1.5708f);
    forward.Copy(side);
    forward.Normalized3();
    *drop = -1.0f;
    {
        Vec4 top;
        Vec4 bottom;
        Vec4 unused;
        Vec4 floor;
        floor.Fill(0.0f);
        top = DAT_10db6ee8[index].position;
        floor.x = -200000.0f;
        bottom.Copy(top);
        bottom.z -= 100000.0f;
        if (DAT_10255f08 != 0)
            DAT_10255f08->CastSegment(0, 0x4000000, &bottom, &top, &floor, 0);
        from.Copy(floor);
        if (floor.x == -200000.0f)
            return 0;
    }
    ground->Copy(from);
    *drop = fabsf(DAT_10db6ee8[index].position.z - from.z);
    from.z += 10.0f;
    from.Copy(from - forward * 10.0f);
    side.Copy(side + from);
    if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(0, 4, 0x4000000, &side, &from, &wallHit, 0) != 0)
    {
        float distance = (wallHit - from).Length();
        if (distance == -1.0f)
        {
            if (wallDistance != 0)
                *wallDistance = -1.0f;
            return 0;
        }
        if (wall != 0)
            wall->Copy(wallHit);
        if (wallDistance != 0)
            *wallDistance = distance;
        side.Fill(0.0f);
        side.y = 100.0f;
        side.RotateZ(-DAT_10db6ee8[index].heading - 1.5708f);
        from.Copy(wallHit + side);
        if (FUN_100248e0(from, &forward, 1000.0f) == 0)
            return 1;
        if (height != 0)
            *height = floorf(forward.z - wallHit.z);
        return 2;
    }
    if (wallDistance != 0)
        *wallDistance = -1.0f;
    return 0;
}
