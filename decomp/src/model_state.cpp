// Model playback state.

#include <string.h>

struct NodeLink
{
    char pad_0000[0xa8];
    int m_00a8;
};

struct SceneNode
{
    int M_10199340();
};

struct ModelState
{
    int m_0000;
    int m_0004;
    float m_0008;
    char m_000c;
    char m_000d;
    char m_000e;
    char pad_000f;
    int m_0010;
    int m_0014;
    int m_0018;
    int m_001c;
    int m_0020;
    char m_0024;
    char m_0025;
    char m_0026;
    char m_0027;
    char m_0028;
    char m_0029;
    char pad_002a;
    char m_002b;
    int m_002c;
    int pad_0030;
    int m_0034;
    int m_0038;
    int m_003c;
    int m_0040;
    ModelState();
};

struct Playback
{
    char pad_0000[0x18];
    int m_0018;
    int m_001c;
    int m_0020;
    int m_0024;
    char pad_0028[0xbc4];
    int m_0bec;
    char pad_0bf0[0x7080];
    int m_7c70;
    int m_7c74;
    int m_7c78;
    int pad_7c7c;
    SceneNode *m_7c80;
    int M_1016a090(SceneNode *node);
};

extern "C" NodeLink *DAT_10faaa88[0x100];

// Clears the m_00a8 flag of every loaded model in the 0x100-entry model table DAT_10faaa88.
extern "C" int FUN_101619a0(void)
{
    for (int i = 0; i < 0x100; i++)
    {
        if (DAT_10faaa88[i] != 0)
            DAT_10faaa88[i]->m_00a8 = 0;
    }
    return 0;
}

// Constructor (see anim_channels.cpp for why callers treat it as throw()).
ModelState::ModelState()
{
    m_002c = 0;
    m_002b = 0;
    m_0004 = 0xa2c2a;
    m_0028 = 0;
    m_0029 = 0;
    m_0024 = 0;
    m_0008 = 1.0f;
    m_0025 = 0;
    m_0027 = 0;
    m_0034 = 0;
    m_0038 = 0;
    m_003c = 0;
    m_0040 = 0;
    m_0000 = 0;
    m_0026 = 1;
    m_000e = 0;
    m_000d = 0;
    m_000c = 0;
    m_0010 = 0;
    m_0014 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
}

// Binds the playback to `node` and resets it to the start; the frame count is taken from the node when it has
// more than 8 frames (shorter or missing animations play as 0 frames). Returns 1.
int Playback::M_1016a090(SceneNode *node)
{
    m_001c = -1;
    m_7c78 = -1;
    m_0018 = 0;
    m_0020 = 0;
    m_0024 = 0;
    m_7c74 = 0;
    m_0bec = 0;
    m_7c80 = node;
    if (node != 0)
    {
        int frames = node->M_10199340();
        if (frames > 8)
        {
            m_7c70 = frames;
            return 1;
        }
    }
    m_7c70 = 0;
    return 1;
}
