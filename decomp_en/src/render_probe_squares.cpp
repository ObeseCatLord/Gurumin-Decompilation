// The two squares the render target capability probe draws (EN build). Ported from the Ghidra output and the EN bytes (the
// JP twins, APP.dll 0x101441b0 and 0x10144040, are not matched). The probe (render_target_probe.cpp) draws the black
// background square, then the blue corner square, and reads both back from the 256x256 target "crtcc".
#include "engine.h"
#include "render.h"

// Vertex format of the squares: screen position, diffuse and specular colour, one texture coordinate.
const DWORD PROBE_VERTEX_FORMAT = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
// Bytes of one square vertex.
const UINT PROBE_VERTEX_STRIDE = 0x20;
// Vertices of a square drawn as a strip.
const int PROBE_VERTEX_COUNT = 4;
// The squares start one pixel before the target's first pixel and end past its last (so every pixel is covered).
const float PROBE_SQUARE_START = -1.0f;
// Square edge ends: the background covers the whole 256x256 target, the corner square 32 pixels of it.
const float PROBE_BACKGROUND_END = 256.0f;
const float PROBE_CORNER_END = 33.0f;
// Texture coordinate the unused texture of a square samples (a quarter pixel in from the corner).
const float PROBE_UV_INSET = 0.001f;
// Colours of the probe: opaque black and opaque blue (ARGB).
const unsigned int PROBE_BLACK = 0xff000000;
const unsigned int PROBE_BLUE = 0xff0000ff;
// Depth of the square vertices (nearest), and their w.
const float PROBE_DEPTH = 0.0f;
const float PROBE_RHW = 1.0f;

// Draws the background square of the probe: the whole 256x256 target (from -1 to 256 on both axes) with black diffuse
// and blue specular colour, using colour-only blending (alpha and colour from the vertex colour only, no texture),
// then ends the scene so the target can be read. Returns 1. Called by the capability probe (render_target_probe.cpp).
extern "C" int Render_DrawBackgroundSquare(void)
{
    ScreenVertex quad[PROBE_VERTEX_COUNT];
    Stage0_Unbind();
    quad[0].x = PROBE_SQUARE_START;
    quad[0].y = PROBE_SQUARE_START;
    quad[1].x = PROBE_BACKGROUND_END;
    quad[1].y = PROBE_SQUARE_START;
    quad[2].x = PROBE_SQUARE_START;
    quad[2].y = PROBE_BACKGROUND_END;
    quad[3].x = PROBE_BACKGROUND_END;
    quad[3].y = PROBE_BACKGROUND_END;
    quad[0].u = PROBE_UV_INSET;
    quad[0].v = PROBE_UV_INSET;
    quad[1].u = 1.0f;
    quad[1].v = PROBE_UV_INSET;
    quad[2].u = PROBE_UV_INSET;
    quad[2].v = 1.0f;
    quad[3].u = 1.0f;
    quad[3].v = 1.0f;
    quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = PROBE_BLACK;
    quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = PROBE_BLUE;
    quad[0].z = quad[1].z = quad[2].z = quad[3].z = PROBE_DEPTH;
    quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = PROBE_RHW;
    RenderState_Set(D3DRS_CULLMODE, D3DCULL_CCW);
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(PROBE_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, PROBE_VERTEX_STRIDE);
    Render_Command(RENDER_FLUSH_SCENE, 0, 0);
    return 1;
}

// Draws the corner square of the probe: the first 32 pixels of the target (from -1 to 33 on both axes) with blue
// diffuse and black specular colour, with the same colour-only blending, then ends the scene. Returns 1. Called by the
// capability probe (render_target_probe.cpp) after it drew the background square.
extern "C" int Render_DrawBlueSquare(void)
{
    ScreenVertex quad[PROBE_VERTEX_COUNT];
    Stage0_Unbind();
    quad[0].x = PROBE_SQUARE_START;
    quad[0].y = PROBE_SQUARE_START;
    quad[1].x = PROBE_CORNER_END;
    quad[1].y = PROBE_SQUARE_START;
    quad[2].x = PROBE_SQUARE_START;
    quad[2].y = PROBE_CORNER_END;
    quad[3].x = PROBE_CORNER_END;
    quad[3].y = PROBE_CORNER_END;
    quad[0].u = PROBE_UV_INSET;
    quad[0].v = PROBE_UV_INSET;
    quad[1].u = 1.0f;
    quad[1].v = PROBE_UV_INSET;
    quad[2].u = PROBE_UV_INSET;
    quad[2].v = 1.0f;
    quad[3].u = 1.0f;
    quad[3].v = 1.0f;
    quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = PROBE_BLUE;
    quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = PROBE_BLACK;
    quad[0].z = quad[1].z = quad[2].z = quad[3].z = PROBE_DEPTH;
    quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = PROBE_RHW;
    RenderState_Set(D3DRS_CULLMODE, D3DCULL_CCW);
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(PROBE_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, PROBE_VERTEX_STRIDE);
    Render_Command(RENDER_FLUSH_SCENE, 0, 0);
    return 1;
}
