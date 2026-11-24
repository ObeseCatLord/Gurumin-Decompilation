// SlotTable of the EN build (JP: MeshData in decomp/include/mesh.h): the Direct3D side of a node's mesh. The buffers
// are built from the node's packs by BuildBuffers, one entry per pack.
// This is the one declaration of SlotTable in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_SLOT_TABLE_H
#define GURUMIN_EN_SLOT_TABLE_H

#include "vec4.h"

struct VertexPack;
struct IndexList;
struct Material;

// Bones the vertex shader can skin with at once.
const int MAX_SKIN_BONES = 24;
// Vertex shader constant register the bone matrices start at.
const int BONE_CONSTANT_REGISTER = 11;
// Registers one bone takes (the first three rows of its transposed matrix).
const int BONE_REGISTERS = 3;

// Bytes of the mesh object (SlotTable) a node allocates after its packs changed.
const int SLOT_TABLE_SIZE = 0x50;
// SlotTable::bufferSlot of a mesh that does not come from a shared model file.
const int NO_BUFFER_SLOT = -1;

struct SlotTable
{
    int textureSlot;          // 0x00: texture slot the mesh buffers live in (negative = none)
    int vertexCount;          // 0x04: vertices of all packs
    int indexCount;           // 0x08: indices of all index lists
    int *materialIds;         // 0x0c: material number of each pack
    int *originalMaterial;    // 0x10: material each entry was loaded with
    int *material;            // 0x14: material each entry currently uses
    int *secondaryTextures;   // 0x18: each pack's second texture slot (-1 = none)
    int *doubleSide;          // 0x1c: the material's four edge bytes packed into one word
    int *vertexStarts;        // 0x20: first vertex of each pack
    int *vertexEnds;          // 0x24: end of each pack's vertices
    int *indexStarts;         // 0x28: first index of each pack
    int *indexEnds;           // 0x2c: end of each pack's indices
    char *pointFlags;         // 0x30: each pack's bounding-sphere flag (rounded radius)
    Vec4 *points;             // 0x34: each pack's bounding sphere (centre, radius in w; -1 for a double-sided material)
    Vec4 *boxes;              // 0x38: each pack's bounding box (smallest, largest)
    int first;                // 0x3c: first entry later operations touch
    int last;                 // 0x40: one past the last entry they touch
    int count;                // 0x44: number of entries
    int built;                // 0x48: 1 once the buffers are filled
    int bufferSlot;           // 0x4c: shared model file this mesh was loaded from (negative = none)

    int SetRange(int newFirst, int newLast);            // EN 0x77bf40 (JP 0x10146d90)
    int SetMaterial(int index, int value);              // EN 0x77cfa0 (JP 0x101476e0)
    int ReplaceMaterial(int key, int value);            // EN 0x77cf50 (JP 0x10147690)
    int Reset();                                        // EN 0x7a4010 (JP 0x10145a10): no texture slot, no entries, every pointer cleared
    void Release();                                     // EN 0x7a3c80 (JP 0x10150a90): frees the texture slot and every table
    // EN 0x7a45b0 (JP 0x10150bb0): builds the Direct3D buffers and the per-pack tables from the node's packs.
    // EN 0x7a4360 (JP 0x10145a50): uploads the skinning matrices: each of the `count` bone matrices (at most
    // MAX_SKIN_BONES) times `world`, transposed, goes to the vertex shader constants as three registers.
    void UploadBones(Matrix4 *bones, int count, Matrix4 *world, int unused);
    // EN 0x799880 (JP 0x10151150): draws the mesh with `world` and `view` in drawing mode `mode`; the arguments after it
    // describe the shadow projected onto it (matrix, centre, texture, scale and size), the light direction, the caster's
    // size and the opacity.
    int Draw(const Matrix4 *world, const Matrix4 *view, const Matrix4 *shadowMatrix, int mode, float shadowX,
             float shadowY, float shadowZ, int shadowTexture, float shadowScale, int shadowSize,
             const Vec4 *lightDirection, float modelSize, float alpha);
    // EN 0x780150 (JP 0x10151ba0): draws the mesh with `world` and `view` in drawing mode `mode`, lit by `lights`;
    // `paramsA` and `paramsB` are the node's two draw parameter vectors.
    int DrawLit(const Matrix4 *world, const Matrix4 *view, int mode, const void *lights, const Vec4 *paramsA, const Vec4 *paramsB);
    int BuildBuffers(VertexPack *packs, IndexList *indexLists, int packCount, Material *materials, int flags);
};

// Value of the `key` argument of ReplaceMaterial that matches every entry.
const int MATERIAL_ANY = -1;

#endif
