// SpriteBatch: declared here once (first used by src/effect_sprites.cpp).
#ifndef GURUMIN_SPRITE_BATCH_H
#define GURUMIN_SPRITE_BATCH_H

#include "texture_object.h"

// The effect sprite batch (texture_mgr.cpp Selection).
struct SpriteBatch
{
    int SetBlend(int mode, int unused);          // 0x10173450
    // 0x10188f10: queues a square from (left, top) to (right, bottom) with texture coordinates (u0, v0)-(u1, v1),
    // depth `z`, the alpha and two colours.
    int AddSprite(TextureObject *texture, float left, float top, float right, float bottom, float u0, float v0,
                  float u1, float v1, float z, float alpha, float r, float g, float b, float r2, float g2, float b2,
                  float m_0078);
};

#endif
