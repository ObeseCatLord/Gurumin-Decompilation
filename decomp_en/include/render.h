// Render/device state of the EN build: the Direct3D device, the scene flag, the render state caches, the texture
// binding state and the pooled surface tables. Declared once here (JP: decomp/include/render_vars.h). Texture slots
// (TextureObject, g_textureNames) are in texture_mgr.h.
#ifndef GURUMIN_EN_RENDER_H
#define GURUMIN_EN_RENDER_H

#include <d3d9.h>

#include "texture_mgr.h"

// Entries of the render state cache (indexed by D3DRENDERSTATETYPE).
const int RENDER_STATE_COUNT = 0x100;
// Entries per stage of the texture stage state cache (indexed by D3DTEXTURESTAGESTATETYPE).
const int TEXTURE_STAGE_STATE_COUNT = 0x100;
// Entries of the pooled surface table.
const int SURFACE_POOL_SIZE = 0x200;

// TextureObject pointer value meaning "unknown binding": the next bind always calls SetTexture.
#define TEXTURE_BINDING_UNKNOWN ((TextureObject *)1)

extern "C" IDirect3DDevice9 *g_d3dDevice;                        // EN 0x021a3540 (JP DAT_10f1fcf0): the Direct3D device
extern "C" int g_sceneOpen;                                      // EN 0x021a34d0 (JP DAT_10f6547c): 1 while a Direct3D scene is open
extern "C" int g_quitRequested;                                  // EN 0x021e361c (JP DAT_10f6552c): 1 once Windows asked the game to quit
extern "C" int g_renderToBackBuffer;                             // EN 0x021e47d8 (JP DAT_10f65484): non-zero = render to the back buffer (no swap target)
extern "C" unsigned long g_renderStateCache[RENDER_STATE_COUNT]; // EN 0x021e8e88 (JP DAT_10f623d0): last value set for each render state
extern "C" unsigned long g_textureStageStateCache[][TEXTURE_STAGE_STATE_COUNT]; // EN 0x021e9288: last value set per stage and state
extern "C" TextureObject *g_stage0Texture;                       // EN 0x021e47ec (JP DAT_10f6550c): texture bound to stage 0
extern "C" TextureObject *g_stage1Texture;                       // EN 0x021e47f0 (JP DAT_10f65510): texture bound to stage 1
extern "C" char g_textureNamesAux[TEXTURE_SLOT_COUNT][TEXTURE_NAME_SIZE]; // EN 0x016152d0 (JP DAT_10f8dbb8): second name table, cleared with the first
extern "C" IUnknown *g_pooledSurfaces[SURFACE_POOL_SIZE];        // EN 0x021ff508 (JP DAT_10f1bff0): pooled surfaces (0 = free)
extern "C" int g_pooledSurfaceBytes[SURFACE_POOL_SIZE];          // EN 0x021fed08 (JP DAT_10f1de70): bytes of each pooled surface
extern "C" int g_pooledSurfaceReserved[SURFACE_POOL_SIZE];       // EN 0x021fe508 (JP DAT_10f0cd40): reservation flag of each entry
extern "C" int g_pooledBytesTotal;                               // EN 0x021fd75c (JP DAT_10f0ff80): bytes held by all pooled surfaces

extern "C" int g_backBufferWidth;                                // EN 0x00908b14 (JP DAT_10202b34): back buffer width in pixels
extern "C" int g_backBufferHeight;                               // EN 0x00908b18 (JP DAT_10202b38): back buffer height in pixels
extern "C" int g_deviceBackBufferWidth;                          // EN 0x00908b0c (JP DAT_10202b24): back buffer width set at device creation
extern "C" int g_deviceBackBufferHeight;                         // EN 0x00908b10 (JP DAT_10202b28): back buffer height set at device creation
extern "C" int g_drawCallCount;                                  // EN 0x021fdd04: draw calls issued this frame

// Inline copies of Render_SetRenderState, Render_SetTextureStageState and Render_UnbindTexture1: the original
// inlines them into the texture drawing functions (the out-of-line versions are declared below).
// Sets a render state unless the cache already holds that value. Returns 1 when the state was (re)written, else 0.
// Render_SetRenderState is the out-of-line copy.
inline int RenderState_Set(unsigned long state, unsigned long value)
{
    if (g_renderStateCache[state] == value)
        return 0;
    unsigned long current;
    g_d3dDevice->GetRenderState((D3DRENDERSTATETYPE)state, &current);
    if (current != value)
        g_d3dDevice->SetRenderState((D3DRENDERSTATETYPE)state, value);
    g_renderStateCache[state] = value;
    return 1;
}

// Sets a texture stage state unless the cache already holds that value. Returns 1 when the state was (re)written,
// else 0. Render_SetTextureStageState is the out-of-line copy.
inline int StageState_Set(unsigned long stage, unsigned long type, unsigned long value)
{
    if (g_textureStageStateCache[stage][type] == value)
        return 0;
    unsigned long current;
    g_d3dDevice->GetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)type, &current);
    if (current != value)
        g_d3dDevice->SetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)type, value);
    g_textureStageStateCache[stage][type] = value;
    return 1;
}

// Unbinds texture stage 1 when something is bound there. Render_UnbindTexture1 is the out-of-line copy.
inline void Stage1_Unbind()
{
    if (g_stage1Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(1, 0);
        g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    }
}

// Ends the open Direct3D scene, if any (inline in the original: part of Render_Present and Render_PresentSimple).
inline void Render_CloseScene()
{
    if (g_sceneOpen != 0)
        g_d3dDevice->EndScene();
    g_sceneOpen = 0;
}

extern "C" int Render_EndScene(void);            // EN 0x79f890 (JP 0x1013ed60): ends the open scene
extern "C" int Render_BeginScene(void);          // EN 0x77c060 (JP 0x1013ed20): starts a new scene
extern "C" int Render_SetRenderState(unsigned long state, unsigned long value);  // EN 0x7a4b90 (JP 0x1013ef30)
extern "C" int Render_SetTextureStageState(unsigned long stage, unsigned long type, unsigned long value);  // EN 0x7a5980
extern "C" void Render_UnbindTexture0(void);     // EN 0x7a5950 (JP 0x10143b50)
extern "C" void Render_UnbindTexture1(void);     // EN 0x7a5920 (JP 0x10143b80)
extern "C" int Render_ComposeFrame(void);        // EN 0x7a60b0 (JP 0x101493f0): finishes the 2D overlay
extern "C" int Render_ResetTarget(void);         // EN 0x77dd40 (JP 0x101439e0)
extern "C" int Render_ResetSwapTarget(void);     // EN 0x77dee0 (JP 0x1014b480)
extern "C" int Render_CreateDevice(void);        // EN 0x7a13e0 (JP 0x10148b00): creates the Direct3D device
extern "C" int TexCache_ReleaseAll(void);        // EN 0x7a5ff0 (JP 0x10148350)
extern "C" int Render_Present(void);             // EN 0x7a8080 (JP 0x1015c960)
extern "C" int Render_PresentSimple(void);       // EN 0x7a80f0 (JP 0x1015c9b0)
extern "C" int Texture_FindByName(const char *name);  // EN 0x72c760 (JP 0x10161920)
extern "C" int Render_InitDevice(void);          // EN 0x734790 (JP 0x10161970)

#endif
