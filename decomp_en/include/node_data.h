// The blocks a scene node (node.h) points to and allocates on demand: its animation tracks (NodeExtra) and its
// saved-state block (NodeSaved). Both are allocated raw through the engine allocator and set up by Init/Reset,
// never by a constructor. EN declaration (JP: decomp/include/node_data.h); only the size and the methods used so far.
// This is the one declaration of these types in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_NODE_DATA_H
#define GURUMIN_EN_NODE_DATA_H

#include "vec4.h"
#include "containers.h"

// Bytes of a NodeExtra block.
const int NODE_EXTRA_SIZE = 0x80;
// Bytes of a NodeSaved block.
const int NODE_SAVED_SIZE = 0xdc;

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

// A node's animation data (`Node::extra`): five key tracks and optional overrides for the first three.
struct NodeExtra
{
    KeyTrack tracks[NODE_TRACK_COUNT];  // 0x00: translation, rotation, scale, colour and extra tracks
    KeyTrack *translationOverride;      // 0x64: used instead of the translation track when set
    KeyTrack *rotationOverride;         // 0x68: used instead of the rotation track when set
    KeyTrack *scaleOverride;            // 0x6c: used instead of the scale track when set
    int m_0070[4];                      // 0x70: not used by EN code yet (cleared by Init)

    void Init();                   // EN 0x7419a0 (JP 0x10169660): empty tracks with their default keys
    void Clear();                  // EN 0x740dd0 (JP 0x10169620): frees the tracks' arrays and drops the overrides
};

// Values of NodeSaved::state.
enum NodeSavedState
{
    SAVED_EMPTY = 0,      // nothing saved
    SAVED_HOLDING = 1,    // holds a saved matrix
    SAVED_RETURNING = 2   // lets the tracked rotation return to rest, then ends
};

// A node's saved-state block (`Node::saved`): a saved matrix and the rotation limits used when the node tracks a target.
struct NodeSaved
{
    char m_0000[0x20];             // 0x00: not used by EN code yet
    int state;                     // 0x20: saved state (0 = nothing saved)
    Matrix4 matrix;                // 0x24: the saved matrix restored by the node
    char m_0064[0x78 - 0x64];      // 0x64: not used by EN code yet
    float limitMin[3];             // 0x78: lowest rotation per axis (radians)
    char m_0084[0x88 - 0x84];      // 0x84: not used by EN code yet
    float limitMax[3];             // 0x88: highest rotation per axis
    char m_0094[0xb8 - 0x94];      // 0x94: not used by EN code yet
    int m_00b8;                    // 0xb8: not recovered (cleared by Reset)
    char m_00bc[NODE_SAVED_SIZE - 0xbc];  // 0xbc: not used by EN code yet

    void Reset();                  // EN 0x741950 (JP 0x10166870 ViewLimits::Reset): rotation limits back to +-45 degrees
};

// Limits of the NodeSkin arrays.
const int SKIN_NAME_SIZE = 0x40;    // bone and link names
const int SKIN_MAX_LINKS = 0x40;    // nodes a skinned node depends on
const int SKIN_MAX_BONES = 0x44;    // bones by name
// Bytes of a NodeSkin block.
const int NODE_SKIN_SIZE = 0x561c;

// A skinned node's skinning data (Node::skin): bones and skin matrices.
struct Node;

struct NodeSkin
{
    Matrix4 skinMatrix;                        // 0x0000: the node's matrix for skinning (not used by EN code yet)
    Matrix4 chainMatrix;                       // 0x0040: the node's matrix relative to the skin root (not used by EN code yet)
    Node *bone;                                // 0x0080: the bone node, 0 when it is the node itself or missing
    char boneName[SKIN_NAME_SIZE];             // 0x0084: name of the node this one is bound to
    Matrix4 inverseBone;                       // 0x00c4: inverse of the bone's world matrix
    int isLink;                                // 0x0104: 1 when another skin links this node
    char linkNames[SKIN_MAX_LINKS][SKIN_NAME_SIZE];  // 0x0108: names of the linked nodes
    Node *links[SKIN_MAX_LINKS];               // 0x1108: nodes this skinned node depends on
    Node *bones[SKIN_MAX_BONES];               // 0x1208: the bones, looked up by name
    char boneNames[SKIN_MAX_BONES][SKIN_NAME_SIZE];  // 0x1318: names of the bones
    Matrix4 inverseBones[SKIN_MAX_BONES];      // 0x2418: each bone's inverseBone, given to the bone nodes
    Matrix4 bindPoses[SKIN_MAX_BONES];         // 0x3518: matrices when skinned ([0] = the node's own)
    Matrix4 skinMatrices[SKIN_MAX_LINKS];      // 0x4618: matrices uploaded for drawing
    int boneCount;                             // 0x5618: entries used in the bone arrays (0 = no skinning)

    void Clear();                              // EN 0x741370 (JP 0x10166810): empties the skin data
};

static_assert(sizeof(NodeSkin) == NODE_SKIN_SIZE, "NodeSkin must be 0x561c bytes");

#endif
