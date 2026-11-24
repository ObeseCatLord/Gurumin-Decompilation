// Channel: the block behind one animation plug of a node (Node::plugs[k], allocated with its size in front).
#ifndef GURUMIN_NODE_CHANNEL_H
#define GURUMIN_NODE_CHANNEL_H

#include "motion_list.h"
#include "key_track.h"

struct Channel
{
    int size;                    // size of the whole channel block in bytes
    MotionList keys;             // the motions the plug plays (plugs 6 and 7 give the frame)
};

// The block behind a transform plug (type 0x674d3, slots 8 to 11): a key bank of positions and one of rotation
// quaternions. Node::UpdateWorldWithIk turns the sampled pair into a matrix.
struct TransformChannel
{
    char pad_0000[0x40];
    KeyBank position;            // 0x40: translation keys
    KeyBank rotation;            // 0xac4: rotation quaternion keys
};

#endif
