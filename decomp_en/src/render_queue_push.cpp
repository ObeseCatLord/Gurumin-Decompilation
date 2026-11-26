// Queueing of a node for drawing (EN build). JP twin: decomp/src/render_queue.cpp (RenderQueue_Push, not matched in JP).
#include "engine_memory.h"
#include "map_view.h"
#include "render_queue.h"

// Pool index meaning "this node has no extra data / no matrix".
const signed char RENDER_QUEUE_NO_INDEX = (signed char)0xff;
// Last valid index of the extra-data and matrix pools (the pools stop growing here).
const int RENDER_QUEUE_POOL_LAST = 0x2f;

// RenderQueue_Push: appends `node` to the queue of nodes drawn this frame with its pass `flags`, the two parameter
// vectors and the current draw colour and fade. Optional `extra` data (a light set) and `matrix` are copied into their
// pools and the entry stores the pool index (0xff = none). The queue and the pools stop growing when full (the last
// entry is overwritten). Returns 0. Called when a node is queued for the draw passes.
extern "C" int RenderQueue_Push(Node *node, const void *extra, const Matrix4 *matrix, const Vec4 *paramsA,
                                const Vec4 *paramsB, char flags)
{
    g_renderQueueNodes[g_renderQueueCount] = node;
    g_renderQueuePass[g_renderQueueCount] = flags;
    g_renderQueueMapPass[g_renderQueueCount] = (signed char)g_mapPassRunning;
    g_renderQueueMatrixIndex[g_renderQueueCount] = RENDER_QUEUE_NO_INDEX;
    g_renderQueueLightIndex[g_renderQueueCount] = RENDER_QUEUE_NO_INDEX;
    g_renderQueueParamsA[g_renderQueueCount].Copy(*paramsA);
    g_renderQueueParamsB[g_renderQueueCount].Copy(*paramsB);
    g_renderQueueColour[g_renderQueueCount].Copy(g_drawColour);
    g_renderQueueFade[g_renderQueueCount] = g_drawFade;
    if (extra != 0)
    {
        g_engineCopy(&g_renderQueueLightSets[g_renderQueueExtraCount], extra, sizeof(QueuedLightSet));
        g_renderQueueLightIndex[g_renderQueueCount] = (signed char)g_renderQueueExtraCount;
        if (g_renderQueueExtraCount < RENDER_QUEUE_POOL_LAST)
            g_renderQueueExtraCount++;
    }
    if (matrix != 0)
    {
        g_renderQueueMatrices[g_renderQueueMatrixCount] = *matrix;
        g_renderQueueMatrixIndex[g_renderQueueCount] = (signed char)g_renderQueueMatrixCount;
        if (g_renderQueueMatrixCount < RENDER_QUEUE_POOL_LAST)
            g_renderQueueMatrixCount++;
    }
    if (g_renderQueueCount < RENDER_QUEUE_CAPACITY - 1)
        g_renderQueueCount++;
    return 0;
}
