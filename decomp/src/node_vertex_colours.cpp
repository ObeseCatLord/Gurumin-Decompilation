// Resetting the vertex colours of a scene node's packs, optionally lifting the vertices and deriving texture
// coordinates from their position (used for the flat shadow and decal models).

// A vertex of a pack (0x70 bytes).
struct PackVertex
{
    float position[3];           // 0x00
    char pad_000c[0x14];
    float colour[4];             // 0x20: r, g, b, a
    float uv[2];                 // 0x30
    char pad_0038[8];
    float m_0040[3];             // 0x40
    char pad_004c[0x24];
};

// A vertex pack (scene_node.cpp PackHeader, 0x74 bytes).
struct PackHeader
{
    char pad_0000[0x1c];
    int vertexCount;             // 0x1c
    char pad_0020[0x50];
    PackVertex *vertices;        // 0x70
};

#include "node.h"

// 0x1019c3a0 (Node::ResetVertexColours). Makes every vertex white and clears its m_0040 values. With a non-zero
// `uvScale` the vertices also get alpha 0.7, are raised by 5 and take their texture coordinates from their x and y
// times `uvScale`; with 0 they only get alpha 0.5. Returns 0.
int Node::ResetVertexColours(float uvScale)
{
    if (uvScale != 0.0f)
    {
        for (int i = 0; i < packCount; i++)
        {
            for (int k = 0; k < packs[i].vertexCount; k++)
            {
                packs[i].vertices[k].colour[2] = 1.0f;
                packs[i].vertices[k].colour[1] = 1.0f;
                packs[i].vertices[k].colour[0] = 1.0f;
                packs[i].vertices[k].m_0040[2] = 0;
                packs[i].vertices[k].m_0040[1] = 0;
                packs[i].vertices[k].m_0040[0] = 0;
                packs[i].vertices[k].colour[3] = 0.7f;
                packs[i].vertices[k].position[2] += 5.0f;
                packs[i].vertices[k].uv[0] = uvScale * packs[i].vertices[k].position[0];
                packs[i].vertices[k].uv[1] = uvScale * packs[i].vertices[k].position[1];
            }
        }
    }
    else
    {
        for (int i = 0; i < packCount; i++)
        {
            for (int k = 0; k < packs[i].vertexCount; k++)
            {
                packs[i].vertices[k].colour[2] = 1.0f;
                packs[i].vertices[k].colour[1] = 1.0f;
                packs[i].vertices[k].colour[0] = 1.0f;
                packs[i].vertices[k].m_0040[2] = 0;
                packs[i].vertices[k].m_0040[1] = 0;
                packs[i].vertices[k].m_0040[0] = 0;
                packs[i].vertices[k].colour[3] = 0.5f;
            }
        }
    }
    return 0;
}
