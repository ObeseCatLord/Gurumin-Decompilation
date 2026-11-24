// Effect sprites (EN build): the two sprite tables that effects leave behind, queued into the effect vertex batch.
// Ported from the matched JP source decomp/src/effect_sprites.cpp.
#include "effect_manager.h"
#include "texture_mgr.h"

// Render_Command blend modes the sprite blend numbers 2 to 6 stand for.
const int BLEND_SPRITE_2 = 2;
const int BLEND_SPRITE_3 = 10;
const int BLEND_SPRITE_4 = 11;
const int BLEND_SPRITE_5 = 12;
const int BLEND_SPRITE_6 = 13;
// Size of the 2D space the world sprites are projected to.
const float SCREEN_WIDTH_2D = 640.0f;
const float SCREEN_HEIGHT_2D = 480.0f;
// Alpha factor of a sprite definition that does not start at full alpha.
const float HALF_ALPHA = 0.5f;

// Queues sprite `index` of table A: its world position is projected with `view` to the 640x480 screen and the
// square drawn around it with the sprite's blend mode, its alpha fading with the remaining lifetime. Returns 0
// (nothing is drawn unless the definition is a sprite). Called by EffectManager::Draw.
int EffectManager::DrawSpriteA(int index, const Matrix4 &view, const Matrix4 *camera)
{
    int type = g_effectSpritesA[index].type;
    Render_Command(RENDER_BLEND_MODE, 0, 0);
    if (g_effectDefs[type].depthTest != 0)
        Render_Command(RENDER_DEPTH_TEST, 1, 0);
    else
        Render_Command(RENDER_DEPTH_TEST, 0, 0);
    if (g_effectDefs[type].kind == EFFECT_KIND_SPRITE || g_effectDefs[type].kind == EFFECT_KIND_SPRITE_ALT)
    {
        Vec4 screen;
        screen.Copy(Project(g_effectSpritesA[index].position, view));
        float size = g_effectSpritesA[index].size;
        screen.x = screen.x * SCREEN_WIDTH_2D / 2.0f + SCREEN_WIDTH_2D / 2.0f;
        screen.y = SCREEN_HEIGHT_2D / 2.0f - screen.y * SCREEN_HEIGHT_2D / 2.0f;
        int blend = g_effectDefs[type].blend;
        if (blend == 0)
            g_effectBatch.SetBlend(BLEND_ALPHA, 0);
        else if (blend == 1)
            g_effectBatch.SetBlend(BLEND_ADD, 0);
        else if (blend == 2)
            g_effectBatch.SetBlend(BLEND_SPRITE_2, 0);
        else if (blend == 3)
            g_effectBatch.SetBlend(BLEND_SPRITE_3, 0);
        else if (blend == 4)
            g_effectBatch.SetBlend(BLEND_SPRITE_4, 0);
        else if (blend == 5)
            g_effectBatch.SetBlend(BLEND_SPRITE_5, 0);
        else if (blend == 6)
            g_effectBatch.SetBlend(BLEND_SPRITE_6, 0);
        if (g_effectDefs[type].flat)
            screen.z = 0;
        if (g_effectDefs[type].fullAlphaA != 0)
            g_effectBatch.AddQuad(&g_textures[g_effectDefs[type].texture], screen.x - size, screen.y - size,
                                  screen.x + size, screen.y + size, g_effectSpritesA[index].u0,
                                  g_effectSpritesA[index].v0, g_effectSpritesA[index].u1, g_effectSpritesA[index].v1,
                                  screen.z,
                                  (float)(g_effectSpritesA[index].lifetime - g_effectSpritesA[index].age) *
                                      g_effectSpritesA[index].alpha / (float)g_effectSpritesA[index].lifetime,
                                  g_effectSpritesA[index].colour[0], g_effectSpritesA[index].colour[1],
                                  g_effectSpritesA[index].colour[2], g_effectSpritesA[index].colour2[0],
                                  g_effectSpritesA[index].colour2[1], g_effectSpritesA[index].colour2[2],
                                  g_effectSpritesA[index].m_0078);
        else
            g_effectBatch.AddQuad(&g_textures[g_effectDefs[type].texture], screen.x - size, screen.y - size,
                                  screen.x + size, screen.y + size, g_effectSpritesA[index].u0,
                                  g_effectSpritesA[index].v0, g_effectSpritesA[index].u1, g_effectSpritesA[index].v1,
                                  screen.z,
                                  (float)(g_effectSpritesA[index].lifetime - g_effectSpritesA[index].age) *
                                      (g_effectSpritesA[index].alpha * HALF_ALPHA) / (float)g_effectSpritesA[index].lifetime,
                                  g_effectSpritesA[index].colour[0], g_effectSpritesA[index].colour[1],
                                  g_effectSpritesA[index].colour[2], g_effectSpritesA[index].colour2[0],
                                  g_effectSpritesA[index].colour2[1], g_effectSpritesA[index].colour2[2],
                                  g_effectSpritesA[index].m_0078);
    }
    return 0;
}

// The same for sprite `index` of table B, whose position is already in screen coordinates. Returns 0. Called by
// EffectManager::Draw.
int EffectManager::DrawSpriteB(int index, const Matrix4 &view, const Matrix4 *camera)
{
    int type = g_effectSpritesB[index].type;
    Render_Command(RENDER_BLEND_MODE, 0, 0);
    if (g_effectDefs[type].depthTest != 0)
        Render_Command(RENDER_DEPTH_TEST, 1, 0);
    else
        Render_Command(RENDER_DEPTH_TEST, 0, 0);
    if (g_effectDefs[type].kind == EFFECT_KIND_SPRITE || g_effectDefs[type].kind == EFFECT_KIND_SPRITE_ALT)
    {
        Vec4 screen;
        screen.Copy(g_effectSpritesB[index].position);
        float size = g_effectSpritesB[index].size;
        int blend = g_effectDefs[type].blend;
        if (blend == 0)
            g_effectBatch.SetBlend(BLEND_ALPHA, 0);
        else if (blend == 1)
            g_effectBatch.SetBlend(BLEND_ADD, 0);
        else if (blend == 2)
            g_effectBatch.SetBlend(BLEND_SPRITE_2, 0);
        else if (blend == 3)
            g_effectBatch.SetBlend(BLEND_SPRITE_3, 0);
        else if (blend == 4)
            g_effectBatch.SetBlend(BLEND_SPRITE_4, 0);
        else if (blend == 5)
            g_effectBatch.SetBlend(BLEND_SPRITE_5, 0);
        else if (blend == 6)
            g_effectBatch.SetBlend(BLEND_SPRITE_6, 0);
        if (g_effectDefs[type].fullAlphaB != 0)
            g_effectBatch.AddQuad(&g_textures[g_effectDefs[type].texture], screen.x - size, screen.y - size,
                                  screen.x + size, screen.y + size, g_effectSpritesB[index].u0,
                                  g_effectSpritesB[index].v0, g_effectSpritesB[index].u1, g_effectSpritesB[index].v1,
                                  screen.z,
                                  (float)(g_effectSpritesB[index].lifetime - g_effectSpritesB[index].age) *
                                      g_effectSpritesB[index].alpha / (float)g_effectSpritesB[index].lifetime,
                                  g_effectSpritesB[index].colour[0], g_effectSpritesB[index].colour[1],
                                  g_effectSpritesB[index].colour[2], g_effectSpritesB[index].colour2[0],
                                  g_effectSpritesB[index].colour2[1], g_effectSpritesB[index].colour2[2],
                                  g_effectSpritesB[index].m_0078);
        else
            g_effectBatch.AddQuad(&g_textures[g_effectDefs[type].texture], screen.x - size, screen.y - size,
                                  screen.x + size, screen.y + size, g_effectSpritesB[index].u0,
                                  g_effectSpritesB[index].v0, g_effectSpritesB[index].u1, g_effectSpritesB[index].v1,
                                  screen.z,
                                  (float)(g_effectSpritesB[index].lifetime - g_effectSpritesB[index].age) *
                                      (g_effectSpritesB[index].alpha * HALF_ALPHA) / (float)g_effectSpritesB[index].lifetime,
                                  g_effectSpritesB[index].colour[0], g_effectSpritesB[index].colour[1],
                                  g_effectSpritesB[index].colour[2], g_effectSpritesB[index].colour2[0],
                                  g_effectSpritesB[index].colour2[1], g_effectSpritesB[index].colour2[2],
                                  g_effectSpritesB[index].m_0078);
    }
    return 0;
}
