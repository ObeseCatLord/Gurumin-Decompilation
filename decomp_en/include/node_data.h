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

// A node's saved-state block (`Node::saved`): a saved matrix and the rotation limits used when the node tracks a target.
struct NodeSaved
{
    char m_0000[0x24];             // 0x00: not used by EN code yet
    Matrix4 matrix;                // 0x24: the saved matrix restored by the node
    char m_0064[NODE_SAVED_SIZE - 0x64];  // 0x64: not used by EN code yet

    void Reset();                  // EN 0x741950 (JP 0x10166870 ViewLimits::Reset): rotation limits back to +-45 degrees
};

// Offset of the bone count in a NodeSkin.
const int NODE_SKIN_BONE_COUNT_OFFSET = 0x5618;

// A skinned node's skinning data (Node::skin): bones and skin matrices.
struct NodeSkin
{
    char m_0000[NODE_SKIN_BONE_COUNT_OFFSET];  // 0x0000: links, bones and matrices (not used by EN code yet)
    int boneCount;                             // 0x5618: entries used in the bone arrays (0 = no skinning)
};

#endif
