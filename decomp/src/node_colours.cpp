// Material colours of a node's mesh parts. A node (scene_node.cpp Node) has `packCount` packs; each pack has
// `partCount` parts of 0x70 bytes, and each part names its material by number.

#include "mesh.h"

#include "node.h"

// 0x1019c560. Copies each part's material colour into the part (r, g, b from the material, alpha from the
// material when positive, else 0) and resets the part's second colour to (0, 0, 0, 1). Returns 0.
int Node::ApplyMaterialColours()
{
    for (int i = 0; i < packCount; i++)
    {
        for (int j = 0; j < packs[i].vertexCount; j++)
        {
            float r = materials[(int)packs[i].vertices[j].material].colour[0];
            float g = materials[(int)packs[i].vertices[j].material].colour[1];
            float b = materials[(int)packs[i].vertices[j].material].colour[2];
            float a = 0.0f < materials[(int)packs[i].vertices[j].material].alpha ? materials[(int)packs[i].vertices[j].material].alpha : 0;
            packs[i].vertices[j].colour[0] = r;
            packs[i].vertices[j].colour[1] = g;
            packs[i].vertices[j].colour[2] = b;
            packs[i].vertices[j].colour[3] = a;
            packs[i].vertices[j].emissive[0] = 0;
            packs[i].vertices[j].emissive[1] = 0;
            packs[i].vertices[j].emissive[2] = 0;
            packs[i].vertices[j].emissive[3] = 1.0f;
        }
    }
    return 0;
}
