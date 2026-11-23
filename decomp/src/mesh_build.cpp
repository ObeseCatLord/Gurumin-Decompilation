// The Direct3D buffers of a scene node's mesh: builds them from the node's vertex packs and index lists.

#include <string.h>
#include "d3d9_min.h"

#include "vec4.h"
#include "matrix4.h"
#include "mesh.h"
#include "engine_memory.h"
#include "render_vars.h"

extern "C" void FUN_101477c0(int slot);          // Texture_ReleaseSlot
extern "C" int FUN_10161920(const char *name);   // Texture_FindByName: slot holding `name`, or -1
extern "C" int FUN_10147880(IUnknownLike **vertexBuffer, int vertexBytes, IUnknownLike **indexBuffer, int indexBytes);

// Size of one Direct3D vertex (the Vertex of a pack, copied as it is) and of an index.
#define VERTEX_BYTES 0x70
#define INDEX_BYTES 2
// Texture slot of a pack without a texture.
#define NO_TEXTURE (-1)
// Radius value that marks a double-sided material's bounding sphere.
#define DOUBLE_SIDED_RADIUS (-1.0f)

// 0x10150bb0 (BuildBuffers). Builds the mesh's per-pack tables and its vertex and index buffers from the node's
// `count` vertex packs and their index lists: material, texture slots (looked up in `materials` by name unless it
// is 0), double-sided edge bytes, vertex and index ranges, bounding sphere and box. The pack vertices are copied
// into the locked vertex buffer and the indices are written with the pack's first vertex added. Does nothing when
// there is no device, or when the mesh is built already and `flags` is 0. Returns 1, or 0 when it did nothing, `count`
// is 0 or no buffer slot is free.
int MeshData::M_10150bb0(VertexPack *packs, IndexList *indexLists, int count, Material *materials, int flags)
{
    if (DAT_10f1fcf0 == 0 || (flags == 0 && built == 1))
        return 0;
    built = 0;
    IUnknownLike *vertexBuffer = 0;
    IUnknownLike *indexBuffer = 0;
    if (textureSlot >= 0)
        FUN_101477c0(textureSlot);
    if (count != 0)
    {
        if (materialIds != 0)
            DAT_11b1ba40(materialIds);
        if (this->materials != 0)
            DAT_11b1ba40(this->materials);
        if (secondaryTextures != 0)
            DAT_11b1ba40(secondaryTextures);
        if (textureCopies != 0)
            DAT_11b1ba40(textureCopies);
        if (doubleSide != 0)
            DAT_11b1ba40(doubleSide);
        if (vertexStarts != 0)
            DAT_11b1ba40(vertexStarts);
        if (vertexEnds != 0)
            DAT_11b1ba40(vertexEnds);
        if (indexStarts != 0)
            DAT_11b1ba40(indexStarts);
        if (indexEnds != 0)
            DAT_11b1ba40(indexEnds);
        if (pointFlags != 0)
            DAT_11b1ba40(pointFlags);
        if (points != 0)
            DAT_11b1ba40(points);
        if (boxes != 0)
            DAT_11b1ba40(boxes);
        int tableBytes = count * 4;
        materialIds = (int *)DAT_11b0b938(tableBytes, "m_materialid");
        this->materials = (int *)DAT_11b0b938(tableBytes, "m_texid");
        secondaryTextures = (int *)DAT_11b0b938(tableBytes, "m_texid2");
        textureCopies = (int *)DAT_11b0b938(tableBytes, "m_texid");
        doubleSide = (int *)DAT_11b0b938(tableBytes, "m_doubleside");
        vertexStarts = (int *)DAT_11b0b938(tableBytes, "m_vertexst");
        vertexEnds = (int *)DAT_11b0b938(tableBytes, "m_vertexed");
        indexStarts = (int *)DAT_11b0b938(tableBytes, "m_indexst");
        indexEnds = (int *)DAT_11b0b938(tableBytes, "m_indexed");
        pointFlags = (char *)DAT_11b0b938(count, "m_pointflag");
        points = (Vec4 *)DAT_11b0b938(count * sizeof(Vec4), "m_point");
        boxes = (Vec4 *)DAT_11b0b938(count * 2 * sizeof(Vec4), "m_bbox");

        int i = 0;
        int totalIndices = 0;
        int totalVertices = 0;
        for (i = 0; i < count; i++)
        {
            VertexPack *pack = &packs[i];
            vertexStarts[i] = totalVertices;
            indexStarts[i] = totalIndices;
            if (pack->vertexCount != 0)
            {
                totalIndices += indexLists[i].count;
                totalVertices += pack->vertexCount;
                pointFlags[i] = 0;
                if (pack->radius != 0.0f)
                {
                    pointFlags[i] = (char)(int)pack->radius;
                    points[i].x = pack->center[0];
                    points[i].y = pack->center[1];
                    points[i].z = pack->center[2];
                    points[i].w = pack->radius;
                    if (pack->m_0040 == 1)
                        points[i].w = DOUBLE_SIDED_RADIUS;
                }
                boxes[i * 2].Copy(pack->boundsMin);
                boxes[i * 2 + 1].Copy(pack->boundsMax);
                this->materials[i] = NO_TEXTURE;
                secondaryTextures[i] = NO_TEXTURE;
                if (materials != 0)
                {
                    this->materials[i] = NO_TEXTURE;
                    if (materials[pack->material].textures[0][0] != 0)
                    {
                        this->materials[i] = FUN_10161920(materials[pack->material].textures[0]);
                        textureCopies[i] = this->materials[i];
                    }
                    else if (materials[pack->material].textures[1][0] != 0)
                    {
                        this->materials[i] = FUN_10161920(materials[pack->material].textures[1]);
                        textureCopies[i] = this->materials[i];
                    }
                    if (materials[pack->material].textures[3][0] != 0)
                        secondaryTextures[i] = FUN_10161920(materials[pack->material].textures[3]);
                }
                materialIds[i] = pack->material;
                if (materials != 0)
                {
                    doubleSide[i] = materials[pack->material].m_017c[0];
                    doubleSide[i] |= materials[pack->material].m_017c[1] << 8;
                    doubleSide[i] |= materials[pack->material].m_017c[2] << 16;
                    doubleSide[i] |= materials[pack->material].m_017c[3] << 24;
                }
                else
                    doubleSide[i] = 0;
            }
            vertexEnds[i] = totalVertices;
            indexEnds[i] = totalIndices;
        }
        m_0044 = count;
        m_003c = 0;
        packCount = count;
        int vertexBytes = totalVertices * VERTEX_BYTES;
        textureSlot = FUN_10147880(&vertexBuffer, vertexBytes, &indexBuffer, totalIndices * INDEX_BYTES);
        if (textureSlot < 0)
        {
            Release();
            return 0;
        }
        if (DAT_10f655c0 != 0)
            DAT_10f60690[textureSlot] = 1;
        unsigned short *indexData;
        char *vertexData;
        this->vertexCount = 0;
        this->indexCount = 0;
        ((IDirect3DBuffer9Like *)indexBuffer)->Lock(0, totalIndices * INDEX_BYTES, (void **)&indexData, 0);
        ((IDirect3DBuffer9Like *)vertexBuffer)->Lock(0, vertexBytes, (void **)&vertexData, 0);
        for (i = 0; i < count; i++)
        {
            for (int j = 0; j < indexLists[i].count; j++)
                indexData[indexStarts[i] + j] = (unsigned short)(indexLists[i].indices[j] + vertexStarts[i]);
            memcpy(vertexData + vertexStarts[i] * VERTEX_BYTES, packs[i].vertices, packs[i].vertexCount * VERTEX_BYTES);
            this->indexCount += indexLists[i].count;
            this->vertexCount += packs[i].vertexCount;
        }
        ((IDirect3DBuffer9Like *)vertexBuffer)->Unlock();
        ((IDirect3DBuffer9Like *)indexBuffer)->Unlock();
        built = 1;
        return 1;
    }
    return 0;
}
