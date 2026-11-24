// Sounds played at a character (JP twin: char_sound.cpp).

#include "camera.h"
#include "char.h"
#include "game_vars.h"
#include "sound_effects.h"

// Characters from this slot up (the enemies and props) are only heard within HEARING_RANGE of the camera target.
static const int FIRST_DISTANT_SLOT = 0x3a;
// Distance within which such a character's sounds are heard.
static const float HEARING_RANGE = 2500.0f;

// Plays `sound` at character `index`'s position at full volume. Characters 0x3a and up (the enemies and props) are
// only heard within 2500 units of the camera target unless `always` is set. With `optional` set the sound is skipped
// while the voices are off. Called by the character behaviours for their voices and effects. Returns 0.
extern "C" int Character_PlaySound(int index, int sound, int always, int optional)
{
    float distanceSquared =
        (g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
    if (distanceSquared < HEARING_RANGE * HEARING_RANGE || index < FIRST_DISTANT_SLOT || always != 0)
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
