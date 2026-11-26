// Render/device state of the EN build: the Direct3D device, the scene flag, the render state caches, the texture
// binding state and the pooled surface tables. Declared once here (JP: decomp/include/render_vars.h). Texture slots
// (TextureObject, g_textureNames) are in texture_mgr.h.
#ifndef GURUMIN_EN_RENDER_H
#define GURUMIN_EN_RENDER_H

#include <windows.h>
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

extern "C" int g_windowActive;                                   // EN 0x021e8e7c (JP DAT_10f65530): 1 while the game window is active
extern "C" D3DPRESENT_PARAMETERS g_presentParameters;            // EN 0x021f1288 (JP DAT_10f0ff84): present parameters of the device (Reset uses them too)
extern "C" HINSTANCE g_instance;                                 // EN 0x021a3538 (JP DAT_10f140c8): the application instance
extern "C" IDirect3D9 *g_direct3D;                               // EN 0x021a353c (JP DAT_10f141d8): the Direct3D object
extern "C" IDirect3DDevice9 *g_d3dDevice;                        // EN 0x021a3540 (JP DAT_10f1fcf0): the Direct3D device
extern "C" int g_sceneOpen;                                      // EN 0x021a34d0 (JP DAT_10f6547c): 1 while a Direct3D scene is open
extern "C" int g_quitRequested;                                  // EN 0x021e361c (JP DAT_10f6552c): 1 once Windows asked the game to quit
extern "C" int g_renderToBackBuffer;                             // EN 0x021e47d8 (JP DAT_10f65484): non-zero = render to the back buffer (no swap target)
extern "C" unsigned long g_renderStateCache[RENDER_STATE_COUNT]; // EN 0x021e8e88 (JP DAT_10f623d0): last value set for each render state
extern "C" unsigned long g_textureStageStateCache[][TEXTURE_STAGE_STATE_COUNT]; // EN 0x021e9288: last value set per stage and state
extern "C" unsigned long g_samplerStateCache[][TEXTURE_STAGE_STATE_COUNT];  // EN 0x021ed288 (JP DAT_10f141e0): last value set per sampler and state
extern "C" TextureObject *g_stage0Texture;                       // EN 0x021e47ec (JP DAT_10f6550c): texture bound to stage 0
extern "C" TextureObject *g_stage1Texture;                       // EN 0x021e47f0 (JP DAT_10f65510): texture bound to stage 1
extern "C" int g_textures32Bit;                                  // EN 0x021e4814 (JP DAT_10f654a4): non-zero = 32-bit colour textures (else 16-bit)
extern "C" char g_textureNamesAux[TEXTURE_SLOT_COUNT][TEXTURE_NAME_SIZE]; // EN 0x016152d0 (JP DAT_10f8dbb8): second name table, cleared with the first
extern "C" IDirect3DTexture9 *g_pooledSurfaces[SURFACE_POOL_SIZE]; // EN 0x021ff508 (JP DAT_10f1bff0): pooled surfaces (0 = free)
extern "C" int g_pooledSurfaceBytes[SURFACE_POOL_SIZE];          // EN 0x021fed08 (JP DAT_10f1de70): bytes of each pooled surface
extern "C" int g_pooledSurfaceReserved[SURFACE_POOL_SIZE];       // EN 0x021fe508 (JP DAT_10f0cd40): reservation flag of each entry
extern "C" int g_pooledBytesTotal;                               // EN 0x021fd75c (JP DAT_10f0ff80): bytes held by all pooled surfaces

extern "C" int g_backBufferWidth;                                // EN 0x00908b14 (JP DAT_10202b34): back buffer width in pixels
extern "C" int g_backBufferHeight;                               // EN 0x00908b18 (JP DAT_10202b38): back buffer height in pixels
extern "C" float g_screenWidthDivisor;                             // EN 0x00908b20: 640x480 widths are divided by this on a wider screen (1.0 at 4:3): bitmap-font text, HUD digit advances, centred 2D
extern "C" int g_deviceBackBufferWidth;                          // EN 0x00908b0c (JP DAT_10202b24): back buffer width set at device creation
extern "C" int g_deviceBackBufferHeight;                         // EN 0x00908b10 (JP DAT_10202b28): back buffer height set at device creation
// Render target state (JP: decomp/include/render_vars.h).
const int DRAW_LIST_COUNT = 16;               // draw lists that own vertex/index buffers
const int DRAW_LIST_BUFFER_COUNT = 0x46;      // buffers per draw list
const int RENDER_TARGET_SURFACE_COUNT = 8;    // render-target surfaces released on device loss
const int SWAP_TARGET_COUNT = 2;              // swap targets (double-buffered off-screen rendering)
const int SPLIT_SCREEN_LEFT = 1;              // g_splitScreen value: left half
const int SPLIT_SCREEN_RIGHT = 2;             // g_splitScreen value: right half

extern "C" IUnknown *g_drawListBuffers[DRAW_LIST_COUNT][DRAW_LIST_BUFFER_COUNT]; // EN 0x021e3620 (JP DAT_10f0ee00): vertex/index buffers of the draw lists
extern "C" IUnknown *g_renderTargetSurfaces[RENDER_TARGET_SURFACE_COUNT];        // EN 0x021e47a0 (JP DAT_10f1fd04): render-target surfaces
extern "C" IDirect3DSurface9 *g_backBufferSurface;                // EN 0x021a3544 (JP DAT_10f141dc): the back buffer surface
extern "C" D3DVIEWPORT9 g_viewport;                               // EN 0x021a3548 (JP DAT_10f1fd30): the current viewport
extern "C" D3DVIEWPORT9 g_offscreenViewport;                     // EN 0x021a3560 (JP DAT_10f1fd48): the viewport of the off-screen target (TextureObject::BeginRenderTo)
extern "C" D3DVIEWPORT9 g_savedViewport;                         // EN 0x021a35a8 (JP DAT_10f0ece0): viewport restored by the state reset
extern "C" int g_savedWidth;                                      // EN 0x021a35c0 (JP DAT_10f65470): saved width
extern "C" int g_savedHeight;                                     // EN 0x021a35c4 (JP DAT_10f65474): saved height
extern "C" int g_offscreenTargetActive;                           // EN 0x021a35c8 (JP DAT_10f65478): non-zero while an off-screen target is set up (the saved state is kept)
extern "C" IDirect3DSurface9 *g_swapTargets[SWAP_TARGET_COUNT];   // EN 0x021a35cc (JP DAT_10f1fcfc): the two swap target surfaces
extern "C" IDirect3DSurface9 *g_savedRenderTarget;                // EN 0x021a35d4 (JP DAT_10f60d30): saved render target
extern "C" IDirect3DTexture9 *g_swapTextures[SWAP_TARGET_COUNT];  // EN 0x021a35d8 (JP DAT_10f1fd28): textures of the two swap targets
extern "C" IDirect3DSurface9 *g_depthSurface;                     // EN 0x021a35e8 (JP DAT_10f1a864): depth buffer of the back buffer
extern "C" IDirect3DSurface9 *g_swapDepthSurface;                 // EN 0x021a35f4 (JP DAT_10f08d38): depth buffer for the swap targets
extern "C" IDirect3DSurface9 *g_savedDepthSurface;                // EN 0x021a35f8 (JP DAT_10f0d540): saved depth buffer
extern "C" IUnknown *g_dynamicVertexBuffer2D;                     // EN 0x021a3600 (JP DAT_10f654ac): dynamic vertex buffer of the 2D drawing code
extern "C" int g_splitScreen;                                     // EN 0x021e3610 (JP DAT_10f65480): 0 = whole screen, 1 = left half, 2 = right half
extern "C" int g_stageBindingsReset;                              // EN 0x021e3614 (JP DAT_10f6548c): set to 0 whenever the texture stages are unbound by a target reset
// Unbinds the texture stages and restores the saved render target and viewport (render_restore.cpp). EN 0x7a4160 (JP 0x1014b390).
extern "C" void Render_RestoreTarget(void);
// Edge length in pixels of the square depth buffer of the swap targets. EN 0x00908b24.
extern "C" int g_swapDepthSize;
// Format and discard flag of that depth buffer: D3DFMT_D16 and "contents need not be kept".
const D3DFORMAT SWAP_DEPTH_FORMAT = D3DFMT_D16;
// Replaces the swap targets' depth buffer with a square one of `size` pixels. Returns 0. EN 0x7a4110
// (render_swap_depth.cpp).
extern "C" int Render_CreateSwapDepth(int size);
// Presents the back buffer to `window` (ending the open scene first). Returns 1. EN 0x7a6050 (render_swap_depth.cpp).
extern "C" int __stdcall Render_PresentToWindow(HWND window);
// Composes the 2D overlay, ends the scene and presents the device back buffer into the client area of `window`. Returns 1.
// EN 0x7a81a0 (render_present_window.cpp).
extern "C" int __stdcall Render_PresentComposedToWindow(HWND window);
// Unbinds texture stages 0-3. Returns 0. EN 0x7a57c0 (render_present_window.cpp).
extern "C" int Render_UnbindAllTextures(void);
extern "C" int g_swapTargetInUse;                                 // EN 0x021e8d4c (JP DAT_10f65490): swap target in use (0 or 1)
extern "C" int g_swapTargetSaved;                                 // EN 0x021e8d50 (JP DAT_10f65494): swap target in use before a screen transition started
extern "C" int g_transitionsEnabled;                              // EN 0x00908b34 (JP DAT_10203344): non-zero = screen transitions are allowed
extern "C" int g_transitionMode;                                  // EN 0x021a34c0 (JP DAT_10f654c0): transition mode, 0 = none running
extern "C" int g_transitionModeShown;                             // EN 0x021a34b8 (JP DAT_10f654c4): transition mode the frame is drawn with
extern "C" char g_transitionCapturing;                            // EN 0x021a3606 (JP DAT_10f65500): 1 while a transition captures the frame
extern "C" float g_maxViewportSize;                               // EN 0x00908b1c (JP DAT_10202b18): largest viewport size (width and height)
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

// Unbinds texture stage 0 when something is bound there. Render_UnbindTexture0 is the out-of-line copy.
inline void Stage0_Unbind()
{
    if (g_stage0Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(0, 0);
        g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    }
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

// Starts a new Direct3D scene, ending the open one first (inline in the original: part of Render_BeginScene and the
// device recovery).
__forceinline void Render_RestartScene()
{
    if (g_sceneOpen != 0)
        g_d3dDevice->EndScene();
    g_d3dDevice->BeginScene();
    g_sceneOpen = 1;
}

// Ends the open Direct3D scene, if any (inline in the original: part of Render_Present and Render_PresentSimple).
inline void Render_CloseScene()
{
    if (g_sceneOpen != 0)
        g_d3dDevice->EndScene();
    g_sceneOpen = 0;
}

extern "C" void Render_FillRect(int left, int top, int right, int bottom, int colour, float depth, float alpha);  // EN 0x792750: fills a rectangle of the 640x480 screen
extern "C" int g_fadeColourKind3;                // EN 0x00b91950 (JP DAT_104c02e0): colour of screen fade kind 3
extern "C" int g_irisTextureSlot;                // EN 0x00a152bc (JP DAT_104b7224): texture slot of the iris wipe
// Draws the full-screen fade selected by the fade track/volume state (colour fade or iris wipe). EN 0x5defb0 (JP 0x1000dba0).
extern "C" void Fade_Draw(void);
extern "C" int Render_EndScene(void);            // EN 0x79f890 (JP 0x1013ed60): ends the open scene
extern "C" int Render_BeginScene(void);          // EN 0x77c060 (JP 0x1013ed20): starts a new scene
extern "C" int Render_SetSamplerState(unsigned long sampler, unsigned long type, unsigned long value);  // EN 0x7a4ef0 (JP 0x1013f000)
extern "C" int Render_SetRenderState(unsigned long state, unsigned long value);  // EN 0x7a4b90 (JP 0x1013ef30)
extern "C" int Render_SetTextureStageState(unsigned long stage, unsigned long type, unsigned long value);  // EN 0x7a5980
extern "C" void Render_UnbindTexture0(void);     // EN 0x7a5950 (JP 0x10143b50)
extern "C" void Render_UnbindTexture1(void);     // EN 0x7a5920 (JP 0x10143b80)
extern "C" int Render_ComposeFrame(void);        // EN 0x7a60b0 (JP 0x101493f0): finishes the 2D overlay
extern "C" void Render_ResetTarget(void);        // EN 0x77dd40 (JP 0x101439e0)
extern "C" void Render_ResetSwapTarget(void);    // EN 0x77dee0 (JP 0x1014b480)
extern "C" int Render_DrawBackgroundSquare(void);   // EN 0x77c0a0 (JP 0x101441b0): draws the 257x257 black square that is the probe's background
extern "C" int Render_DrawBlueSquare(void);         // EN 0x77c470 (JP 0x10144040): draws the 34x34 blue square in the corner of the target
extern "C" int Render_CreateDevice(void);        // EN 0x7a13e0 (JP 0x10148b00): creates the Direct3D device
extern "C" int SurfacePool_ReleaseAll(void);     // EN 0x7a3ed0 (JP 0x101483a0): releases every pooled surface and clears the pool tables
extern "C" int FileBuffers_Free(void);           // EN 0x7a8a50 (JP 0x101b22b0): frees the two temporary file buffers (not ported)
extern "C" int TexCache_ReleaseAll(void);        // EN 0x7a5ff0 (JP 0x10148350)
extern "C" int Render_Present(void);             // EN 0x7a8080 (JP 0x1015c960)
extern "C" int Render_PresentSimple(void);       // EN 0x7a80f0 (JP 0x1015c9b0)
extern "C" int Texture_FindByName(const char *name);  // EN 0x72c760 (JP 0x10161920)
extern "C" int Render_InitDevice(void);          // EN 0x734790 (JP 0x10161970)

#endif
