// Composition of the finished frame: the full-screen effect (afterimage, skew, ripple) and the final copy of the
// swap target to the back buffer with the colour tint and the fade.

#include <math.h>
#include "d3d9_min.h"
#include "debug_shape.h"
#include "game_vars.h"
#include "mesh_draw_vars.h"
#include "render_overlay.h"
#include "render_state.h"
#include "render_vars.h"

// windows.h-style min/max (each argument is evaluated twice).
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Direct3D render and texture stage states used by the 2D passes.
#define D3DRS_COLORVERTEX 0x8d
#define D3DRS_DIFFUSEMATERIALSOURCE 0x91
#define D3DTSS_COLOROP 1
#define D3DTSS_COLORARG1 2
#define D3DTSS_COLORARG2 3
#define D3DTSS_ALPHAOP 4
#define D3DTSS_ALPHAARG1 5
#define D3DTSS_ALPHAARG2 6
#define D3DTOP_SELECTARG2 3
#define D3DTOP_MODULATE 4
#define D3DTA_DIFFUSE 0
#define D3DTA_TEXTURE 2
#define D3DPT_TRIANGLELIST 4
#define D3DPT_TRIANGLESTRIP 5
#define D3DFMT_INDEX16 101

// Frame-effect quad: texture offsets in texels, the half texel and the one-texel step of the four-tap blur.
#define HALF_TEXEL 0.5f
#define BLEND_INSET 32.5f
#define BLUR_STEP 0.0009765625
// Depth of the full-screen quads.
#define QUAD_DEPTH_FADE 0.99f
#define QUAD_DEPTH_FINAL 0.99999f
// The ripple grid: 10x10 cells of 11x11 vertices; waves of 0.15 per tick moving the texture coordinates by 1/1000.
#define RIPPLE_CELLS 10
#define RIPPLE_VERTICES 11
#define RIPPLE_VERTEX_COUNT 121
#define RIPPLE_INDEX_COUNT 600
#define RIPPLE_WAVE_SPEED 0.15f
#define RIPPLE_WAVE_SIZE 0.001f
#define RIPPLE_GAP 0.02f
#define RIPPLE_LAST_ROW 110
#define RIPPLE_SCALE 10.0f
// Colour channel scale.
#define CHANNEL_SCALE 255.0f

// 0x101493f0 (Render_ComposeFrame). Called by Render_Present at the end of every frame. Applies the frame effect
// DAT_10f65498 to the finished frame and draws the swap target to the back buffer.
extern "C" void FUN_101493f0(void)
{
    OverlayVertex quad[4];
    int savedWidth;
    int savedHeight;
    int clampedWidth;
    int clampedHeight;

    Render_RestoreTextureFilter();
    savedWidth = DAT_10202b24;
    savedHeight = DAT_10202b28;
    clampedWidth = (int)MIN(DAT_10202b18, (float)DAT_10202b24);
    clampedHeight = (int)MIN(DAT_10202b18, (float)DAT_10202b28);
    DAT_10202b24 = savedWidth + DAT_10f654b4;
    DAT_10202b28 = savedHeight + DAT_10f654b8;
    if (DAT_10f65484 == 0)
    {
        if (DAT_10f65498 != FRAME_EFFECT_NONE)
        {
            if (DAT_10f65498 != FRAME_EFFECT_RIPPLE && DAT_10f65498 != FRAME_EFFECT_RIPPLE_WARM)
            {
                FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_WRAP, 0);
                FUN_1013f070(RENDER_ALPHA_TEST, 0, 0);
                if (DAT_10f65498 == FRAME_EFFECT_AFTERIMAGE || DAT_10f65498 == FRAME_EFFECT_BLEND)
                {
                    quad[0].y = quad[0].x = 0;
                    quad[1].y = 0;
                    quad[2].x = 0;
                    quad[3].x = quad[1].x = (float)clampedWidth;
                    quad[3].y = quad[2].y = (float)clampedHeight;
                }
                if (DAT_10f65498 == FRAME_EFFECT_SHAKE)
                {
                    quad[2].x = quad[0].y = 10.0f;
                    quad[1].x = (float)(clampedWidth - 10);
                    quad[2].y = (float)(clampedHeight + 50);
                    quad[1].y = quad[0].x = -50.0f;
                    quad[3].x = (float)(clampedWidth + 50);
                    quad[3].y = (float)(clampedHeight - 10);
                }
                DAT_10f1fcf0->SetTexture(0, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                FUN_10143b80();
                DAT_10f1fcf0->SetTexture(2, 0);
                DAT_10f1fcf0->SetTexture(3, 0);
                float low;
                float right;
                float bottom;
                if (DAT_10f65498 == FRAME_EFFECT_BLEND)
                {
                    low = HALF_TEXEL / DAT_10202b18;
                    quad[0].tex[3].u = quad[0].tex[2].u = quad[0].tex[1].u = quad[0].tex[0].u = low;
                    quad[0].tex[3].v = quad[0].tex[2].v = quad[0].tex[1].v = quad[0].tex[0].v = low;
                    quad[1].tex[3].v = quad[1].tex[2].v = quad[1].tex[1].v = quad[1].tex[0].v = low;
                    quad[2].tex[3].u = quad[2].tex[2].u = quad[2].tex[1].u = quad[2].tex[0].u = low;
                    right = ((float)clampedWidth - HALF_TEXEL) / DAT_10202b18;
                    bottom = ((float)clampedHeight - HALF_TEXEL) / DAT_10202b18;
                }
                else
                {
                    low = BLEND_INSET / DAT_10202b18;
                    quad[0].tex[3].u = quad[0].tex[2].u = quad[0].tex[1].u = quad[0].tex[0].u = low;
                    quad[0].tex[3].v = quad[0].tex[2].v = quad[0].tex[1].v = quad[0].tex[0].v = low;
                    quad[1].tex[3].v = quad[1].tex[2].v = quad[1].tex[1].v = quad[1].tex[0].v = low;
                    quad[2].tex[3].u = quad[2].tex[2].u = quad[2].tex[1].u = quad[2].tex[0].u = low;
                    right = ((float)clampedWidth - BLEND_INSET) / DAT_10202b18;
                    bottom = ((float)clampedHeight - BLEND_INSET) / DAT_10202b18;
                }
                quad[1].tex[3].u = quad[1].tex[2].u = quad[1].tex[1].u = quad[1].tex[0].u = right;
                quad[3].tex[3].u = quad[3].tex[2].u = quad[3].tex[1].u = quad[3].tex[0].u = right;
                quad[2].tex[3].v = quad[2].tex[2].v = quad[2].tex[1].v = quad[2].tex[0].v = bottom;
                quad[3].tex[3].v = quad[3].tex[2].v = quad[3].tex[1].v = quad[3].tex[0].v = bottom;
                quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = (int)DAT_10f6549c << 24 | 0xffffff;
                quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = 0xff000000;
                quad[0].z = quad[1].z = quad[2].z = quad[3].z = 0;
                quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = 1.0f;
                DAT_10f1fcf0->SetPixelShader(0);
                FUN_1013ef30(D3DRS_COLORVERTEX, 1);
                DAT_10f1fcf0->SetFVF(OVERLAY_FVF);
                DAT_10f1fcf0->SetVertexShader(0);
                FUN_1013ef30(D3DRS_DIFFUSEMATERIALSOURCE, 1);
                FUN_1013ef90(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
                FUN_1013ef90(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                FUN_1013ef90(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                FUN_1013ef90(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                FUN_1013ef90(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                FUN_1013ef90(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(OverlayVertex));
                DAT_10202b24 = savedWidth;
                DAT_10202b28 = savedHeight;
                FUN_1013ed60();
                FUN_101439e0();
                FUN_1013ed20();
                DAT_10202b24 = DAT_10202b24 + DAT_10f654b4;
                DAT_10202b28 = DAT_10202b28 + DAT_10f654b8;
            }
            else
            {
                OverlayVertex grid[140];
                short indices[RIPPLE_INDEX_COUNT];
                unsigned int colour;
                int count;
                int row;
                int column;
                int firstOfRow;
                int index;
                FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_WRAP, 0);
                FUN_1013f070(RENDER_ALPHA_TEST, 0, 0);
                DAT_10f1fcf0->SetTexture(0, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                FUN_10143b80();
                DAT_10f1fcf0->SetTexture(2, 0);
                DAT_10f1fcf0->SetTexture(3, 0);
                if (DAT_10f65498 == FRAME_EFFECT_RIPPLE)
                    colour = (int)DAT_10f6549c << 24 | 0xffffff;
                else
                    colour = (int)DAT_10f6549c << 24 | 0xfff0e0;
                float cellWidth = (float)clampedWidth / (DAT_10202b18 * RIPPLE_SCALE);
                float cellHeight = (float)clampedHeight / (DAT_10202b18 * RIPPLE_SCALE);
                index = 0;
                row = 0;
                for (firstOfRow = 0; firstOfRow <= RIPPLE_LAST_ROW; firstOfRow += RIPPLE_VERTICES)
                {
                    float rowY = (float)(row * clampedHeight) / RIPPLE_SCALE;
                    index = firstOfRow;
                    for (column = 0; column <= RIPPLE_CELLS; column++)
                    {
                    OverlayVertex &vertex = grid[index];
                    vertex.y = rowY;
                    vertex.z = 0;
                    vertex.rhw = 1.0f;
                    vertex.x = (float)(column * clampedWidth) / RIPPLE_SCALE;
                    float u = (float)column * cellWidth +
                              (float)sin((float)(unsigned int)DAT_10f65504 * RIPPLE_WAVE_SPEED +
                                         (float)(column - index + row)) * RIPPLE_WAVE_SIZE;
                    vertex.tex[3].u = vertex.tex[2].u = vertex.tex[1].u = vertex.tex[0].u = u;
                    float v;
                    if (DAT_10f65498 == FRAME_EFFECT_RIPPLE || firstOfRow == RIPPLE_LAST_ROW)
                        v = (float)row * cellHeight +
                            (float)cos((float)(unsigned int)DAT_10f65504 * RIPPLE_WAVE_SPEED +
                                       (float)(row - index - column)) * RIPPLE_WAVE_SIZE;
                    else
                        v = ((float)row + RIPPLE_GAP) * cellHeight +
                            (float)cos((float)(unsigned int)DAT_10f65504 * RIPPLE_WAVE_SPEED +
                                       (float)(row - index - column)) * RIPPLE_WAVE_SIZE;
                    vertex.tex[3].v = vertex.tex[2].v = vertex.tex[1].v = vertex.tex[0].v = v;
                    vertex.diffuse = colour;
                    vertex.specular = 0xff000000;
                        index++;
                    }
                    row++;
                }
                count = 0;
                for (row = 0; row < RIPPLE_CELLS; row++)
                {
                    short rowStart = (short)row * RIPPLE_VERTICES;
                    for (column = 0; column < RIPPLE_CELLS; column++)
                    {
                        short cell = (short)column;
                        indices[count++] = cell + RIPPLE_VERTICES + rowStart;
                        indices[count++] = cell + rowStart;
                        indices[count++] = rowStart + 1 + cell;
                        indices[count++] = cell + RIPPLE_VERTICES + rowStart;
                        indices[count++] = rowStart + 1 + cell;
                        indices[count++] = rowStart + RIPPLE_VERTICES + 1 + cell;
                    }
                }
                DAT_10f1fcf0->SetPixelShader(0);
                FUN_1013ef30(D3DRS_COLORVERTEX, 1);
                DAT_10f1fcf0->SetFVF(OVERLAY_FVF);
                DAT_10f1fcf0->SetVertexShader(0);
                FUN_1013ef90(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                FUN_1013ef30(D3DRS_DIFFUSEMATERIALSOURCE, 1);
                FUN_1013ef90(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
                FUN_1013ef90(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                FUN_1013ef90(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                FUN_1013ef90(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                FUN_1013ef90(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                FUN_1013ef90(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_CLAMP, 0);
                FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_CLAMP, 0);
                DAT_10f1fcf0->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, RIPPLE_VERTEX_COUNT, count / 3, indices,
                                                     D3DFMT_INDEX16, grid, sizeof(OverlayVertex));
                FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_WRAP, 0);
                DAT_10202b24 = savedWidth;
                DAT_10202b28 = savedHeight;
                FUN_1013ed60();
                FUN_101439e0();
                FUN_1013ed20();
                DAT_10202b24 = DAT_10202b24 + DAT_10f654b4;
                DAT_10202b28 = DAT_10202b28 + DAT_10f654b8;
            }
        }
        else
        {
            DAT_10202b24 = savedWidth;
            DAT_10202b28 = savedHeight;
            FUN_1013ed60();
            FUN_101439e0();
            FUN_1013ed20();
            DAT_10202b24 = DAT_10202b24 + DAT_10f654b4;
            DAT_10202b28 = DAT_10202b28 + DAT_10f654b8;
            if (DAT_10f654c0 != 0 && DAT_10203344 != 0)
            {
                DAT_10f1fcf0->SetTexture(0, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                FUN_10143b80();
                DAT_10f1fcf0->SetTexture(2, 0);
                DAT_10f1fcf0->SetTexture(3, 0);
                quad[0].y = quad[0].x = 0;
                quad[1].y = 0;
                quad[3].x = quad[1].x = (float)DAT_10202b24;
                quad[2].x = 0;
                quad[3].y = quad[2].y = (float)DAT_10202b28;
                float half = HALF_TEXEL / DAT_10202b18;
                float right = ((float)clampedWidth - HALF_TEXEL) / DAT_10202b18;
                float bottom = ((float)clampedHeight - HALF_TEXEL) / DAT_10202b18;
                quad[0].tex[2].v = quad[0].tex[3].u = quad[0].tex[0].v = quad[0].tex[0].u = half;
                quad[1].tex[2].v = quad[1].tex[0].v = half;
                quad[2].tex[3].u = quad[2].tex[0].u = half;
                quad[1].tex[3].u = quad[1].tex[0].u = right;
                quad[3].tex[3].u = quad[3].tex[0].u = right;
                quad[2].tex[2].v = quad[2].tex[0].v = bottom;
                quad[3].tex[2].v = quad[3].tex[0].v = bottom;
                quad[0].tex[2].u = quad[0].tex[1].u = quad[0].tex[0].u + BLUR_STEP;
                quad[0].tex[3].v = quad[0].tex[1].v = quad[0].tex[0].v + BLUR_STEP;
                quad[1].tex[2].u = quad[1].tex[1].u = quad[1].tex[0].u + BLUR_STEP;
                quad[1].tex[3].v = quad[1].tex[1].v = quad[1].tex[0].v + BLUR_STEP;
                quad[2].tex[2].u = quad[2].tex[1].u = quad[2].tex[0].u + BLUR_STEP;
                quad[2].tex[3].v = quad[2].tex[1].v = quad[2].tex[0].v + BLUR_STEP;
                quad[3].tex[2].u = quad[3].tex[1].u = quad[3].tex[0].u + BLUR_STEP;
                quad[3].tex[3].v = quad[3].tex[1].v = quad[3].tex[0].v + BLUR_STEP;
                quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = RENDER_COLOUR_WHITE;
                quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = 0xff000000;
                quad[0].z = quad[1].z = quad[2].z = quad[3].z = QUAD_DEPTH_FADE;
                quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = 1.0f;
                if (DAT_10f654f8 == 0 || DAT_10f1fd10 == 0)
                {
                    DAT_10f1fcf0->SetPixelShader(0);
                }
                else if (DAT_10f654f8 == 1)
                {
                    Vec4 constant;
                    constant.z = constant.y = constant.x = DAT_10f654fc;
                    constant.w = 0;
                    DAT_10f1fcf0->SetPixelShader(DAT_10f1fd10);
                    DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
                }
                else if (DAT_10f654f8 == 2)
                {
                    Vec4 constant;
                    constant.z = constant.y = constant.x = DAT_10f654fc;
                    constant.w = 0;
                    DAT_10f1fcf0->SetPixelShader(DAT_10f1fd14);
                    DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
                }
                else if (DAT_10f654f8 == 3)
                {
                    DAT_10f1fcf0->SetTexture(1, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    DAT_10f1fcf0->SetTexture(2, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    DAT_10f1fcf0->SetTexture(3, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    Vec4 constant;
                    constant.z = constant.y = constant.x = DAT_10f654fc;
                    constant.w = 0;
                    DAT_10f1fcf0->SetPixelShader(DAT_10f1fd18);
                    DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
                }
                else if (DAT_10f654f8 == 4)
                {
                    DAT_10f1fcf0->SetTexture(1, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    DAT_10f1fcf0->SetTexture(2, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    DAT_10f1fcf0->SetTexture(3, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    Vec4 constant;
                    constant.z = constant.y = constant.x = DAT_10f654fc;
                    constant.w = 0;
                    DAT_10f1fcf0->SetPixelShader(DAT_10f1fd1c);
                    DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
                }
                else if (DAT_10f654f8 == 5)
                {
                    DAT_10f1fcf0->SetTexture(1, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    DAT_10f1fcf0->SetTexture(2, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    DAT_10f1fcf0->SetTexture(3, DAT_10f1fd28[(DAT_10f65490 - 1) & 1]);
                    Vec4 constant;
                    constant.z = constant.y = constant.x = DAT_10f654fc;
                    constant.w = 0;
                    DAT_10f1fcf0->SetPixelShader(DAT_10f1fd20);
                    DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
                }
                FUN_1013f070(RENDER_DEPTH_FUNC_ALWAYS, 1, 0);
                FUN_1013ef30(D3DRS_COLORVERTEX, 1);
                DAT_10f1fcf0->SetFVF(OVERLAY_FVF);
                DAT_10f1fcf0->SetVertexShader(0);
                FUN_1013ef30(D3DRS_DIFFUSEMATERIALSOURCE, 1);
                FUN_1013ef90(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                FUN_1013ef90(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                FUN_1013ef90(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                FUN_1013ef90(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                FUN_1013ef90(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                FUN_1013ef90(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(OverlayVertex));
                Render_RestoreTextureFilter();
                FUN_1013f070(RENDER_DEPTH_FUNC_ALWAYS, 0, 0);
                DAT_10f1fcf0->SetPixelShader(0);
            }
        }
        DAT_10f1fcf0->SetTexture(0, DAT_10f1fd28[DAT_10f65490]);
        FUN_10143b80();
        DAT_10f1fcf0->SetTexture(2, 0);
        DAT_10f1fcf0->SetTexture(3, 0);
        quad[0].y = quad[0].x = 0;
        quad[1].y = 0;
        quad[3].x = quad[1].x = (float)DAT_10202b24;
        quad[2].x = 0;
        quad[3].y = quad[2].y = (float)DAT_10202b28;
        float half = HALF_TEXEL / DAT_10202b18;
        float right = ((float)clampedWidth - HALF_TEXEL) / DAT_10202b18;
        float bottom = ((float)clampedHeight - HALF_TEXEL) / DAT_10202b18;
        quad[0].tex[2].v = quad[0].tex[3].u = quad[0].tex[0].v = quad[0].tex[0].u = half;
        quad[1].tex[2].v = quad[1].tex[0].v = half;
        quad[2].tex[3].u = quad[2].tex[0].u = half;
        quad[1].tex[3].u = quad[1].tex[0].u = right;
        quad[3].tex[3].u = quad[3].tex[0].u = right;
        quad[2].tex[2].v = quad[2].tex[0].v = bottom;
        quad[3].tex[2].v = quad[3].tex[0].v = bottom;
        quad[0].tex[2].u = quad[0].tex[1].u = quad[0].tex[0].u + BLUR_STEP;
        quad[0].tex[3].v = quad[0].tex[1].v = quad[0].tex[0].v + BLUR_STEP;
        quad[1].tex[2].u = quad[1].tex[1].u = quad[1].tex[0].u + BLUR_STEP;
        quad[1].tex[3].v = quad[1].tex[1].v = quad[1].tex[0].v + BLUR_STEP;
        quad[2].tex[2].u = quad[2].tex[1].u = quad[2].tex[0].u + BLUR_STEP;
        quad[2].tex[3].v = quad[2].tex[1].v = quad[2].tex[0].v + BLUR_STEP;
        quad[3].tex[2].u = quad[3].tex[1].u = quad[3].tex[0].u + BLUR_STEP;
        quad[3].tex[3].v = quad[3].tex[1].v = quad[3].tex[0].v + BLUR_STEP;
        int red = (int)MIN(CHANNEL_SCALE, MAX(DAT_10f66978[0].x * CHANNEL_SCALE, 0.0f));
        int green = (int)MIN(CHANNEL_SCALE, MAX(DAT_10f66978[0].y * CHANNEL_SCALE, 0.0f)) << 8;
        int blue = (int)MIN(CHANNEL_SCALE, MAX(DAT_10f66978[0].z * CHANNEL_SCALE, 0.0f)) << 16;
        unsigned int tint;
        if (DAT_10f654c0 == 0 || DAT_10203344 == 0)
            tint = blue | green | red | 0xff000000;
        else
            tint = ((DAT_10f654c4 - DAT_10f654c0) * 0xff) / DAT_10f654c4 << 24 | blue | green | red;
        quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = tint;
        int red2 = (int)MIN(CHANNEL_SCALE, MAX(DAT_10f66978[1].x * CHANNEL_SCALE, 0.0f));
        int green2 = (int)MIN(CHANNEL_SCALE, MAX(DAT_10f66978[1].y * CHANNEL_SCALE, 0.0f)) << 8;
        int blue2 = (int)MIN(CHANNEL_SCALE, MAX(DAT_10f66978[1].z * CHANNEL_SCALE, 0.0f));
        unsigned int tint2 = (0xffffff00 | blue2) << 16 | green2 | red2;
        quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = tint2;
        quad[0].z = quad[1].z = quad[2].z = quad[3].z = QUAD_DEPTH_FINAL;
        quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = 1.0f;
        FUN_1013ef30(D3DRS_COLORVERTEX, 1);
        DAT_10f1fcf0->SetFVF(OVERLAY_FVF);
        DAT_10f1fcf0->SetVertexShader(0);
        FUN_1013ef30(D3DRS_DIFFUSEMATERIALSOURCE, 1);
        FUN_1013ef90(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        FUN_1013ef90(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        FUN_1013ef90(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        FUN_1013ef90(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        FUN_1013ef90(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        FUN_1013ef90(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        FUN_10143b80();
        DAT_10f1fcf0->SetTexture(2, 0);
        DAT_10f1fcf0->SetTexture(3, 0);
        if (DAT_10f654f8 == 0 || DAT_10f1fd10 == 0)
        {
            DAT_10f1fcf0->SetPixelShader(0);
        }
        else if (DAT_10f654f8 == 1)
        {
            Vec4 constant;
            constant.z = constant.y = constant.x = DAT_10f654fc;
            constant.w = 0;
            DAT_10f1fcf0->SetPixelShader(DAT_10f1fd10);
            DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
        }
        else if (DAT_10f654f8 == 2)
        {
            Vec4 constant;
            constant.z = constant.y = constant.x = DAT_10f654fc;
            constant.w = 0;
            DAT_10f1fcf0->SetPixelShader(DAT_10f1fd14);
            DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
        }
        else if (DAT_10f654f8 == 3)
        {
            DAT_10f1fcf0->SetTexture(1, DAT_10f1fd28[DAT_10f65490]);
            DAT_10f1fcf0->SetTexture(2, DAT_10f1fd28[DAT_10f65490]);
            DAT_10f1fcf0->SetTexture(3, DAT_10f1fd28[DAT_10f65490]);
            Vec4 constant;
            constant.z = constant.y = constant.x = DAT_10f654fc;
            constant.w = 0;
            DAT_10f1fcf0->SetPixelShader(DAT_10f1fd18);
            DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
        }
        else if (DAT_10f654f8 == 4)
        {
            DAT_10f1fcf0->SetTexture(1, DAT_10f1fd28[DAT_10f65490]);
            DAT_10f1fcf0->SetTexture(2, DAT_10f1fd28[DAT_10f65490]);
            DAT_10f1fcf0->SetTexture(3, DAT_10f1fd28[DAT_10f65490]);
            Vec4 constant;
            constant.z = constant.y = constant.x = DAT_10f654fc;
            constant.w = 0;
            DAT_10f1fcf0->SetPixelShader(DAT_10f1fd1c);
            DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
        }
        else if (DAT_10f654f8 == 5)
        {
            DAT_10f1fcf0->SetTexture(1, DAT_10f1fd28[DAT_10f65490]);
            DAT_10f1fcf0->SetTexture(2, DAT_10f1fd28[DAT_10f65490]);
            DAT_10f1fcf0->SetTexture(3, DAT_10f1fd28[DAT_10f65490]);
            Vec4 constant;
            constant.z = constant.y = constant.x = DAT_10f654fc;
            constant.w = 0;
            DAT_10f1fcf0->SetPixelShader(DAT_10f1fd20);
            DAT_10f1fcf0->SetPixelShaderConstantF(3, &constant.x, 1);
        }
        FUN_1013f070(RENDER_DEPTH_FUNC_ALWAYS, 1, 0);
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(OverlayVertex));
        FUN_1013f070(RENDER_DEPTH_FUNC_ALWAYS, 0, 0);
        Render_RestoreTextureFilter();
        DAT_10f1fcf0->SetPixelShader(0);
    }
    FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
    int sceneDrawn = 0;
    if (DAT_10f6553c != 0)
        sceneDrawn = ((MenuSceneDrawFn)DAT_10f6553c)(0, 0);
    if (DAT_10f65498 != 0 && (DAT_10f6553c == 0 || sceneDrawn != 0) && DAT_10202b0c > 1)
        DAT_10f65490 = (DAT_10f65490 - 1) & 1;
    DAT_10202b24 = savedWidth;
    DAT_10202b28 = savedHeight;
}
