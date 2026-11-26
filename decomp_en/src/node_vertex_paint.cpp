// Painting the vertices of a node's mesh (EN build, EN-only: no JP twin found).
#include <string.h>
#include "containers.h"
#include "node.h"

// Longest list of vertex numbers SetVertexColours reads when it is not ended by a negative number.
const int VERTEX_LIST_LIMIT = 0x200;
// Texture file name every material gets from SetFlatColour.
#define TEXTURE_NAME_FLAT "white"

// Node::SetFlatColour: points the first texture of every material at "white" and gives every vertex of every pack the colour (r, g, b, a).
// Returns 1. No direct caller recovered.
int Node::SetFlatColour(float r, float g, float b, float a)
{
    for (int i = 0; i < materialCount; i++)
        strcpy(materials[i].textures[0], TEXTURE_NAME_FLAT);
    for (int i = 0; i < packCount; i++)
    {
        for (int k = 0; k < packs[i].vertexCount; k++)
        {
            packs[i].vertices[k].colour[0] = r;
            packs[i].vertices[k].colour[1] = g;
            packs[i].vertices[k].colour[2] = b;
            packs[i].vertices[k].colour[3] = a;
        }
    }
    return 1;
}

// Node::SetVertexColours: sets the red, green and blue of vertices of the first pack to (r, g, b), leaving the alpha
// alone: every vertex when `vertexNumbers` is 0, otherwise the vertices whose numbers the list gives (it ends at the first
// negative number). Returns 1. No direct caller recovered.
int Node::SetVertexColours(const short *vertexNumbers, float r, float g, float b)
{
    int i = 0;
    if (vertexNumbers == 0)
    {
        for (; i < packs[0].vertexCount; i++)
        {
            packs[0].vertices[i].colour[0] = r;
            packs[0].vertices[i].colour[1] = g;
            packs[0].vertices[i].colour[2] = b;
        }
        return 1;
    }
    do
    {
        if (vertexNumbers[i] < 0)
            return 1;
        packs[0].vertices[vertexNumbers[i]].colour[0] = r;
        packs[0].vertices[vertexNumbers[i]].colour[1] = g;
        packs[0].vertices[vertexNumbers[i]].colour[2] = b;
        i++;
    } while (i < VERTEX_LIST_LIMIT);
    return 1;
}
