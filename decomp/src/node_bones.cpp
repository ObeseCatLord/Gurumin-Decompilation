// Bone lookups in a model's node tree. Matrix4 here has the engine's constructor and (empty) destructor, so
// functions that hold matrices by value get an exception frame and need C++ linkage.

struct Matrix4
{
    float m[16];
    Matrix4();    // 0x101ae770
    ~Matrix4();   // 0x1013eda0 (empty)
};

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

struct Node
{
    char pad_0000[0xac];
    unsigned int flags;       // 0xac: 0x100000 = animated through its extra data
    char pad_00b0[0x11c];
    Matrix4 localMatrix;      // 0x1cc
    char pad_020c[0x84];
    Matrix4 *matrixOverride;  // 0x290: used instead of localMatrix when m_0294 is set
    int m_0294;
    char pad_0298[0x10];
    NodeExtra *extra;         // 0x2a8
    char pad_02ac[0x458];
    char skinned;             // 0x704: 1 = the node has skinning data
    char pad_0705[3];
    NodeSkin *skin;           // 0x708
    char pad_070c[0x44];
    int childCount;
    char pad_0754[0x1000];
    Node *children[0x40];     // 0x1754
    // 0x10199380: searches this subtree for bone `id`, accumulating parent matrices; returns its matrix or 0.
    Matrix4 *FindBone(int id, Matrix4 *parent, Matrix4 *work, int flags);
    Matrix4 BoneMatrix(int id, int flags);
    Node *Find(const char *name, int index);   // 0x1019ece0
    int SetupSkins(Node *root);
};

Matrix4 InverseMatrix(const Matrix4 &matrix);   // 0x101b0d40

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
        else if (m_0294 != 0)
            skin->bindPose = *matrixOverride;
        else
            skin->bindPose = localMatrix;
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
