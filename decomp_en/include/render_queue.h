// The queue of nodes to draw this frame and its extra-data and matrix pools (EN build).
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_RENDER_QUEUE_H
#define GURUMIN_EN_RENDER_QUEUE_H

#include "vec4.h"

struct Node;

extern "C" int g_renderQueueMatrixCount;  // EN 0x0217fd94 (JP DAT_11b002b8): entries used in the matrix pool
extern "C" int g_renderQueueExtraCount;   // EN 0x0217fd90 (JP DAT_11b044dc): entries used in the extra-data pool
extern "C" int g_renderQueueCount;        // EN 0x0217fb48 (JP DAT_11b044d0): nodes queued

// Capacity of the queue and of its per-node and matrix pools.
const int RENDER_QUEUE_CAPACITY = 64;
// Number of light sets in the queue's light pool.
const int RENDER_QUEUE_LIGHT_SETS = 48;

// The lights a queued node is drawn with (0x80 bytes each).
struct QueuedLightSet
{
    char m_0000[0x80];   // 0x00: not yet named
};

extern "C" unsigned char g_renderQueuePass[RENDER_QUEUE_CAPACITY];       // EN 0x0217fc50 (JP DAT_11b00030): draw pass of each queued node
extern "C" Node *g_renderQueueNodes[RENDER_QUEUE_CAPACITY];              // EN 0x0217fb50 (JP DAT_11b00070): the queued nodes
extern "C" float g_renderQueueFade[RENDER_QUEUE_CAPACITY];               // EN 0x0217fc90 (JP DAT_11b00170): fade each node is drawn with
extern "C" signed char g_renderQueueMapPass[RENDER_QUEUE_CAPACITY];      // EN 0x0217fe18 (JP DAT_11b00270): map pass value used for the node's draw
extern "C" signed char g_renderQueueLightIndex[RENDER_QUEUE_CAPACITY];   // EN 0x0217fd98: light set (of g_renderQueueLightSets) of a map pass node
extern "C" signed char g_renderQueueMatrixIndex[RENDER_QUEUE_CAPACITY];  // EN 0x0217fdd8: view matrix (of g_renderQueueMatrices) of a map pass node
extern "C" Matrix4 g_renderQueueMatrices[RENDER_QUEUE_CAPACITY];         // EN 0x0218be70: matrices of the queued nodes
extern "C" QueuedLightSet g_renderQueueLightSets[RENDER_QUEUE_LIGHT_SETS];   // EN 0x0218ce70: light sets of the queued nodes
extern "C" Vec4 g_renderQueueParamsA[RENDER_QUEUE_CAPACITY];             // EN 0x0218e680: first per-node draw parameter vector
extern "C" Vec4 g_renderQueueParamsB[RENDER_QUEUE_CAPACITY];             // EN 0x0218ca70: second per-node draw parameter vector
extern "C" Vec4 g_renderQueueColour[RENDER_QUEUE_CAPACITY];              // EN 0x0218ea80 (JP DAT_11b07128): draw colour of each node

// Colour and fade the nodes are drawn with (set per node by DrawList_DrawPass).
extern "C" Vec4 g_drawColour;   // EN 0x021ffd88 (JP DAT_10f66948)
extern "C" float g_drawFade;    // EN 0x021a3534 (JP DAT_10f655c4)

// Empties the queue and its pools. Returns 0. EN 0x74a4f0.
extern "C" int RenderQueue_Clear();

#endif
