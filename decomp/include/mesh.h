// A scene node's mesh (node.h): vertex packs of 0x70-byte vertices, one index list per pack, the materials the
// vertices name by number, and the Direct3D buffers built from them (MeshData). The pack and list methods are in
// containers.cpp, Vertex's in vertex.cpp, MeshData's in mesh_release.cpp and the renderer.
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_MESH_H
#define GURUMIN_MESH_H

#include "vec4.h"
#include "matrix4.h"

// One vertex (28 floats = 0x70 bytes). Plain floats: packs copy vertices as raw blocks.
struct Vertex
{
    float position[4];   // 0x00: x, y, z, 1
    float normal[4];     // 0x10: x, y, z, 1
    float colour[4];     // 0x20: r, g, b, a (Node::ApplyMaterialColours copies the material's)
    float uv[2];         // 0x30: first texture coordinates
    float m_0038;        // 0x38
    float material;      // 0x3c: material number of the vertex (stored as a float)
    float emissive[4];   // 0x40: second colour, reset to (0, 0, 0, 1) by ApplyMaterialColours
    float m_0050[4];     // 0x50
    float m_0060;        // 0x60: 1 for a new vertex
    float m_0064[3];     // 0x64

    void SetPosition(float x, float y, float z);                   // 0x101acea0: position, the rest default
    void Set(float x, float y, float z, float nx, float ny, float nz, float u, float t, float r, float g, float b,
             float a);                                             // 0x101acf10
};

// A vertex pack (0x74 bytes): the vertices drawn with one material, with their bounds.
struct VertexPack
{
    float center[3];         // 0x00: centre of the pack's vertices
    float radius;            // 0x0c: bounding radius (0 = not needed)
    int m_0010;
    int material;            // 0x14: index into the node's materials
    int m_0018;
    int vertexCount;         // 0x1c
    Vec4 boundsMin;          // 0x20: smallest x, y, z of the vertices (w = 1)
    Vec4 boundsMax;          // 0x30: largest x, y, z (w = 1)
    int m_0040;
    int m_0044;
    char pad_0048[0x28];
    Vertex *vertices;        // 0x70: `vertexCount` vertices

    void Init();                                   // 0x101acfa0: empties a pack allocated raw
    int FreeVertices();                            // 0x101acfc0
    int SetVertexCount(int count);                 // 0x101acff0: reallocates the vertex array (contents lost)
    int CopyFrom(const VertexPack *source);        // 0x101ad040
};

// An index list of a pack (8 bytes): the vertex indices, three per triangle. A node's sphere lists (`Node::m_0070`)
// use the same block for one bounding sphere (centre, radius in w) per triangle, allocated by ResizeSpans.
struct IndexList
{
    int count;               // 0x00: entries (indices, or spheres for a sphere list)
    union
    {
        int *indices;        // 0x04: index list: the vertex indices
        Vec4 *spheres;       // 0x04: sphere list: centre and radius (w) of each triangle
    };

    int Free();                                    // 0x101ad0e0: frees the buffer, count 0
    int Resize(int newCount);                      // 0x101ad110: reallocates for `newCount` indices
    int CopyFrom(const IndexList *source);         // 0x101ad160
    void Init();                                   // 0x101ad1b0: clears the pointer of a list allocated raw
    int ResizeSpans(int newCount);                 // 0x101ad1c0: reallocates for `newCount` 16-byte spheres
};

// Texture file name slots of a material.
#define MATERIAL_NAME_SIZE 0x40
#define MATERIAL_TEXTURE_COUNT 4

// A mesh material (0x180 bytes).
struct Material
{
    char name[MATERIAL_NAME_SIZE];                                   // 0x00
    char textures[MATERIAL_TEXTURE_COUNT][MATERIAL_NAME_SIZE];       // 0x40: texture file names (empty = none)
    float colour[3];                                                 // 0x140: r, g, b (1 = white)
    char pad_014c[0x24];
    int m_0170;
    float m_0174;                                                    // 0x174: 255 for a blank material
    float alpha;             // 0x178: vertex alpha, copied by ApplyMaterialColours when positive; packs whose material
                             //        has alpha 1 are drawn first (node_sort_packs.cpp); 0 for a blank material
    unsigned char m_017c[4];
};

// The Direct3D side of a node's mesh: texture slot, buffers built from the packs (M_10150bb0) and the entry of the
// renderer's mesh tables.
struct MeshData
{
    int textureSlot;         // 0x00: texture slot of the mesh, -1 = none
    int pad_0004[2];
    void *m_000c;            // 0x0c..0x38: buffers allocated by M_10150bb0 (vertices, indices, ...), freed by Release
    void *m_0010;
    int *materials;          // 0x14: current material of each entry
    int *secondaryTextures;  // 0x18: each entry's second texture
    void *m_001c;
    void *m_0020;
    void *m_0024;
    void *m_0028;
    void *m_002c;
    void *m_0030;
    void *m_0034;
    void *m_0038;
    int pad_003c[2];
    int m_0044;
    int pad_0048;
    int bufferSlot;          // 0x4c: entry of the mesh buffer tables (DAT_10feb3e0 ...), -1 = none

    void Init();                                                                   // 0x10145a10
    void UploadBones(Matrix4 *bones, int count, Matrix4 *world, int view);         // 0x10145a50 (mesh_release.cpp)
    int SetRange(int first, int last);                                             // 0x10146d90 (SlotTable)
    void M_10147690(int a, int b);
    void M_101476e0(int pack, int value);
    void M_10146db0(Matrix4 *world, const Matrix4 *view, const Vec4 *box);  // 0x10146db0: debug view of the mesh box
    void Release();                                                                // 0x10150a90 (mesh_release.cpp)
    int M_10150bb0(VertexPack *packs, IndexList *indexLists, int packCount, Material *materials, int flags);
    // 0x10151150: draws the mesh with the world matrix and the pass-through arguments (node_draw.cpp).
    int Draw(Matrix4 *worldMatrix, const Matrix4 *view, const Matrix4 *shadowMatrix, int drawMode, float shadowX,
             float shadowY, float shadowZ, int shadowTexture, float shadowScale, int shadowSize,
             const Vec4 *lightDirection, float modelSize, float alpha);
    // 0x10151ba0: draws the mesh with the world matrix and the given lighting parameters (draw_list.cpp).
    int DrawLit(Matrix4 *world, const Matrix4 *view, int mode, const void *lights, Vec4 *a, Vec4 *b);
};

#endif
