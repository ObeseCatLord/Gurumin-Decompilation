// Event callback that plays a cue sound (EN build, EN-only: no JP twin found).
#include "small_b23.h"
#include "switches.h"

// The event number that asks for the cue sound.
const int EVENT_CUE = 1;
// Sound effect id of the cue (next to the switch sounds 0x389-0x38c).
const int SOUND_CUE = 0x38d;
// Volume the cue plays at.
const float CUE_SOUND_VOLUME = 1.0f;

// Plays SOUND_CUE (no position, full volume) when `event` is EVENT_CUE. Returns 0.
extern "C" int Event_PlayCueSound(int event)
{
    if (event == EVENT_CUE)
        Sound_PlayEffect(SOUND_CUE, 0, CUE_SOUND_VOLUME);
    return 0;
}
