// Allocation of arrays of mesh packs and index lists (EN build, EN-only: no JP twin found).
#include "containers.h"
#include "index_list.h"
#include "engine_memory.h"

// VertexPacks_NewArray: allocates `count` VertexPack records from the engine allocator (tag "pp"), empties each with Init
// and clears its radius. Returns the array. No direct caller.
extern "C" VertexPack * __stdcall VertexPacks_NewArray(int count)
{
    VertexPack *packs = (VertexPack *)g_engineAlloc(count * sizeof(VertexPack), "pp");
    for (int i = 0; i < count; i++)
    {
        packs[i].Init();
        packs[i].radius = 0;
    }
    return packs;
}

// IndexLists_NewArray: allocates `count` IndexList records from the engine allocator (tag "id") and empties each with
// Init. Returns the array. No direct caller.
extern "C" IndexList * __stdcall IndexLists_NewArray(int count)
{
    IndexList *lists = (IndexList *)g_engineAlloc(count * sizeof(IndexList), "id");
    for (int i = 0; i < count; i++)
        lists[i].Init();
    return lists;
}
