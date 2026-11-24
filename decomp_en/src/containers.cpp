// EN key tracks, key banks, vertex packs and index lists (JP twins: containers.cpp).

#include <stdio.h>
#include "containers.h"
#include "key_bank.h"
#include "index_list.h"
#include "engine_memory.h"

// Size of one sphere entry of a sphere list (centre and radius as a Vec4).
const int SPHERE_SIZE = sizeof(Vec4);

// Marks every key as a motion frame key. Called by the animation loaders.
int KeyTrack::MarkAll()
{
    for (int i = 0; i < count; i++)
        flags[i] = KEY_FLAG_MOTION;
    return 0;
}

// 1 when the track has a key exactly at `frame` (the keys are sorted, so the search stops early).
int KeyTrack::HasFrame(int frame)
{
    for (int i = 0; i < count; i++)
    {
        if (frames[i] == frame)
            return 1;
        if (frames[i] > frame)
            return 0;
    }
    return 0;
}

// Reallocates the vertex buffer for `count` vertices. Returns 1.
int VertexPack::SetVertexCount(int count)
{
    if (vertices != 0)
        g_engineFree(vertices);
    vertices = 0;
    void *buffer = g_engineAlloc(count * VERTEX_SIZE, "");
    vertices = (Vertex *)buffer;
    vertexCount = count;
    return 1;
}

// Reallocates the index buffer for `newCount` indexes (contents are not kept). Returns 1.
int IndexList::Resize(int newCount)
{
    if (indices != 0)
        g_engineFree(indices);
    indices = 0;
    int *buffer = (int *)g_engineAlloc(newCount * sizeof(int), "");
    indices = buffer;
    count = newCount;
    return 1;
}

// Reallocates the span buffer for `newCount` spheres (contents are not kept). The allocation is tagged
// "m_indexSP(count)" for the engine's memory report. Returns 1.
int IndexList::ResizeSpans(int newCount)
{
    char tag[64];
    if (spheres != 0)
        g_engineFree(spheres);
    spheres = 0;
    sprintf(tag, "m_indexSP(%d)", newCount);
    void *buffer = g_engineAlloc(newCount * SPHERE_SIZE, tag);
    spheres = (Vec4 *)buffer;
    count = newCount;
    return 1;
}
