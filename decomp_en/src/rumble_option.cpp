// Controller vibration option (EN build, EN-only): the wrapper the game calls to start a rumble.
#include "input_rumble.h"

// Starts a rumble of `milliseconds` ms at 1 / `divisor` of the full strength, but only when the player turned vibration
// on in the configuration (USEVB). Always returns 1. Called by the frame and scene code (0x5e75f0, 0x5c3430, 0x6383c0, 0x5c9c50).
extern "C" int __stdcall Rumble_StartIfEnabled(float divisor, int milliseconds)
{
    if (g_vibrationEnabled != 0)
        Input_Rumble(divisor, milliseconds);
    return 1;
}
