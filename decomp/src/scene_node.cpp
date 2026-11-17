// Scene nodes: the node tree of every model (0x18a0 bytes per node). A node has a name, flags, a mesh with packed
// vertex data (packs, index lists, materials), animation plugs, a matrix, a parent and up to 64 named children.
// Most methods here walk the subtree recursively, applying a change to the node and then to every child.
//
// Flags (`flags`, 0xac): 0x4 visible, 0x20 solid wall (wall_flags.cpp), 0x200/0x400/0x800 node kinds that take the
// colour values, 0x1000/0x2000/0x4000/0x400000 matrix states restored from `saved`, 0x8000 special pass marker.

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
    void M_10147690(int a, int b);
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
    char pad_0018[0x58];
    int *m_0070;
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
    char pad_02ac[0x440];
    int m_06ec;
    char pad_06f0[0xc];
    int m_06fc;
    char pad_0700[4];
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
    void M_10191500();
    int M_101a53b0(Node *source, int a, int b);
    void M_10193d10(int index);
    void M_101913f0();
    int CreateChildCopies();
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
    int M_1019b130(unsigned int bits);
    int M_1019b190(unsigned int bits);
    int M_1019b210(unsigned int bits);
    int M_1019b230(int value);
    int M_1019b300(int a, int b);
    int M_1019d680();
    int M_1019d600();
    Node *FindRecursive(const char *name, int *skip);
    Node *FindPrefixRecursive(const char *prefix, int *skip);
    int M_1019e830(int a, int b);
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

// Frees the index buffers of `count` index lists.
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

// Saves the flags of the whole subtree (m_1860).
void Node::M_10191d50()
{
    m_1860 = flags;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191d50();
}

// Restores the flags saved by M_10191d50.
void Node::M_10191d90()
{
    flags = m_1860;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191d90();
}

// Saves the render mode (m_00a0) of the whole subtree.
void Node::M_10191dd0()
{
    m_185c = m_00a0;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191dd0();
}

// Restores the render mode saved by M_10191dd0.
void Node::M_10191e10()
{
    m_00a0 = m_185c;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10191e10();
}

// Clears m_00a8 in the whole subtree. Returns 1.
int Node::M_101944d0()
{
    m_00a8 = 0;
    for (int i = 0; i < childCount; i++)
        children[i]->M_101944d0();
    return 1;
}

// Allocates the node's saved-state block (0xdc bytes) on first use.
int Node::M_101947b0()
{
    if (saved == 0)
    {
        saved = (NodeSaved *)DAT_11b0b938(0xdc, "");
        saved->M_10166870();
    }
    return 1;
}

// Restores the saved matrix when state 0x1000 is set and clears that state.
int Node::M_101947f0()
{
    if (saved != 0 && (flags & 0x1000) != 0)
    {
        matrix = saved->matrix;
        flags &= ~0x1000;
    }
    return 1;
}

// Restores the saved matrix when state 0x400000 is set and clears that state.
int Node::M_10194830()
{
    if (saved != 0 && (flags & 0x400000) != 0)
    {
        matrix = saved->matrix;
        flags &= ~0x400000;
    }
    return 1;
}

// Restores the saved matrix for state 0x2000, clears it and recomputes the world matrices (M_10194510).
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

// Clears state 0x4000 and recomputes the world matrices.
int Node::M_101948c0()
{
    if (saved != 0 && (flags & 0x4000) != 0)
    {
        flags &= ~0x4000;
        M_10194510();
    }
    return 1;
}

// Undoes every saved state and frees the saved-state block.
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

// Allocates the node's 0x80-byte extra block on first use.
int Node::M_101949d0()
{
    if (m_02a8 == 0)
    {
        m_02a8 = (Extra *)DAT_11b0b938(0x80, "");
        m_02a8->M_10169660();
    }
    return 1;
}

// Returns the root of the tree this node belongs to. Not byte-identical yet (see open.tsv).
Node *Node::M_101992f0()
{
    Node *node = this;
    while (node->parent != 0)
        node = node->parent;
    return node;
}

// Returns the highest ancestor reachable through parents in state 0x1000.
Node *Node::M_10199310()
{
    Node *node = this;
    while (node->parent != 0 && (node->parent->flags & 0x1000) != 0)
        node = node->parent;
    return node;
}

// Returns the last animation frame of the subtree (the highest of the nodes' frame counts).
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

// Empties the render queue and its extra-data and matrix pools (render_queue.cpp).
extern "C" void FUN_10199860(void)
{
    DAT_11b002b8 = 0;
    DAT_11b044dc = 0;
    DAT_11b044d0 = 0;
}

// Sets flag bits on the whole subtree. Returns 1.
int Node::M_1019b130(unsigned int bits)
{
    flags |= bits;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b130(bits);
    return 1;
}

// Sets flag bits on this node only.
int Node::M_1019b190(unsigned int bits)
{
    flags |= bits;
    return 1;
}

// Clears flag bits on this node only.
int Node::M_1019b210(unsigned int bits)
{
    flags &= ~bits;
    return 1;
}

// Sets m_00b8 on the whole subtree.
int Node::M_1019b230(int value)
{
    m_00b8 = value;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b230(value);
    return 1;
}

// Restarts the two colour plugs (type 0x4ee33 in plug slots 6 and 7) from (a, b).
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

// Clears the node pool table DAT_11b002d0.
extern "C" void FUN_1019bdb0(void)
{
    memset(DAT_11b002d0, 0, sizeof(DAT_11b002d0));
}

// Allocates and initialises `count` pack headers.
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

// Allocates and initialises `count` index lists.
extern "C" IndexList *__stdcall FUN_1019c360(int count)
{
    IndexList *lists = (IndexList *)DAT_11b0b938(count * sizeof(IndexList), "");
    for (int i = 0; i < count; i++)
        lists[i].M_101ad1b0();
    return lists;
}
// Clears the special pass marker (0x8000) on this node and all its ancestors.
int Node::M_1019d680()
{
    flags &= ~0x8000;
    if (parent != 0)
        parent->M_1019d680();
    return 0;
}

// Node::Find: the index-th node called `name` in the subtree; a leading '@' turns it into a prefix search
// ("@Spot" finds Spot01, Spot02, ...).
int Node::M_1019ece0(const char *name, int value)
{
    int result = value;
    if (*name == '@')
        return M_10191c30(name + 1, &result);
    return M_10191a30(name, &result);
}

// Like M_1019ece0 with the second pair of finders (M_10191b60 / M_10191aa0).
int Node::M_1019ed10(const char *name, int value)
{
    int result = value;
    if (*name == '@')
        return M_10191b60(name + 1, &result);
    return M_10191aa0(name, &result);
}


// Runs the special pass marking (M_1019d600) and the follow-up pass (M_1019d6a0).
int Node::M_101a8760()
{
    M_1019d600();
    M_1019d6a0();
    return 0;
}

// Frees the extra index lists of this node and runs M_1019e120 on the children.
int Node::M_101ab980()
{
    if (m_0070 != 0 && packCount != 0)
        M_101911e0(m_0070, packCount);
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019e120();
    return 0;
}
// Collects up to 8 nodes in state 0x4000 from the subtree into `list`.
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

// Releases the node's shared texture reference and frees its texture data when no one else uses it.
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
// Stores `count` values in the saved block of every node in state 0x1000 in the subtree.
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

// Finds the nth node (0-based) in the subtree whose flags contain all bits of `mask`.
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
// Releases the node's shared mesh and, when it was the last user, frees the mesh, packs and index lists.
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
// Tags nodes named "fire..." with element 1 and "thun..." with element 2 (Gurumin's fire and thunder elements).
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

// Replaces render mode `from` with `to` in the whole subtree.
int Node::M_10193cb0(int from, int to)
{
    if (from == m_00a0)
        m_00a0 = to;
    for (int i = 0; i < childCount; i++)
        children[i]->M_10193cb0(from, to);
    return 0;
}

// Sets colour value m_189c on the subtree's nodes of kinds 0x200/0x400/0x800 that match `mask`.
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

// Sets colour value m_189a the same way.
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

// Sets colour value m_189b the same way.
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

// Enters saved-matrix state 0x400000 with `target` (or the default DAT_11b07118) as the target object.
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

// Advances the cycling child index m_1858 (1..childCount) in the whole subtree (frame-by-frame child swap).
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

// Resets the render queue state and the render options to their defaults (fog distance 300, ...).
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

// Sets the render mode of the subtree (mode 8 is ignored for nodes with an active glyph cache).
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

// Sets the render mode of the subtree except for nodes in mode `except1` or `except2` (-1 = no exception).
int Node::M_1019b0c0(int value, int except1, int except2)
{
    if ((except1 == -1 || m_00a0 != except1) && (except2 == -1 || m_00a0 != except2))
        m_00a0 = value;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b0c0(value, except1, except2);
    return 1;
}

// Clears flag bits on the whole subtree.
int Node::M_1019b1b0(unsigned int bits)
{
    flags &= ~bits;
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019b1b0(bits);
    return 1;
}
// Releases the textures of every node in the subtree that is not in the `keep` list.
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
// Rebuilds the subtree's links after loading: collects linked nodes, rebinds bones, collects the 0x4000
// nodes and stores the links in their saved blocks.
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
// Stores in each linked node (state 0x1000) up to 8 nodes of `list` that share its group.
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
// Copies bind poses from a skinned node's glyph/bone cache into its "bip" bones.
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
// Removes `child` from the child list (destroying it unless `keep`). Returns 0.
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

// Makes private copies of every animation plug's data in the subtree.
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
// Sets `value` on every mesh subset that uses the material called `material`, in the whole subtree.
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
// Creates 32 child copies of this node (for effects drawn many times, such as particles).
int Node::CreateChildCopies()
{
    Node *created[0x20];
    m_06ec = packs->m_0070[2];
    m_06fc = packs->m_0070[2];
    m_1858 = 1;
    for (int i = 0; i < 0x20; i++)
    {
        Node *node = (Node *)DAT_11b0b938(0x18a0, "m_child[m_numchild]");
        created[i] = node;
        node->M_10191500();
        node->M_101a53b0(this, 0, 1);
    }
    childCount = 0x20;
    for (int i = 0; i < 0x20; i++)
    {
        children[i] = created[i];
        created[i]->parent = this;
        children[i]->M_10193d10(i);
        children[i]->m_00a0 = 0xc;
    }
    M_101913f0();
    return 1;
}

// Finds the node called `name` (case-insensitive) in this node's subtree, depth first, this node first.
// With `skip` set, the first *skip matches are passed over (and *skip counts down), so repeated calls find the
// second, third, ... node of that name. Returns 0 when there is no (further) match.
Node *Node::FindRecursive(const char *name, int *skip)
{
    if (_stricmp(this->name, name) == 0)
    {
        if (skip == 0)
            return this;
        if (*skip == 0)
            return this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindRecursive(name, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Marks this node for the special pass (flag 0x8000) when it is flagged 0x20 or has m_0704 set, unless it or its
// parent is in one of the excluded modes (flags 0x3e00 / parent 0x3000); otherwise passes the request on to the
// children. Returns 0 when this node was marked, 1 otherwise.
int Node::M_1019d600()
{
    if ((flags & 0x3e00) == 0 && (parent == 0 || (parent->flags & 0x3000) == 0) && ((flags & 0x20) != 0 || m_0704 != 0))
    {
        flags |= 0x8000;
        return 0;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019d600();
    return 1;
}

// Like FindRecursive, but matches nodes whose name starts with `prefix` (case-insensitive), e.g. "@Spot" finds
// "@Spot01" and "@Spot02".
Node *Node::FindPrefixRecursive(const char *prefix, int *skip)
{
    if (_strnicmp(name, prefix, strlen(prefix)) == 0)
    {
        if (skip == 0)
            return this;
        if (*skip == 0)
            return this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindPrefixRecursive(prefix, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Runs the mesh pass M_10147690(a, b) on this node's mesh when it has materials and either no packed data
// (packCount -1 with no packs) or packed data present, then on every child. Returns 1.
int Node::M_1019e830(int a, int b)
{
    if (materialCount != 0)
    {
        if (packCount == -1 && packs == 0 && mesh != 0)
            mesh->M_10147690(a, b);
        else if (packCount != 0 && packs != 0 && mesh != 0)
            mesh->M_10147690(a, b);
    }
    for (int i = 0; i < childCount; i++)
        children[i]->M_1019e830(a, b);
    return 1;
}
