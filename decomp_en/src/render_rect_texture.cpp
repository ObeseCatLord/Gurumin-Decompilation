// A coloured screen rectangle drawn with a texture object's size (EN build; JP twin: Render_FillRect in engine_10143.cpp).
#include "engine.h"
#include "render.h"
#include "texture_mgr.h"

// Vertex format of the rectangle: screen position, diffuse and specular colour, one texture coordinate.
const DWORD FILL_VERTEX_FORMAT = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
// Bytes of one screen vertex, and the vertices of the strip.
const UINT FILL_VERTEX_STRIDE = 0x20;
const int FILL_VERTEX_COUNT = 4;
// Texture coordinate of the unused edges (a hair in from the corner).
const float FILL_UV_INSET = 0.001f;
// Highest value of the alpha channel, and its shift in a vertex colour.
const int FILL_ALPHA_MAX = 255;
const int FILL_ALPHA_SHIFT = 24;
// Specular colour of the vertices: opaque black.
const unsigned int FILL_SPECULAR = 0xff000000;

// TextureObject::FillRect. Draws the rectangle from (left, top) to (right, bottom), given in the 640x480 screen space, in
// `colour` at opacity `alpha` (0..1) and depth `z`, after unbinding texture stage 0 (stage 0 passes the vertex colour
// through). The texture coordinates run from FILL_UV_INSET to the share of this texture the device back buffer covers (1
// when the texture is not larger). Returns 1. Called by the screen fades.
int TextureObject::FillRect(int left, int top, int right, int bottom, float z, float alpha, unsigned int colour)
{
    ScreenVertex quad[FILL_VERTEX_COUNT];
    Stage0_Unbind();
    float scaleX = (float)g_backBufferWidth / GAME_SCREEN_WIDTH;
    float scaleY = (float)g_backBufferHeight / GAME_SCREEN_HEIGHT;
    float uEnd = 1.0f;
    float vEnd = 1.0f;
    if (g_deviceBackBufferWidth < width)
        uEnd = (float)g_deviceBackBufferWidth / (float)width;
    if (g_deviceBackBufferHeight < height)
        vEnd = (float)g_deviceBackBufferHeight / (float)height;
    quad[3].v = FILL_UV_INSET;
    quad[2].u = FILL_UV_INSET;
    quad[1].v = FILL_UV_INSET;
    quad[0].u = FILL_UV_INSET;
    quad[0].v = FILL_UV_INSET;
    quad[1].y = quad[0].y = (float)top * scaleY;
    quad[3].y = quad[2].y = (float)bottom * scaleY;
    quad[2].x = quad[0].x = (float)left * scaleX;
    quad[3].x = quad[1].x = (float)right * scaleX;
    quad[1].u = quad[3].u = uEnd;
    quad[2].v = quad[3].v = vEnd;
    int opacity = (int)(alpha * 255.0f);
    if (opacity > FILL_ALPHA_MAX)
        opacity = FILL_ALPHA_MAX;
    else if (opacity < 0)
        opacity = 0;
    quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = (opacity << FILL_ALPHA_SHIFT) | colour;
    quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = FILL_SPECULAR;
    quad[0].z = quad[1].z = quad[2].z = quad[3].z = z;
    quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = 1.0f;
    RenderState_Set(D3DRS_CULLMODE, D3DCULL_CCW);
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(FILL_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, FILL_VERTEX_STRIDE);
    return 1;
}
