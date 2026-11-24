// Full-viewport quad drawn through the pixel shader (EN build).
#include "render.h"

const float TEXEL_OFFSET = 1.0f / 512.0f;     // one texel of a 512 texture
const float HALF_TEXEL = 0.5f;                 // coordinates start half a texel in from the edge
const unsigned int FVF_FOUR_SETS = 0x4c4;      // D3DFVF_XYZRHW | DIFFUSE | SPECULAR | TEX4
const int COLOR_VERTEX_STATE = 0x8d;           // D3DRS_COLORVERTEX
const int DIFFUSE_SOURCE_STATE = 0x91;         // D3DRS_DIFFUSEMATERIALSOURCE (1 = D3DMCS_COLOR1)
const int STAGE_COLOR_OP = 1;                  // D3DTSS_COLOROP
const int STAGE_COLOR_ARG1 = 2;                // D3DTSS_COLORARG1
const int STAGE_COLOR_ARG2 = 3;                // D3DTSS_COLORARG2
const int STAGE_ALPHA_OP = 4;                  // D3DTSS_ALPHAOP
const int STAGE_ALPHA_ARG1 = 5;                // D3DTSS_ALPHAARG1
const int STAGE_ALPHA_ARG2 = 6;                // D3DTSS_ALPHAARG2
const int PIXEL_SHADER_SLOT = 1;               // entry of g_renderTargetSurfaces that holds the crossfade pixel shader
const unsigned int OPAQUE_WHITE = 0xffffffff;
const unsigned int OPAQUE_BLACK = 0xff000000;

// Texture coordinates of a vertex for the four sets: the point itself, then one texel (1/512) up-left, left and up of it,
// so the pixel shader can average neighbouring texels.
static void SetOffsetCoordinates(MultiTexVertex &vertex, float u, float v)
{
    vertex.uv[0].u = u;
    vertex.uv[0].v = v;
    vertex.uv[1].u = u - TEXEL_OFFSET;
    vertex.uv[1].v = v - TEXEL_OFFSET;
    vertex.uv[2].u = u - TEXEL_OFFSET;
    vertex.uv[2].v = v;
    vertex.uv[3].u = u;
    vertex.uv[3].v = v - TEXEL_OFFSET;
}

// Binds this texture to stages 0 to 3 and draws a quad covering the largest viewport size through the pixel shader.
// The four texture coordinate sets all cover the texture's square, half a texel in from each edge. The pixel shader is
// cleared afterwards. Returns 0. Called by the screen transition drawing.
int TextureObject::DrawThroughShader()
{
    MultiTexVertex vertices[4];
    g_d3dDevice->SetTexture(0, texture);
    g_d3dDevice->SetTexture(1, texture);
    g_d3dDevice->SetTexture(2, texture);
    g_d3dDevice->SetTexture(3, texture);
    float size = g_maxViewportSize;
    float nearest = HALF_TEXEL / size;
    float farthest = (size - HALF_TEXEL) / size;
    vertices[0].x = 0.0f;
    vertices[0].y = 0.0f;
    vertices[1].x = size;
    vertices[1].y = 0.0f;
    vertices[2].x = 0.0f;
    vertices[2].y = size;
    vertices[3].x = size;
    vertices[3].y = size;
    SetOffsetCoordinates(vertices[0], nearest, nearest);
    SetOffsetCoordinates(vertices[1], farthest, nearest);
    SetOffsetCoordinates(vertices[2], nearest, farthest);
    SetOffsetCoordinates(vertices[3], farthest, farthest);
    for (int i = 0; i < 4; i++)
    {
        vertices[i].diffuse = OPAQUE_WHITE;
        vertices[i].specular = OPAQUE_BLACK;
        vertices[i].z = 0.0f;
        vertices[i].rhw = 1.0f;
    }
    RenderState_Set(COLOR_VERTEX_STATE, 1);
    g_d3dDevice->SetFVF(FVF_FOUR_SETS);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(DIFFUSE_SOURCE_STATE, 1);
    StageState_Set(0, STAGE_ALPHA_OP, D3DTOP_MODULATE);
    StageState_Set(0, STAGE_ALPHA_ARG1, D3DTA_TEXTURE);
    StageState_Set(0, STAGE_ALPHA_ARG2, D3DTA_DIFFUSE);
    StageState_Set(0, STAGE_COLOR_OP, D3DTOP_MODULATE);
    StageState_Set(0, STAGE_COLOR_ARG1, D3DTA_TEXTURE);
    StageState_Set(0, STAGE_COLOR_ARG2, D3DTA_DIFFUSE);
    g_d3dDevice->SetPixelShader((IDirect3DPixelShader9 *)g_renderTargetSurfaces[PIXEL_SHADER_SLOT]);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(MultiTexVertex));
    g_d3dDevice->SetPixelShader(0);
    return 0;
}
