// Globals and helpers of AnimModel::UpdateFrame (anim_model_step.cpp).
#ifndef GURUMIN_ANIM_STEP_VARS_H
#define GURUMIN_ANIM_STEP_VARS_H

#include "matrix4.h"
#include "vec4.h"

extern "C" void (*DAT_1101d854)(int a, int b, Vec4 *position);   // plays the footstep sound at `position` (set by Game_Startup to 0x1005c2e0)
extern "C" int DAT_10203430;                                     // non-zero = models over the pit area sink (UpdateFrame)

Matrix4 RotationY(float angle);                                  // 0x101b1bb0
Matrix4 TranslationMatrix(float x, float y, float z);            // 0x101b1360
extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin);   // 0x101ae4d0 (View_ContainsPoint, vec4.cpp)

#endif
