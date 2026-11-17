#include <string.h>

struct Matrix
{
    float m[16];
};

struct NodeSaved
{
    char pad_0000[0x20];
    int m_0020;
    Matrix matrix;
    char pad_0064[0x34];
    int m_0098[8];
    int m_00b8;
    char pad_00bc[0x20];
    void M_10166870();
};

struct KeyPair
{
    int data[2];
    void M_101ad0e0();
};

struct TextureRef;
extern "C" int FUN_10169c00(TextureRef *texture);

struct Node;

struct GlyphState
{
    char pad_0000[0xc4];
    Matrix m_00c4;
    char pad_0104[0x1104];
    Node *bones[0x44];
    char pad_1318[0x1100];
    Matrix bindPose[0x44];
    Matrix m_3518[0x44];
    char pad_4618[0x1000];
    int m_5618;
};

struct MeshData
{
    void M_10150a90();
    void M_101476e0(int pack, int value);
};
extern "C" int FUN_10169990(MeshData *mesh);

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

struct Material
{
    char name[0x40];
    char pad_0040[0x140];
};

struct PackHeader
{
    char pad_0000[0xc];
    int m_000c;
    int m_0010;
    int material;
    char pad_0018[0x5c];
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
    char name[0x40];
    char pad_0040[0x24];
    int packCount;
    PackHeader *packs;
    KeyPair *indexLists;
    KeyPair *m_0070;
    char pad_0074[0x1c];
    int materialCount;
    struct Material *materials;
    void *m_0098;
    struct MeshData *mesh;
    int m_00a0;
    int pad_00a4;
    int m_00a8;
    unsigned int flags;
    int pad_00b0;
    int group;
    int m_00b8;
    char pad_00bc[4];
    int plugType[16];
    struct Channel *plugs[16];
    int plugTypeCopy[16];
    void *plugData[16];
    char pad_01c0[8];
    int m_01c8;
    Matrix matrix;
    char pad_020c[0x80];
    struct TextureRef *m_028c;
    void *m_0290;
    int m_0294;
    char pad_0298[0x10];
    struct Extra *m_02a8;
    char pad_02ac[0x458];
    char m_0704;
    char pad_0705[3];
    struct GlyphState *m_0708;
    char pad_070c[0x40];
    Node *parent;
    int childCount;
    char childNames[0x40][0x40];
    Node *children[0x40];
    NodeSaved *saved;
    unsigned char m_1858;
    char pad_1859[3];
    int m_185c;
    int m_1860;
    char pad_1864[0x34];
    char element;
    char pad_1899;
    char m_189a;
    char m_189b;
    char m_189c;
    char pad_189d[3];

    void M_10191d50();
    void M_1019eeb0(Node *root, Node **list, int *count);
    void M_101ab9d0(Node *root);
    int M_10191770();
    void M_1019e9a0();
    int RemoveChild(Node *child, int keep);
    int CopyPlugData();
    int SetMaterialValue(const char *material, int value);
    void M_1019eca0();
    int M_10191870(Node **list, int *count);
    int Rebuild();
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
    int M_10191920(Node **list, int *count);
    void M_10191210();
    int M_10191990(const int *values, int count);
    Node *M_10191cc0(unsigned int mask, int nth, int *counter);
    void M_101911b0(PackHeader *packs, int count);
    void ReleaseMesh();
    int M_10191e50();
    int M_10193cb0(int from, int to);
    int M_10194630(char value, unsigned int mask);
    int M_101946b0(char value, unsigned int mask);
    int M_10194730(char value, unsigned int mask);
    int M_101948f0(void *target);
    int M_10194ee0();
    int M_1019b050(int value);
    int M_1019b0c0(int value, int except1, int except2);
    int M_1019b1b0(unsigned int bits);
    int M_1019bca0(Node **keep, int count);
};


extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" void (*DAT_11b1ba40)(void *);
extern "C" char DAT_11b07118[];
extern "C" int DAT_11b002c0;
extern "C" int DAT_11b044e0;
extern "C" int DAT_11b044e4;
extern "C" int DAT_11b044e8;
extern "C" int DAT_11b044ec;
extern "C" int DAT_11b044f4;
extern "C" int DAT_10203474;
extern "C" int DAT_11b044f8;
extern "C" int DAT_11b04500;
extern "C" int DAT_10203478;
extern "C" int DAT_11b04504;
extern "C" int DAT_1020347c;
extern "C" char DAT_11b04508;
extern "C" int DAT_11b0450c;
extern "C" int DAT_11b04510;
extern "C" char DAT_11b04514;
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
        if (plugType[6 + i] != 0 && plugType[6 + i] == 0x4ee33)
        {
            plugs[6 + i]->keys.M_101ad690();
            plugs[6 + i]->keys.M_101ad5c0(0, a, b);
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
    if (m_0070 != 0 && packCount != 0)
        M_101911e0(m_0070, packCount);
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019e120();
    return 0;
}
int Node::M_10191920(Node **list, int *count)
{
    if ((flags & 0x4000) != 0)
    {
        list[*count] = this;
        if (*count < 7)
            (*count)++;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191920(list, count);
    return 1;
}

void Node::M_10191210()
{
    if (FUN_10169c00(m_028c) == 0)
    {
        DAT_11b1ba40(m_028c);
        m_028c = 0;
        if (m_0290 != 0)
            DAT_11b1ba40(m_0290);
        m_0290 = 0;
        m_0294 = 0;
    }
    m_0290 = 0;
    m_0294 = 0;
    m_028c = 0;
}
int Node::M_10191990(const int *values, int count)
{
    if ((flags & 0x1000) != 0)
    {
        for (int i = 0; i < count; i++)
            saved->m_0098[i] = values[i];
        saved->m_00b8 = count;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191990(values, count);
    return 1;
}

Node *Node::M_10191cc0(unsigned int mask, int nth, int *counter)
{
    int local = 0;
    if (counter == 0)
        counter = &local;
    if ((flags & mask) == mask)
    {
        if (++*counter > nth)
            return this;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->M_10191cc0(mask, nth, counter);
            if (found != 0)
                return found;
        }
    }
    return 0;
}
void Node::ReleaseMesh()
{
    if (mesh != 0 && FUN_10169990(mesh) == 0)
    {
        mesh->M_10150a90();
        DAT_11b1ba40(mesh);
        mesh = 0;
        M_101911b0(packs, packCount);
        M_101911e0(indexLists, packCount);
        if (packs != 0)
            DAT_11b1ba40(packs);
        packs = 0;
        if (indexLists != 0)
            DAT_11b1ba40(indexLists);
        indexLists = 0;
        packCount = 0;
    }
}
int Node::M_10191e50()
{
    if (_strnicmp(name, "fire", 4) == 0)
        element = 1;
    if (_strnicmp(name, "thun", 4) == 0)
        element |= 2;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191e50();
    return 0;
}

int Node::M_10193cb0(int from, int to)
{
    if (from == m_00a0)
        m_00a0 = to;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10193cb0(from, to);
    return 0;
}

int Node::M_10194630(char value, unsigned int mask)
{
    if ((flags & mask) != 0)
    {
        if ((flags & 0x400) != 0)
            m_189c = value;
        if ((flags & 0x200) != 0)
            m_189c = value;
        if ((flags & 0x800) != 0)
            m_189c = value;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10194630(value, mask);
    return 0;
}

int Node::M_101946b0(char value, unsigned int mask)
{
    if ((flags & mask) != 0)
    {
        if ((flags & 0x400) != 0)
            m_189a = value;
        if ((flags & 0x200) != 0)
            m_189a = value;
        if ((flags & 0x800) != 0)
            m_189a = value;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_101946b0(value, mask);
    return 0;
}

int Node::M_10194730(char value, unsigned int mask)
{
    if ((flags & mask) != 0)
    {
        if ((flags & 0x400) != 0)
            m_189b = value;
        if ((flags & 0x200) != 0)
            m_189b = value;
        if ((flags & 0x800) != 0)
            m_189b = value;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10194730(value, mask);
    return 0;
}

int Node::M_101948f0(void *target)
{
    if (saved != 0)
    {
        if (target != 0)
            m_0098 = target;
        else
            m_0098 = DAT_11b07118;
        if ((flags & 0x400000) != 0)
        {
            if (saved->m_0020 == 2)
            {
                saved->m_0020 = 1;
                return 1;
            }
        }
        else
        {
            M_101947f0();
            M_10194870();
            M_101948c0();
            M_10194830();
            saved->M_10166870();
            flags |= 0x400000;
        }
    }
    return 1;
}

int Node::M_10194ee0()
{
    if (m_1858 != 0)
    {
        m_1858++;
        if (m_1858 - 1 >= childCount)
            m_1858 = 1;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10194ee0();
    return 0;
}

extern "C" void FUN_101997f0(void)
{
    DAT_11b002c0 = 0;
    DAT_11b044d0 = 0;
    DAT_11b044dc = 0;
    DAT_11b044e0 = 0;
    DAT_11b044e4 = 0;
    DAT_11b044e8 = 0;
    DAT_11b044ec = 0;
    DAT_11b044f4 = 0;
    DAT_10203474 = 1;
    DAT_11b044f8 = 0;
    DAT_11b04500 = 0;
    DAT_10203478 = 300;
    DAT_11b04504 = 0;
    DAT_1020347c = 1;
    DAT_11b04508 = 0;
    DAT_11b0450c = 0;
    DAT_11b04510 = 0;
    DAT_11b04514 = 0;
}

int Node::M_1019b050(int value)
{
    if (value == 8 && m_0708 != 0 && m_0708->m_5618 != 0)
        m_00a0 = 0;
    else
        m_00a0 = value;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b050(value);
    return 1;
}

int Node::M_1019b0c0(int value, int except1, int except2)
{
    if ((except1 == -1 || m_00a0 != except1) && (except2 == -1 || m_00a0 != except2))
        m_00a0 = value;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b0c0(value, except1, except2);
    return 1;
}

int Node::M_1019b1b0(unsigned int bits)
{
    flags &= ~bits;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b1b0(bits);
    return 1;
}
int Node::M_1019bca0(Node **keep, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (this == keep[i])
            break;
    }
    if (i == count && m_028c != 0)
        M_10191210();
    for (int k = 0; k < childCount; k++)
        children[k]->M_1019bca0(keep, count);
    return 1;
}
int Node::Rebuild()
{
    int count = 0;
    Node *list[16];
    M_1019eeb0(this, list, &count);
    M_101ab9d0(this);
    M_10191770();
    M_1019eca0();
    M_10191870(list, &count);
    return 1;
}
int Node::M_10191870(Node **list, int *count)
{
    if ((flags & 0x1000) != 0 && saved != 0)
    {
        int linked = 0;
        for (int i = 0; i < *count; i++)
        {
            if (i >= 8)
                break;
            Node *other = list[i];
            if (group == other->group)
            {
                saved->m_0098[linked] = (int)other;
                linked++;
            }
        }
        saved->m_00b8 = linked;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191870(list, count);
    return 1;
}
int Node::M_10191770()
{
    if (m_0708 != 0 && m_0708->m_5618 != 0)
    {
        for (int i = 0; i < 0x44; i++)
        {
            Node *bone = m_0708->bones[i];
            if (bone != 0 && bone->m_0704 != 0 && _strnicmp(bone->name, "bip", 3) == 0)
            {
                m_0708->bones[i]->m_0708->m_00c4 = m_0708->bindPose[i];
                m_0708->bones[i]->m_0708->m_3518[0] = m_0708->m_3518[i];
            }
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191770();
    return 1;
}
int Node::RemoveChild(Node *child, int keep)
{
    int i;
    for (i = 0; i < childCount; i++)
    {
        if (child == children[i])
            break;
    }
    if (i == childCount)
        return 0;
    if (keep == 0)
    {
        children[i]->M_1019e9a0();
        DAT_11b1ba40(children[i]);
    }
    int j;
    for (j = i; j < childCount - 1; j++)
    {
        children[j] = children[j + 1];
        strcpy(childNames[j], childNames[j + 1]);
    }
    childCount--;
    children[j] = 0;
    return 0;
}
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);

int Node::CopyPlugData()
{
    for (int i = 0; i < 16; i++)
    {
        if (plugs[i] != 0)
        {
            int size = *(int *)plugs[i];
            if (plugData[i] != 0)
                DAT_11b1ba40(plugData[i]);
            plugData[i] = DAT_11b0b938(size, "->m_plugdata[g]");
            memcpy(plugData[i], plugs[i], size);
            plugTypeCopy[i] = plugType[i];
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->CopyPlugData();
    return 0;
}
int Node::SetMaterialValue(const char *material, int value)
{
    for (int m = 0; m < materialCount; m++)
    {
        if (_stricmp(materials[m].name, material) == 0 && packCount != 0 && packs != 0 && mesh != 0)
        {
            for (int i = 0; i < packCount; i++)
            {
                if (packs[i].material == m)
                    mesh->M_101476e0(i, value);
            }
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetMaterialValue(material, value);
    return 1;
}