// Sound_PlayEffect as an inline function (EN build). Some objects contain the body of Sound_PlayEffect (EN 0x63fc10, see
// sound_effects.cpp) inlined into their callers, so the original source must have had it where the compiler could see
// it. This header has the same body for them.
#ifndef GURUMIN_EN_SOUND_EFFECT_INLINE_H
#define GURUMIN_EN_SOUND_EFFECT_INLINE_H

#include <stdio.h>

#include "engine.h"
#include "game_vars.h"
#include "sound_effects.h"

// Same behaviour as Sound_PlayAt: plays sound `name` at world position (x, y, z), looping on channel `loop` (0 = once),
// the position and volume scaled for the sound engine.
inline int Sound_PlayAtInline(const char *name, float x, float y, float z, int loop, float volume)
{
    x = x * 0.15f;
    y = y * 0.15f;
    z = z * 0.15f;
    volume = volume * 0.5f;
    if (g_soundOutputMode == 0)
    {
        if (loop != 0)
            return g_soundArcPlayLooped(MakeDataPath(name), &x, &y, &z, 2, loop, volume);
        return g_soundArcPlay(MakeDataPath(name), &x, &y, &z, 2, 0, volume);
    }
    if (loop != 0)
        return g_soundArcPlayLooped(MakeDataPath(name), &x, &y, &z, 1, loop, volume);
    return g_soundArcPlay(MakeDataPath(name), &x, &y, &z, 1, 0, volume);
}

// Same behaviour as Sound_PlayEffect: plays effect `sound` at `position` (0 = not positional) unless sound effects are
// off; looping effects only start when they are not already playing. Returns the handle of a started loop, else -1.
inline int Sound_PlayEffectInline(int sound, const Vec4 *position, float volume)
{
    char path[PATH_SIZE];
    if (g_soundEffects[sound].notPositional != 0)
        position = 0;
    float scaled;
    if (g_sceneSoundMode != 0)
        scaled = volume * 0.8;
    else if (position == 0)
        scaled = volume * 0.65;
    else
        scaled = volume;
    if (g_soundMuted == 0)
    {
        sprintf(path, "wav\\%s.%s", g_soundEffects[sound].name, g_soundExtension);
        if (position != 0)
        {
            int loop = g_soundEffects[sound].loop;
            int volumePercent = g_soundEffects[sound].volume;
            if (loop != 0)
            {
                float percent = (float)volumePercent;
                if (volumePercent == 0)
                    percent = 100.0f;
                if (g_soundLoopHandle[sound] == SOUND_LOOP_FREE)
                {
                    int handle = Sound_PlayAt(path, position->x, position->y, position->z, loop,
                                              percent * scaled / 100.0f);
                    g_soundLoopHandle[sound] = handle;
                    return handle;
                }
            }
            else
            {
                float percent = (float)volumePercent;
                if (volumePercent == 0)
                    percent = 100.0f;
                Sound_PlayAtInline(path, position->x, position->y, position->z, 0, percent * scaled / 100.0f);
            }
        }
        else
        {
            if (g_soundEffects[sound].loop != 0)
            {
                if (g_soundLoopHandle[sound] == SOUND_LOOP_FREE)
                {
                    int handle = Sound_PlayFlat(path, 0, 0, g_soundEffects[sound].loop, scaled);
                    g_soundLoopHandle[sound] = handle;
                    return handle;
                }
            }
            else
            {
                g_soundArcPlay(MakeDataPath(path), 0, 0, 0, 0, 0, scaled);
            }
        }
    }
    return -1;
}

#endif
