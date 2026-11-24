// The blocks a scene node (node.h) points to and allocates on demand: its animation tracks (NodeExtra) and its
// saved-state block (NodeSaved). Both are allocated raw through the engine allocator and set up by Init/Reset,
// never by a constructor. EN declaration (JP: decomp/include/node_data.h); only the size and the methods used so far.
// This is the one declaration of these types in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_NODE_DATA_H
#define GURUMIN_EN_NODE_DATA_H

#include "vec4.h"

// Bytes of a NodeExtra block.
const int NODE_EXTRA_SIZE = 0x80;
// Bytes of a NodeSaved block.
const int NODE_SAVED_SIZE = 0xdc;

// A node's animation data (`Node::extra`): five key tracks and optional overrides.
struct NodeExtra
{
    char m_0000[NODE_EXTRA_SIZE];  // 0x00: the tracks (layout not needed by EN code yet)

    void Init();                   // EN 0x7419a0 (JP 0x10169660): empty tracks with their default keys
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
// Offsets of the NodeSkin parts used so far.
const int NODE_SKIN_LINKS_OFFSET = 0x1108;
const int NODE_SKIN_BONE_COUNT_OFFSET = 0x5618;

// A skinned node's skinning data (Node::skin): bones and skin matrices.
struct Node;

struct NodeSkin
{
    char m_0000[0x84];                         // 0x0000: matrices and the bone node (not used by EN code yet)
    char boneName[SKIN_NAME_SIZE];             // 0x0084: name of the node this one is bound to
    char m_00c4[0x104 - 0xc4];                 // 0x00c4: inverse bone matrix (not used by EN code yet)
    int isLink;                                // 0x0104: 1 when another skin links this node
    char linkNames[SKIN_MAX_LINKS][SKIN_NAME_SIZE];  // 0x0108: names of the linked nodes
    Node *links[SKIN_MAX_LINKS];               // 0x1108: nodes this skinned node depends on
    char m_1208[0x1318 - 0x1208];              // 0x1208: the bones (not used by EN code yet)
    char boneNames[SKIN_MAX_BONES][SKIN_NAME_SIZE];  // 0x1318: names of the bones
    char m_2418[NODE_SKIN_BONE_COUNT_OFFSET - 0x2418];  // 0x2418: bone matrices (not used by EN code yet)
    int boneCount;                             // 0x5618: entries used in the bone arrays (0 = no skinning)
};

#endif
