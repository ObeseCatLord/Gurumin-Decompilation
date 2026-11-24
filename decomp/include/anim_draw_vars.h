// Globals of AnimModel::Draw: the colour set the wobbling model (root drawMode 0x24) animates, copied from the model
// every frame.
#ifndef GURUMIN_ANIM_DRAW_VARS_H
#define GURUMIN_ANIM_DRAW_VARS_H

#include "vec4.h"

extern "C" int DAT_10f6558c;          // non-zero = models whose root has drawMode 0x24 are drawn with the rainbow wobble
extern "C" Vec4 DAT_10f65888;         // the model's m_024c (all ones while the wobble runs)
extern "C" Vec4 DAT_10f655c8[10];     // copy of the model's m_00b4 colours; the wobble overwrites x/y/z of the odd ones

#endif
