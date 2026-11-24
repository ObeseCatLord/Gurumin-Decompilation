// The blocks a scene node (node.h) points to and allocates on demand: its animation tracks (NodeExtra), its skinning
// data (NodeSkin) and its saved-state block (NodeSaved). All three are allocated raw through the engine allocator
// and set up by their Init/Clear methods, never by a constructor.
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_NODE_DATA_H
#define GURUMIN_NODE_DATA_H

#include "matrix4.h"
#include "vec3.h"
#include "key_track.h"

struct Node;

// Indexes into NodeExtra::tracks.
enum NodeTrack
{
    NODE_TRACK_TRANSLATION = 0,   // position keys (default 0, 0, 0)
    NODE_TRACK_ROTATION = 1,      // rotation quaternions (default 0, 0, 0, 1)
    NODE_TRACK_SCALE = 2,         // scale keys (default 1, 1, 1)
    NODE_TRACK_COLOUR = 3,        // default (1, 1, 1, 1)
    NODE_TRACK_EXTRA = 4,         // default (0, 0, 0, 0)
    NODE_TRACK_COUNT = 5
};

// A node's animation data (0x80 bytes, `Node::extra`): five key tracks and optional overrides for the first three.
// node_extra.cpp samples it (AnimatedMatrix), model_misc.cpp sets it up (Init), camera paths copy its translation.
struct NodeExtra
{
    KeyTrack tracks[NODE_TRACK_COUNT];  // 0x00: indexed by NodeTrack
    KeyTrack *translationOverride;      // 0x64: used instead of the translation track when set
    KeyTrack *rotationOverride;         // 0x68
    KeyTrack *scaleOverride;            // 0x6c
    int m_0070[4];                      // 0x70: copied with the tracks, cleared by Init

    Matrix4 AnimatedMatrix(int frame);  // 0x101693f0 (node_extra.cpp): scale, rotation, translation at `frame`
    void Clear();                       // 0x10169620 (SlotLists::Clear): frees the tracks' lists
    void Init();                        // 0x10169660 (model_misc.cpp): empty tracks with their default keys
};

// Limits of the skin arrays.
#define SKIN_NAME_SIZE 0x40     // bone and link names
#define SKIN_MAX_LINKS 0x40
#define SKIN_MAX_BONES 0x44

// Skinning data of a node (0x561c bytes, `Node::skin`): the bone it hangs on, the nodes linked to it, its bones by
// name and the matrices used to skin it (scene_node.cpp binds them, node_bones.cpp sets up the bind poses,
// draw_list.cpp and node_draw.cpp upload the skin matrices).
struct NodeSkin
{
    Matrix4 skinMatrix;                                // 0x00: the node's matrix for skinning (the pass sets it from chainMatrix)
    Matrix4 chainMatrix;                               // 0x40: the node's matrix relative to the skin root, carried down the skin pass
    Node *bone;                                        // 0x80: the bone node, 0 when it is the node itself or missing
    char boneName[SKIN_NAME_SIZE];                     // 0x84: name of the node this one is bound to
    Matrix4 inverseBone;                               // 0xc4: inverse of the bone's world matrix
    int isLink;                                        // 0x104: 1 when another skin links this node
    char linkNames[SKIN_MAX_LINKS][SKIN_NAME_SIZE];    // 0x108: names of the linked nodes
    Node *links[SKIN_MAX_LINKS];                       // 0x1108: nodes this skinned node depends on
    Node *bones[SKIN_MAX_BONES];                       // 0x1208: the bones, looked up by name
    char boneNames[SKIN_MAX_BONES][SKIN_NAME_SIZE];    // 0x1318: names of the bones
    Matrix4 inverseBones[SKIN_MAX_BONES];              // 0x2418: each bone's inverseBone, given to the bone nodes
    Matrix4 bindPoses[SKIN_MAX_BONES];                 // 0x3518: matrices when skinned ([0] = the node's own)
    Matrix4 skinMatrices[SKIN_MAX_LINKS];              // 0x4618: matrices uploaded for drawing
    int boneCount;                                     // 0x5618: entries used in links, bones and skinMatrices

    void Clear();                                      // 0x10166810: empties the skin data
};

// Values of NodeSaved::state.
#define SAVED_EMPTY 0       // nothing saved
#define SAVED_HOLDING 1     // holds a saved matrix
#define SAVED_RETURNING 2   // let the tracked rotation return to rest, then end (head_look.cpp, world_model_state.cpp)

// A node's saved-state block (0xdc bytes, `Node::saved`): a saved matrix restored by the node, and rotation limits used
// when the node tracks a target (head_look.cpp, world_model_state.cpp).
struct NodeSaved
{
    Vec4 vec0;                // 0x00: IK: the goal point (hip node) or the node's world position (collision node); tracking: ramp counters in y
    Vec4 vec10;               // 0x10: IK node: the simulated point chasing the tip; collision node: the previous world position
    int state;                // 0x20: SAVED_*
    Matrix4 matrix;           // 0x24: the saved matrix
    char pad_0064[4];
    Vec4 angle;               // 0x68: tracking: current angles (x/y/z) and ramp counter in z; IK: bone lengths (x, y); collision node: velocity
    Vec3 limitMin;            // 0x78: lowest rotation per axis (radians)
    int pad_0084;
    Vec3 limitMax;            // 0x88: highest rotation per axis
    int pad_0094;
    Node *linkedNodes[8];     // 0x98: nodes of the same group linked to this one (Node::M_10191990)
    int m_00b8;
    int m_00bc;               // 0xbc: 10000 after the first update of an IK node
    int m_00c0;               // 0xc0: 10000 likewise
    char pad_00c4[0x18];

    void M_10166870();
};

#endif
