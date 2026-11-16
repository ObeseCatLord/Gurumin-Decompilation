#include <string.h>

struct Matrix
{
    float m[16];
};

struct NodeSaved
{
    char pad_0000[0x24];
    Matrix matrix;
    char pad_0064[0x78];
    void M_10166870();
};

struct KeyPair
{
    int data[2];
    void M_101ad0e0();
};

struct Keys
{
    void M_101ad690();
    void M_101ad5c0(int a, int b, int c);
};

struct Channel
{
    int pad_0000;
    Keys keys;
};

struct Extra
{
    void M_10169660();
};

struct PackHeader
{
    char pad_0000[0xc];
    int m_000c;
    char pad_0010[0x64];
    void M_101acfa0();
};

struct IndexList
{
    int data[2];
    void M_101ad1b0();
};

struct KeyPair;

struct Node
{
    char pad_0000[0x64];
    int m_0064;
    char pad_0068[8];
    KeyPair *m_0070;
    char pad_0074[0x2c];
    int m_00a0;
    int pad_00a4;
    int m_00a8;
    unsigned int flags;
    char pad_00b0[8];
    int m_00b8;
    char pad_00bc[0x1c];
    int m_00d8[2];
    char pad_00e0[0x38];
    struct Channel *m_0118[2];
    char pad_0120[0xa8];
    int m_01c8;
    Matrix matrix;
    char pad_020c[0x88];
    int m_0294;
    char pad_0298[0x10];
    struct Extra *m_02a8;
    char pad_02ac[0x4a0];
    Node *parent;
    int childCount;
    char pad_0754[0x1000];
    Node *children[0x40];
    NodeSaved *saved;
    int pad_1858;
    int m_185c;
    int m_1860;
    char pad_1864[0x3c];

    void M_10191d50();
    void M_10191d90();
    void M_10191dd0();
    void M_10191e10();
    int M_101944d0();
    void M_10194510();
    int M_101947b0();
    int M_101947f0();
    int M_10194830();
    int M_10194870();
    int M_101948c0();
    int M_10194980();
    int M_101949d0();
    Node *M_101992f0();
    Node *M_10199310();
    int M_10199340();
    int M_1019b190(unsigned int bits);
    int M_1019b210(unsigned int bits);
    int M_1019b230(int value);
    int M_1019b300(int a, int b);
    int M_1019d680();
    void M_1019d600();
    void M_1019d6a0();
    int M_101a8760();
    void M_101911e0(KeyPair *pairs, int count);
    void M_1019e120();
    int M_101ab980();
    int M_10191a30(const char *name, int *value);
    int M_10191c30(const char *name, int *value);
    int M_10191aa0(const char *name, int *value);
    int M_10191b60(const char *name, int *value);
    int M_1019ece0(const char *name, int value);
    int M_1019ed10(const char *name, int value);
};


extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" void (*DAT_11b1ba40)(void *);
extern "C" int DAT_11b002b8;
extern "C" int DAT_11b044dc;
extern "C" int DAT_11b044d0;
extern "C" int DAT_11b002d0[0x80];

extern "C" void __stdcall FUN_101911e0(KeyPair *pairs, int count)
{
    if (pairs != 0)
    {
        for (; count > 0; count--)
        {
            pairs->M_101ad0e0();
            pairs++;
        }
    }
}

void Node::M_10191d50()
{
    m_1860 = flags;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191d50();
}

void Node::M_10191d90()
{
    flags = m_1860;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191d90();
}

void Node::M_10191dd0()
{
    m_185c = m_00a0;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191dd0();
}

void Node::M_10191e10()
{
    m_00a0 = m_185c;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191e10();
}

int Node::M_101944d0()
{
    m_00a8 = 0;
    for (int i = 0; i < childCount; i++)
        children[i]->M_101944d0();
    return 1;
}

int Node::M_101947b0()
{
    if (saved == 0)
    {
        saved = (NodeSaved *)DAT_11b0b938(0xdc, "");
        saved->M_10166870();
    }
    return 1;
}

int Node::M_101947f0()
{
    if (saved != 0 && (flags & 0x1000) != 0)
    {
        matrix = saved->matrix;
        flags &= ~0x1000;
    }
    return 1;
}

int Node::M_10194830()
{
    if (saved != 0 && (flags & 0x400000) != 0)
    {
        matrix = saved->matrix;
        flags &= ~0x400000;
    }
    return 1;
}

int Node::M_10194870()
{
    if (saved != 0 && (flags & 0x2000) != 0)
    {
        matrix = saved->matrix;
        flags &= ~0x2000;
        M_10194510();
    }
    return 1;
}

int Node::M_101948c0()
{
    if (saved != 0 && (flags & 0x4000) != 0)
    {
        flags &= ~0x4000;
        M_10194510();
    }
    return 1;
}

int Node::M_10194980()
{
    if (saved != 0)
    {
        M_101947f0();
        M_10194870();
        M_101948c0();
        M_10194830();
        DAT_11b1ba40(saved);
        saved = 0;
    }
    return 1;
}

int Node::M_101949d0()
{
    if (m_02a8 == 0)
    {
        m_02a8 = (Extra *)DAT_11b0b938(0x80, "");
        m_02a8->M_10169660();
    }
    return 1;
}

Node *Node::M_101992f0()
{
    Node *node = this;
    while (node->parent != 0)
        node = node->parent;
    return node;
}

Node *Node::M_10199310()
{
    Node *node = this;
    while (node->parent != 0 && (node->parent->flags & 0x1000) != 0)
        node = node->parent;
    return node;
}

int Node::M_10199340()
{
    int last = m_0294;
    if (last < m_01c8)
        last = m_01c8;
    int count = childCount;
    for (int i = 0; i < count; i++)
    {
        int child = children[i]->M_10199340();
        if (child > last)
            last = child;
    }
    return last;
}

extern "C" void FUN_10199860(void)
{
    DAT_11b002b8 = 0;
    DAT_11b044dc = 0;
    DAT_11b044d0 = 0;
}

int Node::M_1019b190(unsigned int bits)
{
    flags |= bits;
    return 1;
}

int Node::M_1019b210(unsigned int bits)
{
    flags &= ~bits;
    return 1;
}

int Node::M_1019b230(int value)
{
    m_00b8 = value;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b230(value);
    return 1;
}

int Node::M_1019b300(int a, int b)
{
    for (int i = 0; i < 2; i++)
    {
        if (m_00d8[i] != 0 && m_00d8[i] == 0x4ee33)
        {
            m_0118[i]->keys.M_101ad690();
            m_0118[i]->keys.M_101ad5c0(0, a, b);
        }
    }
    return 0;
}

extern "C" void FUN_1019bdb0(void)
{
    memset(DAT_11b002d0, 0, sizeof(DAT_11b002d0));
}

extern "C" PackHeader *__stdcall FUN_1019c320(int count)
{
    PackHeader *packs = (PackHeader *)DAT_11b0b938(count * sizeof(PackHeader), "");
    for (int i = 0; i < count; i++)
    {
        packs[i].M_101acfa0();
        packs[i].m_000c = 0;
    }
    return packs;
}

extern "C" IndexList *__stdcall FUN_1019c360(int count)
{
    IndexList *lists = (IndexList *)DAT_11b0b938(count * sizeof(IndexList), "");
    for (int i = 0; i < count; i++)
        lists[i].M_101ad1b0();
    return lists;
}
int Node::M_1019d680()
{
    flags &= ~0x8000;
    if (parent != 0)
        parent->M_1019d680();
    return 0;
}

int Node::M_1019ece0(const char *name, int value)
{
    int result = value;
    if (*name == '@')
        return M_10191c30(name + 1, &result);
    return M_10191a30(name, &result);
}

int Node::M_1019ed10(const char *name, int value)
{
    int result = value;
    if (*name == '@')
        return M_10191b60(name + 1, &result);
    return M_10191aa0(name, &result);
}


int Node::M_101a8760()
{
    M_1019d600();
    M_1019d6a0();
    return 0;
}

int Node::M_101ab980()
{
    if (m_0070 != 0 && m_0064 != 0)
        M_101911e0(m_0070, m_0064);
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019e120();
    return 0;
}