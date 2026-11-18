// Overlap test of two axis-aligned boxes, each given in its own space plus an offset (used by the scene node code
// at 0x10193140).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    void Fill(const float &value);               // 0x101acc40: sets all four components
    Vec4 operator+(const Vec4 &other) const;     // 0x101ac9e0
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
    Vec4 Scaled(const float &factor);            // 0x101acb00
};

struct Box
{
    Vec4 min;
    Vec4 max;
};

// 0x10191fd0 (Box_Overlap). Moves box `first` by `firstOffset` and box `second` by `secondOffset` and returns 1 when
// they overlap on all three axes (touching faces do not count), else 0. On overlap `centreDelta`, when given,
// receives the centre of the first box minus the centre of the second, and `negatedDelta`, when given, its negation
// (it reads `centreDelta`, so both must be given for that).
extern "C" int __stdcall FUN_10191fd0(const Box *first, const Vec4 *firstOffset, const Box *second,
                                      const Vec4 *secondOffset, Vec4 *centreDelta, Vec4 *negatedDelta)
{
    Vec4 a[2];
    Vec4 b[2];
    a[0].x = first->min.x + firstOffset->x;
    a[0].y = first->min.y + firstOffset->y;
    a[0].z = first->min.z + firstOffset->z;
    a[1].x = first->max.x + firstOffset->x;
    a[1].y = first->max.y + firstOffset->y;
    a[1].z = first->max.z + firstOffset->z;
    b[0].x = second->min.x + secondOffset->x;
    b[0].y = second->min.y + secondOffset->y;
    b[0].z = second->min.z + secondOffset->z;
    b[1].x = second->max.x + secondOffset->x;
    b[1].y = second->max.y + secondOffset->y;
    b[1].z = second->max.z + secondOffset->z;
    if (a[1].x <= b[0].x || a[0].x >= b[1].x || a[1].y <= b[0].y || a[0].y >= b[1].y || a[1].z <= b[0].z ||
        a[0].z >= b[1].z)
        return 0;
    if (centreDelta != 0)
    {
        Vec4 centreA;
        Vec4 centreB;
        centreA.Copy((a[0] + a[1]).Scaled(0.5f));
        centreB.Copy((b[0] + b[1]).Scaled(0.5f));
        centreDelta->Copy(centreA - centreB);
    }
    if (negatedDelta != 0)
    {
        Vec4 zero;
        zero.Fill(0.0f);
        negatedDelta->Copy(zero - *centreDelta);
    }
    return 1;
}
