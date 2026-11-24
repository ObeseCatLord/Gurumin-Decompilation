// Declarations used by Scene_DrawFrame (src/scene_draw.cpp) only: the scene's render targets, its debug switch and
// the engine calls it makes. This is the one declaration of each of them: include this header, never redeclare one.
#ifndef GURUMIN_SCENE_DRAW_VARS_H
#define GURUMIN_SCENE_DRAW_VARS_H

#include "matrix4.h"
#include "vec4.h"

extern "C" int DAT_104b7208;      // texture slot of the sheet with the radar markers and the gradient bars
extern "C" int DAT_108360e8;      // texture slot of the first crossfade target
extern "C" int DAT_108360ec;      // texture slot of the second crossfade target
extern "C" int DAT_108360f0;      // texture slot of the screen-size target the scene is drawn into when DAT_10f654e4 is 1
extern "C" int DAT_108360fc;      // texture slot of the minimap target
extern "C" int DAT_104c0238;      // non-zero = the map's visibility cell boxes are drawn near the player
extern "C" unsigned char DAT_103088b0;   // the frame counter (low byte) the outline was last drawn in
extern "C" int DAT_10f65580;      // 1 while the crossfade pass draws
extern "C" float DAT_10f66910;    // the height the radar markers fade around

Matrix4 LookAt(Vec4 eye, Vec4 target, const Vec4 *up, int roll);      // 0x101b1c20
Matrix4 Perspective(float nearPlane, float farPlane, float fieldOfView);   // 0x101b0c10
Matrix4 Multiply(const Matrix4 &left, const Matrix4 &right);          // 0x101b10f0

extern "C" void FUN_1014b480(void);                    // Render_ResetSwapTarget
extern "C" void FUN_101439e0(void);                    // Render_ResetTarget
void FUN_1000f7f0(int *slots, int count);              // Map_DrawTopView (map_view.cpp): the arguments are unused
extern "C" void FUN_10199860(void);                    // RenderQueue_Clear
extern "C" int FUN_101999a0(const Matrix4 *unused, const Matrix4 *view, Vec4 *lights, unsigned int pass);   // DrawList_DrawPass
extern "C" void FUN_10149100(void);                    // SwapTarget_DrawPreview
extern "C" void FUN_10166520(void);                    // ModelBank::DrawLensFlares
extern "C" int FUN_101446b0(void);                     // Render_UnbindTextures
extern "C" int FUN_1000c340(void);                     // Overlay_DrawFullScreen
extern "C" int FUN_1000d6b0(const Matrix4 *view);      // KeyPrompt_DrawAll
int FUN_1000e340();                                    // Debris_Draw
int ResetMeshSlots();                                  // 0x1000d950
extern "C" int FUN_10143e00(int left, int top, int right, int bottom, unsigned int colour, float z, float alpha);   // Render_FillRect

#endif
