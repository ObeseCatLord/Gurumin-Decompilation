// Sound effects played at a character's position.
#include "world.h"

#include "vec4.h"
#include "anim_model.h"

extern "C" Vec4 DAT_118e6cb8;  // the player's position here (player_reset.cpp calls it the player velocity)
extern "C" char DAT_104c0332;  // when set, sounds requested with `optional` are skipped
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);  // Sound_PlayEffect

// 0x10065b10 (Character_PlaySound). Plays `sound` at character `index`'s position at full volume. Characters 0x3a
// and up (the enemies and props) are only heard within 2500 units of the player unless `always` is set. With
// `optional` set the sound is skipped while DAT_104c0332 is set. Returns 0.
extern "C" int FUN_10065b10(int index, int sound, int always, int optional)
{
    float distanceSquared = (DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3();
    if (distanceSquared < 2500.0f * 2500.0f || index < 0x3a || always != 0)
    {
        Vec4 position;
        position.Set(DAT_10db6ee8[index].matrix._41, DAT_10db6ee8[index].matrix._42, DAT_10db6ee8[index].matrix._43, 0);
        if (optional != 0)
        {
            if (DAT_104c0332 == 0)
            {
                FUN_1003ad20(sound, &position, 1.0f);
                return 0;
            }
        }
        else
        {
            FUN_1003ad20(sound, &position, 1.0f);
        }
    }
    return 0;
}
