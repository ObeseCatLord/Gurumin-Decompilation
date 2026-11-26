// Inline forms of three character helpers (EN build). The original declares Char_StartEffect (EN 0x6a2c20),
// Character_PlaySound (EN 0x649cd0) and KeyPrompt_Show (EN 0x5a2150) as inline functions of a header: the compiler
// expands them at some call sites and calls the out-of-line copies at others. The behaviours that show both forms call
// these inline versions (same bodies as dialogs.cpp, char_state.cpp and char_sound.cpp), which the compiler expands
// the same way. This is the one declaration of the inline forms in decomp_en.
#ifndef GURUMIN_EN_CHAR_INLINE_H
#define GURUMIN_EN_CHAR_INLINE_H

#include "camera.h"
#include "char.h"
#include "dialogs.h"
#include "effect_manager.h"
#include "game_vars.h"
#include "sound_effects.h"

// Characters from this slot up (the enemies and props) are only heard within the hearing range of the camera target.
const int INLINE_FIRST_DISTANT_SLOT = 0x3a;
// Distance within which such a character's sounds are heard.
const float INLINE_HEARING_RANGE = 2500.0f;

// Starts effect `effect` for character slot `index` and records its handle in that slot (as Char_StartEffect).
inline int Char_StartEffectInline(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                                  int value, int frame, const Matrix4 *matrix, const EffectOptions *options)
{
    g_effectManager.Start(effect, position, parent, unused, owner, value, frame, matrix, options,
                          &g_charEffectHandle[index]);
    return 0;
}

// Plays `sound` at character `index`'s position at full volume (as Character_PlaySound).
inline int Character_PlaySoundInline(int index, int sound, int always, int optional)
{
    float distanceSquared =
        (g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
    if (distanceSquared < INLINE_HEARING_RANGE * INLINE_HEARING_RANGE || index < INLINE_FIRST_DISTANT_SLOT ||
        always != 0)
    {
        Vec4 position;
        position.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                     g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                     g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
        if (optional != 0)
        {
            if (g_voicesOff == 0)
            {
                Sound_PlayEffect(sound, &position, 1.0f);
                return 0;
            }
        }
        else
        {
            Sound_PlayEffect(sound, &position, 1.0f);
        }
    }
    return 0;
}

// Shows key prompt `key` over character `index` for `frames` frames (as KeyPrompt_Show).
inline int KeyPrompt_ShowInline(int index, int key, int frames)
{
    if (g_keyPromptShown[index] != 0 && g_keyPromptKey[index] == key)
    {
        g_keyPromptFrames[index] = frames;
        return 0;
    }
    g_keyPromptFrames[index] = frames;
    g_keyPromptShown[index] = 1;
    g_keyPromptKey[index] = key;
    return 0;
}

#endif
