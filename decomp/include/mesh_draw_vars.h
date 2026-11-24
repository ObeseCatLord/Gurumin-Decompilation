// Globals and helpers of the mesh drawing code (MeshData::Draw, mesh_draw.cpp).
#ifndef GURUMIN_MESH_DRAW_VARS_H
#define GURUMIN_MESH_DRAW_VARS_H

// One entry (0x118 bytes) of the table of vertex shaders the mesh drawing code picks from.
struct MeshShaderSlot
{
    void *shader;              // 0x00: the vertex shader (DAT_10f0ee1c is the first entry's)
    char pad_0004[0x114];
};

extern "C" MeshShaderSlot DAT_10f0ee1c[];   // vertex shader table
extern "C" int DAT_10f65544;                // vertex stream / shader slot the next mesh draw uses (0 only)
extern "C" int DAT_10f655b8;                // first pack added to a mesh's pack range when it is drawn

extern "C" void FUN_1013ef90(int stage, int type, int value);   // 0x1013ef90 Render_SetTextureStageState (cached)
extern "C" int FUN_10161070(int index);                          // 0x10161070 binds texture slot `index` to stage 0

#endif
