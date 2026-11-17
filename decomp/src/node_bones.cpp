// Bone lookups in a model's node tree. Matrix4 here has the engine's constructor and (empty) destructor, so
// functions that hold matrices by value get an exception frame and need C++ linkage.

struct Matrix4
{
    float m[16];
    Matrix4();    // 0x101ae770
    ~Matrix4();   // 0x1013eda0 (empty)
};

Matrix4 IdentityMatrix();  // 0x101b0ba0

struct Node
{
    char pad_0000[0x750];
    int childCount;
    char pad_0754[0x1000];
    Node *children[0x40];     // 0x1754
    // 0x10199380: searches this subtree for bone `id`, accumulating parent matrices; returns its matrix or 0.
    Matrix4 *FindBone(int id, Matrix4 *parent, Matrix4 *work, int flags);
    Matrix4 BoneMatrix(int id, int flags);
};

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
