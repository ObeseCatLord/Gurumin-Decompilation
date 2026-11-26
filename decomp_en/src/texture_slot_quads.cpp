// Quads drawn from pixel rectangles of a texture slot (EN build, EN-only: no JP twin found).
#include "texture_mgr.h"

// Colour channel values of a white, untinted quad: both colours' red, green and blue.
const float QUAD_WHITE = 1.0f;
const float QUAD_BLACK = 0.0f;

// TriangleBatch::AppendSlotQuad: appends a quad into (left, top)-(right, bottom) showing the pixel rectangle
// (srcLeft, srcTop)-(srcRight, srcBottom) of texture slot `slot`, with texture coordinates worked out from the
// texture's size and a white first colour. A negative slot appends nothing. Returns 1 (0 for a negative slot). No
// direct caller recovered.
int TriangleBatch::AppendSlotQuad(int slot, int left, int top, int right, int bottom, float srcLeft, float srcTop,
                                  float srcRight, float srcBottom, float z, float alpha, float rotation)
{
    if (slot < 0)
        return 0;
    AppendQuad((float)left, (float)top, (float)right, (float)bottom, srcLeft / (float)g_textures[slot].width,
               srcTop / (float)g_textures[slot].height, srcRight / (float)g_textures[slot].width,
               srcBottom / (float)g_textures[slot].height, z, alpha, QUAD_WHITE, QUAD_WHITE, QUAD_WHITE, QUAD_BLACK,
               QUAD_BLACK, QUAD_BLACK, rotation);
    return 1;
}

// Texture_DrawPixelRect: draws the pixel rectangle (srcLeft, srcTop)-(srcRight, srcBottom) of texture slot `slot` at
// (left, top) at its own size, at depth `z`, faded by `alpha` and turned by `angle`; a negative slot draws nothing.
// Returns 1 (0 for a negative slot). No direct caller recovered.
extern "C" int Texture_DrawPixelRect(int slot, int left, int top, float srcLeft, float srcTop, float srcRight,
                                     float srcBottom, float z, float alpha, float angle)
{
    if (slot < 0)
        return 0;
    g_textures[slot].DrawTinted(left, top, (int)(left + srcRight - srcLeft), (int)(top + srcBottom - srcTop),
                                srcLeft / (float)g_textures[slot].width, srcTop / (float)g_textures[slot].height,
                                srcRight / (float)g_textures[slot].width, srcBottom / (float)g_textures[slot].height, z,
                                alpha, angle, 0.0f, ASPECT_STRETCH);
    return 1;
}
