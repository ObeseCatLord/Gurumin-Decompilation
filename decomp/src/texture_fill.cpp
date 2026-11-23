// Texture objects: the speech-bubble panel fill.
#include "d3d9_min.h"

#include "texture_object.h"
#include "texture_bind.h"

#include "render_vars.h"

extern "C" void FUN_1013ef30(int state, int value);                   // Render_SetRenderState (cached)
extern "C" void FUN_1013ef90(int stage, int type, int value);         // Render_SetTextureStageState (cached)
extern "C" void FUN_10143b50(void);                                   // Render_UnbindTexture0

#define FILL_POINTS 40
#define FILL_RING_POINTS 20
#define FILL_SHADE_RING_STRIDE 17
#define FILL_SHADE_COLOUR 0xffffff

// 0x1014ce40 (TextureObject::DrawFill). Fills a speech-bubble panel: a triangle strip zigzagging between an inner
// ring and an outer ring of 20 points each, the rectangle pulled 8 - `border` pixels inwards. The inner ring runs a
// cut-corner outline with a tail bulging below the bottom edge at its middle, the outer ring is the same outline
// 3 pixels further out with a longer tail. Points are given in 640x480 screen coordinates and scaled to the back
// buffer; the texture is mapped 1 / 64 per pixel. The outer ring has alpha `alpha` (0..1), the inner ring none.
// Draws untextured when `textured` is 0. Returns 1.
int TextureObject::DrawFill(int left, int top, int right, int bottom, float z, float alpha, int textured, int border)
{
    PanelVertex vertices[FILL_POINTS];
    int source;
    if (textured != 0)
    {
        BindStage0();
        FUN_1013ef90(0, 1, 4);
        FUN_1013ef90(0, 2, 2);
        FUN_1013ef90(0, 3, 0);
        FUN_1013ef90(0, 4, 4);
        source = 2;                         //   texture
    }
    else
    {
        FUN_10143b50();
        FUN_1013ef90(0, 1, 4);
        FUN_1013ef90(0, 2, 2);
        FUN_1013ef90(0, 3, 0);
        FUN_1013ef90(0, 4, 4);
        source = 0;                         //   diffuse
    }
    FUN_1013ef90(0, 5, source);
    FUN_1013ef90(0, 6, 0);                  //   diffuse
    float scaleX = (float)DAT_10202b34 / 640.0f;
    left += 8 - border;
    top += 8 - border;
    right += border - 8;
    bottom += border - 8;
    int innerRight = right - 3;
    int innerBottom = bottom - 4;
    int innerLeft = left + 3;
    float scaleY = (float)DAT_10202b38 / 480.0f;
    int a = (int)(alpha * 255.0f);
    if (a > 255)
        a = 255;
    else if (a < 0)
        a = 0;
    int tailX = (innerRight + innerLeft) / 2;
    int outerTailX = (right + left) / 2;
    int ys[FILL_POINTS] = {top + 11, top + 7, top + 5, top + 3, top + 3, top + 5, top + 7, top + 11, bottom - 12,
                           bottom - 8, bottom - 6, innerBottom, innerBottom, bottom + 13 + border, innerBottom,
                           innerBottom, bottom - 6, bottom - 8, bottom - 12, top + 11,
                           top + 8, top + 4, top + 2, top, top, top + 2, top + 4, top + 8, bottom - 8,
                           innerBottom, bottom - 2, bottom, bottom, bottom + 20 + border, bottom, bottom,
                           bottom - 2, innerBottom, bottom - 8, top + 8};
    int xs[FILL_POINTS] = {innerLeft, left + 5, left + 7, left + 11, right - 11, right - 7, right - 5, innerRight,
                           innerRight, right - 5, right - 7, right - 11, tailX + 4 + border, tailX,
                           tailX - border - 4, left + 11, left + 7, left + 5, innerLeft, innerLeft,
                           left, left + 2, left + 4, left + 8, right - 8, right - 4, right - 2, right,
                           right, right - 2, right - 4, right - 8, outerTailX + 6 + border, outerTailX,
                           outerTailX - border - 6, left + 8, left + 4, left + 2, left, left};
    int shades[FILL_POINTS] = {1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3, 3, 3, 3, 1, 1, 1, 1,
                               1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 1, 1, 1, 1, 1, 1};
    unsigned int colours[4] = {FILL_SHADE_COLOUR, FILL_SHADE_COLOUR, FILL_SHADE_COLOUR, FILL_SHADE_COLOUR};
    for (int i = 0; i < FILL_POINTS; i++)
    {
        int ring = i & 1;
        int pair = i / 2;
        int point = pair + ring * FILL_RING_POINTS;
        int x = xs[point];
        int y = ys[point];
        vertices[i].x = (float)x * scaleX;
        vertices[i].y = (float)y * scaleY;
        vertices[i].u = (float)x / 64.0f;
        vertices[i].v = (float)y / 64.0f;
        if (ring == 0)
            vertices[i].colour = colours[shades[pair]];
        else
            vertices[i].colour = colours[shades[ring * FILL_SHADE_RING_STRIDE + pair]] | a << 24;
        vertices[i].specular = 0xff000000;
        vertices[i].z = z;
        vertices[i].rhw = 1.0f;
    }
    FUN_1013ef30(0xe, 1);                   // D3DRS_ZWRITEENABLE
    FUN_1013ef30(0x8d, 1);                  // D3DRS_COLORVERTEX
    DAT_10f1fcf0->SetFVF(0x1c4);
    DAT_10f1fcf0->SetVertexShader(0);
    FUN_1013ef30(0x91, 1);                  // D3DRS_DIFFUSEMATERIALSOURCE = D3DMCS_COLOR1
    DAT_10f0d5c8++;
    DAT_10f1fcf0->DrawPrimitiveUP(5 /* D3DPT_TRIANGLESTRIP */, 38, vertices, 0x20);
    return 1;
}
