// Delayed sound effects (EN build; JP twin: sound_queue.cpp): up to 10 sounds wait before they are played.
#include "sound_delayed.h"
#include "sound_effect_inline.h"

// Sound_PlayDelayed: queues `sound` to play after `delay` frames, at `position` (0 = not positional). When all slots
// are in use, the sound that would play soonest is played now and its slot is reused (its position flag stays as it
// was). Returns 0. Called by the character code (enemy voices, attack sounds).
extern "C" int Sound_PlayDelayed(int sound, int delay, const Vec4 *position)
{
    int soonest = 0;
    int shortest = 999999;
    for (int i = 0; i < DELAYED_SOUND_COUNT; i++)
    {
        if (g_delayedSoundFrames[i] == 0)
        {
            g_delayedSoundFrames[i] = delay;
            g_delayedSoundId[i] = sound;
            if (position != 0)
            {
                g_delayedSoundPositional[i] = 1;
                g_delayedSoundPosition[i].Copy(*position);
                return 0;
            }
            g_delayedSoundPositional[i] = 0;
            return 0;
        }
        if (shortest > g_delayedSoundFrames[i])
        {
            shortest = g_delayedSoundFrames[i];
            soonest = i;
        }
    }
    if (g_delayedSoundPositional[soonest] != 0)
        Sound_PlayEffectInline(g_delayedSoundId[soonest], &g_delayedSoundPosition[soonest], 1.0f);
    else
        Sound_PlayEffect(g_delayedSoundId[soonest], 0, 1.0f);
    g_delayedSoundFrames[soonest] = delay;
    g_delayedSoundId[soonest] = sound;
    return 0;
}
