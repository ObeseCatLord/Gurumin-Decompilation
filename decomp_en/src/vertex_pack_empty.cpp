// Emptying a vertex pack (EN build, EN-only: no JP twin found).
#include "containers.h"
#include "engine_memory.h"

// Frees the vertex buffer when there is one, then clears the buffer pointer and the vertex count. No direct caller.
void VertexPack::EmptyVertices()
{
    if (vertices != 0)
        g_engineFree(vertices);
    vertices = 0;
    vertexCount = 0;
}
