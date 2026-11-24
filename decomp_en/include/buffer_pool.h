// Direct3D surface pools of the EN build: per texture slot surface tables, a 0x200 entry pool of reusable buffers
// (declared in render.h) and the VRAM total. Declared once here (JP: decomp/include/render_vars.h).
#ifndef GURUMIN_EN_BUFFER_POOL_H
#define GURUMIN_EN_BUFFER_POOL_H

#include <d3d9.h>
#include "render.h"

// Texture slots that own surfaces.
const int SURFACE_SLOT_COUNT = 0x5a0;

extern "C" int g_slotTableA[SURFACE_SLOT_COUNT];           // EN 0x021f39d8 (JP DAT_10f1e670): per-slot value, cleared with the pools
extern "C" int g_slotTableB[SURFACE_SLOT_COUNT];           // EN 0x021f5058 (JP DAT_10f628e8): per-slot value
extern "C" IUnknown *g_slotSurfacesA[SURFACE_SLOT_COUNT];  // EN 0x021f66d8 (JP DAT_10f1c7f0): first surface of each slot
extern "C" IUnknown *g_slotSurfacesB[SURFACE_SLOT_COUNT];  // EN 0x021f7d58 (JP DAT_10f0d5e0): second surface of each slot
extern "C" IUnknown *g_slotSurfacesC[SURFACE_SLOT_COUNT];  // EN 0x021f93d8 (JP DAT_10f60d38): third surface of each slot
extern "C" IUnknown *g_slotSurfacesD[SURFACE_SLOT_COUNT];  // EN 0x021faa58 (JP DAT_10f1a870): fourth surface of each slot
extern "C" int g_vramInUse;                                // EN 0x021fd758 (JP DAT_10f1a86c): VRAM tracked for the slots
extern "C" int g_pooledSurfaceUsed[SURFACE_POOL_SIZE];     // EN 0x021fdd08 (JP DAT_10f1fd90): 1 = pool entry in use

// Clears all texture slot tables and the VRAM total. EN 0x77bf70 (JP 0x10147700).
extern "C" void BufferPool_Clear(void);
// Releases the surface of pool entry `index` and frees the entry. EN 0x7a3f20 (JP 0x10148520).
extern "C" int TexCache_Free(int index);
// Releases the two main surfaces of every texture slot (after a device reset) and clears their tables.
// EN 0x7a5f90 (JP 0x10147760).
extern "C" void BufferPool_ReleaseDynamic(void);

#endif
