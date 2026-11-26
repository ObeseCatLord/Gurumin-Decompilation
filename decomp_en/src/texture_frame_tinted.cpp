// Menu panel frame drawn in a colour (EN build; JP: decomp/src/texture_bind.cpp, TextureObject::DrawFrameAltTinted).
#include "engine.h"
#include "render.h"
#include "texture_mgr.h"

// Vertex format of the frame: screen position, diffuse and specular colour, one texture coordinate.
const DWORD FRAME_VERTEX_FORMAT = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
// Bytes of one screen vertex, the points on the outline of the frame and the triangles of its fan.
const UINT FRAME_VERTEX_STRIDE = 0x20;
const int FRAME_POINT_COUNT = 18;
const int FRAME_FAN_TRIANGLES = 16;
// Bytes of the vertex buffer: the outline points plus the room the original reserves.
const int FRAME_VERTEX_ROOM = 34;
// The frame is drawn this many units inside the rectangle, minus the border; its corners are cut by 2, 4, 8 and 10.
const int FRAME_INSET = 8;
const int FRAME_CUT_SMALL = 2;
const int FRAME_CUT_MEDIUM = 4;
const int FRAME_CUT_LARGE = 8;
const int FRAME_CUT_DIAGONAL = 10;
// Highest value of the alpha channel, and its shift in a vertex colour.
const int FRAME_ALPHA_MAX = 255;
const int FRAME_ALPHA_SHIFT = 24;
// Specular colour of the vertices: opaque black.
const unsigned int FRAME_SPECULAR = 0xff000000;

// TextureObject::DrawFrameAltTinted. Draws the panel frame (an 18 point outline drawn as a triangle fan) of the rectangle
// (left, top)-(right, bottom) of the 640x480 screen, pulled in by FRAME_INSET minus `border`, in the colour `rgb`
// (0xRRGGBB) at opacity `alpha` (0..1) and depth `z`. With `textured` the frame takes this texture's pixels (the texture
// coordinates are the point's position in texels); else texture stage 0 is unbound and the vertex colour is used alone.
// Returns 1. Called by the menu panels.
int TextureObject::DrawFrameAltTinted(int left, int top, int right, int bottom, float z, float alpha, int textured,
                                      int border, unsigned int rgb)
{
    ScreenVertex vertices[FRAME_VERTEX_ROOM];
    if (textured != 0)
    {
        BindStage0();
        StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    else
    {
        Stage0_Unbind();
        StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    float scaleX = (float)g_backBufferWidth / GAME_SCREEN_WIDTH;
    left += FRAME_INSET - border;
    top += FRAME_INSET - border;
    right += border - FRAME_INSET;
    bottom += border - FRAME_INSET;
    float scaleY = (float)g_backBufferHeight / GAME_SCREEN_HEIGHT;
    int opacity = (int)(alpha * 255.0f);
    if (opacity > FRAME_ALPHA_MAX)
        opacity = FRAME_ALPHA_MAX;
    else if (opacity < 0)
        opacity = 0;
    int xs[FRAME_POINT_COUNT] = {left + FRAME_CUT_DIAGONAL, left, left + FRAME_CUT_SMALL, left + FRAME_CUT_MEDIUM,
                                 left + FRAME_CUT_LARGE, right - FRAME_CUT_LARGE, right - FRAME_CUT_MEDIUM,
                                 right - FRAME_CUT_SMALL, right, right, right - FRAME_CUT_SMALL, right - FRAME_CUT_MEDIUM,
                                 right - FRAME_CUT_LARGE, left + FRAME_CUT_LARGE, left + FRAME_CUT_MEDIUM,
                                 left + FRAME_CUT_SMALL, left, left};
    int ys[FRAME_POINT_COUNT] = {top + FRAME_CUT_DIAGONAL, top + FRAME_CUT_LARGE, top + FRAME_CUT_MEDIUM,
                                 top + FRAME_CUT_SMALL, top, top, top + FRAME_CUT_SMALL, top + FRAME_CUT_MEDIUM,
                                 top + FRAME_CUT_LARGE, bottom - FRAME_CUT_LARGE, bottom - FRAME_CUT_MEDIUM,
                                 bottom - FRAME_CUT_SMALL, bottom, bottom, bottom - FRAME_CUT_SMALL,
                                 bottom - FRAME_CUT_MEDIUM, bottom - FRAME_CUT_LARGE, top + FRAME_CUT_LARGE};
    for (int i = 0; i < FRAME_POINT_COUNT; i++)
    {
        float x = (float)xs[i];
        float y = (float)ys[i];
        vertices[i].x = x * scaleX;
        vertices[i].y = y * scaleY;
        vertices[i].diffuse = opacity << FRAME_ALPHA_SHIFT | rgb;
        vertices[i].specular = FRAME_SPECULAR;
        vertices[i].z = z;
        vertices[i].rhw = 1.0f;
        vertices[i].u = x / (float)width;
        vertices[i].v = y / (float)height;
    }
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(FRAME_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, FRAME_FAN_TRIANGLES, vertices, FRAME_VERTEX_STRIDE);
    return 1;
}

// Texture coordinate margin of the stretched frame, in texels: the coordinates start this far (divided by the frame's
// size) before the frame and run over the frame's size plus FRAME_STRETCH_EXTRA.
const float FRAME_STRETCH_MARGIN = 5.0f;
const int FRAME_STRETCH_EXTRA = 15;
// Colour of the vertices below the alpha channel: white.
const unsigned int FRAME_WHITE = 0xffffff;

// TextureObject::DrawFrameAltStretched. Like DrawFrameAlt (the 18 point frame outline of the rectangle (left, top)-
// (right, bottom), pulled in by FRAME_INSET minus `border`, drawn white at opacity `alpha` and depth `z`), but the
// texture is stretched over the frame instead of mapped pixel for pixel: the coordinates run from FRAME_STRETCH_MARGIN
// over the frame's size across its size plus FRAME_STRETCH_EXTRA. `textured` as in DrawFrameAltTinted. Returns 1.
// Called by the screen fade.
int TextureObject::DrawFrameAltStretched(int left, int top, int right, int bottom, float z, float alpha, int textured,
                                         int border)
{
    ScreenVertex vertices[FRAME_VERTEX_ROOM];
    if (textured != 0)
    {
        BindStage0();
        StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    else
    {
        Stage0_Unbind();
        StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    float scaleX = (float)g_backBufferWidth / GAME_SCREEN_WIDTH;
    left += FRAME_INSET - border;
    top += FRAME_INSET - border;
    right += border - FRAME_INSET;
    bottom += border - FRAME_INSET;
    float scaleY = (float)g_backBufferHeight / GAME_SCREEN_HEIGHT;
    int opacity = (int)(alpha * 255.0f);
    if (opacity > FRAME_ALPHA_MAX)
        opacity = FRAME_ALPHA_MAX;
    else if (opacity < 0)
        opacity = 0;
    int xs[FRAME_POINT_COUNT] = {left + FRAME_CUT_DIAGONAL, left, left + FRAME_CUT_SMALL, left + FRAME_CUT_MEDIUM,
                                 left + FRAME_CUT_LARGE, right - FRAME_CUT_LARGE, right - FRAME_CUT_MEDIUM,
                                 right - FRAME_CUT_SMALL, right, right, right - FRAME_CUT_SMALL, right - FRAME_CUT_MEDIUM,
                                 right - FRAME_CUT_LARGE, left + FRAME_CUT_LARGE, left + FRAME_CUT_MEDIUM,
                                 left + FRAME_CUT_SMALL, left, left};
    int ys[FRAME_POINT_COUNT] = {top + FRAME_CUT_DIAGONAL, top + FRAME_CUT_LARGE, top + FRAME_CUT_MEDIUM,
                                 top + FRAME_CUT_SMALL, top, top, top + FRAME_CUT_SMALL, top + FRAME_CUT_MEDIUM,
                                 top + FRAME_CUT_LARGE, bottom - FRAME_CUT_LARGE, bottom - FRAME_CUT_MEDIUM,
                                 bottom - FRAME_CUT_SMALL, bottom, bottom, bottom - FRAME_CUT_SMALL,
                                 bottom - FRAME_CUT_MEDIUM, bottom - FRAME_CUT_LARGE, top + FRAME_CUT_LARGE};
    float uLeft = (float)left;
    float uSpan = (float)(right - left + FRAME_STRETCH_EXTRA);
    float uOffset = FRAME_STRETCH_MARGIN / (float)(right - left);
    float vTop = (float)top;
    float vSpan = (float)(bottom - top + FRAME_STRETCH_EXTRA);
    float vOffset = FRAME_STRETCH_MARGIN / (float)(bottom - top);
    for (int i = 0; i < FRAME_POINT_COUNT; i++)
    {
        float x = (float)xs[i];
        float y = (float)ys[i];
        vertices[i].x = x * scaleX;
        vertices[i].y = y * scaleY;
        vertices[i].diffuse = opacity << FRAME_ALPHA_SHIFT | FRAME_WHITE;
        vertices[i].specular = FRAME_SPECULAR;
        vertices[i].z = z;
        vertices[i].rhw = 1.0f;
        vertices[i].u = (x - uLeft) / uSpan + uOffset;
        vertices[i].v = (y - vTop) / vSpan + vOffset;
    }
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(FRAME_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, FRAME_FAN_TRIANGLES, vertices, FRAME_VERTEX_STRIDE);
    return 1;
}
