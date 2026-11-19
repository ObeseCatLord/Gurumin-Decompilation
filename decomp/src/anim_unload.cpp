// Unloading an animated model (AnimModel, see model_params.cpp for the fields used while it moves). Matrix4 here
// has the engine's constructor and empty destructor, like in node_bones.cpp, because IdentityMatrix() returns a
// temporary.

struct Matrix4
{
    float m[16];
    Matrix4();    // 0x101ae770
    ~Matrix4();   // 0x1013eda0 (empty)
};

Matrix4 IdentityMatrix();  // 0x101b0ba0

struct Node
{
    void Release();  // 0x1019e9a0: frees the node's meshes and children
};

struct AnimModel
{
    char pad_0000[0x298];
    void *m_0298;                // 0x298: buffer owned by the model (freed on unload)
    char pad_029c[0x2004];
    int m_22a0;
    char pad_22a4[8];
    char m_22ac;
    char pad_22ad[0x3f];
    Node *root;                  // 0x22ec: the model's node tree (its entry in the model table)
    int id;                      // 0x22f0
    int m_22f4;
    int m_22f8;
    int m_22fc;
    char pad_2300[8];
    void *collider;              // 0x2308: the map collision this model moves against
    char pad_230c[0x1c];
    Matrix4 world;               // 0x2328: world matrix
    int m_2368;
    float motionSpeed;           // 0x236c
    char pad_2370[0xa8];
    int m_2418;
    char pad_241c[0x60];
    int m_247c;
    int Unload();
    int M_10161f80();            // 0x10161f80 (anim_reset.cpp AnimModel::ResetState): resets the motion state
};

extern "C" Node *DAT_10faaa88[0x100];  // the model table
extern "C" int DAT_10ff19e8[0x100];    // collision data of each model table entry
extern "C" void (*DAT_11b1ba40)(void *block);  // the engine's free

// 0x10162c50. Unloads the model: removes it from the model table, frees its node tree and its buffer, and resets
// the position (identity world matrix), speed, collision and motion state. Returns 1.
int AnimModel::Unload()
{
    m_22a0 = 0;
    m_22ac = 0;
    for (int i = 0; i < 0x100; i++)
    {
        if (DAT_10faaa88[i] == root)
        {
            DAT_10faaa88[i] = 0;
            DAT_10ff19e8[i] = 0;
        }
    }
    if (root != 0)
    {
        root->Release();
        DAT_11b1ba40(root);
    }
    if (m_0298 != 0)
    {
        DAT_11b1ba40(m_0298);
        m_0298 = 0;
    }
    root = 0;
    id = 0;
    m_247c = 0;
    m_2368 = 0;
    motionSpeed = 0;
    collider = 0;
    world = IdentityMatrix();
    m_2418 = 0;
    m_22f4 = 0;
    m_22f8 = 0;
    m_22fc = 0;
    M_10161f80();
    return 1;
}
