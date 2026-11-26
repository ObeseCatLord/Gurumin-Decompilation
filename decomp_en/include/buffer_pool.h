// Direct3D surface pools of the EN build: per texture slot surface tables, a 0x200 entry pool of reusable buffers
// (declared in render.h) and the VRAM total. Declared once here (JP: decomp/include/render_vars.h).
#ifndef GURUMIN_EN_BUFFER_POOL_H
#define GURUMIN_EN_BUFFER_POOL_H

#include <d3d9.h>
#include "render.h"
#include "vec4.h"

// Texture slots that own surfaces.
const int SURFACE_SLOT_COUNT = 0x5a0;
// Bytes a mesh buffer is rounded up to.
const int MESH_BUFFER_GRANULARITY = 0x100;

extern "C" IDirect3DVertexBuffer9 *g_slotTextureA[SURFACE_SLOT_COUNT];  // EN 0x021f39d8 (JP DAT_10f1a870): vertex buffer of each mesh buffer slot
extern "C" IDirect3DIndexBuffer9 *g_slotTextureB[SURFACE_SLOT_COUNT];   // EN 0x021f5058 (JP DAT_10f60d38): index buffer of each mesh buffer slot
extern "C" IUnknown *g_slotSurfacesA[SURFACE_SLOT_COUNT]; // EN 0x021f66d8 (JP DAT_10f1c7f0): first surface of each slot (counted in the VRAM total)
extern "C" IUnknown *g_slotSurfacesB[SURFACE_SLOT_COUNT]; // EN 0x021f7d58 (JP DAT_10f0d5e0): second surface of each slot
extern "C" int g_slotBytesC[SURFACE_SLOT_COUNT];          // EN 0x021f93d8 (JP DAT_10f60d38): bytes of the first surface, part of the VRAM total
extern "C" int g_slotBytesD[SURFACE_SLOT_COUNT];          // EN 0x021faa58 (JP DAT_10f1a870): more bytes of the first surface, part of the VRAM total
extern "C" int g_slotAge[SURFACE_SLOT_COUNT];              // EN 0x021fc0d8: frames since each slot was last used (counted up by TextureSlots_Age)
extern "C" char g_slotMarked[SURFACE_SLOT_COUNT];         // EN 0x021fd760 (JP DAT_10f60690): per slot mark, cleared when a mesh buffer pair is created
extern "C" int g_vramInUse;                                // EN 0x021fd758 (JP DAT_10f1a86c): VRAM tracked for the slots
extern "C" int g_surfacePoolHead;                          // EN 0x021fdd00: reset to 0 with the surface pool (no other user found)
extern "C" int g_pooledSurfaceUsed[SURFACE_POOL_SIZE];     // EN 0x021fdd08 (JP DAT_10f1fd90): 1 = pool entry in use
extern "C" int g_pooledBytesBudget;                        // EN 0x00909344 (JP DAT_1020340c): bytes the pooled video-memory textures may hold

// Reserves a free entry of the surface pool and returns its index. EN 0x7a1080 (JP 0x10148410).
extern "C" int BufferPool_Alloc(void);
// Clears all texture slot tables and the VRAM total. EN 0x77bf70 (JP 0x10147700).
extern "C" void BufferPool_Clear(void);
// Releases the surface of pool entry `index` and frees the entry (`width` and `height` are passed by callers but
// not used). EN 0x7a3f20 (JP 0x10148520).
extern "C" int TexCache_Free(int index, int width, int height);
// Releases both textures and both surfaces of texture slot `slot` (out-of-range slots are ignored), takes the first
// surface's bytes off the VRAM total and clears the slot's entries. EN 0x77e2b0 (JP 0x101477c0).
extern "C" int Texture_ReleaseSlot(int slot);
// Releases the two main surfaces of every texture slot (after a device reset) and clears their tables.
// EN 0x7a5f90 (JP 0x10147760).
extern "C" void BufferPool_ReleaseDynamic(void);

// Releases all four surfaces of every texture slot and clears the tables. EN 0x77bfd0 (JP 0x10148280).
extern "C" void BufferPool_ReleaseAll(void);
// Resets four vectors to the "empty box" value (-30000 on each axis, w = 1) and their four counts to 0; the first
// argument is not used. EN 0x77d8b0 (JP 0x10147620).
extern "C" int Bounds_ResetFour(void *unused, Vec4 *bounds, int *counts);
// Creates a mesh buffer pair in the first free slot: a vertex buffer and a 16-bit index buffer in system memory, each
// rounded up to the next multiple of 256 bytes. Returns the slot, or -1 when no slot is free or a buffer cannot be
// created. EN 0x77d4b0 (JP 0x10147880).
extern "C" int MeshBuffers_Create(IDirect3DVertexBuffer9 **vertexBuffer, int vertexBytes, IDirect3DIndexBuffer9 **indexBuffer, int indexBytes);

// Empties the surface pool: zeroes the byte total, the head value and the three per-entry tables (see
// surface_pool_reset.cpp). Returns nothing. EN 0x7a1f10. Called by Render_CreateDevice (EN 0x7a13e0).
extern "C" void SurfacePool_Reset(void);

#endif
