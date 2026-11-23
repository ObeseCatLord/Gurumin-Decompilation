// Texture manager, triangle batches (Selection = TriangleBatch in tri_batch.cpp), model owners and helpers.

#include "texture_object.h"

struct TextureManager
{
    int M_10173480();
    int M_101734d0();
    int M_10173510();
    void M_1015c730();  // Shutdown (render_shutdown.cpp)
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

#include "matrix4.h"

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

#include "anim_model.h"

#include "node.h"

struct Animator
{
    void M_101687e0();
    int M_101692c0();
    int M_10184160();
};

#include "mesh.h"

#include "render_vars.h"
#include "engine_memory.h"
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
            DAT_118d0ba8[i].Unload();
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
            DAT_118d0ba8[i].Recreate();
    }
    return 1;
}

// Runs M_10169e10 on every loaded texture (restore the texture data).
int TextureManager::M_10173510()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_118d0ba8[i].Restore();
    }
    return 1;
}

// Unloads all textures and shuts the renderer down (M_1015c730 = TextureManager::Shutdown, render_shutdown.cpp).
void TextureManager::M_10175e70()
{
    M_10173480();
    M_1015c730();
}

// Returns the playback's current frame, creating the playback object on first use.
int AnimModel::M_1017b1c0()
{
    if (motion == 0)
    {
        motion = (Playback *)DAT_11b0b938(0x7c84, "");
        motion->Init();
        motion->BindModel(this);
    }
    return motion->layer;
}

// Advances the animation one step and returns its state.
int Animator::M_10184160()
{
    M_101687e0();
    return M_101692c0();
}

// After a map change: unloads every texture that was not marked as kept and has no generated alpha.
extern "C" int FUN_10188fb0(void)
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0 && DAT_10fa6db8[i] == 0 && DAT_118d0ba8[i].generatedAlpha == 0)
            FUN_10174c70(i, 1);
    }
    return 0;
}

// Frees the vertex data of `count` pack headers (VertexPack::FreeVertices on each).
extern "C" void __stdcall FUN_101911b0(VertexPack *packs, int count)
{
    if (packs != 0)
    {
        for (; count > 0; count--)
        {
            packs->FreeVertices();
            packs++;
        }
    }
}


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
        DAT_118d0ba8[slot].Unload();
        DAT_10ffe5f8[slot][0] = 0;
    }
    return DAT_10fdeb90[slot];
}


// Starts motion `motion` on this model (0x1017b0e0, CharModel::M_1017b0e0 in other files): creates the playback
// object on first use, then hands the request to it. `a`/`b` are the playback options (loop mode, blend frames).
void AnimModel::PlayMotion(int layer, int a, int b, float speed)
{
    if (motion == 0)
    {
        motion = (Playback *)DAT_11b0b938(0x7c84, "");
        motion->Init();
        motion->BindModel(this);
    }
    if (important != 0)
        DAT_11b04508 = 1;
    motion->Play(layer, a, b, speed);
    DAT_11b04508 = 0;
}

// Loads model file `path` into this model's existing node tree (0x1017aeb0, CharModel::M_1017aeb0 elsewhere).
// Counts the loads, looks up the biped root "Bip01" (the result is unused in the shipped code), resets m_2494 and
// rebinds the motion playback to the new model. Returns 0 when the model has no node tree yet.
int AnimModel::LoadModel(const char *path)
{
    if (root == 0)
        return 0;
    root->PreScanFile(path, 0);
    if (loadCount < 3)
        loadCount++;
    root->Find("Bip01", 0);
    ZeroVec unused1;
    ZeroVec unused2;
    m_2494 = 0;
    if (motion != 0)
        motion->BindModel(this);
    return 1;
}

// Updates this model's collision: releases the saved matrix states of the node tree (Node::ReleaseSavedStates) and collides the model's position and radius
// with an identity-like work matrix (Node::StartProximityPass, scene_node.cpp).
// Returns 0.
int AnimModel::UpdateCollision()
{
    Matrix4 work;
    if (root != 0)
        root->ReleaseSavedStates();
    m_2384 = 0;
    root->StartProximityPass(matrix.m, &work, 0, radius, 0);
    return 0;
}
