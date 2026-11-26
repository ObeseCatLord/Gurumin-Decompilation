// Per-frame update of the running effects (EN build). No matched JP twin: ported from the EN bytes.
#include "effect_manager.h"

// Frames a fading light lasts beyond the definition's lightFrames.
const int EFFECT_LIGHT_FADE_EXTRA = 10;
// EffectOptions::flags bits.
const int EFFECT_FLAG_FOLLOWS_MATRIX = 1;
const int EFFECT_FLAG_FOLLOWS_OWNER = 2;

// EffectManager::Update: steps every running effect of each list that passes the filters, ages the sprites of both
// tables (freeing them at the end of their lifetime) and advances the fade of the effect lights. Called by the frame
// update (the two callers pass the filter flags).
int EffectManager::Update(int onlyFollowing, int runOwnerEffects, int runPaused)
{
    for (int list = 0; list < EFFECT_LIST_COUNT; list++)
    {
        int index = g_effectListFirst[list];
        if (index > EFFECT_NONE)
        {
            do
            {
                if (g_effectRecords[index].id != EFFECT_NONE
                    && (runOwnerEffects != 0 || (g_effectRecords[index].options.flags & EFFECT_FLAG_FOLLOWS_OWNER) == 0)
                    && (onlyFollowing == 0 || (g_effectRecords[index].options.flags & EFFECT_FLAG_FOLLOWS_MATRIX) != 0)
                    && (g_effectRecords[index].pause == 0 || *g_effectRecords[index].pause == 0 || runPaused != 0))
                    Step(index, list);
                index = g_effectRecords[index].next;
            } while (index > EFFECT_NONE);
        }
    }
    for (int i = 0; i < EFFECT_SPRITE_COUNT; i++)
    {
        EffectSprite *sprite = &g_effectSpritesA[i];
        if (sprite->type != EFFECT_NONE)
        {
            sprite->age++;
            if (sprite->age >= sprite->lifetime)
                sprite->type = EFFECT_NONE;
        }
    }
    for (int i = 0; i < EFFECT_SPRITE_COUNT; i++)
    {
        EffectSprite *sprite = &g_effectSpritesB[i];
        if (sprite->type != EFFECT_NONE)
        {
            sprite->age++;
            if (sprite->age >= sprite->lifetime)
                sprite->type = EFFECT_NONE;
        }
    }
    for (int i = 0; i < EFFECT_LIGHT_COUNT; i++)
    {
        EffectLight *light = &g_effectLights[i];
        if (light->active != 0 && light->age != 0)
        {
            light->age++;
            int fadeFrames = g_effectDefs[light->definition].lightFrames + EFFECT_LIGHT_FADE_EXTRA;
            light->fade = 1.0f - (float)light->age / (float)fadeFrames;
            if (light->age >= fadeFrames)
            {
                light->fade = 0;
                light->age = 0;
                light->active = 0;
            }
        }
    }
    return 0;
}
