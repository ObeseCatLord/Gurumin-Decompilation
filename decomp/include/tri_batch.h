// TriangleBatch: a list of vertices (three per triangle) collected during a frame and drawn with one texture and
// blend mode. Methods: tri_batch.cpp (drawing) and texture_mgr.cpp (buffer, blend, adding vertices and quads).
#ifndef GURUMIN_TRI_BATCH_H
#define GURUMIN_TRI_BATCH_H

struct TextureObject;   // texture_object.h
struct ScreenVertex;    // screen_vertex.h

// TriangleBatch::blendMode before any SetBlend.
#define BLEND_MODE_UNSET -1

struct TriangleBatch
{
    ScreenVertex *vertices;  // 0x00: vertex buffer (screen vertices, 0x20 bytes each)
    int capacity;            // 0x04: number of vertices the buffer holds
    int count;               // 0x08: number of vertices queued (three per triangle)
    int pad_000c;
    TextureObject *texture;  // 0x10: texture to draw with (0 = nothing to draw)
    int blendMode;           // 0x14: render mode passed to FUN_1013f070 while drawing (BLEND_MODE_UNSET = none yet)

    int DrawWith(TextureObject *texture);           // 0x10172a40
    int SetBuffer(void *buffer, int capacity);      // 0x10172a80
    int Reset();                                    // 0x10172aa0
    // 0x10172ac0 (screen_vertices.cpp): copies `count` vertices given for a 640x480 screen to the end of the
    // buffer, scaling their positions to the back buffer; returns 1.
    int AppendVertices(const ScreenVertex *source, int count);
    // 0x10172cb0 (tri_batch_quad.cpp): appends a quad (two triangles) from (left, top) to (right, bottom) at depth z with texture
    // coordinates (u0, v0)-(u1, v1), the alpha and two colours; the corners are turned by `rotation` when not 0.
    int AppendQuad(float left, float top, float right, float bottom, float u0, float v0, float u1, float v1, float z,
                    float alpha, float r, float g, float b, float r2, float g2, float b2, float rotation);
    int Flush();                                    // 0x101733f0
    int SetBlend(int mode, int unused);             // 0x10173450
    int AddVertices(TextureObject *texture, void *source, int count);   // 0x10188eb0
    // 0x10188f10: AppendQuad with `texture`, flushing first when the texture changes or the buffer is full.
    int AddQuad(TextureObject *texture, float left, float top, float right, float bottom, float u0, float v0,
                float u1, float v1, float z, float alpha, float r, float g, float b, float r2, float g2, float b2,
                float m_0078);
};

extern "C" TriangleBatch DAT_1021cffc;   // the 2D batch (falling drops, menus)
extern "C" TriangleBatch DAT_1101c804;   // the effect sprite batch

#endif
