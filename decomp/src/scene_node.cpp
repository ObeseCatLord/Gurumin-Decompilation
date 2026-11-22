// Scene nodes: the node tree of every model (0x18a0 bytes per node). A node has a name, flags, a mesh with packed
// vertex data (packs, index lists, materials), animation plugs, a matrix, a parent and up to 64 named children.
// Most methods here walk the subtree recursively, applying a change to the node and then to every child.
//
// Flags (`flags`, 0xac): 0x4 visible, 0x20 solid wall (wall_flags.cpp), 0x200/0x400/0x800 node kinds that take the
// colour values, 0x1000/0x2000/0x4000/0x400000 matrix states restored from `saved`, 0x8000 special pass marker.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"

struct NodeSaved
{
    char pad_0000[0x20];
    int m_0020;
    Matrix4 matrix;
    char pad_0064[0x34];
    int m_0098[8];
    int m_00b8;
    char pad_00bc[0x20];
    void M_10166870();
};

struct KeyPair
{
    int count;
    union
    {
        int *indices;            // index list: the vertex indices (three per triangle)
        struct Vec4 *spheres;    // sphere list: centre and radius (w) of each triangle
    };
    void M_101ad0e0();
    void Init();                                 // 0x101ad1b0
    void Copy(const KeyPair *source);            // 0x101ad160
    int Resize(int count);                       // 0x101ad1c0 (IndexSpanList::Resize)
};

// A reference-counted cache entry (mesh_cache.cpp): the motion of a node points to one holding its cache slot.
struct TextureRef
{
    int slot;
};
extern "C" int FUN_10169c00(TextureRef *texture);
extern "C" int FUN_10169be0(TextureRef *texture);  // adds a reference

struct Node;

struct GlyphState
{
    char pad_0000[0x84];
    char parentName[0x40];      // 0x84: name of the node this one is bound to
    Matrix4 m_00c4;
    int pad_0104;
    char linkNames[0x40][0x40]; // 0x108: names of the linked nodes
    Node *links[0x40];          // 0x1108: nodes this skinned node depends on (m_5618 of them)
    Node *bones[0x44];
    char boneNames[0x44][0x40]; // 0x1318: names of the bones
    Matrix4 bindPose[0x44];
    Matrix4 m_3518[0x44];
    char pad_4618[0x1000];
    int m_5618;
    void Clear();               // 0x10166810: empties the skin data
};

// A vertex of a pack, 0x70 bytes (position first).
struct PackVertex
{
    float position[3];
    char pad_000c[0x64];
};

struct MeshData
{
    char pad_0000[0x4c];
    int bufferSlot;              // 0x4c: entry of the mesh buffer tables (DAT_10feb3e0 ...), -1 = none
    int M_10150bb0(struct PackHeader *packs, KeyPair *indexLists, int packCount, struct Material *materials, int flags);
    void Release();  // 0x10150a90 (mesh_release.cpp)
    void M_101476e0(int pack, int value);
    void M_10147690(int a, int b);
    void Init();                                 // 0x10145a10
};
extern "C" int FUN_10169990(MeshData *mesh);
extern "C" int FUN_10169970(MeshData *mesh);     // MeshCache_AddRef

struct Keys
{
    void M_101ad690();
    void M_101ad5c0(int a, int b, int c);
};

struct Channel
{
    int size;                    // size of the whole channel block in bytes
    Keys keys;
};

// A key track (vec4.cpp KeyTrack), 0x14 bytes.
struct KeyTrack
{
    char data[0x14];
    void Copy(const KeyTrack *source);           // 0x101ade10
};

struct Extra
{
    KeyTrack tracks[5];          // 0x00
    char pad_0064[0xc];
    int m_0070[4];               // 0x70
    Matrix4 AnimatedMatrix(int frame);            // 0x101693f0 (node_extra.cpp)
    void M_10169660();
    void M_10169620();           // 0x10169620 (SlotLists::Clear): frees the extra data's lists
};

// Mesh material (0x180 bytes).
struct Material
{
    char name[0x40];
    char textures[4][0x40];      // 0x40: texture file names (empty = none)
    float colour[3];             // 0x140: r, g, b (1 = white)
    char pad_014c[0x24];
    int m_0170;
    float alpha;                 // 0x174: 0..255
    int m_0178;
    unsigned char m_017c[4];
};

struct PackHeader
{
    float center[3];             // 0x00: centre of the pack's vertices
    float radius;                // 0x0c: bounding radius (0 = not needed)
    int m_0010;
    int material;
    int pad_0018;
    int vertexCount;             // 0x1c
    float boxMin[4];             // 0x20: (w = 1)
    float boxMax[4];             // 0x30
    char pad_0040[0x30];
    PackVertex *vertices;        // 0x70: the vertices
    void M_101acfa0();
    void Copy(const PackHeader *source);         // 0x101ad040
};

struct IndexList
{
    int data[2];
    void M_101ad1b0();
};

struct KeyPair;

Matrix4 InverseMatrix(const Matrix4 &matrix);      // 0x101b0d40 (node_bones.cpp)

struct Node
{
    char name[0x40];
    char folder[0x10];           // 0x40: texture folder of the model, inherited by children
    char m_0050[0x14];           // 0x50: second inherited name
    int packCount;
    PackHeader *packs;
    KeyPair *indexLists;
    KeyPair *m_0070;
    int m_0074;
    int m_0078;
    int m_007c;                  // 0x7c: number of entries in m_0080 and m_0084
    PackHeader *m_0080;
    KeyPair *m_0084;
    char m_0088;
    char isStatic;               // 0x89: 1 once the node was collected as static scenery (or is the collector)
    short staticCount;           // 0x8a: on the collector: number of entries in `staticNodes`
    Node **staticNodes;          // 0x8c: on the collector: the collected nodes (room for 0x200)
    int materialCount;
    struct Material *materials;
    void *m_0098;
    struct MeshData *mesh;
    int m_00a0;
    int built;                   // 0xa4: 1 once BuildMesh has created the mesh buffers
    int m_00a8;
    unsigned int flags;
    int m_00b0;
    int group;
    int m_00b8;
    char pad_00bc[4];
    int plugType[16];
    struct Channel *plugs[16];
    int plugTypeCopy[16];
    void *plugData[16];
    int m_01c0;
    int m_01c4;
    int m_01c8;
    Matrix4 matrix;
    Matrix4 inverseMatrix;        // 0x20c: inverse of `matrix`
    Matrix4 worldMatrix;          // 0x24c: the node's matrix in world space

    struct TextureRef *m_028c;
    Matrix4 *frames;              // 0x290: the node's matrix for each animation frame
    int m_0294;                  // 0x294: number of frames
    char pad_0298[0x10];
    struct Extra *m_02a8;
    char pad_02ac[0x20];
    int frame;                   // 0x2cc: current animation frame (-1, -2, ...: a stored pose)
    unsigned char kind;          // 0x2d0: node kind from the file (0xff = none: decided by the name prefix)
    char pad_02d1[3];
    Matrix4 poses[16];            // 0x2d4: the current pose ([0]) and stored poses
    Vec4 bounds[3];              // 0x6d4: bounding sphere (w = radius) and the two box corners
    char m_0704;
    char pad_0705[3];
    struct GlyphState *m_0708;
    char m_070c[0x40];
    Node *parent;
    int childCount;
    char childNames[0x40][0x40];
    Node *children[0x40];
    NodeSaved *saved;
    unsigned char m_1858;
    char pad_1859[3];
    int m_185c;
    int m_1860;
    void *m_1864;
    Vec4 m_1868;
    char m_1878[7];
    char pad_187f;
    int m_1880[5];
    char pad_1894[4];
    char element;
    char pad_1899;
    char m_189a;
    char m_189b;
    char m_189c;
    char pad_189d[3];

    void M_10191d50();
    int M_1019eeb0(Node *root, Node **list, int *count);
    int CopyMesh(Node *source, int slot);
    int BlendFrames(int from, int to, float t);
    int CopyAnimation(Node *source, int slot);
    void M_101ab9d0(Node *root);
    int M_10191770();
    void Release();
    int RemoveChild(Node *child, int keep);
    int CopyPlugData();
    int SetMaterialValue(const char *material, int value);
    void M_10191500();
    void M_101a2290();
    Node *AddChild(const char *childName, int position);
    int AllocMaterials(int count);
    int Clone(Node *source, int a, int b);
    int ComputeBounds();
    int BuildCollisionSpheres();

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
    int M_10194f40(float *position, Matrix4 *matrix, int a, float radius, int b, int depth);
    int StartProximityPass(float *position, Matrix4 *matrix, int a, float radius, int b);
    int ReleaseSavedStates();
    // Puts the saved matrix back when the saved block holds one, and marks the block empty.
    void RestoreSavedMatrix()
    {
        if (saved->m_0020 != 0)
            matrix = saved->matrix;
        saved->m_0020 = 0;
    }
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
    int ClearSpecialPassOnLinks();
    int M_101a8760();
    void M_101911e0(KeyPair *pairs, int count);
    void M_1019e120();
    int M_101ab980();
    Node *M_10191a30(const char *name, int *value);
    Node *M_10191c30(const char *name, int *value);
    int FindRecursiveStripped(const char *name, int *skip);
    int BuildMesh(int recurse, int flags);
    int CollectNearSegment(int unused, const Vec4 *from, const Vec4 *to, float range, Node **found, int *foundCount,
                           Vec4 *closest);
    int FindPrefixRecursiveStripped(const char *prefix, int *skip);
    Node *Find(const char *name, int index);
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
    Node *Root();
    int SetUniqueName(const char *name);
    void MakeNameUnique();
    void ReleaseUniqueMeshData();
    void CollectStatic(Node *collector);
    int Rename(const char *newName, const char *oldName);
    int TagRegion(const Box2 *boxes, const char (*names)[0x40], Matrix4 *matrix);
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
extern "C" float DAT_11b044f8;  // radius of the current proximity pass (used by M_10194f40)
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

// Starts a proximity pass over the subtree: clears m_00a8 everywhere, remembers `radius` and runs the recursive
// pass M_10194f40 from depth 0. Returns 1. (It sits here, after M_101944d0, because the compiler keeps `this` in
// edx across that call only when it can see M_101944d0's body.)
int Node::StartProximityPass(float *position, Matrix4 *matrix, int a, float radius, int b)
{
    M_101944d0();
    DAT_11b044f8 = radius;
    M_10194f40(position, matrix, a, radius, b, 0);
    return 1;
}

// Finds the collision nodes (flag 0x400, active m_00a8) in the subtree that lie near the segment from-to: within
// `range` plus 0.578 of their bounds radius. Bones are skipped unless DAT_11b04510 is set. Each hit is appended to
// `found` (when given) and counted in *foundCount; without a list the search stops at the first hit. *closest
// receives the closest segment point of the last node tested. Returns 1
// when a node was found.
int Node::CollectNearSegment(int unused, const Vec4 *from, const Vec4 *to, float range, Node **found, int *foundCount,
                             Vec4 *closest)
{
    if (DAT_11b04510 != 0 || m_0704 == 0)
    {
        if ((flags & 0x400) == 0)
        {
            for (int i = 0; i < childCount; i++)
            {
                int hit = children[i]->CollectNearSegment(unused, from, to, range, found, foundCount, closest);
                if (hit != 0 && found == 0)
                    return hit;
            }
        }
        else if (m_00a8 != 0)
        {
            if (worldMatrix.GetTranslation().DistanceToSegment(*from, *to, *closest) < bounds[0].w * 0.578f + range)
            {
                if (found != 0)
                {
                    found[*foundCount] = this;
                    (*foundCount)++;
                }
                return 1;
            }
        }
    }
    return 0;
}

// Releases the saved matrix states of the whole subtree: states 0x1000, 0x2000 and 0x400000 put their saved
// matrix back, state 0x4000 just empties the block, and the temporary states 0x40000/0x80000 are cleared and free
// the saved block (M_10194980). Returns 1.
int Node::ReleaseSavedStates()
{
    if (saved != 0)
    {
        if (flags & 0x1000)
            RestoreSavedMatrix();
        if (flags & 0x4000)
            saved->m_0020 = 0;
        if (flags & 0x2000)
            RestoreSavedMatrix();
        if (flags & 0x400000)
            RestoreSavedMatrix();
        if (flags & 0x40000)
        {
            flags &= ~0x40000;
            RestoreSavedMatrix();
            M_10194980();
        }
        else if (flags & 0x80000)
        {
            flags &= ~0x80000;
            RestoreSavedMatrix();
            M_10194980();
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ReleaseSavedStates();
    return 1;
}

// Creates a child node called `childName` ("none" for 0) at `position` in the child list (appended when the
// position is negative or past the end). The child inherits the folder names, starts in render mode 8 with flag
// 0x40 and is prepared by M_101a2290. Returns the child, or 0 when this node already has 64 children.
Node *Node::AddChild(const char *childName, int position)
{
    if (childCount > 0x3f)
        return 0;
    Node *child = (Node *)DAT_11b0b938(0x18a0, "m_child[m_numchild]");
    child->M_10191500();
    child->parent = this;
    strcpy(child->folder, folder);
    strcpy(child->m_0050, m_0050);
    child->m_00a0 = 8;
    child->flags = 0x40;
    if (childName == 0)
        strcpy(child->name, "none");
    else
        strcpy(child->name, childName);
    if (position >= childCount || position < 0)
    {
        children[childCount] = child;
        childCount++;
        children[childCount - 1]->M_101a2290();
        return children[childCount - 1];
    }
    for (int i = childCount - 1; i >= position; i--)
        children[i + 1] = children[i];
    children[position] = child;
    childCount++;
    children[position]->M_101a2290();
    return children[position];
}

// Replaces the node's material array with `count` blank materials (no name or textures, white, alpha 255).
// Does nothing for count 0. Returns 0.
int Node::AllocMaterials(int count)
{
    if (count != 0)
    {
        if (materialCount != 0)
            DAT_11b1ba40(materials);
        materials = (Material *)DAT_11b0b938(count * sizeof(Material), "m_material");
        for (int i = 0; i < count; i++)
        {
            materials[i].name[0] = 0;
            materials[i].textures[0][0] = 0;
            materials[i].textures[1][0] = 0;
            materials[i].textures[2][0] = 0;
            materials[i].textures[3][0] = 0;
            materials[i].colour[0] = 1.0f;
            materials[i].colour[1] = 1.0f;
            materials[i].colour[2] = 1.0f;
            materials[i].m_0178 = 0;
            materials[i].m_017c[0] = 0;
            materials[i].m_017c[1] = 0;
            materials[i].m_017c[2] = 0;
            materials[i].m_017c[3] = 0;
            materials[i].m_0170 = 0;
            materials[i].alpha = 255.0f;
        }
        materialCount = count;
    }
    return 0;
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

// Restores the saved matrix for state 0x2000, clears it and releases the saved states of the subtree.
int Node::M_10194870()
{
    if (saved != 0 && (flags & 0x2000) != 0)
    {
        matrix = saved->matrix;
        flags &= ~0x2000;
        ReleaseSavedStates();
    }
    return 1;
}

// Clears state 0x4000 and releases the saved states of the subtree.
int Node::M_101948c0()
{
    if (saved != 0 && (flags & 0x4000) != 0)
    {
        flags &= ~0x4000;
        ReleaseSavedStates();
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
        packs[i].radius = 0;
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

// Follow-up of the special pass marking: clears the marker (0x8000) again on nodes in matrix states 0x2000 or
// 0x1000, clears it on every node a skinned node depends on (and their ancestors), and recurses into the children.
// Returns 1.
int Node::ClearSpecialPassOnLinks()
{
    if (flags & 0x2000)
        flags &= ~0x8000;
    if (flags & 0x1000)
        flags &= ~0x8000;
    if (m_0708 != 0 && m_0708->m_5618 != 0)
    {
        for (int i = 0; i < m_0708->m_5618; i++)
            m_0708->links[i]->M_1019d680();
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ClearSpecialPassOnLinks();
    return 1;
}

// Node::Find: the index-th node called `name` in the subtree; a leading '@' turns it into a prefix search
// ("@Spot" finds Spot01, Spot02, ...).
Node *Node::Find(const char *name, int value)
{
    int result = value;
    if (*name == '@')
        return M_10191c30(name + 1, &result);
    return M_10191a30(name, &result);
}

// Like Find with the second pair of finders (FindPrefixRecursiveStripped / FindRecursiveStripped), which ignore the name
// prefixes of special nodes.
int Node::M_1019ed10(const char *name, int value)
{
    int result = value;
    if (*name == '@')
        return FindPrefixRecursiveStripped(name + 1, &result);
    return FindRecursiveStripped(name, &result);
}

// Runs the special pass marking (M_1019d600) and the follow-up pass (ClearSpecialPassOnLinks).
int Node::M_101a8760()
{
    M_1019d600();
    ClearSpecialPassOnLinks();
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
        if (frames != 0)
            DAT_11b1ba40(frames);
        frames = 0;
        m_0294 = 0;
    }
    frames = 0;
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
// 0x1019e9a0 (Node::Release). Frees everything the node owns, then its children (recursively) and resets it with
// M_10191500: the static-node list, the second pack and index lists, the extra data, the skin and saved blocks, the
// materials, the texture reference (with more than one frame and no reference the game stops on purpose), the
// second key list, the mesh (the same stop when there are packs without a mesh) and the plug channels and data.
void Node::Release()
{
    if (staticNodes != 0)
    {
        DAT_11b1ba40(staticNodes);
        staticNodes = 0;
    }
    isStatic = 0;
    staticCount = 0;
    M_101911b0(m_0080, m_007c);
    M_101911e0(m_0084, m_007c);
    if (m_0080 != 0)
        DAT_11b1ba40(m_0080);
    if (m_0084 != 0)
        DAT_11b1ba40(m_0084);
    m_0080 = 0;
    m_0084 = 0;
    if (m_02a8 != 0)
    {
        m_02a8->M_10169620();
        DAT_11b1ba40(m_02a8);
    }
    m_02a8 = 0;
    if (m_0708 != 0)
        DAT_11b1ba40(m_0708);
    m_0708 = 0;
    if (saved != 0)
        DAT_11b1ba40(saved);
    saved = 0;
    if (m_1864 != 0)
        DAT_11b1ba40(m_1864);
    m_1864 = 0;
    if (materials != 0)
        DAT_11b1ba40(materials);
    materials = 0;
    if (m_028c != 0)
        M_10191210();
    else if (m_0294 > 1)
        fread(0, 1, 1, 0);
    if (m_0070 != 0)
    {
        M_101911e0(m_0070, packCount);
        DAT_11b1ba40(m_0070);
        m_0070 = 0;
    }
    if (mesh != 0)
        ReleaseMesh();
    else if (packCount != 0)
        fread(0, 1, 1, 0);
    mesh = 0;
    packs = 0;
    indexLists = 0;
    packCount = 0;
    for (int j = 0; j < 16; j++)
    {
        if (plugs[j] != 0)
        {
            DAT_11b1ba40(plugs[j]);
            plugs[j] = 0;
            plugType[j] = 0;
        }
    }
    for (int k = 0; k < 16; k++)
    {
        if (plugData[k] != 0)
        {
            DAT_11b1ba40(plugData[k]);
            plugData[k] = 0;
            plugTypeCopy[k] = 0;
        }
    }
    for (int i = 0; i < childCount; i++)
    {
        children[i]->Release();
        DAT_11b1ba40(children[i]);
    }
    M_10191500();
}

// Releases the node's shared mesh and, when it was the last user, frees the mesh, packs and index lists.
void Node::ReleaseMesh()
{
    if (mesh != 0 && FUN_10169990(mesh) == 0)
    {
        mesh->Release();
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
        children[i]->Release();
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
    bounds[1].z = packs->vertices[0].position[2];
    bounds[2].z = packs->vertices[0].position[2];
    m_1858 = 1;
    for (int i = 0; i < 0x20; i++)
    {
        Node *node = (Node *)DAT_11b0b938(0x18a0, "m_child[m_numchild]");
        created[i] = node;
        node->M_10191500();
        node->Clone(this, 0, 1);
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

// Like FindRecursive, but compares the name without the prefix of special nodes: collision ("CDL", kinds below
// 0x40) names skip 6 characters and overlay ("OVF", kind 0xfd) names 7. Nodes without a kind (0xff) are
// recognised by the prefix itself. Returns the node (as an int, like the other finders here) or 0.
int Node::FindRecursiveStripped(const char *name, int *skip)
{
    int offset = 0;
    if (kind != 0xff)
    {
        if (kind < 0x40)
            offset = 6;
        else if (kind == 0xfd)
            offset = 7;
    }
    else
    {
        if (this->name[0] == 'C' && this->name[1] == 'D' && this->name[2] == 'L')
            offset = 6;
        if (this->name[0] == 'O' && this->name[1] == 'V' && this->name[2] == 'F')
            offset = 7;
    }
    if (_stricmp(this->name + offset, name) == 0)
    {
        if (skip == 0)
            return (int)this;
        if (*skip == 0)
            return (int)this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            int found = children[i]->FindRecursiveStripped(name, skip);
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

// Prefix version of FindRecursiveStripped (names that start with `prefix` after the special-node prefix).
int Node::FindPrefixRecursiveStripped(const char *prefix, int *skip)
{
    int offset = 0;
    if (kind != 0xff)
    {
        if (kind < 0x40)
            offset = 6;
        else if (kind == 0xfd)
            offset = 7;
    }
    else
    {
        if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
            offset = 6;
        if (name[0] == 'O' && name[1] == 'V' && name[2] == 'F')
            offset = 7;
    }
    if (_strnicmp(name + offset, prefix, strlen(prefix)) == 0)
    {
        if (skip == 0)
            return (int)this;
        if (*skip == 0)
            return (int)this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            int found = children[i]->FindPrefixRecursiveStripped(prefix, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Builds the node's mesh buffers from its packed vertex data (or, for a node without packs, from its index lists
// alone) unless it is a bone (m_0704), and marks the node as built; with `recurse` the whole subtree follows.
// Returns 1.
int Node::BuildMesh(int recurse, int flags)
{
    if (packs == 0 && packCount == -1)
    {
        if (m_0704 == 0)
        {
            mesh->M_10150bb0(0, indexLists, -1, materials, flags);
            built = 1;
        }
    }
    else if (packs != 0 && m_0704 == 0)
    {
        mesh->M_10150bb0(packs, indexLists, packCount, materials, flags);
        built = 1;
    }
    if (recurse != 0)
    {
        for (int i = 0; i < childCount; i++)
            children[i]->BuildMesh(1, 0);
    }
    return 1;
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

// 0x101992f0. The top node of the tree this node belongs to (following the parent links).
Node *Node::Root()
{
    Node *node = this;
    while (node->parent != 0)
        node = node->parent;
    return node;
}

// 0x101a52d0. Names this node `name`, or "name(0)", "name(1)", ... (the first that no node of the tree uses yet)
// when the tree already has a node called `name`. Returns 1.
int Node::SetUniqueName(const char *name)
{
    char unique[256];
    int number = 0;
    Node *root = Root();
    if (root->Find(name, 0) == 0)
    {
        strcpy(this->name, name);
        return 1;
    }
    sprintf(unique, "%s(%d)", name, 0);
    while (root->Find(unique, 0) != 0)
    {
        number++;
        sprintf(unique, "%s(%d)", name, number);
    }
    strcpy(this->name, unique);
    return 1;
}

// 0x101a2290. Makes this node's name unique in its tree (called when a child is added, 0x101abc30): the node first
// takes a random name "uni<rand><rand>" so that the search does not find the node itself, then gets its old name,
// or "name(1)", "name(2)", ... when another node already has it.
void Node::MakeNameUnique()
{
    char unique[256];
    char base[256];
    Node *root = Root();
    strcpy(base, name);
    strcpy(unique, name);
    sprintf(name, "uni%d%d", rand(), rand());
    int number = 1;
    while (root->Find(unique, 0) != 0)
    {
        sprintf(unique, "%s(%d)", base, number);
        number++;
    }
    strcpy(name, unique);
}

// 0x10199cd0. Finds the first of the 64 regions (`boxes`, with names in `names`; an empty name = unused) that
// contains the position of `matrix`, or of this node when `matrix` is 0, and prefixes this node's name with
// "CDL<region>-". Returns 1 when a region was found, else 0.
int Node::TagRegion(const Box2 *boxes, const char (*names)[0x40], Matrix4 *matrix)
{
    char tagged[256];
    int i;
    if (matrix != 0)
    {
        for (i = 0; i < 0x40; i++)
        {
            if (names[i][0] != 0 && matrix->GetTranslation().InBox(&boxes[i]) != 0)
            {
                sprintf(tagged, "CDL%02d-%s", i, name);
                sprintf(name, tagged);
                return 1;
            }
        }
    }
    else
    {
        for (i = 0; i < 0x40; i++)
        {
            if (names[i][0] != 0 && worldMatrix.GetTranslation().InBox(&boxes[i]) != 0)
            {
                sprintf(tagged, "CDL%02d-%s", i, name);
                sprintf(name, tagged);
                return 1;
            }
        }
    }
    return 0;
}

// 0x1019ed40. Renames a node and every reference to it. Called with `oldName` = 0 on the node itself: the whole
// tree (from its root) replaces the node's current name by `newName` in the skinning names (bound-to name, link
// names, bone names), then the node takes the new name (returns 0). Called with a name, it does the replacing in
// this node and all nodes below it (returns 1).
int Node::Rename(const char *newName, const char *oldName)
{
    if (oldName == 0)
    {
        Node *root = Root();
        if (root != 0)
        {
            root->Rename(newName, name);
            strcpy(name, newName);
            return 0;
        }
        Rename(newName, name);
        strcpy(name, newName);
        return 0;
    }
    if (m_0708 != 0)
    {
        if (_stricmp(m_0708->parentName, oldName) == 0)
            strcpy(m_0708->parentName, newName);
        int i;
        for (i = 0; i < 0x40; i++)
        {
            if (_stricmp(m_0708->linkNames[i], oldName) == 0)
                strcpy(m_0708->linkNames[i], newName);
        }
        for (i = 0; i < 0x44; i++)
        {
            if (_stricmp(m_0708->boneNames[i], oldName) == 0)
                strcpy(m_0708->boneNames[i], newName);
        }
    }
    for (int j = 0; j < childCount; j++)
        children[j]->Rename(newName, oldName);
    return 1;
}

extern "C" int DAT_10feb3e0[];   // per mesh buffer slot: 1 = owned by a single node
extern "C" int DAT_10fee5e0[];
extern "C" int DAT_10ff2ff8[];
extern "C" int DAT_10fe01d8[];

// 0x10191280. Frees the pack and index data of every node in this subtree whose mesh buffer is not shared (its slot
// in DAT_10feb3e0 is 1), and resets the slot. Nodes are skipped when their name (after a "CDL..." or "OvF..." prefix
// of 6 or 7 characters) starts with "CA", "CS" or "CT", when they are flagged (m_1860 bit 4, flags 4 or 0xf00) or
// have no mesh buffer.
void Node::ReleaseUniqueMeshData()
{
    int skip = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        skip = 6;
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
        skip = 7;
    if (!((name[skip] == 'C' && name[skip + 1] == 'A') || (name[skip] == 'C' && name[skip + 1] == 'S') ||
          (name[skip] == 'C' && name[skip + 1] == 'T')) &&
        (m_1860 & 4) == 0 && (flags & 4) == 0 && (flags & 0xf00) == 0 && mesh != 0 && mesh->bufferSlot >= 0 &&
        DAT_10feb3e0[mesh->bufferSlot] == 1)
    {
        DAT_10fee5e0[mesh->bufferSlot] = -1;
        DAT_10ff2ff8[mesh->bufferSlot] = 0;
        DAT_10fe01d8[mesh->bufferSlot] = 0;
        M_101911b0(packs, packCount);
        M_101911e0(indexLists, packCount);
        if (m_0070 != 0)
        {
            M_101911e0(m_0070, packCount);
            DAT_11b1ba40(m_0070);
            m_0070 = 0;
        }
        if (packs != 0)
            DAT_11b1ba40(packs);
        packs = 0;
        if (indexLists != 0)
            DAT_11b1ba40(indexLists);
        indexLists = 0;
        packCount = -1;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ReleaseUniqueMeshData();
}

// 0x1019d750. Called with no collector on the root, which becomes the collector: every node below it gets its kind
// from its name ("CDLnn..." = nn, "OvF..." = 0xfd, else 0xfe) and nodes with mesh data that are flagged (flags or
// m_1860 bit 4) or whose name (after the prefix) starts with "CS", "CA" or "CT" are marked static and appended to
// the collector's list ("FCHILD", allocated on first use).
void Node::CollectStatic(Node *collector)
{
    int i;
    if (collector == 0)
    {
        isStatic = 1;
        for (i = 0; i < childCount; i++)
            children[i]->CollectStatic(this);
        return;
    }
    kind = 0xfe;
    int skip = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
    {
        kind = (name[3] - '0') * 10 + name[4] - '0';
        skip = 6;
    }
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
    {
        kind = 0xfd;
        skip = 7;
    }
    if (packs == 0 || indexLists == 0 || packCount == 0 ||
        ((flags & 4) == 0 && (m_1860 & 4) == 0 &&
         (name[skip] != 'C' || (name[skip + 1] != 'S' && name[skip + 1] != 'A' && name[skip + 1] != 'T'))))
    {
        for (i = 0; i < childCount; i++)
            children[i]->CollectStatic(collector);
        return;
    }
    isStatic = 1;
    if (collector->staticNodes == 0)
        collector->staticNodes = (Node **)DAT_11b0b938(0x800, "FCHILD");
    collector->staticNodes[collector->staticCount] = this;
    collector->staticCount++;
    for (i = 0; i < childCount; i++)
        children[i]->CollectStatic(collector);
}

// 0x1019eeb0. Prepares the skinning of this node and, recursively, its children: stores the inverse of the node
// matrix; "bip"/"bone" nodes are marked as bones and get skin data. A skinned node (skin data with links) looks up
// its linked nodes and bones under `root`, marks them and passes its parent name on to them (to the bones only while
// every earlier bone is a leaf or a bone itself). Nodes with flag 0x400000, 0x1000 or 0x4000 get their saved block
// (0x1000: with the linked root stored in it, 0x4000: the node is added to `list`). Returns 1.
int Node::M_1019eeb0(Node *root, Node **list, int *count)
{
    inverseMatrix = InverseMatrix(matrix);
    if (_strnicmp(name, "bip", 3) == 0 || _strnicmp(name, "bone", 4) == 0)
    {
        m_0704 = 1;
        if (m_0708 == 0)
        {
            m_0708 = (GlyphState *)DAT_11b0b938(0x561c, "bone");
            m_0708->Clear();
        }
    }
    if (m_0708 != 0 && m_0708->m_5618 != 0)
    {
        int i;
        for (i = 0; i < 0x40; i++)
        {
            if (m_0708->linkNames[i][0] != 0)
            {
                m_0708->links[i] = root->Find(m_0708->linkNames[i], 0);
                m_0708->links[i]->m_0704 = 1;
                if (m_0708->links[i]->m_0708 == 0)
                {
                    m_0708->links[i]->m_0708 = (GlyphState *)DAT_11b0b938(0x561c, "bone");
                    m_0708->links[i]->m_0708->Clear();
                }
                m_0708->links[i]->m_0708->pad_0104 = 1;
                strcpy(m_0708->links[i]->m_0708->parentName, m_0708->parentName);
            }
        }
        int chain = 1;
        for (i = 0; i < 0x44; i++)
        {
            if (m_0708->boneNames[i][0] != 0)
            {
                m_0708->bones[i] = root->Find(m_0708->boneNames[i], 0);
                if (m_0708->bones[i] != 0)
                {
                    if (_strnicmp(m_0708->bones[i]->name, "bip", 3) == 0 ||
                        _strnicmp(m_0708->bones[i]->name, "bone", 4) == 0)
                        m_0708->bones[i]->m_0704 = 1;
                    Node *bone = m_0708->bones[i];
                    if (bone->m_0704 == 0 && bone->childCount != 0)
                        chain = 0;
                    if (i > 0 && chain && bone != 0)
                    {
                        if (bone->m_0708 == 0)
                        {
                            m_0708->bones[i]->m_0708 = (GlyphState *)DAT_11b0b938(0x561c, "bone");
                            m_0708->bones[i]->m_0708->Clear();
                        }
                        strcpy(m_0708->bones[i]->m_0708->parentName, m_0708->parentName);
                    }
                }
            }
        }
    }
    if ((flags & 0x400000) != 0)
        M_101947b0();
    else if ((flags & 0x1000) != 0)
    {
        M_101947b0();
        *(Node **)&saved->pad_0064[0] = M_10199310();
    }
    else if ((flags & 0x4000) != 0)
    {
        M_101947b0();
        list[*count] = this;
        (*count)++;
    }
    for (int child = 0; child < childCount; child++)
        children[child]->M_1019eeb0(root, list, count);
    return 1;
}

extern "C" MeshData *DAT_10fa7478[];                 // per mesh buffer slot: the mesh
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);   // the engine's memcpy
extern "C" int FUN_10169840(const char *folder, const char *name, int *freeSlot);  // ModelNames_Find: slot of folder+name, -1 = none (a free slot in *freeSlot)
extern "C" int FUN_10169760(void);                   // ModelNames_FindFree
extern "C" int FUN_10169780(int slot, const char *folder, const char *name);   // ModelNames_SetJoined
extern "C" int FUN_10174ce0(const char *path, int a, int b);  // Texture_LoadIti

// 0x1019b350. Gives this node the mesh of `source`: with `slot` 0 the cached mesh of this node's folder and name is
// shared when there is one (otherwise the free cache slot found is used); with `slot` non-zero a new name slot is
// taken. Without a shared mesh the packs and index lists are copied, a new mesh is made and cached in the slot. Then the
// materials are copied (loading their textures), and the flag at 0xa0 and the bounds. Returns 0 without a source,
// else 1.
int Node::CopyMesh(Node *source, int slot)
{
    if (source == 0)
        return 0;
    m_00a8 = 0;
    if (source->packCount != 0)
    {
        if (slot == 0)
        {
            int found = FUN_10169840(folder, name, &slot);
            if (found >= 0)
            {
                mesh = DAT_10fa7478[found];
                packs = (PackHeader *)DAT_10ff2ff8[found];
                indexLists = (KeyPair *)DAT_10fe01d8[found];
                packCount = DAT_10fee5e0[found];
                FUN_10169970(mesh);
                goto copyMaterials;
            }
        }
        else
        {
            slot = FUN_10169760();
            FUN_10169780(slot, folder, name);
        }
        if (packs != 0)
            M_101911b0(packs, packCount);
        if (indexLists != 0)
            M_101911e0(indexLists, packCount);
        if (packs != 0)
            DAT_11b1ba40(packs);
        if (indexLists != 0)
            DAT_11b1ba40(indexLists);
        if (materials != 0)
            DAT_11b1ba40(materials);
        packs = (PackHeader *)DAT_11b0b938(source->packCount * sizeof(PackHeader), "m_vpack");
        indexLists = (KeyPair *)DAT_11b0b938(source->packCount * sizeof(KeyPair), "m_vpackindex");
        packCount = source->packCount;
        for (int i = 0; i < packCount; i++)
        {
            packs[i].M_101acfa0();
            packs[i].Copy(&source->packs[i]);
            indexLists[i].Init();
            indexLists[i].Copy(&source->indexLists[i]);
        }
        if (mesh != 0)
            fread(0, 1, 1, 0);           // a node that still has a mesh here is a bug: crash on purpose
        mesh = (MeshData *)DAT_11b0b938(0x50, "CpMCObj");
        mesh->Init();
        mesh->bufferSlot = slot;
        if (slot >= 0)
        {
            DAT_10fa7478[slot] = mesh;
            DAT_10ff2ff8[slot] = (int)packs;
            DAT_10fe01d8[slot] = (int)indexLists;
            DAT_10fee5e0[slot] = packCount;
            FUN_10169970(mesh);
        }
    }
copyMaterials:
    if (source->materialCount != 0)
    {
        materials = (Material *)DAT_11b0b938(source->materialCount * 0x180, "m_material");
        DAT_11b07530(materials, source->materials, source->materialCount * 0x180);
        materialCount = source->materialCount;
        for (int i = 0; i < materialCount; i++)
        {
            if (materials[i].textures[0][0] != 0)
                FUN_10174ce0(materials[i].textures[0], 0, 0);
        }
    }
    m_00a0 = source->m_00a0;
    bounds[0].Copy(source->bounds[0]);
    bounds[1].Copy(source->bounds[1]);
    bounds[2].Copy(source->bounds[2]);
    return 1;
}

// 0x101b1420: `out` = a blended towards b by t (mode 1: rotation interpolated for skeleton roots).
Matrix4 *MatrixBlend(Matrix4 *out, Matrix4 a, Matrix4 b, float t, int mode);

// 0x1019b630. Sets the current pose of this node and its subtree to frame `from` (a stored pose for -1, -2, ...)
// blended towards frame `to` by `t`. The skeleton root "Bip01" (when DAT_11b04508 is set) and "charbase..." nodes
// blend their rotation (mode 1; charbase only with DAT_11b04508 set). Returns 0 for no node, else 1.
int Node::BlendFrames(int from, int to, float t)
{
    if (this == 0)
        return 0;
    if (frames != 0)
    {
        if (from < 0)
        {
            if (m_0294 > to)
            {
                int pose = -1 - from;
                if (DAT_11b04508 != 0 && _stricmp(name, "Bip01") == 0)
                    MatrixBlend(&poses[0], poses[pose], frames[to], t, 1);
                else if (_strnicmp(name, "charbase", 8) == 0)
                {
                    if (DAT_11b04508 != 0)
                        MatrixBlend(&poses[0], poses[pose], frames[to], t, 1);
                    else
                        MatrixBlend(&poses[0], poses[pose], frames[to], t, 0);
                }
                else
                    MatrixBlend(&poses[0], poses[pose], frames[to], t, 0);
            }
        }
        else if (m_0294 > from && m_0294 > to)
        {
            if (DAT_11b04508 != 0 && _stricmp(name, "Bip01") == 0)
                MatrixBlend(&poses[0], frames[from], frames[to], t, 1);
            else if (_strnicmp(name, "charbase", 8) == 0)
            {
                if (DAT_11b04508 != 0)
                    MatrixBlend(&poses[0], frames[from], frames[to], t, 1);
                else
                    MatrixBlend(&poses[0], frames[from], frames[to], t, 0);
            }
            else
                MatrixBlend(&poses[0], frames[from], frames[to], t, 0);
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->BlendFrames(from, to, t);
    return 1;
}

extern "C" TextureRef *DAT_10fdf1d0[];                // per motion cache slot: the reference
extern "C" Matrix4 *DAT_1101c820[];                    //   the frame matrices
extern "C" int DAT_11017600[];                        //   the number of frames
extern "C" int FUN_10169ab0(const char *folder, const char *name, int *freeSlot);  // motion name slot, -1 = none
extern "C" int FUN_101699d0(void);                   // a free motion name slot
extern "C" int FUN_101699f0(int slot, const char *folder, const char *name);       // names a motion slot

// 0x1019b890. Gives this node the animation of `source`: the plug channels (copied whole), the extra key tracks
// (the first three all from the source's second track, as in the original), the node matrix and the frame
// matrices. With `slot` 0 the cached motion of this node's name is shared when there is one; otherwise a copy is
// made and cached (in a new slot for `slot` non-zero). The pose the node had (its current frame or stored pose) is
// kept in poses[15]. Returns 0.
int Node::CopyAnimation(Node *source, int slot)
{
    Matrix4 current;
    if (source != 0)
    {
        if (frame < 0)
        {
            int pose = -1 - frame;
            if (pose >= 15)
                pose = 15;
            current = poses[pose];
        }
        else if ((flags & 0x100000) != 0 && m_02a8 != 0)
            current = m_02a8->AnimatedMatrix(frame);
        else if (frames != 0)
        {
            int index = frame;
            if (m_0294 <= index)
                index = m_0294 - 1;
            current = frames[index];
        }
        else
            current = matrix;
    }
    int g;
    for (g = 0; g < 16; g++)
    {
        if (plugs[g] != 0)
        {
            DAT_11b1ba40(plugs[g]);
            plugs[g] = 0;
            plugType[g] = 0;
        }
    }
    if (m_028c != 0)
        M_10191210();
    if (source == 0)
        return 0;
    strcpy(m_0050, source->m_0050);
    for (g = 0; g < 16; g++)
    {
        if (source->plugs[g] != 0)
        {
            int size = source->plugs[g]->size;
            plugs[g] = (Channel *)DAT_11b0b938(size, "->m_plugdata[g]");
            memcpy(plugs[g], source->plugs[g], size);
            plugType[g] = source->plugType[g];
        }
    }
    if (source->m_02a8 != 0)
    {
        M_101949d0();
        m_02a8->tracks[0].Copy(&source->m_02a8->tracks[1]);
        m_02a8->tracks[1].Copy(&source->m_02a8->tracks[1]);
        m_02a8->tracks[2].Copy(&source->m_02a8->tracks[1]);
        m_02a8->tracks[3].Copy(&source->m_02a8->tracks[3]);
        m_02a8->tracks[4].Copy(&source->m_02a8->tracks[4]);
        m_02a8->m_0070[0] = source->m_02a8->m_0070[0];
        m_02a8->m_0070[1] = source->m_02a8->m_0070[1];
        m_02a8->m_0070[2] = source->m_02a8->m_0070[2];
        m_02a8->m_0070[3] = source->m_02a8->m_0070[3];
    }
    matrix = source->matrix;
    if (source->m_028c != 0)
    {
        int motionSlot = -1;
        if (slot == 0)
        {
            int found = FUN_10169ab0(m_0050, name, &motionSlot);
            if (found >= 0)
            {
                m_028c = DAT_10fdf1d0[found];
                frames = DAT_1101c820[found];
                m_0294 = DAT_11017600[found];
                FUN_10169be0(m_028c);
                goto keepPose;
            }
        }
        else
        {
            motionSlot = FUN_101699d0();
            FUN_101699f0(motionSlot, m_0050, name);
        }
        m_028c = (TextureRef *)DAT_11b0b938(4, "motion");
        m_0294 = source->m_0294;
        frames = (Matrix4 *)DAT_11b0b938(source->m_0294 << 6, "tmp[i]->m_animat");
        memcpy(frames, source->frames, source->m_0294 << 6);
        m_028c->slot = motionSlot;
        if (motionSlot >= 0)
        {
            DAT_10fdf1d0[motionSlot] = m_028c;
            DAT_1101c820[motionSlot] = frames;
            DAT_11017600[motionSlot] = m_0294;
            FUN_10169be0(m_028c);
        }
    }
keepPose:
    poses[15] = current;
    return 0;
}

// 0x101a53b0. Makes this node a copy of `source` and its whole subtree: names, matrices and stored poses, flags and
// settings, the saved block, the plug channels, the animation (CopyAnimation, cache flag `motionSlot`) and the mesh
// (CopyMesh, cache flag `meshSlot`), the skin data, and a new copy of every child. Returns 1.
int Node::Clone(Node *source, int motionSlot, int meshSlot)
{
    Release();
    strcpy(name, source->name);
    strcpy(folder, source->folder);
    strcpy(m_0050, source->m_0050);
    matrix = source->matrix;
    worldMatrix = source->worldMatrix;
    int i;
    for (i = 0; i < 16; i++)
        poses[i] = source->poses[i];
    flags = source->flags;
    m_00b0 = source->m_00b0;
    group = source->group;
    m_1868.Copy(source->m_1868);
    m_1878[0] = source->m_1878[0];
    m_1880[0] = source->m_1880[0];
    m_1880[1] = source->m_1880[1];
    m_1880[2] = source->m_1880[2];
    m_1880[3] = source->m_1880[3];
    m_1880[4] = source->m_1880[4];
    m_1878[1] = source->m_1878[1];
    m_1878[2] = source->m_1878[2];
    m_1878[3] = source->m_1878[3];
    m_1878[4] = source->m_1878[4];
    m_1878[5] = source->m_1878[5];
    m_1878[6] = source->m_1878[6];
    m_00a0 = source->m_00a0;
    m_01c0 = source->m_01c0;
    m_01c8 = source->m_01c8;
    m_01c4 = source->m_01c4;
    if (source->saved != 0)
    {
        saved = (NodeSaved *)DAT_11b0b938(0xdc, "ik");
        saved->M_10166870();
        *saved = *source->saved;
    }
    for (int g = 0; g < 16; g++)
    {
        if (source->plugs[g] != 0)
        {
            int size = source->plugs[g]->size;
            plugs[g] = (Channel *)DAT_11b0b938(size, "->m_plugdata[g]");
            memcpy(plugs[g], source->plugs[g], size);
            plugType[g] = source->plugType[g];
        }
    }
    CopyAnimation(source, motionSlot);
    CopyMesh(source, meshSlot);
    ComputeBounds();
    if (source->m_0708 != 0 && source->m_0708->m_5618 != 0)
    {
        m_0708 = (GlyphState *)DAT_11b0b938(0x561c, "bone");
        m_0708->Clear();
        strcpy(m_0708->parentName, source->m_0708->parentName);
        m_0708->m_5618 = source->m_0708->m_5618;
        for (i = 0; i < source->m_0708->m_5618; i++)
            strcpy(m_0708->linkNames[i], source->m_0708->linkNames[i]);
        for (i = 0; i < 0x44; i++)
        {
            strcpy(m_0708->boneNames[i], source->m_0708->boneNames[i]);
            m_0708->bindPose[i] = source->m_0708->bindPose[i];
            m_0708->m_3518[i] = source->m_0708->m_3518[i];
        }
    }
    strcpy(m_070c, source->m_070c);
    childCount = source->childCount;
    for (i = 0; i < source->childCount; i++)
    {
        strcpy(childNames[i], source->childNames[i]);
        children[i] = (Node *)DAT_11b0b938(0x18a0, "dup");
        children[i]->M_10191500();
        children[i]->Clone(source->children[i], motionSlot, meshSlot);
        children[i]->parent = this;
    }
    m_00a8 = 0;
    return 1;
}

// 0x1019d8e0. Computes the bounds of this node's packs (box corners and centre; the radius only for packs that
// already have one, nodes with flag 4, and "CS"/"CA"/"CT" nodes, after a "CDL"/"OvF" prefix of 6/7 characters) and
// of the whole node (bounds: sphere and box). Returns 1.
int Node::ComputeBounds()
{
    int prefix = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        prefix = 6;
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
        prefix = 7;
    if (packCount != -1)
    {
        float maxX = -999999.0f;
        float maxY = -999999.0f;
        float maxZ = -999999.0f;
        float minX = 999999.0f;
        float minY = 999999.0f;
        float minZ = 999999.0f;
        for (int i = 0; i < packCount; i++)
        {
            float packMaxX = -999999.0f;
            float packMaxY = -999999.0f;
            float packMaxZ = -999999.0f;
            float packMinX = 999999.0f;
            float packMinY = 999999.0f;
            float packMinZ = 999999.0f;
            const PackVertex *vertex = packs[i].vertices;
            for (int j = 0; j < packs[i].vertexCount; j++)
            {
                if (vertex->position[0] < packMinX)
                    packMinX = vertex->position[0];
                if (vertex->position[0] > packMaxX)
                    packMaxX = vertex->position[0];
                if (vertex->position[1] < packMinY)
                    packMinY = vertex->position[1];
                if (vertex->position[1] > packMaxY)
                    packMaxY = vertex->position[1];
                if (vertex->position[2] < packMinZ)
                    packMinZ = vertex->position[2];
                if (vertex->position[2] > packMaxZ)
                    packMaxZ = vertex->position[2];
                vertex++;
            }
            if (packMinX < minX)
                minX = packMinX;
            if (packMaxX > maxX)
                maxX = packMaxX;
            if (packMinY < minY)
                minY = packMinY;
            if (packMaxY > maxY)
                maxY = packMaxY;
            if (packMinZ < minZ)
                minZ = packMinZ;
            if (packMaxZ > maxZ)
                maxZ = packMaxZ;
            packs[i].boxMin[0] = packMinX;
            packs[i].boxMin[1] = packMinY;
            packs[i].boxMin[2] = packMinZ;
            packs[i].boxMin[3] = 1.0f;
            packs[i].boxMax[0] = packMaxX;
            packs[i].boxMax[1] = packMaxY;
            packs[i].boxMax[2] = packMaxZ;
            packs[i].boxMax[3] = 1.0f;
            packs[i].center[0] = (packMinX + packMaxX) / 2.0f;
            packs[i].center[1] = (packMinY + packMaxY) / 2.0f;
            packs[i].center[2] = (packMinZ + packMaxZ) / 2.0f;
            if (packs[i].radius != 0.0f || (flags & 4) != 0 ||
                (name[prefix] == 'C' && (name[prefix + 1] == 'S' || name[prefix + 1] == 'A' || name[prefix + 1] == 'T')))
            {
                Vec4 size;
                size.x = packMaxX - packMinX;
                size.y = packMaxY - packMinY;
                size.z = packMaxZ - packMinZ;
                packs[i].radius = size.Length() / 2.0f;
            }
        }
        Vec4 size;
        size.x = maxX - minX;
        size.y = maxY - minY;
        size.z = maxZ - minZ;
        bounds[0].x = (minX + maxX) / 2.0f;
        bounds[0].y = (minY + maxY) / 2.0f;
        bounds[0].z = (minZ + maxZ) / 2.0f;
        float radius = size.Length() / 2.0f;
        bounds[1].x = minX;
        bounds[1].y = minY;
        bounds[1].z = minZ;
        bounds[2].x = maxX;
        bounds[1].w = 1.0f;
        bounds[2].y = maxY;
        bounds[2].z = maxZ;
        bounds[2].w = 1.0f;
        bounds[0].w = radius;
    }
    return 1;
}

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// 0x1019e120. For collision nodes ("CA"/"CS"/"CT" names after a "CDL"/"OvF" prefix, flag 4 or flags 0xf00) builds
// the sphere list of every pack: one sphere per triangle, centred in the triangle's bounding box, with 1.1 times
// the distance to its farthest corner as radius. Recurses into the children. The MIN/MAX macros evaluate their
// arguments twice, as in the original. Returns 1.
int Node::BuildCollisionSpheres()
{
    Vec4 a;
    Vec4 b;
    Vec4 c;
    int prefix = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        prefix = 6;
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
        prefix = 7;
    if (((name[prefix] == 'C' && name[prefix + 1] == 'A') || (name[prefix] == 'C' && name[prefix + 1] == 'S') ||
         (name[prefix] == 'C' && name[prefix + 1] == 'T') || (flags & 4) != 0 || (flags & 0xf00) != 0) &&
        packCount > 0)
    {
        M_101911e0(m_0070, packCount);
        if (m_0070 != 0)
            DAT_11b1ba40(m_0070);
        m_0070 = (KeyPair *)DAT_11b0b938(packCount * sizeof(KeyPair), "m_vpackindexsp");
        for (int p = 0; p < packCount; p++)
        {
            m_0070[p].Init();
            m_0070[p].Resize(indexLists[p].count / 3);
            for (int i = 0; i < indexLists[p].count; i += 3)
            {
                int first = indexLists[p].indices[i];
                int second = indexLists[p].indices[i + 1];
                int third = indexLists[p].indices[i + 2];
                a.x = packs[p].vertices[first].position[0];
                a.y = packs[p].vertices[first].position[1];
                a.z = packs[p].vertices[first].position[2];
                b.x = packs[p].vertices[second].position[0];
                b.y = packs[p].vertices[second].position[1];
                b.z = packs[p].vertices[second].position[2];
                c.x = packs[p].vertices[third].position[0];
                c.y = packs[p].vertices[third].position[1];
                c.z = packs[p].vertices[third].position[2];
                m_0070[p].spheres[i / 3].x = (MAX(MAX(a.x, b.x), c.x) + MIN(MIN(a.x, b.x), c.x)) / 2.0f;
                m_0070[p].spheres[i / 3].y = (MAX(MAX(a.y, b.y), c.y) + MIN(MIN(a.y, b.y), c.y)) / 2.0f;
                m_0070[p].spheres[i / 3].z = (MAX(MAX(a.z, b.z), c.z) + MIN(MIN(a.z, b.z), c.z)) / 2.0f;
                float da = m_0070[p].spheres[i / 3].Distance(a);
                float db = m_0070[p].spheres[i / 3].Distance(b);
                float dc = m_0070[p].spheres[i / 3].Distance(c);
                m_0070[p].spheres[i / 3].w = MAX(MAX(da, db), dc) * 1.1f;
            }
        }
    }
    for (int child = 0; child < childCount; child++)
        children[child]->BuildCollisionSpheres();
    return 1;
}
