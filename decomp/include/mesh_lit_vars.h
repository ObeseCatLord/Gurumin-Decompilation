// Globals and helpers of the lit mesh drawing code (MeshData::DrawLit, mesh_draw_lit.cpp).
#ifndef GURUMIN_MESH_LIT_VARS_H
#define GURUMIN_MESH_LIT_VARS_H

#include "vec4.h"
#include "bounds.h"

struct IUnknownLike;

extern "C" int DAT_10203414;            // non-zero: the plain mode 2 is not drawn with the lit shaders
extern "C" float DAT_10f65598;          // second height of the view (the cel shader's depth constants use it +-20)
extern "C" Vec4 DAT_10f0d5cc;           // colour of the debug box drawing mode 0x1f4
extern "C" Bounds DAT_10f657e8[4];      // the four light boxes the lit shaders take
extern "C" float DAT_10f628d4[4];       // how many lights each of the four boxes holds (as floats)
extern "C" int DAT_10f655a4;            // non-zero: the packs draw with device state 8 set to 2
extern "C" Vec4 DAT_10f65898;           // minimum corner of the first pack's box in the range
extern "C" Vec4 DAT_10f66958;           // maximum corner of the first pack's box in the range

extern "C" int FUN_10147620(void *, Bounds *bounds, int *counts);                                // 0x10147620 (buffer_pool.cpp)
extern "C" int FUN_10147a20(int slot, IUnknownLike **vertexBuffer, IUnknownLike **indexBuffer);   // 0x10147a20 GetMeshBuffers
extern "C" int FUN_101610b0(int index);                                                           // 0x101610b0 binds texture slot `index` to stage 1

#endif
