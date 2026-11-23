// Reordering a scene node's vertex packs so that the packs whose material has m_0178 == 1 are drawn first, before
// the others (the mesh is rebuilt from the new order), and merging a node's equal vertices.

#include <string.h>

// Mesh material (scene_node.cpp Material, 0x180 bytes), only the value tested here.
struct Material
{
    char pad_0000[0x178];
    float m_0178;                // 0x178: 1 = the pack goes first
    char pad_017c[4];
};

// A vertex pack (scene_node.cpp PackHeader, 0x74 bytes).
struct PackHeader
{
    char pad_0000[0x14];
    int material;                // 0x14: index into the node's materials
    char pad_0018[4];
    int vertexCount;             // 0x1c
    char pad_0020[0x50];
    struct Vertex *vertices;     // 0x70: 0x70-byte vertices
    void Init();                                 // 0x101acfa0
    void Copy(const PackHeader *source);         // 0x101ad040
    int SetVertexCount(int count);               // 0x101acff0: reallocates the vertex array
};

// One vertex of a pack (position, normal, colour, texture coordinates...), compared and copied whole.
struct Vertex
{
    int data[0x1c];
};

// The index list of a vertex pack (8 bytes).
struct KeyPair
{
    int count;
    int *indices;
    void Init();                                 // 0x101ad1b0
    void Copy(const KeyPair *source);          // 0x101ad160
};

// The mesh buffers built from the packs (scene_node.cpp MeshData).
struct MeshData
{
    char pad_0000[0x4c];
    int bufferSlot;              // 0x4c: entry of the mesh buffer tables, -1 = none
    void Init();                                 // 0x10145a10
    void Release();                              // 0x10150a90
};
extern "C" int FUN_10169990(MeshData *mesh);     // MeshCache_Release: drops one reference, returns those left

#include "node.h"

extern "C" void *(*DAT_11b0b938)(int size, const char *tag);   // the engine's allocator
extern "C" void (*DAT_11b1ba40)(void *);                      // the engine's free
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);   // the engine's memcpy

// 0x1019c690 (Node::SortPacks). Drops this node's mesh when it was its last user, copies the packs and their index
// lists into new arrays (first those whose material has m_0178 == 1, then the others), frees the old ones and gives
// the node a new, empty mesh. Returns 0 for a node without packs, else 1.
int Node::SortPacks()
{
    int i;
    if (packCount == 0)
        return 0;
    if (mesh != 0 && FUN_10169990(mesh) == 0)
    {
        mesh->Release();
        DAT_11b1ba40(mesh);
        mesh = 0;
    }
    PackHeader *newPacks = (PackHeader *)DAT_11b0b938(packCount * sizeof(PackHeader), "vp");
    KeyPair *newLists = (KeyPair *)DAT_11b0b938(packCount * sizeof(KeyPair), "vpi");
    for (i = 0; i < packCount; i++)
    {
        newPacks[i].Init();
        newLists[i].Init();
    }
    int count = 0;
    for (i = 0; i < packCount; i++)
    {
        if (materials[packs[i].material].m_0178 == 1.0f)
        {
            newPacks[count].Copy(&packs[i]);
            newLists[count].Copy(&indexLists[i]);
            count++;
        }
    }
    for (i = 0; i < packCount; i++)
    {
        if (materials[packs[i].material].m_0178 != 1.0f)
        {
            newPacks[count].Copy(&packs[i]);
            newLists[count].Copy(&indexLists[i]);
            count++;
        }
    }
    M_101911b0(packs, packCount);
    M_101911e0(indexLists, packCount);
    if (packs != 0)
        DAT_11b1ba40(packs);
    if (indexLists != 0)
        DAT_11b1ba40(indexLists);
    packs = newPacks;
    indexLists = newLists;
    mesh = (MeshData *)DAT_11b0b938(0x50, "CVPCObjB");
    mesh->Init();
    mesh->bufferSlot = -1;
    return 1;
}

// 0x1019e610 (Node::WeldVertices). Drops this node's mesh when it was its last user, then merges equal vertices in
// every pack: each index gets the first identical vertex (compared byte for byte), the pack keeps only the distinct
// vertices and its indices are rewritten. The node gets a new, empty mesh. Returns 0 for a node without packs,
// else 1.
int Node::WeldVertices()
{
    if (packCount == 0)
        return 0;
    if (mesh != 0 && FUN_10169990(mesh) == 0)
    {
        mesh->Release();
        DAT_11b1ba40(mesh);
        mesh = 0;
    }
    for (int p = 0; p < packCount; p++)
    {
        int unique = 0;
        Vertex *vertices = (Vertex *)DAT_11b0b938(packs[p].vertexCount * sizeof(Vertex), "vt");
        int *indices = (int *)DAT_11b0b938(indexLists[p].count * sizeof(int), "it");
        for (int i = 0; i < indexLists[p].count; i++)
        {
            int index = indexLists[p].indices[i];
            int j;
            for (j = 0; j < unique; j++)
            {
                if (memcmp(&vertices[j], &packs[p].vertices[index], sizeof(Vertex)) == 0)
                {
                    index = j;
                    break;
                }
            }
            if (j == unique)
            {
                vertices[unique] = packs[p].vertices[index];
                index = unique;
                unique++;
            }
            indices[i] = index;
        }
        packs[p].SetVertexCount(unique);
        DAT_11b07530(packs[p].vertices, vertices, unique * sizeof(Vertex));
        DAT_11b07530(indexLists[p].indices, indices, indexLists[p].count * sizeof(int));
        DAT_11b1ba40(vertices);
        DAT_11b1ba40(indices);
    }
    mesh = (MeshData *)DAT_11b0b938(0x50, "CVPCObjD");
    mesh->Init();
    mesh->bufferSlot = -1;
    return 1;
}
