// Triangle batches: lists of vertices (three per triangle) collected during a frame and drawn with one texture.

#include "texture_object.h"

struct TriangleBatch
{
    void *vertices;          // 0x00: vertex buffer
    int pad_0004;
    int count;               // 0x08: number of vertices queued (three per triangle)
    int pad_000c;
    TextureObject *texture;  // 0x10: texture to draw with (0 = nothing to draw)
    int blendMode;           // 0x14: render mode passed to FUN_1013f070 while drawing
    int M_10172a40(TextureObject *texture);
    int Flush();
};

extern "C" void FUN_1013f070(int state, int value, int extra);  // render-state switch (0 = blend mode)
extern "C" int DAT_10ff1dec;                                     // batches still pending this frame

// Draws the queued triangles with `texture` (instead of the batch's own); nothing happens with fewer than
// three vertices or without a texture. Returns the draw call's result, or 0.
int TriangleBatch::M_10172a40(TextureObject *texture)
{
    if (count < 3)
        return 0;
    if (texture != 0)
        return texture->DrawTriangles(vertices, count / 3);
    return 0;
}

// Draws the queued triangles with the batch's texture and blend mode, then empties the batch.
// The render state is switched to the batch's mode for the draw and back to 0 afterwards.
int TriangleBatch::Flush()
{
    if (count >= 3)
    {
        if (texture != 0)
        {
            DAT_10ff1dec--;
            FUN_1013f070(0, blendMode, 0);
            texture->DrawTriangles(vertices, count / 3);
            FUN_1013f070(0, 0, 0);
        }
        count = 0;
    }
    return 0;
}
