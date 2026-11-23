// Mesh buffers in video memory. Each mesh slot owns a pair of system-memory buffers (DAT_10f1a870 vertices,
// DAT_10f60d38 indices, see buffer_pool.cpp) and optionally a copy of both in default-pool memory
// (DAT_10f0d5e0, DAT_10f1c7f0). DAT_10f181e0 ages the video copies; DAT_10f1a86c counts their bytes.

#include <string.h>
#include "d3d9_min.h"

#include "render_vars.h"
#include "render_state.h"

#define MESH_SLOT_COUNT 0x5a0
#define MESH_SCAN_STEP 10
#define MESH_VIDEO_USAGE 0x208
#define MESH_INDEX_FORMAT_16 0x65
#define LOCK_DISCARD 0x2000
#define MESH_OVER_BUDGET(slot) (DAT_10f1e670[slot] + DAT_10f628e8[slot] + DAT_10f1a86c > DAT_10203410)

#define CHECK_OLDEST_SKIPPING(k)                                                                  \
    if (DAT_10f0d5e0[i + (k)] != 0 && DAT_10f181e0[i + (k)] >= oldestAge && i + (k) != skipSlot) \
    {                                                                                             \
        oldestAge = DAT_10f181e0[i + (k)];                                                        \
        oldest = i + (k);                                                                         \
    }

#define CHECK_OLDEST(k)                                                  \
    if (DAT_10f0d5e0[i + (k)] != 0 && DAT_10f181e0[i + (k)] > oldestAge) \
    {                                                                    \
        oldestAge = DAT_10f181e0[i + (k)];                               \
        oldest = i + (k);                                                \
    }

// Index of the oldest resident video copy other than `skipSlot` (ties go to the later slot), or -1.
static __forceinline int FindOldestVideoCopySkipping(int skipSlot)
{
    int oldestAge = DAT_10f181e0[skipSlot];
    int oldest = -1;
    int i = 0;
    do
    {
        CHECK_OLDEST_SKIPPING(0)
        CHECK_OLDEST_SKIPPING(1)
        CHECK_OLDEST_SKIPPING(2)
        CHECK_OLDEST_SKIPPING(3)
        CHECK_OLDEST_SKIPPING(4)
        CHECK_OLDEST_SKIPPING(5)
        CHECK_OLDEST_SKIPPING(6)
        CHECK_OLDEST_SKIPPING(7)
        CHECK_OLDEST_SKIPPING(8)
        CHECK_OLDEST_SKIPPING(9)
        i += MESH_SCAN_STEP;
    } while (i < MESH_SLOT_COUNT);
    return oldest;
}

// Index of the resident video copy that is older than slot `slot` itself, the oldest of them, or -1.
static __forceinline int FindOldestVideoCopy(int slot)
{
    int oldestAge = DAT_10f181e0[slot];
    int oldest = -1;
    int i = 0;
    do
    {
        CHECK_OLDEST(0)
        CHECK_OLDEST(1)
        CHECK_OLDEST(2)
        CHECK_OLDEST(3)
        CHECK_OLDEST(4)
        CHECK_OLDEST(5)
        CHECK_OLDEST(6)
        CHECK_OLDEST(7)
        CHECK_OLDEST(8)
        CHECK_OLDEST(9)
        i += MESH_SCAN_STEP;
    } while (i < MESH_SLOT_COUNT);
    return oldest;
}

// 0x10147a20 (GetMeshBuffers). Returns the vertex and index buffers to draw mesh slot `slot` with. With hardware
// vertex processing, an unflagged slot is drawn from video-memory copies, created on first use by evicting the
// oldest copies until they fit the budget DAT_10203410 and the allocations succeed. The slot's age restarts at 0
// and every other age grows by one. Without hardware vertex processing, or when nothing is left to evict, the
// system-memory buffers are returned. Returns 1, or 0 when a lock fails or the slot is out of range.
extern "C" int FUN_10147a20(int slot, IUnknownLike **vertexBuffer, IUnknownLike **indexBuffer)
{
    if (slot < 0 || slot >= MESH_SLOT_COUNT)
        return 0;
    void *vertexSource;
    void *vertexDestination;
    void *indexSource;
    void *indexDestination;
    int *slotAge = &DAT_10f181e0[slot];
    *slotAge = 0;
    if (DAT_10f6557c == 0 || DAT_10f60690[slot] != 0)
        goto useSystemBuffers;
    IUnknownLike **videoVertices = &DAT_10f0d5e0[slot];
    if (*videoVertices != 0)
    {
        *vertexBuffer = *videoVertices;
        *indexBuffer = DAT_10f1c7f0[slot];
        return 1;
    }
    if (MESH_OVER_BUDGET(slot))
    {
        do
        {
            int victim = FindOldestVideoCopySkipping(slot);
            if (victim == -1)
                goto useSystemBuffers;
            FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
            DAT_10f1a86c -= DAT_10f1e670[victim] + DAT_10f628e8[victim];
            DAT_10f181e0[victim] = 0;
            DAT_10f0d5e0[victim]->Release();
            DAT_10f1c7f0[victim]->Release();
            DAT_10f1c7f0[victim] = 0;
            DAT_10f0d5e0[victim] = 0;
        } while (MESH_OVER_BUDGET(slot));
    }
    while (DAT_10f1fcf0->CreateVertexBuffer(DAT_10f628e8[slot], MESH_VIDEO_USAGE, 0, 0, videoVertices, 0) != 0)
    {
        int victim = FindOldestVideoCopy(slot);
        if (victim == -1)
            goto useSystemBuffers;
        DAT_10f1a86c -= DAT_10f1e670[victim] + DAT_10f628e8[victim];
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        DAT_10f0d5e0[victim]->Release();
        DAT_10f1c7f0[victim]->Release();
        DAT_10f0d5e0[victim] = 0;
        DAT_10f1c7f0[victim] = 0;
    }
    IUnknownLike **videoIndices = &DAT_10f1c7f0[slot];
    int *indexBytes = &DAT_10f1e670[slot];
    if (DAT_10f1fcf0->CreateIndexBuffer(*indexBytes, MESH_VIDEO_USAGE, MESH_INDEX_FORMAT_16, 0, videoIndices, 0) != 0)
    {
      do
      {
          FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
          int victim = FindOldestVideoCopy(slot);
          if (victim == -1)
          {
              (*videoVertices)->Release();
              *videoVertices = 0;
              goto useSystemBuffers;
          }
          DAT_10f1a86c -= DAT_10f1e670[victim] + DAT_10f628e8[victim];
          DAT_10f0d5e0[victim]->Release();
          DAT_10f1c7f0[victim]->Release();
          DAT_10f0d5e0[victim] = 0;
          DAT_10f1c7f0[victim] = 0;
      } while (DAT_10f1fcf0->CreateIndexBuffer(*indexBytes, MESH_VIDEO_USAGE, MESH_INDEX_FORMAT_16, 0, videoIndices, 0) != 0);
    }
    DAT_10f1a86c += DAT_10f628e8[slot] + *indexBytes;
    {
        IUnknownLike **systemVertices = &DAT_10f1a870[slot];
        if (static_cast<IDirect3DBuffer9Like *>(*systemVertices)->Lock(0, DAT_10f628e8[slot], &vertexSource, 0) != 0)
            return 0;
        if (static_cast<IDirect3DBuffer9Like *>(*videoVertices)->Lock(0, DAT_10f628e8[slot], &vertexDestination, LOCK_DISCARD) != 0)
        {
            static_cast<IDirect3DBuffer9Like *>(*systemVertices)->Unlock();
            return 0;
        }
        memcpy(vertexDestination, vertexSource, DAT_10f628e8[slot]);
        static_cast<IDirect3DBuffer9Like *>(*systemVertices)->Unlock();
        static_cast<IDirect3DBuffer9Like *>(*videoVertices)->Unlock();
        IUnknownLike **systemIndices = &DAT_10f60d38[slot];
        if (static_cast<IDirect3DBuffer9Like *>(*systemIndices)->Lock(0, *indexBytes, &indexSource, 0) != 0)
            return 0;
        if (static_cast<IDirect3DBuffer9Like *>(*videoIndices)->Lock(0, *indexBytes, &indexDestination, LOCK_DISCARD) != 0)
        {
            static_cast<IDirect3DBuffer9Like *>(*systemIndices)->Unlock();
            return 0;
        }
        memcpy(indexDestination, indexSource, *indexBytes);
        static_cast<IDirect3DBuffer9Like *>(*systemIndices)->Unlock();
        static_cast<IDirect3DBuffer9Like *>(*videoIndices)->Unlock();
    }
    for (int i = 0; i < MESH_SLOT_COUNT; i++)
        DAT_10f181e0[i]++;
    *slotAge = 0;
    *vertexBuffer = *videoVertices;
    *indexBuffer = *videoIndices;
    return 1;
useSystemBuffers:
    *vertexBuffer = DAT_10f1a870[slot];
    *indexBuffer = DAT_10f60d38[slot];
    return 1;
}
