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
    int M_1019b630(int fromFrame, int toFrame, int frames);   // 0x1019b630: sets up a blend between two frames
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
    int m_0000;
    float m_0004;             // 0x04: frame position (-1 while blending)
    float m_0008;             // 0x08: playback speed (1.0)
    char m_000c;
    char pad_000d[3];
    int m_0010;
    int m_0014;
    int m_0018;
    int m_001c;               // 0x1c: layer playing (0x90 = blending into m_7c7c, -1 = none)
    int m_0020;               // 0x20: current frame
    int m_0024;
    int m_0028;
    int m_002c[16];
    int m_006c[16];           // -1 = unused
    // Per-layer animation state, 0x90 layers (Init clears active and end; SetLayer starts one).
    int m_00ac[0x90];         // 0xac: 1 while the layer is active
    int m_02ec[0x90];         // 0x2ec: first frame
    int m_052c[0x90];         // 0x52c: last frame
    int m_076c[0x90];
    int m_09ac[0x90];
    int m_0bec;
    int m_0bf0[0x90];         // 0xbf0: playback direction (+1 forwards, -1 backwards)
    int m_0e30[0x90];
    int m_1070[0x90][16];     // 0x1070: per-layer values, cleared to 0 when the layer starts
    int m_3470[0x90][16];
    int m_5870[0x90][16];     // 0x5870: per-layer values, set to -2 (unused) when the layer starts
    int m_7c70;
    int m_7c74;
    int m_7c78;
    int m_7c7c;               // 0x7c7c: layer being blended into
    SceneNode *m_7c80;
    void Init();
    int SetLayer(int layer, int a, int first, int last, int b, int c);
    int M_1016a090(SceneNode *node);
    int M_1016a020(struct SceneOwner *owner);
    int Play(int layer, int restart, int blendFrames, float speed);
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

// Resets a newly allocated playback object: speed 1, no node, all tables cleared and the 16 slot ids at -1.
void Playback::Init()
{
    m_000c = 0;
    m_0018 = 0;
    m_001c = -1;
    m_0020 = 0;
    m_0004 = 0;
    m_0024 = 0;
    m_0008 = 1.0f;
    m_7c74 = 0;
    m_0bec = 0;
    m_7c78 = -1;
    m_0000 = 0;
    memset(m_00ac, 0, sizeof(m_00ac));
    memset(m_052c, 0, sizeof(m_052c));
    for (int i = 0; i < 16; i++)
    {
        m_002c[i] = 0;
        m_006c[i] = -1;
    }
    m_0010 = 0;
    m_0014 = 0;
    m_0018 = 0;
    m_0020 = 0;
    m_0028 = 0;
    m_001c = -1;
}

// Starts animation layer `layer` (0..0x8f) playing from frame `first` to `last` (backwards when last < first) with
// the values a, b, c, and resets its per-layer tables. Returns 1, or 0 for a layer number out of range.
// Not byte-identical yet: the original writes the two 16-entry fills as unrolled stores (see open.tsv).
int Playback::SetLayer(int layer, int a, int first, int last, int b, int c)
{
    if (layer > 0x8f)
        return 0;
    m_0e30[layer] = b;
    m_076c[layer] = a;
    m_09ac[layer] = c;
    m_052c[layer] = last;
    m_02ec[layer] = first;
    m_00ac[layer] = 1;
    m_0bf0[layer] = first <= last ? 1 : -1;
    for (int i = 0; i < 16; i++)
        m_1070[layer][i] = 0;
    for (int i = 0; i < 16; i++)
        m_5870[layer][i] = -2;
    return 1;
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

// Object that owns a scene (texture_mgr.cpp: SceneOwner::M_1017b1c0 creates the Scene and binds it here).
struct SceneOwner
{
    char pad_0000[0x22ec];
    SceneNode *root;  // 0x22ec: root node of the owner's model
};

// Same as M_1016a090, binding the playback to `owner`'s root node.
int Playback::M_1016a020(SceneOwner *owner)
{
    m_001c = -1;
    m_7c78 = -1;
    m_0018 = 0;
    m_0020 = 0;
    m_0024 = 0;
    m_7c74 = 0;
    m_0bec = 0;
    m_7c80 = owner->root;
    if (owner->root != 0)
    {
        int frames = owner->root->M_10199340();
        if (frames > 8)
        {
            m_7c70 = frames;
            return 1;
        }
    }
    m_7c70 = 0;
    return 1;
}

extern "C" int DAT_110175f8;  // number of motion blends started

// 0x1016a2e0 (Playback::Play). Starts layer `layer` at `speed`. Nothing happens (returns 0) for an inactive or
// out-of-range layer, or, unless `restart` is set, when the layer is already playing, being blended into or is the
// one the current layer chains to (m_076c); a layer listed in the current layer's follow-up table (m_5870) is
// left to come up by itself (returns 1). Without `blendFrames` the layer starts at once from its first frame;
// otherwise the playback blends into it over `blendFrames` + 1 frames (at most 15). Returns 1.
int Playback::Play(int layer, int restart, int blendFrames, float speed)
{
    if (layer < 0 || layer > 0x8f || m_00ac[layer] == 0)
        return 0;
    if (m_001c == layer && restart == 0 && m_076c[layer] != -1)
        return 0;
    if (m_0bec == 2 && m_7c78 == layer)
        return 0;
    if (m_001c != -1 && m_7c74 != 0 && m_7c7c == layer && restart == 0)
        return 0;
    if (m_001c != -1 && restart == 0 && m_001c == m_076c[layer])
        return 0;
    if (m_001c != -1 && restart == 0)
    {
        for (int i = 0; i < 8; i++)
        {
            int from = m_001c;
            if (from == 0x90)
                from = m_7c7c;
            if (m_5870[from][i] == layer)
                return 1;
        }
    }
    m_7c78 = -1;
    m_0bec = m_09ac[layer];
    m_0024 = 0;
    m_0008 = speed;
    if (blendFrames == 0)
    {
        m_7c7c = 0;
        m_7c74 = 0;
        m_001c = layer;
        m_0020 = m_02ec[layer];
        m_0004 = (float)m_02ec[layer];
        m_0bec = m_09ac[layer];
        return 1;
    }
    m_7c74 = 1;
    m_7c7c = layer;
    if (m_7c80 != 0)
    {
        if (blendFrames >= 14)
            blendFrames = 14;
        DAT_110175f8++;
        m_7c80->M_1019b630(m_0020, m_02ec[layer], blendFrames + 1);
        m_0014 = blendFrames;
    }
    m_001c = 0x90;
    m_0020 = -1;
    m_0004 = -1.0f;
    return 1;
}
