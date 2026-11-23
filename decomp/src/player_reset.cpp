// Player movement state reset (used when a map starts or the player respawns).

#include "vec4.h"

#include "camera_vars.h"
#include "char_vars.h"

// Zeroes the player position and velocity, the camera position and target, and the related per-frame state.
// Returns 0.
extern "C" int FUN_1016a8f0(void)
{
    DAT_118e69e8.Fill(0.0f);
    DAT_118e6cb8.Fill(0.0f);
    DAT_11af3e28.Fill(0.0f);
    DAT_118e6c78.Fill(0.0f);
    DAT_1101d888 = 0;
    DAT_1101d898 = 0;
    DAT_1101de18 = 0;
    DAT_1101dde4 = 0;
    DAT_1101de1c = 0;
    DAT_10fabe8c = 0;
    DAT_10faaa80 = 0;
    DAT_10203444 = -100;
    return 0;
}
