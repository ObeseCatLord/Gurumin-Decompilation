// Object shadows drawn together with a shadow caster (EN build, EN-only: no JP twin found).
#ifndef GURUMIN_EN_SHADOW_OBJECTS_H
#define GURUMIN_EN_SHADOW_OBJECTS_H

#include "char.h"
#include "node.h"

// Objects farther than this (x/y distance) from the shadow's centre are left out of the shadow pass.
const float SHADOW_OBJECT_RANGE = 800.0f;
// x position at or below which a model is not in the world.
const float MODEL_NOT_IN_WORLD_X = -100000.0f;

// Draws the node tree of every object slot (OBJECT_SLOT_FIRST up to ENEMY_SLOT_END) whose model is in the world and lies
// within SHADOW_OBJECT_RANGE of the shadow centre (shadowX, shadowY, shadowZ), except the caster `model` itself, with
// Node::Draw and the shadow pass arguments. `a3` is not used. Returns 1. Not called directly. EN 0x5e0430
// (shadow_texture.cpp).
extern "C" int ShadowPass_DrawNearbyObjects(DrawList *list, Matrix4 *view, int a3, const Matrix4 *shadowMatrix, float shadowX,
                                            float shadowY, float shadowZ, int shadowTexture, float shadowScale, int shadowSize,
                                            const Vec4 *lightDirection, float modelSize, AnimModel *model, float alpha);

#endif
