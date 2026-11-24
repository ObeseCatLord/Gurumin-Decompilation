// Reordering and welding of a node's vertex packs (EN build). Ported from the matched JP source
// decomp/src/node_sort_packs.cpp.
#include <string.h>

#include "containers.h"
#include "engine_memory.h"
#include "index_list.h"
#include "mesh_cache.h"
#include "node.h"
#include "slot_table.h"

// Node::WeldVertices: drops this node's mesh when it was its last user, then merges equal vertices in every pack: each
// index gets the first identical vertex (compared byte for byte), the pack keeps only the distinct vertices and its
// indices are rewritten. The node gets a new, empty mesh. Returns 0 for a node without packs, else 1.
int Node::WeldVertices()
{
    if (packCount == 0)
        return 0;
    if (mesh != 0 && MeshCache_Release(mesh) == 0)
    {
        mesh->Release();
        g_engineFree(mesh);
        mesh = 0;
    }
    for (int p = 0; p < packCount; p++)
    {
        int unique = 0;
        Vertex *vertices = (Vertex *)g_engineAlloc(packs[p].vertexCount * sizeof(Vertex), "vt");
        int *indices = (int *)g_engineAlloc(indexLists[p].count * sizeof(int), "it");
        for (int i = 0; i < indexLists[p].count; i++)
        {
            int index = indexLists[p].indices[i];
            int j;
            for (j = 0; j < unique; j++)
            {
                if (memcmp(&vertices[j], &packs[p].vertices[index], sizeof(Vertex)) == 0)
                {
                    index = j;
                    break;
                }
            }
            if (j == unique)
            {
                vertices[unique] = packs[p].vertices[index];
                index = unique;
                unique++;
            }
            indices[i] = index;
        }
        packs[p].SetVertexCount(unique);
        g_engineCopy(packs[p].vertices, vertices, unique * sizeof(Vertex));
        g_engineCopy(indexLists[p].indices, indices, indexLists[p].count * sizeof(int));
        g_engineFree(vertices);
        g_engineFree(indices);
    }
    mesh = (SlotTable *)g_engineAlloc(SLOT_TABLE_SIZE, "CVPCObjD");
    mesh->Reset();
    mesh->bufferSlot = NO_BUFFER_SLOT;
    return 1;
}
