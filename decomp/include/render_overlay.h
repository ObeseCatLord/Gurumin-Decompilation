// The renderer's frame composition (src/render_compose.cpp): the vertex of the full-screen quads and the
// declarations of the scene and state helpers it calls. One declaration of each: include this header.
#ifndef GURUMIN_RENDER_OVERLAY_H
#define GURUMIN_RENDER_OVERLAY_H

#include "render_device.h"
#include "render_state.h"

// One texture coordinate pair.
struct OverlayTexCoord
{
    float u;
    float v;
};

// Pre-transformed vertex with two colours and four texture coordinate sets (FVF 0x4c4: XYZRHW, DIFFUSE,
// SPECULAR, TEX4), 0x38 bytes. The four-tap blur pixel shaders sample the four sets.
struct OverlayVertex
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int diffuse;
    unsigned int specular;
    OverlayTexCoord tex[4];
};

// FVF of OverlayVertex.
#define OVERLAY_FVF 0x4c4

// DAT_10f65498: the full-screen effect applied to the finished frame.
enum FrameEffect
{
    FRAME_EFFECT_NONE = 0,         // the frame is only drawn to the back buffer (with the fade)
    FRAME_EFFECT_AFTERIMAGE = 1,   // the previous frame blended over it, slightly zoomed
    FRAME_EFFECT_SHAKE = 2,        // the previous frame blended over it, skewed
    FRAME_EFFECT_BLEND = 3,        // the previous frame blended over it, same size
    FRAME_EFFECT_RIPPLE = 4,       // the frame rippled over a 10x10 grid
    FRAME_EFFECT_RIPPLE_WARM = 5   // the same, tinted warm and with a thin gap at the bottom
};

// DAT_10f6553c (the menu scene routine) is called with two unused arguments.
typedef int (*MenuSceneDrawFn)(int, int);

// The pixel shaders of the post effect DAT_10f654f8 = 1..5.
extern "C" void *DAT_10f1fd10;
extern "C" void *DAT_10f1fd14;
extern "C" void *DAT_10f1fd18;
extern "C" void *DAT_10f1fd1c;
extern "C" void *DAT_10f1fd20;

extern "C" void FUN_1013ef30(int state, int value);   // 0x1013ef30 Render_SetRenderState (cached)
extern "C" int FUN_1013ed20(void);                    // 0x1013ed20 Render_BeginScene
extern "C" int FUN_1013ed60(void);                    // 0x1013ed60 Render_EndScene
extern "C" void FUN_101439e0(void);                   // 0x101439e0 Render_ResetTarget

// Sets the texture filter back to linear sampling when the copy swap effect (DAT_10f654c8) is on, else to point.
__forceinline void Render_RestoreTextureFilter()
{
    if (DAT_10f654c8 == 0)
        FUN_1013f070(RENDER_TEXTURE_FILTER, 0, 0);
    else
        FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
}

// 0x101493f0 (Render_ComposeFrame, render_compose.cpp). Called by Render_Present.
extern "C" void FUN_101493f0(void);

#endif
