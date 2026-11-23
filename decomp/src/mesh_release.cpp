// Mesh data of a scene node (scene_node.cpp Node::mesh): the texture slot and the buffers BuildMesh creates.

#include "d3d9_min.h"

#include "vec4.h"

#include "matrix4.h"

Matrix4 Transpose(const Matrix4 &matrix);       // 0x101b1090

// The first three rows of a transposed bone matrix: one vertex-shader constant each.
struct BoneRows
{
    Vec4 row[3];
};

#include "render_vars.h"

#include "mesh.h"

extern "C" void FUN_101477c0(int slot);          // Texture_ReleaseSlot
extern "C" void (*DAT_11b1ba40)(void *block);    // the engine's free

// 0x10150a90. Releases the mesh: its texture slot and every buffer, then clears the fields.
void MeshData::Release()
{
    if (textureSlot >= 0)
        FUN_101477c0(textureSlot);
    textureSlot = -1;
    if (m_000c != 0)
        DAT_11b1ba40(m_000c);
    if (materials != 0)
        DAT_11b1ba40(materials);
    if (m_0010 != 0)
        DAT_11b1ba40(m_0010);
    if (secondaryTextures != 0)
        DAT_11b1ba40(secondaryTextures);
    if (m_001c != 0)
        DAT_11b1ba40(m_001c);
    if (m_0020 != 0)
        DAT_11b1ba40(m_0020);
    if (m_0024 != 0)
        DAT_11b1ba40(m_0024);
    if (m_0028 != 0)
        DAT_11b1ba40(m_0028);
    if (m_002c != 0)
        DAT_11b1ba40(m_002c);
    if (m_0030 != 0)
        DAT_11b1ba40(m_0030);
    if (m_0034 != 0)
        DAT_11b1ba40(m_0034);
    if (m_0038 != 0)
        DAT_11b1ba40(m_0038);
    m_0044 = 0;
    secondaryTextures = 0;
    m_000c = 0;
    materials = 0;
    m_0010 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0024 = 0;
    m_0028 = 0;
    m_002c = 0;
    m_0030 = 0;
    m_0034 = 0;
    m_0038 = 0;
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
