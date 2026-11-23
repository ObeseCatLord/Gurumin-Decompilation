// Saved node states of the world's models (WorldRenderer at 0x10836c70; see world_frame.cpp for its other fields).

#include "vec4.h"

// A node's saved block (scene_node.cpp NodeSaved): the state flag and a box used by the target state.
struct NodeSaved
{
    char pad_0000[0x20];
    int state;                // 0x20: 2 = ending
    char pad_0024[0x54];
    float boxMinX;            // 0x78
    float boxMinY;            // 0x7c
    float boxMinZ;            // 0x80
    int pad_0084;
    float boxMaxX;            // 0x88
    float boxMaxY;            // 0x8c
    float boxMaxZ;            // 0x90
};

#include "node.h"

// A model record (0x249c bytes, the layout of a Character's model part).
struct ModelRecord
{
    char pad_0000[0x22f4];
    Node *node;               // 0x22f4
    char pad_22f8[0x1a4];
};

struct WorldRenderer
{
    char pad_00000[0x8860];
    ModelRecord *models;      // 0x8860
    char pad_08864[0x136cf4];
    Vec4 targets[0x80];       // 0x13f558: target of each model's state
    char active[0x80];        // 0x13fd58: 1 while the model's state runs
    int SetModelState(int index, int mode);
};

// 0x10166f20 (WorldRenderer::SetModelState). For model `index`: mode 1 starts the state of its linked node (unless it
// has one already) with the box (-1.22, -0.27, -0.17)..(1.22, 0.27, 0.17) and its target; mode 0 ends a running
// state; mode -5 drops it. Returns 0.
int WorldRenderer::SetModelState(int index, int mode)
{
    Node *node = models[index].node;
    if (node != 0)
    {
        if (mode == 1)
        {
            if (node->parent->saved == 0)
            {
                active[index] = 1;
                models[index].node->parent->AllocSaved();
                models[index].node->parent->saved->boxMinX = -1.22f;
                models[index].node->parent->saved->boxMinY = -0.27f;
                models[index].node->parent->saved->boxMinZ = -0.17f;
                models[index].node->parent->saved->boxMaxX = 1.22f;
                models[index].node->parent->saved->boxMaxY = 0.27f;
                models[index].node->parent->saved->boxMaxZ = 0.17f;
                models[index].node->parent->EnterTargetState(&targets[index]);
                return 0;
            }
        }
        else if (mode == 0)
        {
            if (node->parent->saved != 0)
            {
                node->parent->saved->state = 2;
                return 0;
            }
        }
        else if (mode == -5)
        {
            active[index] = 0;
            if (models[index].node->parent->saved != 0)
                models[index].node->parent->FreeSaved();
        }
    }
    return 0;
}
