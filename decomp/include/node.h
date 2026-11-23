// A scene node (0x18a0 bytes): one part of a loaded model (.it3 file) with its mesh, matrices, animation plugs and up
// to 64 children. Models, maps and attached props are trees of nodes. Most methods are defined in scene_node.cpp;
// node_init.cpp (Init), node_world.cpp (UpdateWorld), node_draw.cpp (Draw), node_bones.cpp (bones and skins),
// node_spheres.cpp (collision spheres), node_colours.cpp / node_vertex_colours.cpp / node_sort_packs.cpp (mesh data),
// node_cache.cpp (Load), node_pool.cpp and model_prescan.cpp define the rest.
// This is the one declaration of Node: include this header, never redeclare it.
#ifndef GURUMIN_NODE_H
#define GURUMIN_NODE_H

#include "matrix4.h"

// Types the node only points to. The mesh types are declared in mesh.h, the extra/skin/saved blocks in node_data.h;
// include those headers to use them.
struct VertexPack;   // a vertex pack of the mesh (mesh.h)
struct IndexList;    // an index list of a pack, or a sphere list (mesh.h)
struct Material;     // a mesh material: colour, textures (mesh.h)
struct MeshData;     // the Direct3D buffers built from the packs (mesh.h)
struct Channel;      // an animation plug's channel
struct TextureRef;   // a shared texture reference
struct NodeExtra;    // the node's animation tracks (node_data.h)
struct NodeSkin;     // skinning data: bones and skin matrices (node_data.h)
struct NodeSaved;    // saved-state block (node_data.h)
struct GameFile;     // an open data file (model_prescan.cpp)
struct VisData;      // visibility cells and their boxes (visibility.cpp)

// Called instead of the normal mesh drawing when set (node_draw.cpp passes the node's world matrix, the view matrix and
// Draw's own arguments).
typedef void (*NodeDrawCallback)(Matrix4 *world, const Matrix4 *view, int a3, int a4, int a5, int a6, int a7,
                                 int a8, int a9, int a10, int a11, int a12, int a13, int a14);

// Limits of the fixed-size arrays in a node.
enum NodeLimits
{
    NODE_NAME_SIZE = 0x40,     // characters in a node name, including the terminator
    NODE_MAX_CHILDREN = 0x40,  // children (and child names) a node can hold
    NODE_PLUG_COUNT = 16,      // animation plugs per node
    NODE_POSE_COUNT = 16       // the current pose and the stored poses
};

// Node::flags bits whose meaning is known.
enum NodeFlags
{
    NODE_FLAG_DRAWN = 0x10,               // drawn this frame
    NODE_FLAG_HIDDEN = 0x20,              // not drawn
    NODE_FLAG_NEVER_CULLED = 0x40,        // skips the view-volume test
    NODE_FLAG_COLLISION_PROBE = 0x400,    // a collision probe: test this node itself, not its children
    NODE_FLAG_SPECIAL_PASSES = 0xe00,     // drawn in one of the special passes
    NODE_FLAG_SKIP_SUBTREE = 0x8000,      // the subtree is not updated or drawn
    NODE_FLAG_ANIMATED_BY_EXTRA = 0x100000,  // animated through its extra data (NodeExtra)
    NODE_FLAG_NO_STATE_9 = 0x800000       // drawn with render state 9 turned off
};

struct Node
{
    char name[NODE_NAME_SIZE];   // 0x000: the node's name ("CDLnn..." = collision region)
    char folder[0x10];           // 0x040: texture folder of the model, inherited by children
    char m_0050[0x10];           // 0x050: second inherited name
    int m_0060;                  // 0x060
    int packCount;               // 0x064: entries in `packs`
    VertexPack *packs;           // 0x068: the mesh's vertex packs
    IndexList *indexLists;       // 0x06c: one index list per pack
    IndexList *m_0070;           // 0x070: one sphere list per pack (bounding sphere per triangle)
    int m_0074;                  // 0x074
    NodeDrawCallback drawCallback;  // 0x078: draws the node instead of the mesh code when set
    int m_007c;                  // 0x07c: number of entries in m_0080 and m_0084
    VertexPack *m_0080;          // 0x080
    IndexList *m_0084;           // 0x084
    char m_0088;                 // 0x088
    char isStatic;               // 0x089: 1 once the node was collected as static scenery (or is the collector)
    short staticCount;           // 0x08a: on the collector: number of entries in `staticNodes`
    Node **staticNodes;          // 0x08c: on the collector: the collected nodes (room for 0x200)
    int materialCount;           // 0x090: entries in `materials`
    Material *materials;         // 0x094
    void *m_0098;                // 0x098
    MeshData *mesh;              // 0x09c: the built mesh buffers (0 = none)
    int drawMode;                // 0x0a0: 2 = drawn with a light set of its own
    int built;                   // 0x0a4: 1 once BuildMesh has created the mesh buffers
    int hasCollision;            // 0x0a8: non-zero when the node has collision
    unsigned int flags;          // 0x0ac: NodeFlags
    float cullMargin;            // 0x0b0: margin of the view-volume test
    int group;                   // 0x0b4
    int m_00b8;                  // 0x0b8: set on the whole subtree by M_1019b230
    int *frameSource;            // 0x0bc: when set, the frame shown is read from here
    int plugType[NODE_PLUG_COUNT];         // 0x0c0: 0x4ee33 = the plug gives the frame
    Channel *plugs[NODE_PLUG_COUNT];       // 0x100
    int plugTypeCopy[NODE_PLUG_COUNT];     // 0x140
    void *plugData[NODE_PLUG_COUNT];       // 0x180
    int animated;                // 0x1c0: non-zero = the mesh holds several frames
    int frameSize;               // 0x1c4: primitives per frame
    int frameCount;              // 0x1c8: frames in the mesh
    Matrix4 matrix;              // 0x1cc: the node's own matrix (relative to its parent)
    Matrix4 inverseMatrix;       // 0x20c: inverse of `matrix`
    Matrix4 worldMatrix;         // 0x24c: the node's matrix in world space
    TextureRef *m_028c;          // 0x28c: shared texture reference
    Matrix4 *frameMatrices;      // 0x290: the node's matrix for each animation frame (0 = use `matrix`)
    int frameMatrixCount;        // 0x294: entries in `frameMatrices`
    Vec4 m_0298;                 // 0x298
    NodeExtra *extra;            // 0x2a8: the node's animation tracks
    Vec4 tint;                   // 0x2ac: colour multiplier, w = alpha (1 = fully visible, node_fade.cpp); all 1 after Init
    char pad_02bc[0x10];         // 0x2bc: not seen yet
    int frame;                   // 0x2cc: current animation frame (-1, -2, ...: a stored pose)
    unsigned char kind;          // 0x2d0: node kind from the file (0xff = none: decided by the name prefix)
    Matrix4 poses[NODE_POSE_COUNT];  // 0x2d4: the current pose ([0]) and the stored poses (frames -1 to -16)
    Vec4 bounds;                 // 0x6d4: bounding sphere (centre relative to the node, radius in w)
    Vec4 boundsMin;              // 0x6e4: lower corner of the mesh's box
    Vec4 boundsMax;              // 0x6f4: upper corner of the mesh's box
    char skinned;                // 0x704: 1 = the node has skinning data (also skipped by the sphere tests unless
                                 //        DAT_11b04510 is set)
    NodeSkin *skin;              // 0x708: skinning data
    char m_070c[0x40];           // 0x70c: a name
    Node *parent;                // 0x74c: the parent node (0 = root)
    int childCount;              // 0x750
    char childNames[NODE_MAX_CHILDREN][NODE_NAME_SIZE];  // 0x754
    Node *children[NODE_MAX_CHILDREN];                   // 0x1754
    NodeSaved *saved;            // 0x1854: saved-state block (allocated by AllocSaved)
    unsigned char m_1858;        // 0x1858: cycling child index (1..childCount), advanced by M_10194ee0
    int m_185c;                  // 0x185c
    int m_1860;                  // 0x1860: saved flags (M_10191d50)
    VisData *visibilityCells;    // 0x1864: visibility cells (0 = none)
    Vec4 m_1868;                 // 0x1868: a second scale, 1 on every axis after Init
    unsigned char m_1878;        // 0x1878: 0xe0 after Init
    char pathKind;               // 0x1879: 2 = camera path (camera_path_select.cpp)
    char m_187a;                 // 0x187a
    unsigned char m_187b;        // 0x187b: 45 after Init
    unsigned char m_187c;        // 0x187c: copied into the camera path list
    unsigned char m_187d;        // 0x187d: copied into the camera path list
    unsigned char m_187e;        // 0x187e: 0xff after Init
    float m_1880;                // 0x1880: 1.0 after Init
    int m_1884;                  // 0x1884
    float m_1888;                // 0x1888: 45.0 after Init
    int m_188c;                  // 0x188c
    int m_1890;                  // 0x1890
    int m_1894;                  // 0x1894
    char element;                // 0x1898: 1 = fire, 2 = thunder (from the node name, M_10191e50)
    unsigned char m_1899;        // 0x1899: 30 after Init
    unsigned char m_189a;        // 0x189a: colour value (M_101946b0)
    unsigned char m_189b;        // 0x189b: colour value (M_10194730)
    char m_189c;                 // 0x189c: colour value (M_10194630)

    // Life cycle and loading
    void Init();                                          // 0x10191500: every field to its default (node_init.cpp)
    void Release();                                       // 0x1019e9a0: frees the node's meshes and children
    void ReleaseMesh();                                   // 0x101913f0
    void ReleaseUniqueMeshData();                         // 0x10191280
    void M_10191210();                                    // 0x10191210: releases the shared texture reference
    int Load(const char *path, int noCopy, int a, int b); // 0x101ac010 (node_cache.cpp)
    int LoadIt3(const char *path, int a, int b);          // 0x101abe30 (Model_LoadIt3)
    int PreScanFile(const char *path, int arg);           // 0x101abd60 (Model_LoadIt3Alt)
    int ScanChunks(GameFile *file, int arg);              // 0x101a4a60 (It3_ScanChunks)
    // 0x101a58a0 (It3_ParseChunks): reads the model's chunks, checking them against the name key sum and xor.
    int ParseChunks(GameFile *file, int a, int b, int keySum, unsigned int keyCheck);
    int Clone(Node *source, int a, int b);                // 0x101a53b0: copies a loaded node tree into this node
    int CopyMesh(Node *source, int slot);                 // 0x1019b350
    int CopyAnimation(Node *source, int slot);            // 0x1019b890
    int CopyPlugData();                                   // 0x1019bd10
    int BlendFrames(int from, int to, float t);           // 0x1019b630
    int CreateChildCopies();                              // 0x101abb70
    int BuildMesh(int recurse, int flags);                // 0x1019afa0: creates the mesh buffers
    int Rebuild();                                        // 0x101ac1a0
    int ProjectPlanarUVs(int material);                   // 0x1019bdd0: planar texture coordinates (scene_node.cpp)
    int AllocMaterials(int count);                        // 0x1019c1e0
    int ComputeBounds();                                  // 0x1019d8e0
    int BuildCollisionSpheres();                          // 0x1019e120
    int M_101ab980();                                     // 0x101ab980: frees the extra index lists
    void M_101911b0(VertexPack *packs, int count);        // 0x101911b0: frees the packs' vertex data
    void M_101911e0(IndexList *lists, int count);         // 0x101911e0: frees the index lists' data
    int SortPacks();                                      // 0x1019c690 (node_sort_packs.cpp)
    int WeldVertices();                                   // 0x1019e610 (node_sort_packs.cpp)
    int ApplyMaterialColours();                           // 0x1019c560 (node_colours.cpp)
    int ResetVertexColours(float uvScale);                // 0x1019c3a0 (node_vertex_colours.cpp)
    int SetMaterialValue(const char *material, int value);  // 0x1019e8c0 (Node_SetMaterialValue)
    int ReplaceMaterial(int key, int value);              // 0x1019e830: over the whole model

    // Children and names
    Node *AddChild(const char *childName, int position);  // 0x101abc30
    int RemoveChild(Node *child, int keep);               // 0x101a2f30
    Node *Find(const char *name, int index);              // 0x1019ece0: the index-th node called `name`, or 0
    int M_1019ed10(const char *name, int value);          // 0x1019ed10: Find with the "stripped" finders
    Node *FindRecursive(const char *name, int *skip);     // 0x10191a30
    Node *FindPrefixRecursive(const char *prefix, int *skip);  // 0x10191c30
    int FindRecursiveStripped(const char *name, int *skip);    // 0x10191aa0
    int FindPrefixRecursiveStripped(const char *prefix, int *skip);  // 0x10191b60
    Node *FindByFlags(unsigned int mask, int nth, int *counter);     // 0x10191cc0
    Node *Root();                                         // 0x101992f0: the top of the tree
    Node *M_10199310();                                   // 0x10199310: highest ancestor in state 0x1000
    int Rename(const char *newName, const char *oldName); // 0x1019ed40
    int SetUniqueName(const char *name);                  // 0x101a52d0
    void MakeNameUnique();                                // 0x101a2290
    int M_10191770();                                     // 0x10191770
    int M_10191870(Node **list, int *count);              // 0x10191870
    int M_10191920(Node **list, int *count);              // 0x10191920: collects up to 8 nodes in state 0x4000
    int M_10191990(Node *const *nodes, int count);        // 0x10191990: links `nodes` to the state-0x1000 nodes
    int M_1019eeb0(Node *root, Node **list, int *count);  // 0x1019eeb0: links nodes and bones under `root`
    int M_1019eca0();                                     // 0x1019eca0 (node_pool.cpp)

    // Flags, render modes and colours
    int SetFlags(unsigned int bits);                      // 0x1019b130: on this node and all children
    int ClearFlags(unsigned int bits);                    // 0x1019b1b0: on this node and all children
    int M_1019b190(unsigned int bits);                    // 0x1019b190: sets flag bits on this node only
    int M_1019b210(unsigned int bits);                    // 0x1019b210: clears flag bits on this node only
    int M_1019b230(int value);                            // 0x1019b230: m_00b8 on the whole subtree
    int M_1019b050(int value);                            // 0x1019b050: render mode of the subtree
    int M_1019b0c0(int value, int except1, int except2);  // 0x1019b0c0
    int M_10193cb0(int from, int to);                     // 0x10193cb0: render mode `from` -> `to` in the subtree
    void M_10193d10(int index);                           // 0x10193d10
    int M_10194630(char value, unsigned int mask);        // 0x10194630: m_189c on matching kinds
    int M_101946b0(unsigned char value, unsigned int mask); // 0x101946b0: m_189a on matching kinds
    int M_10194730(char value, unsigned int mask);        // 0x10194730: m_189b on matching kinds
    int M_10191e50();                                     // 0x10191e50: element from the node names
    void M_10191d50();                                    // 0x10191d50: saves the subtree's flags
    void M_10191d90();                                    // 0x10191d90
    void M_10191dd0();                                    // 0x10191dd0
    void M_10191e10();                                    // 0x10191e10
    int RestartColourPlugs(const char *motion, int frames);  // 0x1019b300
    int M_1019bca0(Node **keep, int count);               // 0x1019bca0: releases textures not in `keep`
    void CollectStatic(Node *collector);                  // 0x1019d750
    int M_1019d600();                                     // 0x1019d600: special pass marking
    int M_1019d680();                                     // 0x1019d680: clears the special pass marker upwards
    int ClearSpecialPassOnLinks();                        // 0x1019d6a0
    int M_101a8760();                                     // 0x101a8760: both special pass steps

    // Saved states
    int AllocSaved();                                     // 0x101947b0: allocates `saved` on first use
    int EnterTargetState(void *target);                   // 0x101948f0: enters the tracking state (0 = default)
    int FreeSaved();                                      // 0x10194980: frees `saved`
    int M_101947f0();                                     // 0x101947f0: leaves state 0x1000
    int M_10194830();                                     // 0x10194830: leaves state 0x400000
    int M_10194870();                                     // 0x10194870: leaves state 0x2000
    int M_101948c0();                                     // 0x101948c0: leaves state 0x4000
    int M_101949d0();                                     // 0x101949d0: allocates the 0x80-byte extra block
    int M_101944d0();                                     // 0x101944d0
    int ReleaseSavedStates();                             // 0x10194510
    void RestoreSavedMatrix();                            // inline (scene_node.cpp)

    // Animation, matrices and bones
    int UpdateWorld(const Matrix4 &parent, int frame);    // 0x10194c60 (node_world.cpp)
    int M_10194ee0();                                     // 0x10194ee0: advances the cycling child index
    int M_10199340();                                     // 0x10199340: last animation frame of the subtree
    // 0x10199380: searches this subtree for bone `id`, accumulating parent matrices; returns its matrix or 0.
    Matrix4 *FindBone(int id, Matrix4 *parent, Matrix4 *work, int frame);
    Matrix4 BoneMatrix(int id, int flags);                // 0x101a23b0
    int SetupSkins(Node *root);                           // 0x101ab9d0

    // Drawing
    int Draw(const Matrix4 *parent, const Matrix4 *view, int a3, int a4, int a5, int a6, int a7, int a8, int a9,
             int a10, int a11, int a12, int a13, int a14);  // 0x101995a0 (node_draw.cpp)
    // 0x10199e20: queues the subtree for drawing under `parent` with `view` (map_view.cpp, mesh_slots.cpp).
    void QueueDraw(const Matrix4 *parent, const Matrix4 *view, const Vec4 *direction, int a, const Vec4 *colour,
                   int b);

    // Collision
    int Collide(int a, int b, Vec4 *position, Vec4 *velocity, Vec4 *hit);  // 0x101a8e60
    // 0x101a9670: casts the segment against the nodes called `name` (0 = all) whose kind and mask match; writes
    // the nearest hit point and, when `t` is given, its position along the segment. Returns 1 on a hit.
    int RayCast(const char *name, unsigned kind, unsigned mask, const Vec4 *to, const Vec4 *from, Vec4 *hit,
                float *t);
    // 0x101aaa20: casts the segment `from`-`to` against the collision of the nodes called `name` (0 = all) with
    // the given mask; the nearest hit point goes to `hit` and, when `t` is given, its position along the segment.
    int CastSegment(const char *name, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t);
    // 0x101aa080: same arguments and result as CastSegment (a second variant of the same search).
    int CastSegment2(const char *name, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t);
    // 0x101a1640: tests a sphere of `radius` at `point` (in `matrix` space) against the collision below this node,
    // skipping nodes not called `name` (0 = any) and nodes called `excludeName`; the hit normal goes to `normal`
    // and the penetration value to `depth`. Returns 1 on a hit.
    int Query(Matrix4 *matrix, Vec4 *point, float radius, Vec4 *normal, float *depth, const char *name,
              const char *excludeName);
    int SpheresOverlap(const Vec4 *boundsA, const Vec4 *offsetA, const Vec4 *boundsB, const Vec4 *offsetB,
                       Vec4 *aToB, Vec4 *bToA);           // 0x10193060 (node_spheres.cpp)
    int FindTouchingSphere(int unused, const Vec4 *center, float radius, Node **found, int *foundCount);  // 0x1019f210
    int CollectNearSegment(int unused, const Vec4 *from, const Vec4 *to, float range, Node **found, int *foundCount,
                           Vec4 *closest);                // 0x10191ec0
    int M_10194f40(float *position, Matrix4 *matrix, int a, float radius, int b, int depth);  // 0x10194f40
    int StartProximityPass(float *position, Matrix4 *matrix, int a, float radius, int b);     // 0x101a2370
    // 0x1019f340: tests a moving model (probe points) against this collision node (model_params.cpp).
    int M_1019f340(int mask, Matrix4 *world, int a, int b, int *out5, Vec4 *probe, int probeCount, int *hitCount,
                   Vec4 *hitPoint, Vec4 *hitNormal, int limit, int *out12, int c, int d, int e);
    int UpdateVisibleCells();                             // 0x10199be0 (visibility.cpp)
    int TagRegion(const Box2 *boxes, const char (*names)[NODE_NAME_SIZE], Matrix4 *matrix);  // 0x10199cd0
};

#endif
