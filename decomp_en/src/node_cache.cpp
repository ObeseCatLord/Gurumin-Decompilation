// The model file cache behind Node::Load (EN build). Ported from the matched JP source decomp/src/node_cache.cpp.
#include <string.h>
#include "engine.h"
#include "engine_memory.h"
#include "node_cache.h"

// Name the engine's leak report shows for the cached node trees.
const char NODE_CACHE_TAG[] = "ccnode";

// Node::Load: releases this node and gives it model `path`: copied from the cache when that file is cached (returns 1),
// else after loading it into a free cache slot or, with the cache full, into a random slot whose model is dropped
// (returns the slot). With `noCopy` the model is only cached, not copied into this node. Called for every model file the
// game loads.
int Node::Load(const char *path, int noCopy, int a, int b)
{
    Release();
    for (int i = 0; i < NODE_CACHE_SIZE; i++)
    {
        if (g_cachedNodes[i] != 0 && _stricmp(path, g_cachedNodeNames[i]) == 0)
        {
            if (noCopy == 0)
                Clone(g_cachedNodes[i], a, b);
            return 1;
        }
    }
    int slot;
    for (slot = 0; slot < NODE_CACHE_SIZE; slot++)
    {
        if (g_cachedNodes[slot] == 0)
        {
            g_cachedNodes[slot] = (Node *)g_engineAlloc(sizeof(Node), NODE_CACHE_TAG);
            g_cachedNodes[slot]->Init();
            g_cachedNodes[slot]->LoadIt3(path, a, b);
            strcpy(g_cachedNodeNames[slot], path);
            if (noCopy == 0)
                Clone(g_cachedNodes[slot], a, b);
            return slot;
        }
    }
    slot = Random() % NODE_CACHE_SIZE;
    g_cachedNodes[slot]->Release();
    g_cachedNodes[slot] = (Node *)g_engineAlloc(sizeof(Node), NODE_CACHE_TAG);
    g_cachedNodes[slot]->Init();
    g_cachedNodes[slot]->LoadIt3(path, a, b);
    strcpy(g_cachedNodeNames[slot], path);
    if (noCopy == 0)
        Clone(g_cachedNodes[slot], a, b);
    return slot;
}
