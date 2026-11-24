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
struct AnimModel;

// Node::flags bits whose meaning is known.
enum NodeFlags
{
    NODE_FLAG_CHASING = 0x1000,            // a chasing node (its simulated point follows the tip)
    NODE_FLAG_TWO_BONE_LIMB = 0x2000,      // a two-bone limb (saved matrix restored by RestoreState2000)
    NODE_FLAG_COLLISION_SPHERE = 0x4000,   // a collision sphere node
    NODE_FLAG_SKIP_SUBTREE = 0x8000,       // the subtree is not updated or drawn (also the "special pass" marker)
    NODE_FLAG_TARGET_TRACKING = 0x400000   // tracks a target (saved matrix restored by RestoreState400000)
};

// Limits and sizes of the fixed-size parts of a node.
enum NodeLimits
{
    NODE_NAME_SIZE = 0x40,      // characters in a node name, including the terminator
    NODE_MAX_CHILDREN = 0x40,   // children (and child names) a node can hold
    NODE_PLUG_COUNT = 16,       // animation plugs per node
    NODE_SIZE = 0x18a0          // sizeof(Node)
};

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

// Motion frame of the current update pass, set by StartProximityPass. EN 0x217faf0 (JP DAT_11b044f8).
extern "C" int g_updatePassFrame;

struct Node
{
    char name[NODE_NAME_SIZE];     // 0x000: the node's name ("fire..." / "thun..." set the element)
    char m_0040[0x64 - 0x40];      // 0x040: not used by EN code yet
    int packCount;                 // 0x064: vertex packs of the mesh (also the entries of triangleSpheres)
    char m_0068[0x70 - 0x68];      // 0x068: not used by EN code yet
    IndexList *triangleSpheres;    // 0x070: one sphere list per pack (0 = none)
    char m_0074[0xa0 - 0x74];      // 0x074: not used by EN code yet
    int drawMode;                  // 0x0a0: render mode of the node
    char m_00a4[0xa8 - 0xa4];      // 0x0a4: not used by EN code yet
    int hasCollision;              // 0x0a8: non-zero when the node has collision
    unsigned int flags;            // 0x0ac: NodeFlags
    char m_00b0[0xb8 - 0xb0];      // 0x0b0: not used by EN code yet
    AnimModel *model;              // 0x0b8: the model the tree belongs to
    char m_00bc[0xc0 - 0xbc];      // 0x0bc: not used by EN code yet
    int plugType[NODE_PLUG_COUNT];     // 0x0c0: type of each animation plug (PLUG_TYPE_FRAME gives the frame)
    Channel *plugs[NODE_PLUG_COUNT];   // 0x100: the channel behind each plug
    char m_0140[0x1c8 - 0x140];    // 0x140: not used by EN code yet
    int frameCount;                // 0x1c8: frames in the mesh
    Matrix4 matrix;                // 0x1cc: the node's own matrix (relative to its parent)
    char m_020c[0x294 - 0x20c];    // 0x20c: not used by EN code yet
    int frameMatrixCount;          // 0x294: entries of the per-frame matrices
    char m_0298[0x2a8 - 0x298];    // 0x298: not used by EN code yet
    NodeExtra *extra;              // 0x2a8: the node's animation tracks (allocated by AllocExtra)
    char m_02ac[0x708 - 0x2ac];    // 0x2ac: not used by EN code yet
    NodeSkin *skin;                // 0x708: skinning data (0 = none)
    char m_070c[0x74c - 0x70c];    // 0x70c: not used by EN code yet
    Node *parent;                  // 0x74c: the parent node (0 = root)
    int childCount;                // 0x750: entries of children
    char m_0754[0x1754 - 0x754];   // 0x754: the child names (not used by EN code yet)
    Node *children[NODE_MAX_CHILDREN];  // 0x1754: the child nodes
    NodeSaved *saved;              // 0x1854: saved-state block (allocated by AllocSaved)
    unsigned char cycleIndex;      // 0x1858: cycling child index (1..childCount), advanced by CycleChild
    int savedDrawMode;             // 0x185c: render mode kept by SaveRenderMode
    int savedFlags;                // 0x1860: flags kept by SaveFlags
    char m_1864[0x1898 - 0x1864];  // 0x1864: not used by EN code yet
    unsigned char element;         // 0x1898: NodeElement bits from the node name
    char m_1899[NODE_SIZE - 0x1899];   // 0x1899: not used by EN code yet

    // Name lookups in the subtree (scene_node.cpp). `skip` counts down the matches still to pass over.
    Node *Find(const char *name, int index);                   // EN 0x7532c0 (JP 0x1019ece0)
    Node *FindRecursive(const char *name, int *skip);          // EN 0x7531d0 (JP 0x10191a30)
    Node *FindPrefixRecursive(const char *prefix, int *skip);  // EN 0x753240 (JP 0x10191c30)
    // Same lookups with the second pair of finders, which ignore the name prefixes of special nodes.
    Node *Find2(const char *name, int index);                          // EN 0x753740 (JP 0x1019ed10)
    Node *FindRecursiveStripped(const char *name, int *skip);          // EN 0x753590 (JP 0x10191aa0)
    Node *FindPrefixRecursiveStripped(const char *prefix, int *skip);  // EN 0x753660 (JP 0x10191b60)

    // Special passes (scene_node.cpp).
    int MarkSpecialPass();                 // EN 0x75a900 (JP 0x1019d600)
    int ClearSpecialPassOnLinks();         // EN 0x75a970 (JP 0x1019d6a0)
    int RunSpecialPass();                  // EN 0x75aa50 (JP 0x101a8760)
    int ClearSpecialPassUp();              // EN 0x75aa30 (JP 0x1019d680)

    // Tree navigation.
    Node *Root();                          // EN 0x7541b0 (JP 0x101992f0)
    Node *GetLinkedRoot();                 // EN 0x754180 (JP 0x10199310)
    int GetLastFrame();                    // EN 0x754120 (JP 0x10199340)

    // On-demand blocks.
    int AllocSaved();                      // EN 0x76b2f0 (JP 0x101947b0)
    int AllocExtra();                      // EN 0x76b530 (JP 0x101949d0)
    int FreeSaved();                       // EN 0x750100 (JP 0x10194980, with ClearState4000 inlined)
    int RestoreState1000();                // EN 0x750270 (JP 0x101947f0)
    int RestoreState2000();                // EN 0x7501a0 (JP 0x10194870)
    int RestoreState400000();              // EN 0x750210 (JP 0x10194830)
    int ReleaseSavedStates();              // EN 0x76a5f0 (JP 0x10194510)
    int FreeExtraIndexLists();             // EN 0x750a00 (JP 0x101ab980)
    int BuildCollisionSpheres();           // EN 0x74fcf0 (JP 0x1019e120)
    void Release();                        // EN 0x7698e0 (JP 0x1019e9a0): frees the node's meshes and children

    // Whole-subtree setters and savers.
    void SaveFlags();                      // EN 0x7498c0 (JP 0x10191d50): flags into savedFlags
    void SaveRenderMode();                 // EN 0x749990 (JP 0x10191dd0): drawMode into savedDrawMode
    void RestoreFlags();                   // EN 0x76a870 (JP 0x10191d90): savedFlags back into flags
    void RestoreRenderMode();              // EN 0x76a940 (JP 0x10191e10): savedDrawMode back into drawMode
    int ClearFlags(unsigned int bits);     // EN 0x750320 (JP 0x1019b1b0)
    int SetFlags(unsigned int bits);       // EN 0x76b730 (JP 0x1019b130)
    int SetRenderMode(int mode);           // EN 0x76d330 (JP 0x1019b050)
    int SetModel(AnimModel *owner);        // EN 0x76d420 (JP 0x1019b230)
    int ClearHasCollision();               // EN 0x76a5b0 (JP 0x101944d0)

    // Animation.
    int CycleChild();                      // EN 0x755650 (JP 0x10194ee0)
    int RestartColourPlugs(const char *motion, int frames);  // EN 0x76adb0
    int TagElements();                     // EN 0x76b200 (JP 0x10191e50)
    int UpdateWorldWithIk(float *parentWorld, Matrix4 *view, Matrix4 *billboard, int frame, int skinParent,
                          int forceChildren);                // EN 0x763e70 (JP 0x10194f40)
    int StartProximityPass(float *position, Matrix4 *matrix, Matrix4 *reference, int frame, int b);  // EN 0x768c70
};

#endif
