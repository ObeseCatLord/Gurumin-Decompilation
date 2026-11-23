// Averaging the collision normals of a model's collision nodes against the world.

#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "map_vars.h"


// 0x10024ac0 (Collision_AverageNormals). Tests every collision probe node (flag 0x400) of the tree below `node`, at
// its position plus its bounds centre plus `offset`, as a sphere of 0.578 times its radius against the world. Each
// hit's normal (left normalised in `normal`) is folded into the running average `average`, weighted by the number of
// hits so far (`count`). Returns 0.
extern "C" int FUN_10024ac0(Node *node, const Vec4 *offset, Matrix4 *matrix, Vec4 *normal, Vec4 *average, int *count,
                            float *depth)
{
    for (int i = 0; i < node->childCount; i++)
        FUN_10024ac0(node->children[i], offset, matrix, normal, average, count, depth);
    if ((node->flags & 0x400) != 0)
    {
        Vec4 probe;
        probe.Copy(node->worldMatrix.GetTranslation() + node->bounds + *offset);
        if (DAT_10255f08 != 0 &&
            DAT_10255f08->Query(matrix, &probe, node->bounds.w * 0.578f, normal, depth, 0, 0) != 0)
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
