// Bone lookups in a model's node tree. Matrix4 here has the engine's constructor and (empty) destructor, so
// functions that hold matrices by value get an exception frame and need C++ linkage.

#include "matrix4.h"

Matrix4 IdentityMatrix();  // 0x101b0ba0

// Extra data of a node (scene_node.cpp Extra).
struct NodeExtra
{
    Matrix4 AnimatedMatrix(int frame);   // 0x101693f0 (node_extra.cpp)
};

struct Node;

// Skinning data of a node: the bone it hangs on (looked up by name) and the matrices used to skin it.
struct NodeSkin
{
    char pad_0000[0x80];
    Node *bone;               // 0x80: the bone node, 0 when it is the node itself or missing
    char boneName[0x40];      // 0x84
    Matrix4 inverseBone;      // 0xc4: inverse of the bone's world matrix
    char pad_0104[0x3414];
    Matrix4 bindPose;         // 0x3518: the node's matrix when skinned
};

#include "node.h"

Matrix4 InverseMatrix(const Matrix4 &matrix);   // 0x101b0d40

// 0x10199380 (Node::FindBone). Searches this subtree for the node `id` (a node address), accumulating world matrices:
// this node's matrix for `frame` (a negative frame -1-n picks override matrix n, at most 15; animated nodes sample
// their extra data; otherwise the override matrix of the frame, clamped to the last one, or the local matrix) times
// `parent`. The bone's matrix is stored in `work` and returned; 0 when the subtree does not hold it.
Matrix4 *Node::FindBone(int id, Matrix4 *parent, Matrix4 *work, int frame)
{
    Matrix4 matrix;
    if (frame < 0)
    {
        int index = -1 - frame;
        if (index > 15)
            index = 15;
        matrix = frameMatrices[index] * *parent;
    }
    else if ((flags & 0x100000) && extra != 0)
        matrix = extra->AnimatedMatrix(frame) * *parent;
    else if (frameMatrices != 0)
    {
        int index = frame;
        if (frameMatrixCount <= frame)
            index = frameMatrixCount - 1;
        matrix = frameMatrices[index] * *parent;
    }
    else
        matrix = matrix * *parent;
    if (id == (int)this)
    {
        *work = matrix;
        return work;
    }
    for (int i = 0; i < childCount; i++)
    {
        Matrix4 *found = children[i]->FindBone(id, &matrix, work, frame);
        if (found != 0)
            return found;
    }
    return 0;
}

// Returns the world matrix of bone `id` in this model's tree, starting from the identity; when no child has the
// bone, the identity itself.
Matrix4 Node::BoneMatrix(int id, int flags)
{
    Matrix4 parent;
    parent = IdentityMatrix();
    Matrix4 work;
    for (int i = 0; i < childCount; i++)
    {
        Matrix4 *found = children[i]->FindBone(id, &parent, &work, flags);
        if (found != 0)
            return *found;
    }
    return parent;
}

// 0x101ab9d0 (Node::SetupSkins). For every skinned node of this subtree: stores its bind pose (its animated matrix
// when it is animated through its extra data, else its local matrix or the override), looks up its bone by name in
// `root` (not itself) and stores the inverse of the bone's world matrix. Returns 1.
int Node::SetupSkins(Node *root)
{
    if (skinned != 0)
    {
        if ((flags & 0x100000) && extra != 0)
            skin->bindPose = extra->AnimatedMatrix(0);
        else if (frameMatrixCount != 0)
            skin->bindPose = *frameMatrices;
        else
            skin->bindPose = matrix;
        skin->bone = root->Find(skin->boneName, 0);
        if (skin->bone == this)
            skin->bone = 0;
        if (skin->bone != 0)
            skin->inverseBone = InverseMatrix(skin->bone->BoneMatrix((int)this, 0));
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetupSkins(root);
    return 1;
}
