// Mesh data of a scene node (scene_node.cpp Node::mesh): the texture slot and the buffers BuildMesh creates.

#include "d3d9_min.h"

#include "vec4.h"

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                  // 0x101ae770
    ~Matrix4();                                 // 0x1013eda0 (empty)
    Matrix4 operator*(const Matrix4 &other);    // 0x101b0670: matrix product
};

Matrix4 Transpose(const Matrix4 &matrix);       // 0x101b1090

// The first three rows of a transposed bone matrix: one vertex-shader constant each.
struct BoneRows
{
    Vec4 row[3];
};

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;      // the Direct3D device

struct MeshData
{
    int textureSlot;          // 0x00: texture slot of the mesh, -1 = none
    int pad_0004[2];
    void *m_000c;             // 0x0c..0x38: buffers allocated by M_10150bb0 (vertices, indices, ...)
    void *m_0010;
    void *m_0014;
    void *m_0018;
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
    void Release();
    void UploadBones(Matrix4 *bones, int count, Matrix4 *world, int unused);
};

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
    if (m_0014 != 0)
        DAT_11b1ba40(m_0014);
    if (m_0010 != 0)
        DAT_11b1ba40(m_0010);
    if (m_0018 != 0)
        DAT_11b1ba40(m_0018);
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
    m_0018 = 0;
    m_000c = 0;
    m_0014 = 0;
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
