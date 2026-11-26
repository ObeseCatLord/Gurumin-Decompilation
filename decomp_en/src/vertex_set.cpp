// Vertex setter overloads (EN build): the shorter forms of Vertex::Set (vertex.cpp) that default the normal, the colour
// or the texture coordinates. No caller is known for any of them.
#include "containers.h"

// Vertex::Set (position and texture coordinates): the normal points along +z and the colour is opaque white; every
// other field is reset.
void Vertex::Set(float x, float y, float z, float u, float v)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    uv[0] = u;
    position[3] = 1.0f;
    normal[0] = 0;
    normal[1] = 0;
    normal[2] = 1.0f;
    normal[3] = 1.0f;
    colour[0] = 1.0f;
    colour[1] = 1.0f;
    colour[2] = 1.0f;
    colour[3] = 1.0f;
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

// Vertex::Set (position, normal and texture coordinates): the colour is opaque white; every other field is reset.
void Vertex::Set(float x, float y, float z, float nx, float ny, float nz, float u, float v)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    normal[0] = nx;
    normal[1] = ny;
    normal[2] = nz;
    uv[0] = u;
    position[3] = 1.0f;
    normal[3] = 1.0f;
    colour[0] = 1.0f;
    colour[1] = 1.0f;
    colour[2] = 1.0f;
    colour[3] = 1.0f;
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

// Vertex::Set (position, normal and colour): the texture coordinates are zero; every other field is reset.
void Vertex::Set(float x, float y, float z, float nx, float ny, float nz, float r, float g, float b, float a)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
    normal[0] = nx;
    normal[1] = ny;
    normal[2] = nz;
    colour[0] = r;
    colour[1] = g;
    colour[2] = b;
    position[3] = 1.0f;
    normal[3] = 1.0f;
    colour[3] = a;
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
