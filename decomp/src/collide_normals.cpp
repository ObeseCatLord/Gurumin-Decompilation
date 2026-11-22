// Averaging the collision normals of a model's collision nodes against the world.

#include "vec4.h"

#include "matrix4.h"

// Scene node (scene_node.cpp Node), only the fields used here.
struct Node
{
    char pad_0000[0xac];
    unsigned int flags;       // 0xac: 0x400 = collision probe node
    char pad_00b0[0x19c];
    Matrix4 worldMatrix;      // 0x24c
    char pad_028c[0x448];
    Vec4 bounds;              // 0x6d4: bounding sphere (centre relative to the node, radius in w)
    char pad_06e4[0x6c];
    int childCount;           // 0x750
    char childNames[0x40][0x40];
    Node *children[0x40];     // 0x1754
};

// The world collision (scene root node); see collide_query.cpp.
struct CollisionWorld
{
    // 0x101a1640: tests a sphere of `radius` at `point` (in `matrix` space); the hit normal goes to `normal`.
    int Query(Matrix4 *matrix, Vec4 *point, float radius, Vec4 *normal, int mask, int a, int b);
};

extern "C" CollisionWorld *DAT_10255f08;

// 0x10024ac0 (Collision_AverageNormals). Tests every collision probe node (flag 0x400) of the tree below `node`, at
// its position plus its bounds centre plus `offset`, as a sphere of 0.578 times its radius against the world. Each
// hit's normal (left normalised in `normal`) is folded into the running average `average`, weighted by the number of
// hits so far (`count`). Returns 0.
extern "C" int FUN_10024ac0(Node *node, const Vec4 *offset, Matrix4 *matrix, Vec4 *normal, Vec4 *average, int *count,
                            int mask)
{
    for (int i = 0; i < node->childCount; i++)
        FUN_10024ac0(node->children[i], offset, matrix, normal, average, count, mask);
    if ((node->flags & 0x400) != 0)
    {
        Vec4 probe;
        probe.Copy(node->worldMatrix.GetTranslation() + node->bounds + *offset);
        if (DAT_10255f08 != 0 &&
            DAT_10255f08->Query(matrix, &probe, node->bounds.w * 0.578f, normal, mask, 0, 0) != 0)
        {
            int hits = ++*count;
            float weight = 1.0f / (float)hits;
            float keep = 1.0f - weight;
            normal->Normalized3();
            average->Copy(*average * keep + *normal * weight);
            average->Normalized3();
        }
    }
    return 0;
}
