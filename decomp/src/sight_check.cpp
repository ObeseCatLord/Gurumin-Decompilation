// Whether a character can see a point: within a range (heights count 2.5 times), within an angle of its heading or
// very near, and optionally with no wall in between.

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    float Length();                              // 0x101ac4e0: length of x/y/z
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

struct CollisionWorld
{
    // 0x101a9670: casts a segment from `from` to `to` against the faces in `mask`; returns non-zero on a hit.
    int Raycast(int a, int b, int mask, const Vec4 *from, const Vec4 *to, Vec4 *hit, int c);
};

extern "C" CollisionWorld *DAT_10255f08;         // the collision world (0 before a map is loaded)

// 0x10043e20 (Char_CanSee). Returns 1 when `eye` (looking along `heading`) sees `target`: their distance, with the
// height difference counted 2.5 times, is at most `range`, and the target lies within `angleRange` of the heading
// or within `nearRange`. With `checkWalls` the segment from `eye` to `target` must also be free of walls. Else 0.
extern "C" int FUN_10043e20(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange,
                            float nearRange, int checkWalls)
{
    Vec4 offset;
    offset.Copy(*eye - *target);
    float distance;
    {
        Vec4 stretched = offset;
        stretched.z = offset.z * 2.5f;
        distance = stretched.Length();
    }
    if (distance > range)
        return 0;
    float angle = (float)atan2(offset.x, offset.y);
    angle += 3.1415f;
    angle -= heading;
    while (angle > 3.1415f)
        angle -= 6.283f;
    while (angle < -3.1415f)
        angle += 6.283f;
    if ((float)fabs(angle) > angleRange && distance > nearRange)
        return 0;
    if (checkWalls != 0)
    {
        Vec4 hit;
        if (DAT_10255f08 != 0 && DAT_10255f08->Raycast(0, 4, 0x4000000, eye, target, &hit, 0) != 0)
            return 0;
    }
    return 1;
}
