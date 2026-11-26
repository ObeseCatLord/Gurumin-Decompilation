// Building a node's mesh from a triangle list: the vertices are sorted into one vertex pack per material (EN build,
// JP twin node_mesh_build.cpp).
#include <stdio.h>
#include <string.h>

#include "containers.h"
#include "engine_memory.h"
#include "index_list.h"
#include "node.h"
#include "node_mesh_build.h"
#include "slot_table.h"

// Material numbers a vertex can carry (the size of the per-material tables).
const int MESH_MAX_MATERIALS = 0x2000;
// MeshData::bufferSlot of a new mesh: no entry in the renderer's buffer tables yet.
const int MESH_NO_BUFFER_SLOT = -1;
// Triangle corners, and the pack and index list allocations' tags.
const int TRIANGLE_CORNERS = 3;
static const char BLANK_MATERIAL_TAG[] = "m_material";
static const char PACK_ARRAY_TAG[] = "pp";
static const char INDEX_LIST_ARRAY_TAG[] = "id";
static const char MESH_TAG[] = "CVPCObjA";

// Allocates `count` empty vertex packs (the body of VertexPacks_NewArray, inlined here).
static inline VertexPack *NewPackArray(int count)
{
    VertexPack *packs = (VertexPack *)g_engineAlloc(count * sizeof(VertexPack), PACK_ARRAY_TAG);
    for (int i = 0; i < count; i++)
    {
        packs[i].Init();
        packs[i].radius = 0;
    }
    return packs;
}

// Allocates `count` empty index lists (the body of IndexLists_NewArray, inlined here).
static inline IndexList *NewIndexListArray(int count)
{
    IndexList *lists = (IndexList *)g_engineAlloc(count * sizeof(IndexList), INDEX_LIST_ARRAY_TAG);
    for (int i = 0; i < count; i++)
        lists[i].Init();
    return lists;
}

// Node::BuildMesh (triangle list). Called by the model loader with a triangle list: `count` indices (`indices`, three
// per triangle) into `vertices`, each vertex carrying its material number in `sourceMaterial`. Releases the old mesh
// and sphere lists, gives the node one blank material when it has none, makes one vertex pack and index list per
// material used, copies each triangle's vertices into its material's pack (the index list just counts up) and creates a
// new mesh table. Then applies the material colours (unless `skinned` data forbids it, see m_1894), splits the mesh
// into cells when g_splitMeshIntoCells is set, welds the vertices, sorts the packs and computes the bounds. Returns 1,
// or 0 for an empty list.
int Node::BuildMesh(Vertex *vertices, int *indices, int count)
{
    int materialVertexCount[MESH_MAX_MATERIALS];  // vertices that carry each material
    int packVertexCount[MESH_MAX_MATERIALS];      // vertices of each material's pack
    int listOfMaterial[MESH_MAX_MATERIALS];       // index list of each material
    int packFill[MESH_MAX_MATERIALS];             // vertices copied into each material's pack so far
    int packOfMaterial[MESH_MAX_MATERIALS];       // vertex pack of each material

    if (mesh != 0)
        ReleaseMesh();
    if (triangleSpheres != 0 && packCount > 0)
    {
        IndexList *spheres = triangleSpheres;
        int remaining = packCount;
        do
        {
            spheres->FreeSpheres();
            spheres++;
            remaining--;
        } while (remaining != 0);
    }
    if (triangleSpheres != 0)
        g_engineFree(triangleSpheres);
    triangleSpheres = 0;
    if (count == 0)
        return 0;

    memset(materialVertexCount, 0, sizeof(materialVertexCount));
    memset(packFill, 0, sizeof(packFill));

    if (materialCount == 0)
    {
        // A blank white material without textures.
        materials = (Material *)g_engineAlloc(sizeof(Material), BLANK_MATERIAL_TAG);
        materials[0].textures[0][0] = 0;
        materials[0].textures[1][0] = 0;
        materials[0].textures[2][0] = 0;
        materials[0].textures[3][0] = 0;
        materials[0].colour[0] = 1.0f;
        materials[0].colour[1] = 1.0f;
        materials[0].colour[2] = 1.0f;
        materials[0].alpha = 0;
        materialCount = 1;
    }
    // Looks for the first material without a texture (the result is not used).
    int blank = 0;
    while (blank < materialCount && materials[blank].textures[0][0] != 0)
        blank++;

    // Count the vertices of each material, keeping each vertex's material number as a whole number.
    for (int i = 0; i < count; i++)
    {
        int material = (int)vertices[i].sourceMaterial;
        materialVertexCount[material]++;
        vertices[i].material = (float)material;
    }

    // One pack and one index list per material used.
    int packTotal = 0;
    for (int material = 0; material < MESH_MAX_MATERIALS; material++)
    {
        if (materialVertexCount[material] != 0)
        {
            listOfMaterial[material] = packTotal;
            packOfMaterial[material] = packTotal;
            packTotal++;
            packVertexCount[material] = materialVertexCount[material];
        }
    }
    packs = NewPackArray(packTotal);
    indexLists = NewIndexListArray(packTotal);
    packCount = packTotal;
    for (int material = 0; material < MESH_MAX_MATERIALS; material++)
    {
        if (materialVertexCount[material] != 0)
        {
            int list = listOfMaterial[material];
            int vertexCount = packVertexCount[material];
            packs[list].SetVertexCount(vertexCount);
            indexLists[list].Resize(vertexCount);
            packs[list].material = material;
        }
    }

    // Copy each triangle into its material's pack.
    for (int i = 0; i < count; i += TRIANGLE_CORNERS)
    {
        int vertex = indices[i];
        int material = (int)vertices[vertex].sourceMaterial;
        int pack = packOfMaterial[material];
        int fill = packFill[material];
        packs[pack].vertices[fill] = vertices[vertex];
        indexLists[pack].indices[fill] = fill;
        packs[pack].vertices[fill + 1] = vertices[indices[i + 1]];
        indexLists[pack].indices[fill + 1] = fill + 1;
        packs[pack].vertices[fill + 2] = vertices[indices[i + 2]];
        indexLists[pack].indices[fill + 2] = fill + 2;
        fill += TRIANGLE_CORNERS;
        packFill[material] = fill;
        packs[pack].vertexCount = fill;
        indexLists[pack].count = fill;
    }

    if (mesh != 0)
        fread(0, 1, 1, 0);   // the old mesh must be gone: crash on purpose (fread on a null file)
    mesh = (SlotTable *)g_engineAlloc(sizeof(SlotTable), MESH_TAG);
    mesh->Reset();
    mesh->bufferSlot = MESH_NO_BUFFER_SLOT;
    if (m_1894 == 0)
        ApplyMaterialColours();
    if (g_splitMeshIntoCells != 0)
        SplitIntoCells();
    WeldVertices();
    SortPacks();
    ComputeBounds();
    return 1;
}
