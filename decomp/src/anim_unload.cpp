// Unloading an animated model (AnimModel, see model_params.cpp for the fields used while it moves). Matrix4 here
// has the engine's constructor and empty destructor, like in node_bones.cpp, because IdentityMatrix() returns a
// temporary.

#include "matrix4.h"

Matrix4 IdentityMatrix();  // 0x101b0ba0

#include "node.h"

#include "anim_model.h"

#include "render_vars.h"
#include "engine_memory.h"

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
    if (motion != 0)
    {
        DAT_11b1ba40(motion);
        motion = 0;
    }
    root = 0;
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
    ResetState();
    return 1;
}
