// Channel: the block behind one animation plug of a node (Node::plugs[k], allocated with its size in front).
#ifndef GURUMIN_NODE_CHANNEL_H
#define GURUMIN_NODE_CHANNEL_H

#include "motion_list.h"

struct Channel
{
    int size;                    // size of the whole channel block in bytes
    MotionList keys;             // the motions the plug plays (plugs 6 and 7 give the frame)
};

#endif
