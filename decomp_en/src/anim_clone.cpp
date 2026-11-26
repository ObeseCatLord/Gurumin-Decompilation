// Giving a model its node tree (EN build): by cloning another model's tree or by loading a model file. Ported from the
// matched JP source decomp/src/anim_clone.cpp.
#include "anim_model.h"
#include "engine.h"
#include "engine_memory.h"
#include "node.h"
#include "weapon_attach.h"

// AnimModel::CloneModel: gives this model a copy of `source`'s node tree. Returns 0 when `source` has no tree.
// Otherwise the motion state is reset like a new model's (mode 1, identity matrix, speed scale from `source`), the old
// tree is removed from the model file table, released and freed, a new node is allocated and cloned from
// `source->root`, the playback state is allocated when missing and bound, the meshes are built and the bones looked up
// (the loaded root, "Dummy05" as head, the toes and the spine fall back to the loaded root; "@Face" twice, the
// collision node and "Bip01"). The wobble vectors are set from the model's size, the shadow matrix is reset, the tree
// is entered in the first free entry of the model file table and the nodes are tagged. Returns 1. Called by the object
// and character spawners.
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
    sortBias = 0;
    m_008c = 0;
    m_022c.Fill(0.0f);
    uiAnimStarted = 0;
    childRunning = 0;
    m_025c = Random() % M_025C_RANGE;
    m_0240 = 0;
    m_2408.Fill(0.0f);
    pullTarget = 0;
    speedScale = 1.0f;
    shadowHidden = 0;
    dropCollider = 0;
    motion.playing = 0;
    m_2418 = 0;
    m_22a0 = 0;
    loadedRoot = 0;
    heading = 0;
    motionFrame = 0;
    verticalSpeed = 0;
    collider = 0;
    matrix = IdentityMatrix();
    m_2418 = 0;
    headNode = 0;
    leftToeNode = 0;
    rightToeNode = 0;
    holdState = HOLD_FREE;
    bankIndex = 0;
    burst = 0;
    blastAngle = 0;
    blastPush = 0;
    locked = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = SHADOW_OPACITY_PLACED;
    loadCount = 0;
    idling = 0;
    m_2474 = 0;
    m_0080 = 0;
    important = 0;
    m_02a1 = 0;
    carriedFlag = 0;
    m_02a2 = 0;
    m_17e8 = 0;
    m_0266 = 0;
    mode = 1;
    pullTarget = 0;
    pushFlag = 0;
    alpha = 1.0f;
    m_02a0 = 0;
    m_237c = 1.0f;
    shadowHidden = 0;
    trackC.playing = 0;
    motion.playing = 0;
    trackB.playing = 0;
    m_2380 = FRESH_M_2380;
    collisionResult = 0;
    moveStep.x = 0;
    moveStep.y = 0;
    moveStep.z = 0;
    m_2394 = 0;
    collisionLimit = 0;
    verticalSpeed = 0;
    placedHeight = 0;
    blendFrames = 0;
    walkingRoute = 0;
    scale.Fill(1.0f);
    speedScale = source->speedScale;
    headNode = 0;
    motionFrame = 0;
    collider = 0;

    // Release the old tree.
    if (root != 0)
    {
        for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
        {
            if (g_modelFileRoots[i] == root)
            {
                g_modelFileRoots[i] = 0;
                g_modelFileMatrices[i] = 0;
            }
        }
        root->Release();
        g_engineFree(root);
    }

    // Clone the source's tree into a fresh node.
    root = (Node *)g_engineAlloc(NODE_SIZE, "m_frm");
    root->Init();
    root->Clone(source->root, 0, 0);
    Playback *state = playback;
    if (state == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        state = playback;
    }
    state->Init();
    playback->BindModel(this);
    m_2320 = 0;
    root->Rebuild();
    root->BuildMesh(1, 0);

    // Look up the bones.
    loadedRoot = root->FindByFlags(MODEL_FLAG_LOADED_ROOT, 0, 0);
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
    root->SetModel(this);
    if (loadedRoot != 0)
        loadedRoot->worldMatrix.m[MATRIX_TRANSLATION_X] = 0;
    if (loadedRoot == 0)
        loadedRoot = root;
    collider = root->FindByFlags(MODEL_FLAG_COLLIDER, 0, 0);
    if (collider != 0)
        colliderReach = collider->bounds.w * MODEL_REACH_SCALE;
    if (root != 0)
        bipNode = root->Find("Bip01", 0);
    else
        bipNode = 0;

    // Wobble directions and random offsets scaled by the model's size.
    if (loadedRoot != 0)
    {
        float reach = loadedRoot->bounds.w * MODEL_REACH_SCALE;
        m_0174[0].Set(0, 0, 3.0f, 0);
        m_0174[1].Set(-1.0f, 0, 1.5f, 0);
        m_0174[2].Set(1.0f, 0, 1.5f, 0);
        m_0174[3].Set(-1.0f, 0, 0, 0);
        m_0174[4].Set(1.0f, 0, 0, 0);
        m_0174[5].Set(0, 0, -1.0f, 0);
        for (int i = 0; i < WOBBLE_OFFSET_ENTRIES; i += 2)
        {
            m_0174[i / 2].Copy(m_0174[i / 2] * (reach * WOBBLE_DIRECTION_SCALE));
            m_0174[i / 2].y += WOBBLE_LIFT;
            m_00b4[i].Set((float)(Random() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(Random() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(Random() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF), 0);
            m_00b4[i].Normalized3();
            m_00b4[i].Copy(m_00b4[i] * (reach * WOBBLE_OFFSET_SCALE));
        }
        float weight = reach * WOBBLE_WEIGHT_SCALE;
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
    speedChangeFrames = 0;

    // Enter the new tree in the first free entry of the model file table.
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] == 0)
        {
            g_modelFileRoots[i] = root;
            g_modelFileMatrices[i] = &matrix;
            break;
        }
    }
    targetHeading = 0;
    heading = 0;
    m_2418 = 0;
    root->TagElements();
    root->SetColourA(NODE_COLOUR_OPAQUE, NODE_ALL_FLAGS);
    root->ClearHasCollision();
    return 1;
}

// AnimModel::LoadFile: like CloneModel, but the node tree is loaded from the .it3 file `path` (Node::LoadIt3) instead
// of being cloned, the speed scale stays 1, and while g_refreshRenderMode is set the new tree's render mode is
// refreshed (Node::SetRenderModeExcept). Called by the object and character spawners. Returns 1.
int AnimModel::LoadFile(const char *path)
{
    m_0012 = 0;
    m_0013 = 0;
    m_0088 = 0;
    spineNode = 0;
    childRunning = 0;
    m_0090.Fill(0.0f);
    m_0088 = 0;
    sortBias = 0;
    m_008c = 0;
    m_0074 = 0;
    m_022c.Fill(0.0f);
    uiAnimStarted = 0;
    m_0240 = 0;
    pullTarget = 0;
    speedScale = 1.0f;
    shadowHidden = 0;
    dropCollider = 0;
    motion.playing = 0;
    heading = 0;
    motionFrame = 0;
    verticalSpeed = 0;
    collider = 0;
    m_2418 = 0;
    m_22a0 = 0;
    m_2408.Fill(0.0f);
    loadedRoot = 0;
    heading = 0;
    motionFrame = 0;
    verticalSpeed = 0;
    collider = 0;
    matrix = IdentityMatrix();
    m_2418 = 0;
    headNode = 0;
    leftToeNode = 0;
    rightToeNode = 0;
    m_025c = Random() % M_025C_RANGE;
    holdState = HOLD_FREE;
    bankIndex = 0;
    burst = 0;
    blastAngle = 0;
    blastPush = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = SHADOW_OPACITY_PLACED;
    loadCount = 0;
    m_0080 = 0;
    m_02a1 = 0;
    important = 0;
    carriedFlag = 0;
    m_02a2 = 0;
    m_2474 = 0;
    idling = 0;
    locked = 0;
    m_17e8 = 0;
    m_0266 = 0;
    mode = 1;
    pullTarget = 0;
    pushFlag = 0;
    alpha = 1.0f;
    m_02a0 = 0;
    m_237c = 1.0f;
    shadowHidden = 0;
    trackC.playing = 0;
    motion.playing = 0;
    trackB.playing = 0;
    m_2380 = FRESH_M_2380;
    collisionResult = 0;
    moveStep.x = 0;
    moveStep.y = 0;
    moveStep.z = 0;
    m_2394 = 0;
    collisionLimit = 0;
    verticalSpeed = 0;
    placedHeight = 0;
    blendFrames = 0;
    walkingRoute = 0;
    scale.Fill(1.0f);
    headNode = 0;
    motionFrame = 0;
    collider = 0;

    // Release the old tree.
    if (root != 0)
    {
        for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
        {
            if (g_modelFileRoots[i] == root)
            {
                g_modelFileRoots[i] = 0;
                g_modelFileMatrices[i] = 0;
            }
        }
        root->Release();
        g_engineFree(root);
    }

    // Load the file into a fresh node.
    root = (Node *)g_engineAlloc(NODE_SIZE, "m_frm");
    root->Init();
    root->LoadIt3(path, 0, 0);
    Playback *state = playback;
    if (state == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        state = playback;
    }
    state->Init();
    playback->BindModel(this);
    if (g_refreshRenderMode != 0)
        root->SetRenderModeExcept(RENDER_MODE_DEFAULT, RENDER_MODE_DEFAULT, MODEL_LOAD_RENDER_MODE_EXCEPT);
    m_2320 = 0;
    root->Rebuild();
    root->BuildMesh(1, 0);

    // Look up the bones.
    loadedRoot = root->FindByFlags(MODEL_FLAG_LOADED_ROOT, 0, 0);
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
    root->SetModel(this);
    if (loadedRoot != 0)
        loadedRoot->worldMatrix.m[MATRIX_TRANSLATION_X] = 0;
    if (loadedRoot == 0)
        loadedRoot = root;
    collider = root->FindByFlags(MODEL_FLAG_COLLIDER, 0, 0);
    if (collider != 0)
        colliderReach = collider->bounds.w * MODEL_REACH_SCALE;
    if (root != 0)
        bipNode = root->Find("Bip01", 0);
    else
        bipNode = 0;

    // Wobble directions and random offsets scaled by the model's size.
    if (loadedRoot != 0)
    {
        float reach = loadedRoot->bounds.w * MODEL_REACH_SCALE;
        m_0174[0].Set(0, 0, 3.0f, 0);
        m_0174[1].Set(-1.0f, 0, 1.5f, 0);
        m_0174[2].Set(1.0f, 0, 1.5f, 0);
        m_0174[3].Set(-1.0f, 0, 0, 0);
        m_0174[4].Set(1.0f, 0, 0, 0);
        m_0174[5].Set(0, 0, -1.0f, 0);
        for (int i = 0; i < WOBBLE_OFFSET_ENTRIES; i += 2)
        {
            m_0174[i / 2].Copy(m_0174[i / 2] * (reach * WOBBLE_DIRECTION_SCALE));
            m_0174[i / 2].y += WOBBLE_LIFT;
            m_00b4[i].Set((float)(Random() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(Random() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF),
                          (float)(Random() % OFFSET_RANDOM_RANGE - OFFSET_RANDOM_HALF), 0);
            m_00b4[i].Normalized3();
            m_00b4[i].Copy(m_00b4[i] * (reach * WOBBLE_OFFSET_SCALE));
        }
        float weight = reach * WOBBLE_WEIGHT_SCALE;
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
    speedChangeFrames = 0;

    // Enter the new tree in the first free entry of the model file table.
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] == 0)
        {
            g_modelFileRoots[i] = root;
            g_modelFileMatrices[i] = &matrix;
            break;
        }
    }
    targetHeading = 0;
    heading = 0;
    m_2418 = 0;
    root->TagElements();
    root->SetColourA(NODE_COLOUR_OPAQUE, NODE_ALL_FLAGS);
    root->ClearHasCollision();
    return 1;
}
