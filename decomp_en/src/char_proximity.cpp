// Idle characters that watch for the player (JP twin: char_proximity.cpp).

#include "char.h"
#include "char_proximity.h"

// For an idle character (state 0): plays its idle motion and, when the player is within 10 * values[1] units,
// resets its state (the character notices the player). Called by the behaviour dispatcher for idle characters.
// Returns 0.
extern "C" int Char_IdleWatchPlayer(int index)
{
    float distance2 = (g_characterModels[0].matrix.GetTranslation() - g_characterModels[index].matrix.GetTranslation())
                          .LengthSquared3();
    if (g_charState[index].state == 0)
    {
        g_characterModels[index].PlayMotion(IDLE_MOTION_LAYER, 0, IDLE_MOTION_BLEND, 1.0f);
        if (distance2 < (float)(g_charState[index].values[CHARSTATE_WATCH_RANGE] *
                                g_charState[index].values[CHARSTATE_WATCH_RANGE] * WATCH_RANGE_SCALE))
            Char_ResetState(index);
    }
    return 0;
}
