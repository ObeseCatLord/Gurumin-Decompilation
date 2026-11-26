// Updating the world matrices of a scene node tree (EN build). Ported from the JP source decomp/src/node_world.cpp
// (APP.dll 0x10194c60, unmatched) and the EN bytes. Matrix4 has the engine's constructor and empty destructor, so the
// matrix temporaries get an exception frame.
#include "node.h"
#include "node_data.h"
#include "motion_list.h"

// Highest stored pose a negative frame can select (poses[0] is the current pose).
const int NODE_LAST_POSE = NODE_POSE_COUNT - 1;

// Sets this node's world matrix to its matrix for `frame` times `parent`, then does the same for every child with the
// new world matrix. The frame comes from `frameSource` when there is one, or from plugs 6 and 7 when they give it. A
// negative frame takes one of the stored pose matrices, an animated node samples its extra data, and a node with frame
// matrices takes the one for the frame (clamped to the last). Returns 0 without changing anything for a node flagged
// NODE_FLAG_SKIP_SUBTREE, else 1. Called for the root of a model every frame and recursively for the children.
int Node::UpdateWorld(const Matrix4 &parent, int frame)
{
    Matrix4 unused;
    int shownFrame = frameSource != 0 ? *frameSource : frame;
    if (flags & NODE_FLAG_SKIP_SUBTREE)
        return 0;
    for (int i = PLUG_COLOUR_FIRST; i < PLUG_COLOUR_FIRST + PLUG_COLOUR_COUNT; i++)
    {
        if (plugType[i] != 0 && g_framePlugsDisabled == 0 && plugType[i] == PLUG_TYPE_FRAME)
            shownFrame = plugs[i]->keys.FrameAt(g_updatePassFrame, 0, 0);
    }
    if (shownFrame < 0)
    {
        int back = -1 - shownFrame;
        if (frameMatrices != 0)
        {
            if (back > NODE_LAST_POSE)
                back = NODE_LAST_POSE;
            worldMatrix = poses[back] * parent;
        }
        else
            worldMatrix = matrix * parent;
    }
    else if (!(flags & NODE_FLAG_ANIMATED) || extra == 0)
    {
        if (frameMatrices != 0)
        {
            int index = shownFrame;
            if (frameMatrixCount <= shownFrame)
                index = frameMatrixCount - 1;
            worldMatrix = frameMatrices[index] * parent;
        }
        else
            worldMatrix = matrix * parent;
    }
    else
        worldMatrix = extra->AnimatedMatrix(shownFrame) * parent;
    for (int i = 0; i < childCount; i++)
        children[i]->UpdateWorld(worldMatrix, shownFrame);
    return 1;
}
