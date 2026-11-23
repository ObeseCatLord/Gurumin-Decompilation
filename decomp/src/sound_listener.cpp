// The 3D sound listener follows the camera: once per frame the camera vectors are smoothed (unless locked) and the
// listener position and direction are sent to the sound process.

#include "vec4.h"

#include "camera_vars.h"
#include "char_vars.h"
#include "map_vars.h"
#include "sound_vars.h"

// Sound command 0x235 (app_0002.cpp, declared there with int parameters): the listener's volume, position,
// direction and up vector.
extern "C" int FUN_100017c0(float volume, float x, float y, float z, float dx, float dy, float dz, float ux, float uy,
                            float uz);

// 0x10021c80 (Sound_UpdateListener). Moves the camera position and target 20 % towards their goals (unless
// DAT_104c0354 is set), then places the listener at 0.15 times the point halfway between target and position, with
// the view direction, sends it to the sound process (unless DAT_104c01b0 is set) and keeps a copy (the position
// scaled by 0.15 once more). Returns 0.
extern "C" int FUN_10021c80(void)
{
    if (DAT_104c0354 == 0)
    {
        DAT_118e69e8.Copy(DAT_118e69e8 * 0.8f + DAT_11af3e28 * 0.2f);
        DAT_118e6cb8.Copy(DAT_118e6cb8 * 0.8f + DAT_118e6c78 * 0.2f);
    }
    // the listener (in its own scope: its vectors share the stack with the smoothing temporaries)
    {
        Vec4 direction;
        direction.Copy(DAT_118e6cb8 - DAT_118e69e8);
        direction.Normalized3();
        Vec4 position;
        position.Copy(DAT_118e6cb8 + (DAT_118e69e8 - DAT_118e6cb8) * 0.5f);
        position.Copy(position * 0.15f);
        if (DAT_104c01b0 == 0)
            FUN_100017c0(1.0f, position.x, position.y, position.z, direction.x, direction.y, direction.z, 0, 0, -1.0f);
        DAT_10c67f40.Set(position.x * 0.15f, position.y * 0.15f, position.z * 0.15f, 0);
        DAT_10c67fe0.Set(direction.x, direction.y, direction.z, 0);
    }
    return 0;
}
