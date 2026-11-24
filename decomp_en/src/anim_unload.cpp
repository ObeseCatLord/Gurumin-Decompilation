// Unloading a model (EN build). Ported from the matched JP source decomp/src/anim_unload.cpp.
#include "anim_model.h"
#include "engine_memory.h"
#include "node.h"

// AnimModel::Unload: removes the model from the model file table, frees its node tree and its motion playback state, and
// resets position (identity world matrix), speed, collision and pose state. Returns 1. Called when a model is replaced
// or released.
int AnimModel::Unload()
{
    m_22a0 = 0;
    m_22ac = 0;
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] == root)
        {
            g_modelFileRoots[i] = 0;
            g_modelFileMatrices[i] = 0;
        }
    }
    if (root != 0)
    {
        root->Release();
        g_engineFree(root);
    }
    if (playback != 0)
    {
        g_engineFree(playback);
        playback = 0;
    }
    root = 0;
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
    ResetState();
    return 1;
}
