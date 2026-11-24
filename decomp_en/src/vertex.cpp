// Vertex setters (EN build). Ported from the matched JP source decomp/src/vertex.cpp.
#include "containers.h"

// Sets the position and resets everything else: normal (0, 0, 1), white colour, zero UVs. Called when a mesh is built.
void Vertex::SetPosition(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    position[3] = 1.0f;
    normal[0] = 0;
    normal[1] = 0;
    normal[2] = 1.0f;
    normal[3] = 1.0f;
    colour[0] = 1.0f;
    colour[1] = 1.0f;
    colour[2] = 1.0f;
    colour[3] = 1.0f;
    uv[0] = 0;
    uv[1] = 0;
    sourceMaterial = 0;
    material = 0;
    emissive[0] = 0;
    emissive[1] = 0;
    emissive[2] = 0;
    emissive[3] = 0;
    m_0050[0] = 0;
    m_0050[1] = 0;
    m_0050[2] = 0;
    m_0050[3] = 0;
    m_0060 = 1.0f;
    m_0064[0] = 0;
    m_0064[1] = 0;
    m_0064[2] = 0;
}

// Sets position, normal, colour and the first texture coordinates in one call; everything else gets its default.
void Vertex::Set(float x, float y, float z, float nx, float ny, float nz, float u, float v, float r, float g, float b, float a)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    position[3] = 1.0f;
    normal[0] = nx;
    normal[1] = ny;
    normal[2] = nz;
    normal[3] = 1.0f;
    colour[0] = r;
    colour[1] = g;
    colour[2] = b;
    colour[3] = a;
    uv[0] = u;
    uv[1] = v;
    sourceMaterial = 0;
    material = 0;
    emissive[0] = 0;
    emissive[1] = 0;
    emissive[2] = 0;
    emissive[3] = 0;
    m_0050[0] = 0;
    m_0050[1] = 0;
    m_0050[2] = 0;
    m_0050[3] = 0;
    m_0060 = 1.0f;
    m_0064[0] = 0;
    m_0064[1] = 0;
    m_0064[2] = 0;
}
