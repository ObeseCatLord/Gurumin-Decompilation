// Full-screen quad drawn from three textures through a pixel shader (EN build, EN-only: no JP twin).
#include "render.h"
#include "texture_mgr.h"

const float SHADER3_HALF_TEXEL = 0.5f;         // coordinates start half a texel in from the edge
const unsigned int SHADER3_FVF = 0x4c4;        // D3DFVF_XYZRHW | DIFFUSE | SPECULAR | TEX4
const int SHADER3_COLOR_VERTEX_STATE = 0x8d;   // D3DRS_COLORVERTEX
const int SHADER3_DIFFUSE_SOURCE_STATE = 0x91; // D3DRS_DIFFUSEMATERIALSOURCE (1 = D3DMCS_COLOR1)
const int SHADER3_PIXEL_SHADER_SLOT = 2;       // entry of g_renderTargetSurfaces that holds this pixel shader
const unsigned int SHADER3_WHITE = 0xffffffff;
const unsigned int SHADER3_BLACK = 0xff000000;

// TextureObject::DrawThroughShader3: binds this texture to stage 0, `second` to stage 1 and `third` to stage 2, draws a
// quad as large as the back buffer through the pixel shader (all four coordinate sets map the part of the square
// textures the back buffer covers, half a texel in from each edge) and unbinds stages 1 to 3 again. Returns 0. Called
// by the screen transition drawing.
int TextureObject::DrawThroughShader3(TextureObject *second, TextureObject *third)
{
    MultiTexVertex vertices[4];
    g_d3dDevice->SetTexture(0, texture);
    g_d3dDevice->SetTexture(1, second->texture);
    g_d3dDevice->SetTexture(2, third->texture);
    float width = (float)g_backBufferWidth;
    float height = (float)g_backBufferHeight;
    vertices[0].x = 0.0f;
    vertices[0].y = 0.0f;
    vertices[1].x = width;
    vertices[1].y = 0.0f;
    vertices[2].x = 0.0f;
    vertices[2].y = height;
    vertices[3].x = width;
    vertices[3].y = height;
    vertices[0].uv[3].u = vertices[0].uv[2].u = vertices[0].uv[1].u = vertices[0].uv[0].u = SHADER3_HALF_TEXEL / g_maxViewportSize;
    vertices[0].uv[3].v = vertices[0].uv[2].v = vertices[0].uv[1].v = vertices[0].uv[0].v = SHADER3_HALF_TEXEL / g_maxViewportSize;
    vertices[1].uv[3].u = vertices[1].uv[2].u = vertices[1].uv[1].u = vertices[1].uv[0].u =
        ((g_maxViewportSize > width ? width : g_maxViewportSize) - SHADER3_HALF_TEXEL) / g_maxViewportSize;
    vertices[1].uv[3].v = vertices[1].uv[2].v = vertices[1].uv[1].v = vertices[1].uv[0].v = SHADER3_HALF_TEXEL / g_maxViewportSize;
    vertices[2].uv[3].u = vertices[2].uv[2].u = vertices[2].uv[1].u = vertices[2].uv[0].u = SHADER3_HALF_TEXEL / g_maxViewportSize;
    vertices[2].uv[3].v = vertices[2].uv[2].v = vertices[2].uv[1].v = vertices[2].uv[0].v =
        ((g_maxViewportSize > height ? height : g_maxViewportSize) - SHADER3_HALF_TEXEL) / g_maxViewportSize;
    vertices[3].uv[3].u = vertices[3].uv[2].u = vertices[3].uv[1].u = vertices[3].uv[0].u =
        ((g_maxViewportSize > width ? width : g_maxViewportSize) - SHADER3_HALF_TEXEL) / g_maxViewportSize;
    vertices[3].uv[3].v = vertices[3].uv[2].v = vertices[3].uv[1].v = vertices[3].uv[0].v =
        ((g_maxViewportSize > height ? height : g_maxViewportSize) - SHADER3_HALF_TEXEL) / g_maxViewportSize;
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse = SHADER3_WHITE;
    vertices[0].specular = vertices[1].specular = vertices[2].specular = vertices[3].specular = SHADER3_BLACK;
    vertices[0].z = vertices[1].z = vertices[2].z = vertices[3].z = 1.0f;
    vertices[0].rhw = vertices[1].rhw = vertices[2].rhw = vertices[3].rhw = 1.0f;
    RenderState_Set(SHADER3_COLOR_VERTEX_STATE, 1);
    g_d3dDevice->SetFVF(SHADER3_FVF);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(SHADER3_DIFFUSE_SOURCE_STATE, 1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_d3dDevice->SetPixelShader((IDirect3DPixelShader9 *)g_renderTargetSurfaces[SHADER3_PIXEL_SHADER_SLOT]);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(MultiTexVertex));
    g_d3dDevice->SetPixelShader(0);
    if (g_stage1Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(1, 0);
        g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    }
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    return 0;
}
