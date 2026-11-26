// Resetting the effect tables when the effect textures are reloaded (EN build, EN-only: no JP twin found).
#include "effect_manager.h"

// Sets the texture slot of every effect definition to -1 (looked up again on demand) and marks every sprite of table A and
// table B free. Returns 0. Called by the texture reload at EN 0x7361c0.
extern "C" int Effects_ForgetTexturesAndSprites(void)
{
    for (int i = 0; i < EFFECT_DEFINITION_COUNT; i++)
        g_effectDefs[i].texture = -1;
    for (int i = 0; i < EFFECT_SPRITE_COUNT; i++)
        g_effectSpritesA[i].type = EFFECT_NONE;
    for (int i = 0; i < EFFECT_SPRITE_COUNT; i++)
        g_effectSpritesB[i].type = EFFECT_NONE;
    return 0;
}
