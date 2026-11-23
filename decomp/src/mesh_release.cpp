// Mesh data of a scene node (scene_node.cpp Node::mesh): the texture slot and the buffers BuildMesh creates.

#include "d3d9_min.h"

#include "vec4.h"

#include "matrix4.h"

Matrix4 Transpose(const Matrix4 &matrix);       // 0x101b1090

#include "bone_rows.h"

#include "render_vars.h"

#include "mesh.h"

extern "C" void FUN_101477c0(int slot);          // Texture_ReleaseSlot
#include "engine_memory.h"

// 0x10150a90. Releases the mesh: its texture slot and every buffer, then clears the fields.
void MeshData::Release()
{
    if (textureSlot >= 0)
        FUN_101477c0(textureSlot);
    textureSlot = -1;
    if (materialIds != 0)
        DAT_11b1ba40(materialIds);
    if (materials != 0)
        DAT_11b1ba40(materials);
    if (textureCopies != 0)
        DAT_11b1ba40(textureCopies);
    if (secondaryTextures != 0)
        DAT_11b1ba40(secondaryTextures);
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
    m_0044 = 0;
    secondaryTextures = 0;
    materialIds = 0;
    materials = 0;
    textureCopies = 0;
    doubleSide = 0;
    vertexStarts = 0;
    vertexEnds = 0;
    indexStarts = 0;
    indexEnds = 0;
    pointFlags = 0;
    points = 0;
    boxes = 0;
}

// 0x10145a50. Uploads the skinning matrices of the mesh: each of the `count` bone matrices (at most 24) is
// multiplied by `world`, transposed, and its first three rows go to the vertex-shader constants from register 11.
void MeshData::UploadBones(Matrix4 *bones, int count, Matrix4 *world, int unused)
{
    Vec4 constants[24 * 3];  // three registers per bone
    for (int i = 0; i < count; i++)
    {
        Matrix4 matrix;
        matrix = bones[i] * *world;
        matrix = Transpose(matrix);
        *(BoneRows *)&constants[i * 3] = *(BoneRows *)&matrix;  // its first three rows
    }
    DAT_10f1fcf0->SetVertexShaderConstantF(11, (const float *)constants, count * 3);
}
