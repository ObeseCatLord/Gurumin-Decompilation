// A scene node: one part of a loaded model (.it3 file) with its mesh, matrices, animation plugs and children.
// EN declaration of Node (the JP one is decomp/include/node.h). Only the members EN code has used so far are
// declared; add fields here, with their EN offsets, when a matched EN function needs them. The gaps are padding.
// sizeof(Node) = 0x18a0 is proven by the effect-node pool stride 0x18a0 at EN 0x016a57d0.
// This is the one declaration of Node in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_NODE_H
#define GURUMIN_EN_NODE_H

#include "vec4.h"

struct NodeExtra;
struct NodeSaved;
struct NodeSkin;
struct IndexList;
struct Channel;
struct SlotTable;
struct VertexPack;
struct Material;
struct TextureRef;
struct VisData;
struct DrawList;
struct AnimModel;
struct GameFile;

// Node::flags bits whose meaning is known.
enum NodeFlags
{
    NODE_FLAG_KEEP_MESH = 0x4,             // the node's mesh data is kept when a unique model file is released
    NODE_FLAG_HIDDEN = 0x20,               // not drawn
    NODE_FLAG_NEVER_CULLED = 0x40,         // skips the view-volume test
    NODE_FLAG_SHAPE_BOX = 0x200,           // debug pass: shown as its box
    NODE_FLAG_SHAPE_SPHERE = 0x400,        // debug pass: shown as its bounding sphere
    NODE_FLAG_SHAPE_SOLID_BOX = 0x800,     // debug pass: shown as its box, filled
    NODE_FLAG_SPECIAL_PASSES = 0xf00,      // any of the 0x100 to 0x800 pass bits
    NODE_FLAG_SPECIAL_SHAPES = 0x3e00,     // debug shape modes (0xe00) and the limb states (0x3000): excluded from the special pass
    NODE_FLAG_LIMB_STATES = 0x3000,        // the limb states 0x1000 and 0x2000
    NODE_FLAG_CHASING = 0x1000,            // a chasing node (its simulated point follows the tip)
    NODE_FLAG_TWO_BONE_LIMB = 0x2000,      // a two-bone limb (saved matrix restored by RestoreState2000)
    NODE_FLAG_COLLISION_SPHERE = 0x4000,   // a collision sphere node
    NODE_FLAG_SKIP_SUBTREE = 0x8000,       // the subtree is not updated or drawn (also the "special pass" marker)
    NODE_FLAG_TEMPORARY_A = 0x40000,       // temporary saved state: undone and freed by ReleaseSavedStates
    NODE_FLAG_TEMPORARY_B = 0x80000,       // second temporary saved state
    NODE_FLAG_ANIMATED = 0x100000,         // animated through its `extra` tracks (matrices sampled from them)
    NODE_FLAG_TARGET_TRACKING = 0x400000   // tracks a target (saved matrix restored by RestoreState400000)
};

// Limits and sizes of the fixed-size parts of a node.
enum NodeLimits
{
    NODE_NAME_SIZE = 0x40,      // characters in a node name, including the terminator
    NODE_MAX_CHILDREN = 0x40,   // children (and child names) a node can hold
    NODE_PLUG_COUNT = 16,       // animation plugs per node
    NODE_FOLDER_SIZE = 0x10,    // characters in an inherited folder name
    NODE_SIZE = 0x18a0          // sizeof(Node)
};

// Node::savedFlags bit that keeps a node's mesh data when its unique model file is released.
const int NODE_SAVED_FLAG_SHARED = 4;
// SetRenderModeExcept argument meaning "no exception".
const int NODE_NO_RENDER_MODE = -1;
// Node::drawMode value that is not applied to skinned nodes with bones (SetRenderMode).
const int NODE_DRAW_MODE_SKIN_EXCLUDED = 8;
// Plug type of a plug that gives the animation frame.
const int PLUG_TYPE_FRAME = 0x4ee33;
// First of the two colour plug slots.
const int PLUG_COLOUR_FIRST = 6;
// Number of colour plug slots.
const int PLUG_COLOUR_COUNT = 2;
// Characters of a node name compared by the element tagging ("fire", "thun").
const int NODE_ELEMENT_PREFIX_LENGTH = 4;
// Node::element values (bit mask).
enum NodeElement
{
    NODE_ELEMENT_FIRE = 1,      // the node name starts with "fire"
    NODE_ELEMENT_THUNDER = 2    // the node name starts with "thun"
};

// Material sizes: texture file names and material names share one size.
const int MATERIAL_NAME_SIZE = 0x40;
const int MATERIAL_TEXTURE_COUNT = 4;
// Material::alpha of a blank material (no vertex alpha).
const float MATERIAL_BLANK_ALPHA = 0.0f;
// Material::m_0174 of a blank material.
const float MATERIAL_BLANK_VALUE = 255.0f;

// A mesh material (0x180 bytes).
struct Material
{
    char name[MATERIAL_NAME_SIZE];                                  // 0x00: the material's name
    char textures[MATERIAL_TEXTURE_COUNT][MATERIAL_NAME_SIZE];      // 0x40: texture file names (empty = none)
    float colour[3];                                                // 0x140: r, g, b (1 = white)
    int m_014c;                                                     // 0x14c: cleared in a blank material
    int m_0150;                                                     // 0x150: cleared
    int m_0154;                                                     // 0x154: cleared
    int m_0158;                                                     // 0x158: not touched by SetBlank
    int m_015c;                                                     // 0x15c: cleared
    int m_0160;                                                     // 0x160: cleared
    int m_0164;                                                     // 0x164: cleared
    int m_0168;                                                     // 0x168: cleared
    int m_016c;                                                     // 0x16c: cleared
    int m_0170;                                                     // 0x170: cleared in a blank material
    float m_0174;                                                   // 0x174: 255 for a blank material
    float alpha;                                                    // 0x178: vertex alpha (0 for a blank material)
    char edge[4];                                                   // 0x17c: edge flags, signed bytes

    void SetBlank(float r, float g, float b, char edgeFlag);        // EN 0x779880 (JP: loop body of AllocMaterials)
};

// Number of nodes CreateChildCopies makes.
const int NODE_CHILD_COPY_COUNT = 0x20;
// Node::drawMode of the child copies CreateChildCopies makes.
const int NODE_DRAW_MODE_CHILD_COPY = 0xc;
// Node::drawMode of a node AddChild creates.
const int NODE_DRAW_MODE_NEW_CHILD = 8;
// Allocator tag of a node created as a child.
#define NODE_CHILD_ALLOC_TAG "m_child[m_numchild]"
// Node::box corners.
enum NodeBoxCorner
{
    NODE_BOX_MIN = 0,   // lower corner
    NODE_BOX_MAX = 1    // upper corner
};
// Nodes the tracking collection of Rebuild can hold.
const int NODE_TRACKED_MAX = 8;
// Nodes the link collection of Rebuild can hold.
const int NODE_LINKS_MAX = 16;
// Characters of the scratch name buffers used when renaming.
const int NODE_NAME_SCRATCH_SIZE = 256;
// Name prefix lengths of special nodes: "CDL<nn>-" (collision region) and "OvF<n>-" (overlay).
const int NODE_COLLISION_PREFIX_LENGTH = 6;
const int NODE_OVERLAY_PREFIX_LENGTH = 7;
// Node::kind values: kinds below NODE_KIND_REGION_LIMIT are collision regions, 0xfd an overlay, 0xff none.
const unsigned char NODE_KIND_NONE = 0xff;
const unsigned char NODE_KIND_PLAIN = 0xfe;   // a node of a map that is neither a collision region nor an overlay
// Bytes of the collector's static node list.
const int NODE_STATIC_LIST_SIZE = 0x800;
const unsigned char NODE_KIND_OVERLAY = 0xfd;
const unsigned char NODE_KIND_REGION_LIMIT = 0x40;
// Regions a map root node can name (TagRegion).
const int NODE_REGION_COUNT = 0x40;
// Bounds radius factor of the collision test against a segment (CollectNearSegment).
const float NODE_SEGMENT_RADIUS_FACTOR = 0.578f;

// Poses a node keeps: the current one and the stored ones.
const int NODE_POSE_COUNT = 16;
// Characters of "charbase" compared by BlendFrames.
const int NODE_CHARBASE_PREFIX_LENGTH = 8;
// MatrixBlend modes: plain interpolation, and interpolation of the rotation (skeleton roots).
const int BLEND_LINEAR = 0;
const int BLEND_ROTATION = 1;
// Non-zero while an important model changes motion (read by the blending and playback code). EN 0x0217fafc (JP DAT_11b04508).
extern "C" char g_importantMotionChange;
// out = a blended towards b by t (mode BLEND_ROTATION interpolates the rotation). EN 0x771830 (JP 0x101b1420).
extern "C" Matrix4 *MatrixBlend(Matrix4 *out, Matrix4 a, Matrix4 b, int t, int mode);

// Motion frame of the current update pass, set by StartProximityPass. EN 0x217faf0 (JP DAT_11b044f8).
extern "C" int g_updatePassFrame;

// The default object a node in the target-tracking state follows. EN 0x0218e670 (JP DAT_11b07118).
extern "C" char g_defaultTargetObject[];
// Called with the path before a model file is loaded when set (the host's load notification). EN 0x021e482c (JP DAT_10f65538).
extern "C" void (*g_modelLoadNotify)(const char *path, int a);
// Non-zero = the segment tests also look at skinned nodes. EN 0x0217b8d4 (JP DAT_11b04510).
extern "C" int g_testSkinnedNodes;

// Called instead of the normal mesh drawing when set (the node's world matrix, the view matrix and Draw's own arguments).
typedef void (*NodeDrawCallback)(Matrix4 *world, const Matrix4 *view, int a3, const Matrix4 *shadowMatrix,
                                 float shadowX, float shadowY, float shadowZ, int shadowTexture,
                                 float shadowScale, int shadowSize, const Vec4 *lightDirection, float modelSize,
                                 AnimModel *model, float alpha);
// Called by QueueDraw after the children when set (the node's world matrix and QueueDraw's own arguments).
typedef void (*NodeQueueCallback)(Matrix4 *world, const Matrix4 *view, const void *lights, unsigned int passMask,
                                  const Vec4 *colour, const Vec4 *addColour);

struct Node
{
    char name[NODE_NAME_SIZE];     // 0x000: the node's name ("fire..." / "thun..." set the element)
    char folder[NODE_FOLDER_SIZE];  // 0x040: texture folder of the model, inherited by children
    char folderB[NODE_FOLDER_SIZE]; // 0x050: second inherited name
    int m_0060;                    // 0x060: not used by EN code yet
    int packCount;                 // 0x064: vertex packs of the mesh (also the entries of triangleSpheres; -1 = none, see ReplaceMaterial)
    VertexPack *packs;             // 0x068: the mesh's vertex packs (0 = none)
    IndexList *indexLists;         // 0x06c: one index list per pack
    IndexList *triangleSpheres;    // 0x070: one sphere list per pack (0 = none)
    NodeQueueCallback queueCallback;  // 0x074: run by QueueDraw after the children when set
    NodeDrawCallback drawCallback;    // 0x078: draws the node instead of the mesh code when set
    int altPackCount;              // 0x07c: entries of altPacks and altIndexLists
    VertexPack *altPacks;          // 0x080: second set of vertex packs, freed with the node (0 = none)
    IndexList *altIndexLists;      // 0x084: second set of index lists, freed with the node (0 = none)
    char m_0088;                   // 0x088: not used by EN code yet
    char isStatic;                 // 0x089: 1 once the node was collected as static scenery (or is the collector)
    short staticCount;             // 0x08a: on the collector: entries of staticNodes
    Node **staticNodes;            // 0x08c: on the collector: the collected nodes (room for NODE_STATIC_LIST_SIZE bytes)
    int materialCount;             // 0x090: entries of the material table
    Material *materials;           // 0x094: the node's materials (materialCount entries)
    void *targetObject;            // 0x098: the object a node in the target-tracking state follows (EnterTargetState)
    SlotTable *mesh;                // 0x09c: the built mesh buffers (0 = none)
    int drawMode;                  // 0x0a0: render mode of the node
    int built;                     // 0x0a4: 1 once BuildMesh has created the mesh buffers
    int hasCollision;              // 0x0a8: non-zero when the node has collision
    unsigned int flags;            // 0x0ac: NodeFlags
    float cullMargin;              // 0x0b0: margin of the view-volume test
    int group;                     // 0x0b4: not used by EN code yet
    AnimModel *model;              // 0x0b8: the model the tree belongs to
    int *frameSource;              // 0x0bc: when set, the frame shown is read from here
    int plugType[NODE_PLUG_COUNT];     // 0x0c0: type of each animation plug (PLUG_TYPE_FRAME gives the frame)
    Channel *plugs[NODE_PLUG_COUNT];   // 0x100: the channel behind each plug
    int plugTypeCopy[NODE_PLUG_COUNT]; // 0x140: plug types kept with the private plug data (CopyPlugData)
    void *plugData[NODE_PLUG_COUNT];   // 0x180: private copies of the plug channels' data (CopyPlugData)
    int animated;                  // 0x1c0: non-zero = the mesh holds several frames
    int frameSize;                 // 0x1c4: primitives per frame
    int frameCount;                // 0x1c8: frames in the mesh
    Matrix4 matrix;                // 0x1cc: the node's own matrix (relative to its parent)
    Matrix4 inverseMatrix;         // 0x20c: inverse of `matrix`
    Matrix4 worldMatrix;           // 0x24c: the node's matrix in world space
    TextureRef *textureRef;        // 0x28c: shared texture reference (ReleaseTexture)
    Matrix4 *frameMatrices;        // 0x290: the node's matrix for each animation frame (0 = use `matrix`)
    int frameMatrixCount;          // 0x294: entries of the per-frame matrices
    Vec4 m_0298;                   // 0x298: not used by EN code yet
    NodeExtra *extra;              // 0x2a8: the node's animation tracks (allocated by AllocExtra)
    Vec4 tint;                     // 0x2ac: colour multiplier, w = alpha (1 = fully visible); all 1 after Init
    Vec4 m_02bc;                   // 0x2bc: not used by EN code yet
    int frame;                     // 0x2cc: current animation frame (-1, -2, ...: a stored pose)
    unsigned char kind;            // 0x2d0: node kind from the file (0xff = none: decided by the name prefix)
    char m_02d1[0x2d4 - 0x2d1];    // 0x2d1: not used by EN code yet
    Matrix4 poses[NODE_POSE_COUNT];    // 0x2d4: the current pose ([0]) and the stored poses (frames -1 to -16)
    Vec4 bounds;                   // 0x6d4: bounding sphere (centre relative to the node, radius in w)
    Vec4 box[2];                   // 0x6e4: the mesh's box: [0] lower, [1] upper corner
    char skinned;                  // 0x704: 1 = the node has skinning data (a bone node when it has none of its own mesh)
    char m_0705[0x708 - 0x705];    // 0x705: not used by EN code yet
    NodeSkin *skin;                // 0x708: skinning data (0 = none)
    char parentName[NODE_NAME_SIZE];  // 0x70c: the name of the parent node (set when the node is attached)
    Node *parent;                  // 0x74c: the parent node (0 = root)
    int childCount;                // 0x750: entries of children
    char childNames[NODE_MAX_CHILDREN][NODE_NAME_SIZE];  // 0x754: the child names
    Node *children[NODE_MAX_CHILDREN];  // 0x1754: the child nodes
    NodeSaved *saved;              // 0x1854: saved-state block (allocated by AllocSaved)
    unsigned char cycleIndex;      // 0x1858: cycling child index (1..childCount), advanced by CycleChild
    int savedDrawMode;             // 0x185c: render mode kept by SaveRenderMode
    int savedFlags;                // 0x1860: flags kept by SaveFlags
    VisData *visibilityCells;      // 0x1864: visibility cells and their boxes of a map root node (0 = none)
    Vec4 fogColour;                // 0x1868: fog colour of a "fogset" root node, 1 on every axis after Init
    unsigned char m_1878;          // 0x1878: 0xe0 after Init
    char pathKind;                 // 0x1879: 2 = camera path
    char m_187a;                   // 0x187a: not used by EN code yet
    unsigned char m_187b;          // 0x187b: 45 after Init
    unsigned char m_187c;          // 0x187c: copied into the camera path list
    unsigned char m_187d;          // 0x187d: copied into the camera path list
    unsigned char m_187e;          // 0x187e: 0xff after Init
    char m_187f;                   // 0x187f: padding
    float fogDensity;              // 0x1880: fog density of a "fogset" root node, 1.0 after Init
    float lightConeInner;          // 0x1884: first value of the "setlightcone" command
    float m_1888;                  // 0x1888: 45.0 after Init
    float fogNear;                 // 0x188c: distance the fog of a "fogset" root node starts at
    float fogFar;                  // 0x1890: distance it is full at
    int m_1894;                    // 0x1894: not used by EN code yet
    unsigned char element;         // 0x1898: NodeElement bits from the node name
    unsigned char brightness;      // 0x1899: brightness of the region the node lies in (the fading nodes' alpha follows it)
    unsigned char colourA;         // 0x189a: colour value set by SetColourA on the shape-mode nodes
    unsigned char colourB;         // 0x189b: colour value set by SetColourB
    unsigned char colourC;         // 0x189c: colour value set by SetColourC
    char m_189d[NODE_SIZE - 0x189d];   // 0x189d: padding up to the end of the node

    // Name lookups in the subtree (scene_node.cpp). `skip` counts down the matches still to pass over.
    Node *Find(const char *name, int index);                   // EN 0x7532c0 (JP 0x1019ece0)
    Node *FindRecursive(const char *name, int *skip);          // EN 0x7531d0 (JP 0x10191a30)
    Node *FindPrefixRecursive(const char *prefix, int *skip);  // EN 0x753240 (JP 0x10191c30)
    // Same lookups with the second pair of finders, which ignore the name prefixes of special nodes.
    Node *FindPrefixNode(const char *prefix);                          // EN 0x7533a0: first node whose name starts with `prefix` (no skip count)
    Node *FindPrefixNodeRecursive(const char *prefix);                 // EN 0x753330: the search behind FindPrefixNode (children last to first)
    Node *Find2(const char *name, int index);                       // EN 0x753740 (JP 0x1019ed10)
    Node *FindRecursiveStripped(const char *name, int *skip);          // EN 0x753590 (JP 0x10191aa0)
    Node *FindPrefixRecursiveStripped(const char *prefix, int *skip);  // EN 0x753660 (JP 0x10191b60)

    Node *FindByFlags(unsigned int mask, int nth, int *counter);   // EN 0x753420 (JP 0x10191cc0)
    int Rename(const char *newName, const char *oldName);          // EN 0x769fa0 (JP 0x1019ed40)
    int SetUniqueName(const char *name);                           // EN 0x76b570 (JP 0x101a52d0)
    void MakeNameUnique();                                         // EN 0x76d540 (JP 0x101a2290)
    int TagRegion(const Box2 *boxes, const char (*names)[NODE_NAME_SIZE], Matrix4 *matrix);  // EN 0x76a130 (JP 0x10199cd0)
    int CollectNearSegment(int unused, const Vec4 *from, const Vec4 *to, float range, Node **found, int *foundCount,
                           Vec4 *closest);                         // EN 0x75a660 (JP 0x10191ec0)

    // Special passes (scene_node.cpp).
    int MarkSpecialPass();                 // EN 0x75a900 (JP 0x1019d600)
    int ClearSpecialPassOnLinks();         // EN 0x75a970 (JP 0x1019d6a0)
    int RunSpecialPass();                  // EN 0x75aa50 (JP 0x101a8760)
    int ClearSpecialPassUp();              // EN 0x75aa30 (JP 0x1019d680)

    // Tree navigation.
    Node *Root();                          // EN 0x7541b0 (JP 0x101992f0)
    Node *GetLinkedRoot();                 // EN 0x754180 (JP 0x10199310)
    int GetLastFrame();                    // EN 0x754120 (JP 0x10199340)

    // Loading and mesh data.
    int Rebuild();                         // EN 0x76eb00 (JP 0x101ac1a0)
    int BlendFrames(int from, int to, int t);   // EN 0x749e00 (JP 0x1019b630)
    void CollectStatic(Node *collector);   // EN 0x75aa70 (JP 0x1019d750)
    // EN 0x759ad0 (JP 0x10193140): tests this tree (moved by `matrix`) against the tree of `other`, one probe offset per call.
    int CollideModel(Matrix4 *matrix, Node *other, Matrix4 *otherMatrix, Vec4 *probes, int probeCount, int *probeIndex,
                     AnimModel **touchedModels, Vec4 *touchedPushes, int *touchedCount, int mode, int *mapHit, int resolve,
                     int *hitMatrix, unsigned int mask);
    // EN 0x74a530 (JP 0x1019f340): tests probe spheres against the meshes of `node` and its children.
    int CollideProbesWithMesh(unsigned int mask, Matrix4 *world, Node *node, float reach, float *nearest, Vec4 *probes,
                              int probeCount, int *probeIndex, Vec4 *hitPoint, Vec4 *hitNormal, unsigned int flags,
                              int *hitFlag, int resolve, int *hitMatrix, int *skipped);
    // EN 0x75db30 (JP 0x101a8e60): moves `position` by `velocity` against this node's meshes (tag filter, mask); fills `hit`.
    int Collide(const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit);
    // EN 0x74c990 (JP 0x101a1640): tests a sphere of `radius` at `point` (in `matrix` space) against the collision below this node, skipping nodes not called `name` (0 = any) and nodes called `excludeName`; the hit normal goes to `normal`, the penetration value to `depth`. Returns 1 on a hit.
    int Query(Matrix4 *matrix, Vec4 *point, float radius, Vec4 *normal, float *depth, const char *name, const char *excludeName);
    // EN 0x761d30 (JP 0x101a9670): casts the segment `from` to `to` against this node's meshes (tag filter, mask); fills `hit`, and `t` (may be 0) with the hit fraction.
    int CastSegment(const char *tag, unsigned int mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t);
    // EN 0x761390 (JP 0x101aa080): same arguments and result as CastSegment (a second variant of the same search).
    int CastSegment2(const char *tag, unsigned int mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t);
    // EN 0x7609a0 (JP 0x101aaa20): casts the ray from `from` to `to` against this node's meshes (tag filter, flags, mask); fills `hit`, and `t` (may be 0). Returns 1 on a hit.
    int CastRay(const char *tag, int flags, unsigned int mask, Vec4 *to, Vec4 *from, Vec4 *hit, float *t);
    // EN 0x755750 (JP 0x1019dca0): ray-triangle test (front face only) for the ray from `origin` along `direction` and the triangle a-b-c; on a hit
    // *hitT is the distance along the ray in units of `direction`, the hit point replaces `a` (x, y, z) and *texU / *texV are the texture
    // coordinates at the hit point interpolated from uvA..uvC. Returns 1 on a hit, else 0. Does not use `this`.
    int IntersectTriangle(const Vec4 *origin, const Vec4 *direction, Vec4 *a, Vec4 *b, const Vec4 *c, float *hitT, float *texU, float *texV,
                          const Vec4 *uvA, const Vec4 *uvB, const Vec4 *uvC);
    // EN 0x750a50 (JP 0x10199e20): queues this node tree for drawing into `list` with `view`, the lights `lights`, pass mask `pass` (-1 = all), fade colour `fade` and `flag`.
    int QueueDraw(DrawList *list, Matrix4 *view, const void *lights, int pass, Vec4 *fade, int flag);
    // Same function (EN 0x750a50) called with the world matrix of the node tree in the first argument.
    int QueueDraw(Matrix4 *world, Matrix4 *view, const void *lights, int pass, Vec4 *fade, int flag);
    int BindLinks(Node *root, Node **list, int *count);   // EN 0x76d770 (JP 0x1019eeb0): links nodes and bones under `root`
    Node *AddChild(const char *childName, int position);  // EN 0x74ebb0 (JP 0x101abc30): creates a child at `position` (appended when negative or past the end)
    int SetupSkins(Node *root);            // EN 0x76dbe0 (JP 0x101ab9d0): stores bind poses and inverse bone matrices for the skinned nodes of this subtree
    int CopyBindPoses();                   // EN 0x76e130 (JP 0x10191770)
    int CollectTrackingNodes(Node **list, int *count);    // EN 0x76ea90 (JP 0x10191920): nodes in state 0x4000 (up to 8)
    int LinkToChasing(Node *const *nodes, int count);     // EN 0x76d6e0 (JP 0x10191990)
    int StoreLinks(Node **list, int *count);              // EN 0x76eb70 (JP 0x10191870)
    int ScanChunks(GameFile *file, int arg);              // EN 0x7587c0 (JP 0x101a4a60): reads the INFO/MAN2 chunks of a model file
    int ParseChunks(GameFile *file, int a, int b, int keySum, unsigned int keyCheck);   // EN 0x755ef0 (JP 0x101a58a0): reads the model's chunks, checking them against the name key sum and xor
    // EN 0x755580 (JP 0x10193060): 1 when the bounding spheres boundsA and boundsB (centre x/y/z, radius w), each moved by its offset, overlap well; the vectors between the moved centres go to aToB and bToA when given. `this` is not used.
    int SpheresOverlap(const Vec4 *boundsA, const Vec4 *offsetA, const Vec4 *boundsB, const Vec4 *offsetB, Vec4 *aToB, Vec4 *bToA);
    // EN 0x75a770 (JP 0x1019f210): looks for nodes below this one whose bounding sphere overlaps the sphere of `radius` around `center`; hits are appended to `found` (when given). Returns 1 on a hit.
    int FindTouchingSphere(int unused, const Vec4 *center, float radius, Node **found, int *foundCount);
    int ResetVertexColours(float uvScale);  // EN 0x74f3c0 (JP 0x1019c3a0): every vertex white with its emissive colour cleared; a non-zero `uvScale` also lifts it and takes texture coordinates from x and y; returns 0
    int WeldVertices();                    // EN 0x75b920 (JP 0x1019e610): merges equal vertices in every pack
    int SortPacks();                       // EN 0x75bef0 (JP 0x1019c690): opaque packs first, then those with vertex alpha
    // EN 0x753820 (JP 0x10199380): searches this subtree for `bone`, accumulating world matrices (this node's matrix for `frame` times `parent`); the bone's matrix is stored in `work` and returned, 0 when the subtree does not hold it.
    Matrix4 *FindBone(Node *bone, Matrix4 *parent, Matrix4 *work, int frame);
    Matrix4 BoneMatrix(Node *bone, int frame);   // EN 0x753a20 (JP 0x101a23b0): world matrix of `bone` below this node, the identity when it is not found
    // EN 0x754250 (JP 0x10191fd0): moves box `first` by `firstOffset` and box `second` by `secondOffset` and returns 1 when they overlap on all three axes; on overlap the centre difference goes to `centreDelta` and its negation to `negatedDelta` (when given). `this` is not used.
    int BoxesOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *second, const Vec4 *secondOffset, Vec4 *centreDelta, Vec4 *negatedDelta);
    // EN 0x754e20 (JP 0x10192a00): the same test for upright cylinders inscribed in the boxes.
    int CylindersOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *second, const Vec4 *secondOffset, Vec4 *delta, Vec4 *negatedDelta);
    // EN 0x754480 (JP 0x10192200): box `first` against the upright cylinder inscribed in box `second`.
    int BoxCylinderOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *second, const Vec4 *secondOffset, Vec4 *delta, Vec4 *negatedDelta);
    // EN 0x754840 (JP 0x10192520): box `first` against the sphere `sphere` (radius w * 0.578), each moved by its offset; the push directions of the three views zero the unused axes of the deltas. `this` is not used.
    int BoxSphereOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *sphere, const Vec4 *sphereOffset, Vec4 *delta, Vec4 *negatedDelta);
    // EN 0x7550b0 (JP 0x10192c90): the upright cylinder inscribed in box `first` against the sphere `sphere`. `this` is not used.
    int CylinderSphereOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *sphere, const Vec4 *sphereOffset, Vec4 *delta, Vec4 *negatedDelta);
    int PreScanFile(const char *path, int arg);           // EN 0x759100 (JP 0x101abd60): loads a model file into this node tree
    int CopyMesh(Node *source, int slot);                 // EN 0x74ddc0 (JP 0x1019b350): gives this node the (shared or copied) mesh of `source`
    int MoveToParent(Node *newParent, int position);      // EN 0x74a050: moves this node under `newParent` at `position` among its children
    int LoadIt3(const char *path, int a, int b);           // EN 0x7584f0 (JP 0x101abe30): loads the .it3 model file `path` into this node
    int Load(const char *path, int noCopy, int a, int b); // EN 0x758630 (JP 0x101ac010): loads the model file `path` into this node
    int RemoveChild(Node *child, int keep);              // EN 0x750450 (JP 0x101a2f30): takes `child` out of the child list (destroying it unless `keep`)
    int BuildMesh(int recurse, int flags);                // EN 0x76d630 (JP 0x1019afa0)
    int CreateChildCopies();               // EN 0x74dd00 (JP 0x101abb70): makes 32 child copies (for effects drawn many times)
    int ShadeWaveCopy(int phase);          // EN 0x74d5b0 (JP 0x10193d10): wave-shaped normals and lighting of a copy
    int Clone(Node *source, int a, int b); // EN 0x74e640 (JP 0x101a53b0): copies a loaded node tree into this node
    int CopyPlugData();                    // EN 0x749900 (JP 0x1019bd10)
    int SetMaterialValue(const char *material, int value);  // EN 0x74a400 (JP 0x1019e8c0)
    void ReleaseTexture();                 // EN 0x769e30 (JP 0x10191210)
    void ReleaseMesh();                    // EN 0x769eb0 (JP 0x101913f0)
    void ReleaseUniqueMeshData();          // EN 0x7507e0 (JP 0x10191280)
    int ReleaseTexturesExcept(Node **keep, int count);    // EN 0x750370 (JP 0x1019bca0)
    void FreePackData(VertexPack *packs, int count);      // EN 0x750970 (JP 0x101911b0): frees the packs' vertex data
    void FreeIndexListData(IndexList *lists, int count);  // EN 0x7509a0 (JP 0x101911e0): frees the index lists' data
    void FreeSphereListData(IndexList *lists, int count); // EN 0x7509d0 (JP 0x101911e0 twin)

    // Render modes and colours.
    int SetRenderModeExcept(int value, int except1, int except2);  // EN 0x76d3a0 (JP 0x1019b0c0)
    int SetColourA(unsigned char value, unsigned int mask);  // EN 0x76b030 (JP 0x101946b0)
    int SetColourB(unsigned char value, unsigned int mask);  // EN 0x76afb0 (JP 0x10194730)
    int SetColourC(char value, unsigned int mask);           // EN 0x76b180 (JP 0x10194630)
    int EnterTargetState(void *target);    // EN 0x76b410 (JP 0x101948f0)

    // On-demand blocks.
    int AllocSaved();                      // EN 0x76b2f0 (JP 0x101947b0)
    int AllocExtra();                      // EN 0x76b530 (JP 0x101949d0)
    int FreeSaved();                       // EN 0x750100 (JP 0x10194980, with ClearState4000 inlined)
    int RestoreState1000();                // EN 0x750270 (JP 0x101947f0)
    int RestoreState2000();                // EN 0x7501a0 (JP 0x10194870)
    int RestoreState400000();              // EN 0x750210 (JP 0x10194830)
    void RestoreSavedMatrix();             // inline helper of ReleaseSavedStates
    int ReleaseSavedStates();              // EN 0x76a5f0 (JP 0x10194510)
    int FreeExtraIndexLists();             // EN 0x750a00 (JP 0x101ab980)
    int BuildCollisionSpheres();           // EN 0x74fcf0 (JP 0x1019e120)
    void Release();                        // EN 0x7698e0 (JP 0x1019e9a0): frees the node's meshes and children
    Node();                                // EN 0x7495f0 (JP 0x1019ebc0): constructs the members, then Init
    ~Node();                               // EN 0x7496d0 (JP 0x10191480): destroys the matrix members
    void Init();                           // EN 0x76a290 (JP 0x10191500): puts every field back to its default

    // EN 0x7693b0 (JP 0x101a24e0): draws this node tree into the shadow texture slot `texture` as seen from the light
    // (see shadow_texture.cpp). `sceneView`, `a3`, `tileX` and `tileY` are not used. Returns 0 when the texture cannot
    // become the render target, else 1.
    int RenderShadowTexture(DrawList *list, const Matrix4 *sceneView, int a3, const Vec4 *casterPosition,
                            const Vec4 *lightPosition, int texture, float casterSize, int tileX, int tileY);

    // EN 0x751ee0 (JP 0x101995a0): draws the subtree under `list` with `view`; the arguments after `view` pass through to
    // the mesh (the shadow pass fills them with its projector matrix, the shadow's centre, texture, scale and size, the
    // light direction, the caster's size, model and opacity).
    int Draw(DrawList *list, Matrix4 *view, int a3, const Matrix4 *shadowMatrix, float shadowX, float shadowY,
             float shadowZ, int shadowTexture, float shadowScale, int shadowSize, const Vec4 *lightDirection,
             float modelSize, AnimModel *model, float alpha);
    // EN 0x752190 (JP 0x101a29e0): draws the subtree as the shadow of a caster at `casterPosition` lit from
    // `lightPosition` (see shadow_texture.cpp). Returns 1.
    int DrawShadow(DrawList *list, Matrix4 *view, int a3, const Vec4 *casterPosition, const Vec4 *lightPosition,
                   int texture, float scale, int size, float casterSize, AnimModel *model, float alpha);
    // EN 0x749b10 (JP 0x10199be0): marks the visibility cells of a map root node that are visible (not ported yet).
    int UpdateVisibleCells();

    // Whole-subtree setters and savers.
    void SaveFlags();                    // EN 0x7498c0 (JP 0x10191d50): flags into savedFlags
    void SaveRenderMode();                 // EN 0x749990 (JP 0x10191dd0): drawMode into savedDrawMode
    void RestoreFlags();                   // EN 0x76a870 (JP 0x10191d90): savedFlags back into flags
    void RestoreRenderMode();              // EN 0x76a940 (JP 0x10191e10): savedDrawMode back into drawMode
    int ClearFlags(unsigned int bits);     // EN 0x750320 (JP 0x1019b1b0)
    int SetFlags(unsigned int bits);       // EN 0x76b730 (JP 0x1019b130)
    int SetOwnFlags(unsigned int bits);    // EN 0x76b710: sets `bits` in this node only (no recursion)
    int ClearOwnFlags(unsigned int bits);  // EN 0x750300: clears `bits` in this node only (no recursion)
    int SetRenderMode(int mode);           // EN 0x76d330 (JP 0x1019b050)
    int SetModel(AnimModel *owner);        // EN 0x76d420 (JP 0x1019b230)
    int UnrecoveredCall(int a, int b);     // EN 0x75f180: large routine (1420 bytes) not recovered yet; called by AnimModel::RunRootCall
    int ClearHasCollision();             // EN 0x76a5b0 (JP 0x101944d0)

    // Animation.
    int ReplaceMaterial(int key, int count);  // EN 0x74a370 (JP 0x1019e830): material swap on the subtree
    int CycleChild();                      // EN 0x755650 (JP 0x10194ee0)
    int RestartColourPlugs(const char *motion, int frames);  // EN 0x76adb0
    int TagElements();                     // EN 0x76b200 (JP 0x10191e50)
    int UpdateWorldWithIk(float *parentWorld, Matrix4 *view, Matrix4 *billboard, int frame, int skinParent,
                          int forceChildren);                // EN 0x763e70 (JP 0x10194f40)
    int StartProximityPass(float *position, Matrix4 *matrix, Matrix4 *reference, int frame, int b);  // EN 0x768c70
};

static_assert(sizeof(Node) == NODE_SIZE, "Node must be 0x18a0 bytes");

#endif
