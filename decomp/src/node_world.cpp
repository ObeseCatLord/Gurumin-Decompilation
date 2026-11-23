// Updating the world matrices of a scene node tree: each node's matrix for the current frame times its parent's
// world matrix. Matrix4 has the engine's constructor and empty destructor, so the matrix temporaries get an
// exception frame.

#include "matrix4.h"

#include "node_data.h"

struct Keys
{
    int M_101ad350(int value, int a, int b);   // 0x101ad350 (containers.cpp MotionList::FrameAt)
};

// A plug of a node (scene_node.cpp Channel).
struct Channel
{
    int pad_0000;
    Keys keys;
};

#include "node.h"

#include "render_vars.h"

// 0x10194c60 (Node::UpdateWorld). Sets this node's world matrix to its matrix for `frame` times `parent`, then does
// the same for every child with the new world matrix. The frame comes from `frameSource` when there is one, or from
// plugs 6 and 7 when they give it. A negative frame takes one of the back matrices, an animated node samples its
// extra data, and a node with frame matrices takes the one for the frame (clamped to the last). Returns 0 without
// changing anything for a node flagged 0x8000, else 1.
int Node::UpdateWorld(const Matrix4 &parent, int frame)
{
    Matrix4 unused;
    if (frameSource != 0)
        frame = *frameSource;
    if (flags & 0x8000)
        return 0;
    for (int i = 6; i < 8; i++)
    {
        if (plugType[i] != 0 && DAT_11b044ec == 0 && plugType[i] == 0x4ee33)
            frame = plugs[i]->keys.M_101ad350(DAT_11b044f8, 0, 0);
    }
    if (frame < 0)
    {
        int back = -1 - frame;
        if (frameMatrices != 0)
        {
            if (back > 15)
                back = 15;
            worldMatrix = poses[back] * parent;
        }
        else
            worldMatrix = matrix * parent;
    }
    else if (!(flags & 0x100000) || extra == 0)
    {
        if (frameMatrices != 0)
        {
            int index = frame;
            if (frameMatrixCount <= frame)
                index = frameMatrixCount - 1;
            worldMatrix = frameMatrices[index] * parent;
        }
        else
            worldMatrix = matrix * parent;
    }
    else
        worldMatrix = extra->AnimatedMatrix(frame) * parent;
    for (int i = 0; i < childCount; i++)
        children[i]->UpdateWorld(worldMatrix, frame);
    return 1;
}
