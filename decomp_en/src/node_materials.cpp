// Node material table and the subtree passes over it (EN build, EN-only: no JP twin found).
#include "node.h"
#include "engine_memory.h"

// Memory report tag of the material table.
static const char MATERIAL_TABLE_TAG[] = "m_material";

// Replaces this node's material table with a copy of the `count` materials at `source`: frees the old table, allocates
// a new one and copies the materials into it. Returns 1. No direct caller.
int Node::SetMaterials(const Material *source, int count)
{
    if (materials != 0)
        g_engineFree(materials);
    materials = (Material *)g_engineAlloc(count * sizeof(Material), MATERIAL_TABLE_TAG);
    materialCount = count;
    g_engineCopy(materials, source, count * sizeof(Material));
    return 1;
}

// Sets MATERIAL_EDGE_MARKED on the first edge flag of every material of this node, then does the same for every child
// subtree. Returns 0. Calls itself on the children; no other direct caller.
int Node::MarkMaterialEdges()
{
    for (int i = 0; i < materialCount; i++)
        materials[i].edge[0] |= MATERIAL_EDGE_MARKED;
    for (int i = 0; i < childCount; i++)
        children[i]->MarkMaterialEdges();
    return 0;
}

// Runs MergeMaterialPacks on this node when it has skinning data, then on every child subtree (the arguments pass
// through unchanged). Returns 0. Calls itself on the children; no other direct caller.
int Node::MergeMaterialPacksInSubtree(Node *source, const char *materialPrefix, const char *otherPrefix)
{
    if (skin != 0)
        MergeMaterialPacks(source, materialPrefix, otherPrefix);
    for (int i = 0; i < childCount; i++)
        children[i]->MergeMaterialPacksInSubtree(source, materialPrefix, otherPrefix);
    return 0;
}
