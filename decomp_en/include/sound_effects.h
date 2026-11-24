// Sound effect table, sound engine entry points and the effect play functions of the EN build (JP: sound_effects.cpp,
// sound_play.cpp, app_0013.cpp). This is the one declaration of them in decomp_en: include this header.
#ifndef GURUMIN_EN_SOUND_EFFECTS_H
#define GURUMIN_EN_SOUND_EFFECTS_H

#include "vec4.h"

// One record (0x40 bytes) of the sound effect table.
struct SoundEffectDef
{
    char name[0x20];       // 0x00: file name without folder and extension
    int loop;              // 0x20: loop channel, 0 = play once
    int volume;            // 0x24: volume in percent, 0 = 100
    int notPositional;     // 0x28: play without position even when one is given
    char m_002c[0x14];     // 0x2c: not used by EN code yet
};

// Sound effect table. EN 0x00fabf00 (JP DAT_108dc158).
extern "C" SoundEffectDef g_soundEffects[];
// File extension of the sound files, e.g. "wav". EN 0x00fab1d0 (JP DAT_108db428).
extern "C" char g_soundExtension[];
// Non-zero while a cut-scene or scene start is running (volumes are lowered). EN 0x0091f534 (JP DAT_104bfe6c).
extern "C" int g_sceneSoundMode;
// Sound output setting: 0 = positional mode 2, non-zero = mode 1. EN 0x00a15160 (JP DAT_104c01b8).
extern "C" int g_soundOutputMode;
// Sound engine: plays a file once. Position and volume travel as raw values. EN 0x00909480 (JP FUN_100016c0).
extern "C" int (*g_soundArcPlay)(const char *file, const float *x, const float *y, const float *z, int mode, int loop, float volume);
// Sound engine: plays a file looping on a channel. EN 0x00909484 (JP FUN_10001980).
extern "C" int (*g_soundArcPlayLooped)(const char *file, const float *x, const float *y, const float *z, int mode, int loop, float volume);

// Plays a sound at a world position, looping on channel `loop` (0 = once). Returns the sound's handle. EN 0x63e8a0.
extern "C" int Sound_PlayAt(const char *name, float x, float y, float z, int loop, float volume);
// Plays a sound without position, looping on channel `loop` (0 = once). EN 0x63e840.
extern "C" int Sound_PlayFlat(const char *name, int unused0, int unused1, int loop, float volume);
// Plays "wav\<name>.<extension>" once at `position` (0 = not positional). Returns -1. EN 0x63faf0.
extern "C" int Sound_PlayWav(const char *name, const Vec4 *position, float volume);
// Plays effect `sound` of the effect table. Returns the handle of a started loop, else -1. EN 0x63fc10.
extern "C" int Sound_PlayEffect(int sound, const Vec4 *position, float volume);

#endif
