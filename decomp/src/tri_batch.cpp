// Triangle batches: lists of vertices (three per triangle) collected during a frame and drawn with one texture.

#include "texture_object.h"

#include "tri_batch.h"

extern "C" void FUN_1013f070(int state, int value, int extra);  // render-state switch (0 = blend mode)
#include "render_vars.h"

// Draws the queued triangles with `texture` (instead of the batch's own); nothing happens with fewer than
// three vertices or without a texture. Returns the draw call's result, or 0.
int TriangleBatch::DrawWith(TextureObject *texture)
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
