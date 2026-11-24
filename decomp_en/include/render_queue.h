// The queue of nodes to draw this frame and its extra-data and matrix pools (EN build).
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_RENDER_QUEUE_H
#define GURUMIN_EN_RENDER_QUEUE_H

extern "C" int g_renderQueueMatrixCount;  // EN 0x0217fd94 (JP DAT_11b002b8): entries used in the matrix pool
extern "C" int g_renderQueueExtraCount;   // EN 0x0217fd90 (JP DAT_11b044dc): entries used in the extra-data pool
extern "C" int g_renderQueueCount;        // EN 0x0217fb48 (JP DAT_11b044d0): nodes queued

#endif
