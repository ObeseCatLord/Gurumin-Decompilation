// Vertex colour reset of a node's mesh (EN build). Ported from the matched JP source decomp/src/node_vertex_colours.cpp.
#include "containers.h"
#include "node.h"

// Alpha the vertices get with and without texture coordinates.
const float VERTEX_ALPHA_TEXTURED = 0.7f;
const float VERTEX_ALPHA_PLAIN = 0.5f;

// Height the textured vertices are raised by.
const float VERTEX_LIFT = 5.0f;

// Node::ResetVertexColours: makes every vertex white and clears its emissive colour. With a non-zero `uvScale` the
// vertices also get alpha VERTEX_ALPHA_TEXTURED, are raised by VERTEX_LIFT and take their texture coordinates from their
// x and y times `uvScale`; with 0 they only get alpha VERTEX_ALPHA_PLAIN. Returns 0.
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
                packs[i].vertices[k].emissive[2] = 0;
                packs[i].vertices[k].emissive[1] = 0;
                packs[i].vertices[k].emissive[0] = 0;
                packs[i].vertices[k].colour[3] = VERTEX_ALPHA_TEXTURED;
                packs[i].vertices[k].position[2] += VERTEX_LIFT;
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
                packs[i].vertices[k].emissive[2] = 0;
                packs[i].vertices[k].emissive[1] = 0;
                packs[i].vertices[k].emissive[0] = 0;
                packs[i].vertices[k].colour[3] = VERTEX_ALPHA_PLAIN;
            }
        }
    }
    return 0;
}
