// Texture manager, triangle batches (Selection = TriangleBatch in tri_batch.cpp), model owners and helpers.

struct TextureObject
{
    char pad_0000[0x34];
    void *texture;
    void M_10169df0();
    void M_10169e10();
    void M_10169e40();
};

struct TextureManager
{
    int M_10173480();
    int M_101734d0();
    int M_10173510();
    void M_1015c730();
    void M_10175e70();
};

struct ZeroVec
{
    float x;
    float y;
    float z;
    float w;
    ZeroVec();
};

struct Matrix4
{
    float m[16];
    Matrix4();
    ~Matrix4();  // 0x1013eda0 (empty)
};

struct Transform
{
    int m_0000;
    ZeroVec position;
    ZeroVec rotation;
    ZeroVec scale;
    Matrix4 matrix;
    Transform();
};

struct Selection
{
    int m_0000;
    int m_0004;
    int m_0008;
    int pad_000c;
    int m_0010;
    int current;
    int M_10172a80(int a, int b);
    int M_10172aa0();
    void M_101733f0();
    int M_10173450(int id, int unused);
    void M_10172ac0(int a, int count);
    int M_10188eb0(int id, int a, int count);
    void M_10172cb0(float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1, float v1, float x2, float y2, float u2, float v2, int colour, int a, int b);
    int AddQuad(int id, float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1, float v1, float x2, float y2, float u2, float v2, int colour, int a, int b);
};

struct Scene
{
    char pad_0000[0x1c];
    int m_001c;
    void M_10169f90();
    void M_1016a020(void *owner);
    void M_1016a2e0(int motion, int a, int b, float speed);
};

// Model owner (the CharModel of a Character, see world.h). `scene` is its motion playback object (0x7c84 bytes,
// Playback in model_state.cpp), created on first use.
struct SceneOwner
{
    char pad_0000[0x265];
    char important;   // 0x265: motion changes of this model are flagged as important (DAT_11b04508) while applied
    char pad_0266[0x32];
    Scene *scene;     // 0x298
    char pad_029c[7];
    unsigned char loadCount;  // 0x2a3: number of models loaded so far (stops counting at 3)
    char pad_02a4[0x2048];
    struct ModelNode *root;   // 0x22ec: root node of the loaded model
    char pad_22f0[0x38];
    float position[4];        // 0x2328: model position used for collision
    char pad_2338[0x30];
    float radius;             // 0x2368: collision radius
    char pad_236c[0x18];
    int m_2384;               // 0x2384: collision result, cleared before each update
    char pad_2388[0x10c];
    int m_2494;               // 0x2494
    int M_1017b1c0();
    void PlayMotion(int motion, int a, int b, float speed);
    int LoadModel(const char *path);
    int UpdateCollision();
};

struct ModelNode
{
    void M_10194510();
    int M_101a2370(float *position, Matrix4 *matrix, int a, float radius, int b);
    void Load(const char *path, int flags);            // 0x101abd60
    ModelNode *Find(const char *name, int index);       // 0x1019ece0
};

struct Animator
{
    void M_101687e0();
    int M_101692c0();
    int M_10184160();
};

struct PackHeader
{
    char data[0x74];
    void M_101acfc0();
};

extern "C" char DAT_10ffe5f8[1600][0x40];
extern "C" char DAT_10f8dbb8[1600][0x40];
extern "C" char DAT_10fa6db8[1600];
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" int FUN_10174c70(int slot, int force);

// Constructor: the member constructors do the work.
Transform::Transform()
{
}

// Sets the batch's vertex buffer and capacity.
int Selection::M_10172a80(int a, int b)
{
    m_0000 = a;
    m_0004 = b;
    return 1;
}

// Empties the batch and forgets its texture.
int Selection::M_10172aa0()
{
    m_0008 = 0;
    m_0010 = 0;
    current = -1;
    return 1;
}

// Switches the batch to texture `id`, flushing the queued triangles when the texture changes.
int Selection::M_10173450(int id, int unused)
{
    if (current != -1)
    {
        if (current != id)
            M_101733f0();
        current = id;
        return 0;
    }
    current = id;
    return 0;
}

// Unloads every texture slot and clears both name tables. Returns 1.
int TextureManager::M_10173480()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
        {
            DAT_118d0ba8[i].M_10169e40();
            DAT_10ffe5f8[i][0] = 0;
            DAT_10f8dbb8[i][0] = 0;
        }
    }
    return 1;
}

// Runs M_10169df0 on every loaded texture (re-create after a device reset).
int TextureManager::M_101734d0()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_118d0ba8[i].M_10169df0();
    }
    return 1;
}

// Runs M_10169e10 on every loaded texture (restore the texture data).
int TextureManager::M_10173510()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_118d0ba8[i].M_10169e10();
    }
    return 1;
}

// Unloads all textures and releases the IME state (M_1015c730).
void TextureManager::M_10175e70()
{
    M_10173480();
    M_1015c730();
}

// Returns the playback's current frame, creating the playback object on first use.
int SceneOwner::M_1017b1c0()
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    return scene->m_001c;
}

// Advances the animation one step and returns its state.
int Animator::M_10184160()
{
    M_101687e0();
    return M_101692c0();
}

// After a map change: unloads every texture that was not marked as kept and has no Direct3D texture.
extern "C" int FUN_10188fb0(void)
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0 && DAT_10fa6db8[i] == 0 && DAT_118d0ba8[i].texture == 0)
            FUN_10174c70(i, 1);
    }
    return 0;
}

// Frees the vertex data of `count` pack headers (VertexPack::FreeVertices on each).
extern "C" void __stdcall FUN_101911b0(PackHeader *packs, int count)
{
    if (packs != 0)
    {
        for (; count > 0; count--)
        {
            packs->M_101acfc0();
            packs++;
        }
    }
}

extern "C" int DAT_10ff1dec;

// Queues `count` vertices for texture `id`, flushing first when the texture changes or the buffer is full.
int Selection::M_10188eb0(int id, int a, int count)
{
    DAT_10ff1dec++;
    if (m_0010 != 0 && m_0010 != id)
        M_101733f0();
    m_0010 = id;
    if (m_0008 + count >= m_0004)
        M_101733f0();
    M_10172ac0(a, count);
    return 0;
}
// Queues a textured quad (two triangles) given three corners with UVs and a colour.
int Selection::AddQuad(int id, float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1, float v1, float x2, float y2, float u2, float v2, int colour, int a, int b)
{
    DAT_10ff1dec++;
    if (m_0010 != 0 && m_0010 != id)
        M_101733f0();
    m_0010 = id;
    if (m_0008 + 6 >= m_0004)
        M_101733f0();
    M_10172cb0(x0, y0, z0, u0, v0, x1, y1, z1, u1, v1, x2, y2, u2, v2, colour, a, b);
    return 0;
}
extern "C" char DAT_10fdeb90[1600];  // reference count per texture slot

// Releases one reference to texture slot `slot`. The texture is unloaded (and its name cleared, freeing the slot)
// when the count reaches zero or when `force` is set. Returns the remaining count, or -1 for an unused slot.
extern "C" int FUN_10174c70(int slot, int force)
{
    if (slot == -1 || DAT_10ffe5f8[slot][0] == 0)
        return -1;
    char refs = --DAT_10fdeb90[slot];
    if (force != 0 || refs == 0)
    {
        DAT_10fdeb90[slot] = 0;
        DAT_118d0ba8[slot].M_10169e40();
        DAT_10ffe5f8[slot][0] = 0;
    }
    return DAT_10fdeb90[slot];
}

extern "C" char DAT_11b04508;  // set while an important model changes motion (read by the playback code)

// Starts motion `motion` on this model (0x1017b0e0, CharModel::M_1017b0e0 in other files): creates the playback
// object on first use, then hands the request to it. `a`/`b` are the playback options (loop mode, blend frames).
void SceneOwner::PlayMotion(int motion, int a, int b, float speed)
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    if (important != 0)
        DAT_11b04508 = 1;
    scene->M_1016a2e0(motion, a, b, speed);
    DAT_11b04508 = 0;
}

// Loads model file `path` into this model's existing node tree (0x1017aeb0, CharModel::M_1017aeb0 elsewhere).
// Counts the loads, looks up the biped root "Bip01" (the result is unused in the shipped code), resets m_2494 and
// rebinds the motion playback to the new model. Returns 0 when the model has no node tree yet.
int SceneOwner::LoadModel(const char *path)
{
    if (root == 0)
        return 0;
    root->Load(path, 0);
    if (loadCount < 3)
        loadCount++;
    root->Find("Bip01", 0);
    ZeroVec unused1;
    ZeroVec unused2;
    m_2494 = 0;
    if (scene != 0)
        scene->M_1016a020(this);
    return 1;
}

// Updates this model's collision: refreshes the node tree (M_10194510) and collides the model's position and radius
// with an identity-like work matrix (Node::M_101a2370, scene_node.cpp).
// Returns 0.
int SceneOwner::UpdateCollision()
{
    Matrix4 matrix;
    if (root != 0)
        root->M_10194510();
    m_2384 = 0;
    root->M_101a2370(position, &matrix, 0, radius, 0);
    return 0;
}
