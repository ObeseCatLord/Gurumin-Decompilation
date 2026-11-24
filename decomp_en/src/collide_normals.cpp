// Averaging the world normals under collision probes (EN build). Ported from the matched JP source
// decomp/src/collide_normals.cpp.
#include "collide_regions.h"

// Collision_AverageNormals: tests every probe node (NODE_FLAG_PROBE) of the tree below `node`, at its position plus its
// bounds centre plus `offset`, as a sphere of PROBE_RADIUS_SCALE times its radius against the world. Each hit's normal
// (left normalised in `normal`) is folded into the running average `average`, weighted by the number of hits so far
// (`count`). Returns 0. Called by the model collision code.
extern "C" int Collision_AverageNormals(Node *node, const Vec4 *offset, Matrix4 *matrix, Vec4 *normal, Vec4 *average, int *count, float *depth)
{
    for (int i = 0; i < node->childCount; i++)
        Collision_AverageNormals(node->children[i], offset, matrix, normal, average, count, depth);
    if ((node->flags & NODE_FLAG_PROBE) != 0)
    {
        Vec4 probe;
        probe.Copy(node->worldMatrix.GetTranslation() + node->bounds + *offset);
        if (g_worldRoot != 0 &&
            g_worldRoot->Query(matrix, &probe, node->bounds.w * PROBE_RADIUS_SCALE, normal, depth, 0, 0) != 0)
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
