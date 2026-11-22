// Keeping the "shaga" effect models (shaga_spawn.cpp) out of the world collision. C++ rather than extern "C": the
// function keeps an exception frame for its Matrix4 local.

#include "vec4.h"

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                  // 0x101ae770
    ~Matrix4();                                 // 0x1013eda0 (empty)
    void SetIdentity();                         // 0x101ae7a0
    Vec4 GetTranslation();                      // 0x101ade80
    void SetTranslation(Vec4 position);         // 0x101adef0
};

struct ShagaNode
{
    char pad_0000[0x1cc];
    Matrix4 matrix;                             // 0x1cc: the model's placement
};

// The world collision (scene root node); see collide_normals.cpp.
struct CollisionWorld
{
    int Query(Matrix4 *matrix, Vec4 *point, float radius, Vec4 *normal, int *hitId, int a, int b);  // 0x101a1640
};

extern "C" ShagaNode *DAT_10446e00[];   // the shaga models (shaga_spawn.cpp)
extern "C" float DAT_1030cd98[];        // their radius
extern "C" CollisionWorld *DAT_10255f08;

// 0x1001af10 (Shaga_PushOut). Moves shaga model `index` out of the world collision: its centre (the placement raised
// by its radius) is tested as a sphere of radius + 1, and while it hits something it is moved 5 units along the hit
// normal (at most 10 times). Returns 0.
int Shaga_PushOut(int index)
{
    Matrix4 identity;
    int steps = 0;
    Vec4 position;
    Vec4 normal;
    int hitId;
    identity.SetIdentity();
    position.Copy(DAT_10446e00[index]->matrix.GetTranslation());
    position.z += DAT_1030cd98[index];
    do
    {
        if (DAT_10255f08 == 0)
            break;
        if (DAT_10255f08->Query(&identity, &position, DAT_1030cd98[index] + 1.0f, &normal, &hitId, 0, 0) == 0)
            break;
        normal.Normalized3();
        normal.Copy(normal * 5.0f);
        position.Copy(position + normal);
        steps++;
    } while (steps < 10);
    position.z -= DAT_1030cd98[index];
    DAT_10446e00[index]->matrix.SetTranslation(position);
    return 0;
}
