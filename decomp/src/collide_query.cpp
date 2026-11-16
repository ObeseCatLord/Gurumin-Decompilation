struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Set(float x, float y, float z, float w);
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
    int Query(Matrix4 *matrix, Vec4 *point, int mask, Vec4 *hit, int *hitId, int a, int b);
};

extern "C" CollisionWorld *DAT_10255f08;

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
