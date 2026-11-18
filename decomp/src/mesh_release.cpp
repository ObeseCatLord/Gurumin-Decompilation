// Mesh data of a scene node (scene_node.cpp Node::mesh): the texture slot and the buffers BuildMesh creates.

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
