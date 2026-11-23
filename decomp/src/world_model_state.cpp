// Saved node states of the world's models (WorldRenderer at 0x10836c70; see world_frame.cpp for its other fields).

#include "vec4.h"

#include "node_data.h"

#include "node.h"

#include "anim_model.h"
#include "world_renderer.h"

// 0x10166f20 (WorldRenderer::SetModelState). For model `index`: mode 1 starts the state of its linked node (unless it
// has one already) with the box (-1.22, -0.27, -0.17)..(1.22, 0.27, 0.17) and its target; mode 0 ends a running
// state; mode -5 drops it. Returns 0.
int WorldRenderer::SetModelState(int index, int mode)
{
    Node *node = models[index].headNode;
    if (node != 0)
    {
        if (mode == 1)
        {
            if (node->parent->saved == 0)
            {
                active[index] = 1;
                models[index].headNode->parent->AllocSaved();
                models[index].headNode->parent->saved->limitMin.x = -1.22f;
                models[index].headNode->parent->saved->limitMin.y = -0.27f;
                models[index].headNode->parent->saved->limitMin.z = -0.17f;
                models[index].headNode->parent->saved->limitMax.x = 1.22f;
                models[index].headNode->parent->saved->limitMax.y = 0.27f;
                models[index].headNode->parent->saved->limitMax.z = 0.17f;
                models[index].headNode->parent->EnterTargetState(&targets[index]);
                return 0;
            }
        }
        else if (mode == 0)
        {
            if (node->parent->saved != 0)
            {
                node->parent->saved->state = SAVED_RETURNING;
                return 0;
            }
        }
        else if (mode == -5)
        {
            active[index] = 0;
            if (models[index].headNode->parent->saved != 0)
                models[index].headNode->parent->FreeSaved();
        }
    }
    return 0;
}
