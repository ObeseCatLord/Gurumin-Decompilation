// Untextured screen rectangles of the EN build (the 2D fills and gradients).
#include "engine.h"
#include "render.h"

// Vertex format of the 2D triangle lists: screen position, diffuse and specular colour, one texture coordinate.
const DWORD QUAD_VERTEX_FORMAT = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
// Bytes of one screen vertex.
const UINT QUAD_VERTEX_STRIDE = 0x20;
// Vertices of a rectangle drawn as a strip.
const int QUAD_VERTEX_COUNT = 4;
// Texture coordinate the unused texture of a rectangle samples (a quarter pixel in from the corner).
const float QUAD_UV_INSET = 0.001f;
// Highest value of the alpha channel, and its shift in a vertex colour.
const int QUAD_ALPHA_MAX = 255;
const int QUAD_ALPHA_SHIFT = 24;
// Specular colour of the vertices: opaque black.
const unsigned int QUAD_SPECULAR = 0xff000000;

// Render_DrawGradientQuad: draws the untextured rectangle from (left, top) to (right, bottom), given in the 640x480
// screen space, blending from colour `topColour` along its top edge to `bottomColour` along its bottom edge at opacity
// `alpha` (0..1) and depth `z`. Returns 1. Called by the arcade result panel drawing.
extern "C" int __stdcall Render_DrawGradientQuad(int left, int top, int right, int bottom, unsigned int topColour,
                                                 unsigned int bottomColour, float z, float alpha)
{
    ScreenVertex quad[QUAD_VERTEX_COUNT];
    Stage0_Unbind();
    quad[0].u = QUAD_UV_INSET;
    quad[0].v = QUAD_UV_INSET;
    quad[1].u = 0;
    quad[1].v = QUAD_UV_INSET;
    quad[2].u = QUAD_UV_INSET;
    quad[2].v = 0;
    quad[3].u = 0;
    quad[3].v = 0;
    float scaleY = (float)g_backBufferHeight / GAME_SCREEN_HEIGHT;
    float scaleX = (float)g_backBufferWidth / GAME_SCREEN_WIDTH;
    float y0 = (float)top * scaleY;
    float y1 = (float)bottom * scaleY;
    float x0 = (float)left * scaleX;
    float x1 = (float)right * scaleX;
    quad[0].y = y0;
    quad[1].y = y0;
    quad[2].y = y1;
    quad[3].y = y1;
    int opacity = (int)(alpha * 255.0f);
    quad[0].x = x0;
    quad[1].x = x1;
    quad[2].x = x0;
    quad[3].x = x1;
    quad[0].x = x0;
    quad[1].x = x1;
    quad[2].x = x0;
    quad[3].x = x1;
    if (opacity > QUAD_ALPHA_MAX)
        opacity = QUAD_ALPHA_MAX;
    else if (opacity < 0)
        opacity = 0;
    quad[0].specular = QUAD_SPECULAR;
    quad[0].diffuse = quad[1].diffuse = (opacity << QUAD_ALPHA_SHIFT) | topColour;
    quad[2].diffuse = quad[3].diffuse = (opacity << QUAD_ALPHA_SHIFT) | bottomColour;
    quad[2].specular = QUAD_SPECULAR;
    quad[1].specular = QUAD_SPECULAR;
    quad[3].specular = QUAD_SPECULAR;
    quad[3].z = z;
    quad[2].z = z;
    quad[1].z = z;
    quad[0].z = z;
    quad[0].rhw = 1.0f;
    quad[1].rhw = 1.0f;
    quad[2].rhw = 1.0f;
    quad[3].rhw = 1.0f;
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(QUAD_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, QUAD_VERTEX_STRIDE);
    return 1;
}
