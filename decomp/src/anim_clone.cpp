// Giving an animated model (AnimModel) a node tree, copied from another model (CloneModel) or loaded from a file
// (LoadFile): the motion state is reset, the old tree is released, a fresh node gets the new tree and its
// well-known bones are looked up. Matrix4 here has the engine's constructor and empty destructor because IdentityMatrix() returns a temporary.

#include "matrix4.h"

Matrix4 IdentityMatrix();                    // 0x101b0ba0

#include "vec4.h"
#include "node.h"
#include "anim_model.h"
#include "render_vars.h"
#include "engine_memory.h"
#include "char_vars.h"

extern "C" int FUN_1013f880(void);           // the game's random number

// Size of a node allocation (sizeof(Node)) and of a motion playback state (sizeof(Playback)).
#define NODE_ALLOC_SIZE 0x18a0
#define PLAYBACK_ALLOC_SIZE 0x7c84
// Node::FindByFlags masks: the node the model is drawn from and its collision node.
#define NODE_FLAG_LOADED_ROOT 0x80
#define NODE_FLAG_COLLIDER 0x100
// Number of entries of the model table (DAT_10faaa88, DAT_10ff19e8).
#define MODEL_TABLE_SIZE 0x100
// The model's reach: this fraction of the drawn node's bounding radius.
#define MODEL_REACH_SCALE 0.578f
// Shadow opacity given to a fresh model.
#define FRESH_SHADOW_OPACITY 15
// m_2380 of a fresh model.
#define FRESH_M_2380 40
// Range of the random value in m_025c.
#define M_025C_RANGE 500
// The random offsets of the m_00b4 vectors are -50..49 on each axis before scaling.
#define OFFSET_RANDOM_RANGE 100
#define OFFSET_RANDOM_HALF 50

// 0x1017b210 (AnimModel::CloneModel). Gives this model a copy of `source`'s node tree (the second argument is not
// used). Returns 0 when `source` has no tree. Otherwise the motion state is reset like a new model's (mode 1,
// identity matrix, speed scale from `source`), this model's old tree is removed from the model table, released and
// freed, a new node is allocated and cloned from `source->root`, the playback state is allocated when missing and
// bound, the meshes are built and the bones looked up (the drawn node, "Dummy05" as head, the toes and the spine
// fall back to the drawn node; "@Face" twice, the collision node and "Bip01"). When the model is drawn, the m_0174
// direction vectors are set from its size and m_00b4 gets random scaled offsets. Finally the shadow matrix is reset,
// the tree is entered in the first free entry of the model table and the nodes are tagged. Returns 1.
int AnimModel::CloneModel(AnimModel *source, int unused)
{
    if (source->root == 0)
        return 0;

    m_0012 = 0;
    m_0013 = 0;
    m_0088 = 0;
    m_0090.Fill(0.0f);
    spineNode = 0;
    m_0074 = 0;
    m_0068 = 0;
    m_008c = 0;
    m_022c.Fill(0.0f);
    m_0076 = 0;
    m_008d = 0;
    m_025c = FUN_1013f880() % M_025C_RANGE;
    m_0240 = 0;
    m_2408.Fill(0.0f);
    m_0268 = 0;
    speedScale = 1.0f;
    shadowHidden = 0;
    m_231c = 0;
    trackA.playing = 0;
    m_2418 = 0;
    m_22a0 = 0;
    loadedRoot = 0;
    heading = 0;
    motionFrame = 0;
    motionSpeed = 0;
    collider = 0;
    matrix = IdentityMatrix();
    m_2418 = 0;
    headNode = 0;
    leftToeNode = 0;
    rightToeNode = 0;
    holdState = HOLD_FREE;
    bankIndex = 0;
    m_0274 = 0;
    m_0278 = 0;
    m_027c = 0;
    locked = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = FRESH_SHADOW_OPACITY;
    loadCount = 0;
    m_0267 = 0;
    m_2474 = 0;
    m_0080 = 0;
    important = 0;
    m_02a1 = 0;
    m_0075 = 0;
    m_02a2 = 0;
    m_17e8 = 0;
    m_0266 = 0;
    mode = 1;
    m_0268 = 0;
    m_2288 = 0;
    alpha = 1.0f;
    m_02a0 = 0;
    m_237c = 1.0f;
    shadowHidden = 0;
    trackC.playing = 0;
    trackA.playing = 0;
    trackB.playing = 0;
    m_2380 = FRESH_M_2380;
    m_2384 = 0;
    m_23d8.x = 0;
    m_23d8.y = 0;
    m_23d8.z = 0;
    m_2394 = 0;
    m_2390 = 0;
    motionSpeed = 0;
    m_2370 = 0;
    m_229c = 0;
    m_0294 = 0;
    scale.Fill(1.0f);
    speedScale = source->speedScale;
    headNode = 0;
    motionFrame = 0;
    collider = 0;

    // Release the old tree.
    if (root != 0)
    {
        for (int i = 0; i < MODEL_TABLE_SIZE; i++)
        {
            if (DAT_10faaa88[i] == root)
            {
                DAT_10faaa88[i] = 0;
                DAT_10ff19e8[i] = 0;
            }
        }
        root->Release();
        DAT_11b1ba40(root);
    }

    // Clone the source's tree into a fresh node.
    root = (Node *)DAT_11b0b938(NODE_ALLOC_SIZE, "m_frm");
    root->Init();
    root->Clone(source->root, 0, 0);
    Playback *playback = motion;
    if (playback == 0)
    {
        motion = (Playback *)DAT_11b0b938(PLAYBACK_ALLOC_SIZE, "");
        playback = motion;
    }
    playback->Init();
    motion->BindModel(this);
    m_2320 = 0;
    root->Rebuild();
    root->BuildMesh(1, 0);

    // Look up the bones.
    loadedRoot = root->FindByFlags(NODE_FLAG_LOADED_ROOT, 0, 0);
    headNode = root->Find("Dummy05", 0);
    if (headNode == 0)
        headNode = loadedRoot;
    leftToeNode = root->Find("Bip01 L Toe0", 0);
    if (leftToeNode == 0)
        leftToeNode = loadedRoot;
    rightToeNode = root->Find("Bip01 R Toe0", 0);
    if (rightToeNode == 0)
        rightToeNode = loadedRoot;
    spineNode = root->Find("Bip01 Spine", 0);
    if (spineNode == 0)
        spineNode = loadedRoot;
    swapNode1 = root->Find("@Face", 0);
    swapNode2 = root->Find("@Face", 1);
    root->M_1019b230(this);    // every node of the tree remembers its model
    if (loadedRoot != 0)
        loadedRoot->worldMatrix._41 = 0;
    if (loadedRoot == 0)
        loadedRoot = root;
    collider = root->FindByFlags(NODE_FLAG_COLLIDER, 0, 0);
    if (collider != 0)
        colliderReach = collider->bounds.w * MODEL_REACH_SCALE;
    if (root != 0)
        bipNode = root->Find("Bip01", 0);
    else
        bipNode = 0;

    // Direction vectors and random offsets scaled by the model's size.
    if (loadedRoot != 0)
    {
        float reach = loadedRoot->bounds.w * MODEL_REACH_SCALE;
        m_0174[0].Set(0, 0, 3.0f, 0);
        m_0174[1].Set(-1.0f, 0, 1.5f, 0);
        m_0174[2].Set(1.0f, 0, 1.5f, 0);
        m_0174[3].Set(-1.0f, 0, 0, 0);
        m_0174[4].Set(1.0f, 0, 0, 0);
        m_0174[5].Set(0, 0, -1.0f, 0);
        for (int i = 0; i < 10; i += 2)
        {
            m_0174[i / 2].Copy(m_0174[i / 2] * (reach * 0.5f));
            m_0174[i / 2].y += 50.0f;
            m_00b4[i].Set((float)(FUN_1013f880() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(FUN_1013f880() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(FUN_1013f880() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF), 0);
            m_00b4[i].Normalized3();
            m_00b4[i].Copy(m_00b4[i] * (reach * 0.3f));
        }
        float weight = reach * 0.15f;
        m_00b4[1].Set(1.0f, 0, 0, weight);
        m_00b4[3].Set(0, 1.0f, 0, weight);
        m_00b4[5].Set(0, 0, 1.0f, weight);
        m_00b4[7].Set(1.0f, 0, 0, weight);
        m_00b4[9].Set(0, 1.0f, 0, weight);
    }

    shadowMatrix = IdentityMatrix();
    m_23c8.x = 0;
    m_23c8.y = 0;
    m_23c8.z = 0;
    move.x = 0;
    move.y = 0;
    move.z = 0;
    velocity.x = 0;
    velocity.y = 0;
    velocity.z = 0;
    m_2388 = 0;

    // Enter the new tree in the first free entry of the model table.
    for (int i = 0; i < MODEL_TABLE_SIZE; i++)
    {
        if (DAT_10faaa88[i] == 0)
        {
            DAT_10faaa88[i] = root;
            DAT_10ff19e8[i] = &matrix;
            break;
        }
    }
    targetHeading = 0;
    heading = 0;
    m_2418 = 0;
    root->M_10191e50();
    root->M_101946b0(0xff, -1);
    root->M_101944d0();
    return 1;
}

// 0x1017b9b0 (AnimModel::LoadFile). Like CloneModel, but the node tree is loaded from the .it3 file `path`
// (Node::LoadIt3) instead of being cloned, the speed scale stays 1, and while DAT_10f654b0 is set M_1019b0c0(0, 0,
// 0x1f) runs on the new tree. Called by the object and character spawners. Returns 1.
int AnimModel::LoadFile(const char *path)
{
    m_0012 = 0;
    m_0013 = 0;
    m_0088 = 0;
    spineNode = 0;
    m_008d = 0;
    m_0090.Fill(0.0f);
    m_0088 = 0;
    m_0068 = 0;
    m_008c = 0;
    m_0074 = 0;
    m_022c.Fill(0.0f);
    m_0076 = 0;
    m_0240 = 0;
    m_0268 = 0;
    speedScale = 1.0f;
    shadowHidden = 0;
    m_231c = 0;
    trackA.playing = 0;
    heading = 0;
    motionFrame = 0;
    motionSpeed = 0;
    collider = 0;
    m_2418 = 0;
    m_22a0 = 0;
    m_2408.Fill(0.0f);
    loadedRoot = 0;
    heading = 0;
    motionFrame = 0;
    motionSpeed = 0;
    collider = 0;
    matrix = IdentityMatrix();
    m_2418 = 0;
    headNode = 0;
    leftToeNode = 0;
    rightToeNode = 0;
    m_025c = FUN_1013f880() % M_025C_RANGE;
    holdState = HOLD_FREE;
    bankIndex = 0;
    m_0274 = 0;
    m_0278 = 0;
    m_027c = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = FRESH_SHADOW_OPACITY;
    loadCount = 0;
    m_0080 = 0;
    m_02a1 = 0;
    important = 0;
    m_0075 = 0;
    m_02a2 = 0;
    m_2474 = 0;
    m_0267 = 0;
    locked = 0;
    m_17e8 = 0;
    m_0266 = 0;
    mode = 1;
    m_0268 = 0;
    m_2288 = 0;
    alpha = 1.0f;
    m_02a0 = 0;
    m_237c = 1.0f;
    shadowHidden = 0;
    trackC.playing = 0;
    trackA.playing = 0;
    trackB.playing = 0;
    m_2380 = FRESH_M_2380;
    m_2384 = 0;
    m_23d8.x = 0;
    m_23d8.y = 0;
    m_23d8.z = 0;
    m_2394 = 0;
    m_2390 = 0;
    motionSpeed = 0;
    m_2370 = 0;
    m_229c = 0;
    m_0294 = 0;
    scale.Fill(1.0f);
    headNode = 0;
    motionFrame = 0;
    collider = 0;

    // Release the old tree.
    if (root != 0)
    {
        for (int i = 0; i < MODEL_TABLE_SIZE; i++)
        {
            if (DAT_10faaa88[i] == root)
            {
                DAT_10faaa88[i] = 0;
                DAT_10ff19e8[i] = 0;
            }
        }
        root->Release();
        DAT_11b1ba40(root);
    }

    // Load the file into a fresh node.
    root = (Node *)DAT_11b0b938(NODE_ALLOC_SIZE, "m_frm");
    root->Init();
    root->LoadIt3(path, 0, 0);
    Playback *playback = motion;
    if (playback == 0)
    {
        motion = (Playback *)DAT_11b0b938(PLAYBACK_ALLOC_SIZE, "");
        playback = motion;
    }
    playback->Init();
    motion->BindModel(this);
    if (DAT_10f654b0 != 0)
        root->M_1019b0c0(0, 0, 0x1f);
    m_2320 = 0;
    root->Rebuild();
    root->BuildMesh(1, 0);

    // Look up the bones.
    loadedRoot = root->FindByFlags(NODE_FLAG_LOADED_ROOT, 0, 0);
    headNode = root->Find("Dummy05", 0);
    if (headNode == 0)
        headNode = loadedRoot;
    leftToeNode = root->Find("Bip01 L Toe0", 0);
    if (leftToeNode == 0)
        leftToeNode = loadedRoot;
    rightToeNode = root->Find("Bip01 R Toe0", 0);
    if (rightToeNode == 0)
        rightToeNode = loadedRoot;
    spineNode = root->Find("Bip01 Spine", 0);
    if (spineNode == 0)
        spineNode = loadedRoot;
    swapNode1 = root->Find("@Face", 0);
    swapNode2 = root->Find("@Face", 1);
    root->M_1019b230(this);    // every node of the tree remembers its model
    if (loadedRoot != 0)
        loadedRoot->worldMatrix._41 = 0;
    if (loadedRoot == 0)
        loadedRoot = root;
    collider = root->FindByFlags(NODE_FLAG_COLLIDER, 0, 0);
    if (collider != 0)
        colliderReach = collider->bounds.w * MODEL_REACH_SCALE;
    if (root != 0)
        bipNode = root->Find("Bip01", 0);
    else
        bipNode = 0;

    // Direction vectors and random offsets scaled by the model's size.
    if (loadedRoot != 0)
    {
        float reach = loadedRoot->bounds.w * MODEL_REACH_SCALE;
        m_0174[0].Set(0, 0, 3.0f, 0);
        m_0174[1].Set(-1.0f, 0, 1.5f, 0);
        m_0174[2].Set(1.0f, 0, 1.5f, 0);
        m_0174[3].Set(-1.0f, 0, 0, 0);
        m_0174[4].Set(1.0f, 0, 0, 0);
        m_0174[5].Set(0, 0, -1.0f, 0);
        for (int i = 0; i < 10; i += 2)
        {
            m_0174[i / 2].Copy(m_0174[i / 2] * (reach * 0.5f));
            m_0174[i / 2].y += 50.0f;
            m_00b4[i].Set((float)(FUN_1013f880() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(FUN_1013f880() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(FUN_1013f880() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF), 0);
            m_00b4[i].Normalized3();
            m_00b4[i].Copy(m_00b4[i] * (reach * 0.3f));
        }
        float weight = reach * 0.15f;
        m_00b4[1].Set(1.0f, 0, 0, weight);
        m_00b4[3].Set(0, 1.0f, 0, weight);
        m_00b4[5].Set(0, 0, 1.0f, weight);
        m_00b4[7].Set(1.0f, 0, 0, weight);
        m_00b4[9].Set(0, 1.0f, 0, weight);
    }

    shadowMatrix = IdentityMatrix();
    m_23c8.x = 0;
    m_23c8.y = 0;
    m_23c8.z = 0;
    move.x = 0;
    move.y = 0;
    move.z = 0;
    velocity.x = 0;
    velocity.y = 0;
    velocity.z = 0;
    m_2388 = 0;

    // Enter the new tree in the first free entry of the model table.
    for (int i = 0; i < MODEL_TABLE_SIZE; i++)
    {
        if (DAT_10faaa88[i] == 0)
        {
            DAT_10faaa88[i] = root;
            DAT_10ff19e8[i] = &matrix;
            break;
        }
    }
    targetHeading = 0;
    heading = 0;
    m_2418 = 0;
    root->M_10191e50();
    root->M_101946b0(0xff, -1);
    root->M_101944d0();
    return 1;
}
