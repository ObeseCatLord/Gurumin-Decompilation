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

// Empties `pack` the way VertexPack::Init does (this object file has the body inline).
static inline void EmptyPack(VertexPack *pack)
{
    pack->vertices = 0;
    pack->center[0] = 0;
    pack->center[1] = 0;
    pack->center[2] = 0;
    pack->radius = 0;
    pack->vertexCount = 0;
    pack->m_0040 = 0;
    pack->m_0044 = 0;
}

// VertexPacks_AllocateArray: allocates `count` VertexPack records from the engine allocator (tag "pp"), empties each
// one in place and clears its radius again. Returns the array. No direct caller.
extern "C" VertexPack *VertexPacks_AllocateArray(int count)
{
    VertexPack *packs = (VertexPack *)g_engineAlloc(count * sizeof(VertexPack), "pp");
    for (int i = 0; i < count; i++)
    {
        EmptyPack(&packs[i]);
        packs[i].radius = 0;
    }
    return packs;
}
