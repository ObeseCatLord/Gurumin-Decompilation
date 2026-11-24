// Saved node states of the world's models (EN build). Ported from the matched JP source
// decomp/src/world_model_state.cpp.
#include "anim_model.h"
#include "node.h"
#include "node_data.h"
#include "world_renderer.h"

// Rotation limits (radians) of a head node while its model tracks a target: the box (-1.22, -0.27, -0.17) to
// (1.22, 0.27, 0.17) around x, y and z.
const float MODEL_STATE_LIMIT_X = 1.22f;
const float MODEL_STATE_LIMIT_Y = 0.27f;
const float MODEL_STATE_LIMIT_Z = 0.17f;
// Values of `mode`: start tracking, let it return to rest, and drop it.
const int MODEL_STATE_START = 1;
const int MODEL_STATE_END = 0;
const int MODEL_STATE_DROP = -5;

// WorldRenderer::SetModelState: for model `index`: mode 1 starts the state of its head node's parent (unless it has
// one already) with the rotation limits above and its target; mode 0 ends a running state; mode -5 drops it.
// Returns 0. Called by the model scripts.
int WorldRenderer::SetModelState(int index, int mode)
{
    Node *node = models[index].headNode;
    if (node != 0)
    {
        if (mode == MODEL_STATE_START)
        {
            if (node->parent->saved == 0)
            {
                active[index] = 1;
                models[index].headNode->parent->AllocSaved();
                models[index].headNode->parent->saved->limitMin[0] = -MODEL_STATE_LIMIT_X;
                models[index].headNode->parent->saved->limitMin[1] = -MODEL_STATE_LIMIT_Y;
                models[index].headNode->parent->saved->limitMin[2] = -MODEL_STATE_LIMIT_Z;
                models[index].headNode->parent->saved->limitMax[0] = MODEL_STATE_LIMIT_X;
                models[index].headNode->parent->saved->limitMax[1] = MODEL_STATE_LIMIT_Y;
                models[index].headNode->parent->saved->limitMax[2] = MODEL_STATE_LIMIT_Z;
                models[index].headNode->parent->EnterTargetState(&targets[index]);
                return 0;
            }
        }
        else if (mode == MODEL_STATE_END)
        {
            if (node->parent->saved != 0)
            {
                node->parent->saved->state = SAVED_RETURNING;
                return 0;
            }
        }
        else if (mode == MODEL_STATE_DROP)
        {
            active[index] = 0;
            if (models[index].headNode->parent->saved != 0)
                models[index].headNode->parent->FreeSaved();
        }
    }
    return 0;
}
