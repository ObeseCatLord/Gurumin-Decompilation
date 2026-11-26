// Setup of an effect model slot (EN build). Ported from the JP source decomp/src/effect_model.cpp (EffectModel::M_1000b9f0).
#include "char.h"
#include "effect_model.h"
#include "engine.h"
#include "engine_memory.h"
#include "head_look.h"

// Folder of Path_Data the effect models are looked up in.
const int EFFECT_MODEL_FOLDER = 0;
// Name the model node allocation is tagged with.
const char EFFECT_MODEL_TAG[] = "m_frm";

// EffectModel::Setup: loads the effect model `path` and plays frames `start`..`end`. The model starts at the player's
// matrix with white colour and no rotation. A negative `start` counts back from the end (it is kept in the frame counter
// and `start` becomes 0); a negative `end` means "up to the model's last frame". Any previous model of the slot is
// released first. Returns 1. Called when an effect model is placed.
int EffectModel::Setup(const char *path, int a, int start, int end)
{
    color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    visible = 0;
    this->start = start;
    frame = 0;
    this->end = end;
    matrix = g_characterModels[PLAYER_SLOT].matrix;
    rotation.Set(0.0f, 0.0f, 0.0f, 0.0f);
    ownCamera = 0;
    if (start < 0)
    {
        frame = -start;
        this->start = 0;
    }
    value = a;
    if (node != 0)
    {
        node->Release();
        g_engineFree(node);
    }
    keys.Clear();
    node = (Node *)g_engineAlloc(NODE_SIZE, EFFECT_MODEL_TAG);
    node->Init();
    node->Load(Path_Data(path, EFFECT_MODEL_FOLDER), 0, 0, 0);
    if (end < 0)
        this->end = node->GetLastFrame();
    node->Rebuild();
    node->BuildMesh(1, 0);
    changed = 0;
    return 1;
}
