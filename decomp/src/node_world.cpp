// Updating the world matrices of a scene node tree: each node's matrix for the current frame times its parent's
// world matrix. Matrix4 has the engine's constructor and empty destructor, so the matrix temporaries get an
// exception frame.

struct Matrix4
{
    float m[16];
    Matrix4();                                   // 0x101ae770
    ~Matrix4();                                  // 0x1013eda0 (empty)
    Matrix4 operator*(const Matrix4 &other);     // 0x101b0670: matrix product
};

// A node's animation data (node_extra.cpp NodeExtra).
struct NodeExtra
{
    Matrix4 AnimatedMatrix(int frame);           // 0x101693f0
};

struct Keys
{
    int M_101ad350(float value, int a, int b);   // 0x101ad350 (containers.cpp MotionList::FrameAt)
};

// A plug of a node (scene_node.cpp Channel).
struct Channel
{
    int pad_0000;
    Keys keys;
};

// A scene node (scene_node.cpp Node), only the fields used here.
struct Node
{
    char pad_0000[0xac];
    unsigned int flags;          // 0xac: 0x8000 = not updated, 0x100000 = animated by its extra data
    char pad_00b0[0xc];
    int *frameSource;            // 0xbc: when set, the frame shown is read from here
    int plugType[16];            // 0xc0: 0x4ee33 = the plug gives the frame
    Channel *plugs[16];          // 0x100
    char pad_0140[0x8c];
    Matrix4 matrix;              // 0x1cc: the node's own matrix
    char pad_020c[0x40];
    Matrix4 world;               // 0x24c: the node's matrix in world space
    char pad_028c[4];
    Matrix4 *frameMatrices;      // 0x290: one matrix per frame (0 = use `matrix`)
    int frameMatrixCount;        // 0x294
    char pad_0298[0x10];
    NodeExtra *extra;            // 0x2a8
    char pad_02ac[0x28];
    Matrix4 backMatrices[16];    // 0x2d4: matrices for the negative frames -1 to -16
    char pad_06d4[0x7c];
    int childCount;              // 0x750
    char childNames[0x40][0x40];
    Node *children[0x40];        // 0x1754
    int UpdateWorld(const Matrix4 &parent, int frame);
};

extern "C" int DAT_11b044ec;     // non-zero = the plugs do not give the frame
extern "C" float DAT_11b044f8;   // the value the frame-giving plugs are sampled at

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
            world = backMatrices[back] * parent;
        }
        else
            world = matrix * parent;
    }
    else if (!(flags & 0x100000) || extra == 0)
    {
        if (frameMatrices != 0)
        {
            int index = frame;
            if (frameMatrixCount <= frame)
                index = frameMatrixCount - 1;
            world = frameMatrices[index] * parent;
        }
        else
            world = matrix * parent;
    }
    else
        world = extra->AnimatedMatrix(frame) * parent;
    for (int i = 0; i < childCount; i++)
        children[i]->UpdateWorld(world, frame);
    return 1;
}
