// Sound effect play functions (EN build). JP twins: sound_effects.cpp, sound_play.cpp, app_0013.cpp.
#include <stdio.h>

#include "engine.h"
#include "game_vars.h"
#include "sound_effects.h"

// World coordinates are scaled into the sound engine's space; volumes are halved for the sound engine.
const float SOUND_WORLD_SCALE = 0.15f;
const float SOUND_ENGINE_VOLUME = 0.5f;
// Output mode values passed to the sound engine for positional sounds.
const int SOUND_MODE_ONE = 1;
const int SOUND_MODE_TWO = 2;
// Volume factors of the effect play functions.
const double VOLUME_CUTSCENE = 0.8;      // while g_sceneSoundMode is set
const double VOLUME_FLAT = 0.65;         // not positional
const double VOLUME_POSITIONAL = 0.95;   // positional, outside a cut-scene
const float PERCENT_FULL = 100.0f;       // an effect volume of 0 means this
// Handle value of a returned "no handle".
const int SOUND_NO_HANDLE = -1;

// Plays sound `name` at world position (x, y, z), looping on channel `loop` (0 = play once), at `volume`. The
// position is scaled for the sound engine. Called by the effect functions below.
extern "C" int Sound_PlayAt(const char *name, float x, float y, float z, int loop, float volume)
{
    x = x * SOUND_WORLD_SCALE;
    y = y * SOUND_WORLD_SCALE;
    z = z * SOUND_WORLD_SCALE;
    volume = volume * SOUND_ENGINE_VOLUME;
    if (g_soundOutputMode == 0)
    {
        if (loop != 0)
            return g_soundArcPlayLooped(MakeDataPath(name), &x, &y, &z, SOUND_MODE_TWO, loop, volume);
        return g_soundArcPlay(MakeDataPath(name), &x, &y, &z, SOUND_MODE_TWO, 0, volume);
    }
    if (loop != 0)
        return g_soundArcPlayLooped(MakeDataPath(name), &x, &y, &z, SOUND_MODE_ONE, loop, volume);
    return g_soundArcPlay(MakeDataPath(name), &x, &y, &z, SOUND_MODE_ONE, 0, volume);
}

// Plays sound `name` without position, looping on channel `loop` when it is non-zero. The two unused arguments are
// ignored. Called by Sound_PlayEffect.
extern "C" int Sound_PlayFlat(const char *name, int unused0, int unused1, int loop, float volume)
{
    if (loop != 0)
        return g_soundArcPlayLooped(MakeDataPath(name), 0, 0, 0, 0, loop, volume);
    return g_soundArcPlay(MakeDataPath(name), 0, 0, 0, 0, 0, volume);
}

// Sound_PlayWav: plays "wav\<name>.<extension>" at `position` (0 = not positional) unless sound effects are off. The
// volume is scaled first: by 0.8 for positional sounds in a cut-scene, otherwise by 0.95 (positional) or 0.65 (not
// positional). Returns -1. Called through the script command table.
extern "C" int Sound_PlayWav(const char *name, const Vec4 *position, float volume)
{
    char path[PATH_SIZE];
    if (g_sceneSoundMode != 0)
    {
        if (position != 0)
            volume = volume * VOLUME_CUTSCENE;
    }
    else
    {
        if (position == 0)
            volume = volume * VOLUME_FLAT;
        else
            volume = volume * VOLUME_POSITIONAL;
    }
    if (g_soundMuted == 0)
    {
        sprintf(path, "wav\\%s.%s", name, g_soundExtension);
        if (position != 0)
            Sound_PlayAt(path, position->x, position->y, position->z, 0, volume);
        else
            g_soundArcPlay(MakeDataPath(path), 0, 0, 0, 0, 0, volume);
    }
    return SOUND_NO_HANDLE;
}

// Sound_PlayEffect: plays sound effect `sound` of the effect table at `position` (0 = not positional), unless sound
// effects are off. The volume is lowered like in Sound_PlayWav and, for positional sounds, multiplied by the
// effect's own volume percent. A looping effect only starts when it is not already playing; its handle is kept in
// g_soundLoopHandle and returned. Returns -1 otherwise. Called through the script command table.
extern "C" int Sound_PlayEffect(int sound, const Vec4 *position, float volume)
{
    char path[PATH_SIZE];
    SoundEffectDef &effect = g_soundEffects[sound];
    if (effect.notPositional != 0)
        position = 0;
    if (g_sceneSoundMode != 0)
        volume = volume * VOLUME_CUTSCENE;
    else if (position == 0)
        volume = volume * VOLUME_FLAT;
    if (g_soundMuted == 0)
    {
        sprintf(path, "wav\\%s.%s", effect.name, g_soundExtension);
        if (position != 0)
        {
            int loop = effect.loop;
            float percent = (float)effect.volume;
            if (loop != 0)
            {
                if (effect.volume == 0)
                    percent = PERCENT_FULL;
                if (g_soundLoopHandle[sound] == SOUND_LOOP_FREE)
                {
                    int handle = Sound_PlayAt(path, position->x, position->y, position->z, loop,
                                              percent * volume / PERCENT_FULL);
                    g_soundLoopHandle[sound] = handle;
                    return handle;
                }
            }
            else
            {
                if (effect.volume == 0)
                    percent = PERCENT_FULL;
                Sound_PlayAt(path, position->x, position->y, position->z, 0, percent * volume / PERCENT_FULL);
            }
        }
        else
        {
            if (effect.loop != 0)
            {
                if (g_soundLoopHandle[sound] == SOUND_LOOP_FREE)
                {
                    int handle = Sound_PlayFlat(path, 0, 0, effect.loop, volume);
                    g_soundLoopHandle[sound] = handle;
                    return handle;
                }
            }
            else
            {
                g_soundArcPlay(MakeDataPath(path), 0, 0, 0, 0, 0, volume);
            }
        }
    }
    return SOUND_NO_HANDLE;
}
