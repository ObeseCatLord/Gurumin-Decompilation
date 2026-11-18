// Collision queries against the world (DAT_10255f08).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Set(float x, float y, float z, float w);
    void Copy(const Vec4 &source);
    Vec4 operator+(const Vec4 &other) const;
    Vec4 Scaled(const float &scale);
};

struct Matrix4
{
    float m[16];
    Matrix4();
    ~Matrix4();
    void SetIdentity();
};

struct CollisionWorld
{
    // Node::Collide (0x101a8e60) on the world's root node: the segment `from`-`to` against the collision of the
    // given kinds; the hit point goes to `hit`. Returns 1 on a hit.
    int Collide(int name, int mask, Vec4 *from, Vec4 *to, Vec4 *hit);
    int Query(Matrix4 *matrix, Vec4 *point, int mask, Vec4 *hit, int *hitId, int a, int b);
    // Casts the segment `from`-`to` against the nodes called `name` (0 = all) whose flags match; writes the
    // nearest hit point and, when `t` is given, its position along the segment. Returns 1 on a hit.
    int RayCast(const char *name, unsigned kind, unsigned mask, const Vec4 *to, const Vec4 *from, Vec4 *hit,
                float *t);
};

extern "C" CollisionWorld *DAT_10255f08;

// 0x1000ff80: 1 when the point (x, y, z) touches world collision of the kinds in `mask`.
int Collision_ProbePoint(float x, float y, float z, int mask)
{
    Matrix4 matrix;
    matrix.SetIdentity();
    Vec4 point;
    Vec4 hit;
    int hitId;
    point.Set(x, y, z, 0);
    return DAT_10255f08->Query(&matrix, &point, mask, &hit, &hitId, 0, 0);
}

// Casts a ray from `from` to `to` against world collision (kind 4, mask 0x4000000) of the nodes called `name`.
// Returns 1 on a hit and stores the hit point in `hit` unless `testOnly` is set; 0 when nothing is hit or no world
// is loaded.
extern "C" int FUN_100246b0(const Vec4 *from, const Vec4 *to, Vec4 *hit, int testOnly, const char *name)
{
    Vec4 point;
    if (testOnly != 0)
    {
        if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(name, 4, 0x4000000, to, from, &point, 0) != 0)
            return 1;
    }
    else
    {
        if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(name, 4, 0x4000000, to, from, &point, 0) != 0)
        {
            hit->Copy(point);
            return 1;
        }
    }
    return 0;
}

// 0x10024c40. Ledge test: 1 when there is world collision (mask 0x4000000) below the point `distance` units along
// `direction` from `position`. The ray runs from 90 units above that point to `depth` units below it. Enemy AI
// uses it (depth 300, distance 15) to stop at edges. Returns 0 when nothing is hit or no world is loaded.
extern "C" int FUN_10024c40(const Vec4 *position, Vec4 *direction, float depth, int distance)
{
    Vec4 from;
    Vec4 to;
    Vec4 hit;
    from.Copy(direction->Scaled((float)distance));
    from.Copy(from + *position);
    to.Copy(from);
    from.z += 90.0f;
    to.z -= depth;
    if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, 0x4000000, &from, &to, &hit) != 0)
        return 1;
    return 0;
}
