// SoundDef: declared here once (first used by src/sound_effects.cpp).
#ifndef GURUMIN_SOUND_DEF_H
#define GURUMIN_SOUND_DEF_H

// Sound effect table: one 0x40-byte record per effect id.
struct SoundDef
{
    char name[0x20];          // file name without folder and extension
    int loop;                 // 0x20: loop mode, 0 = play once
    int volume;               // 0x24: volume in percent, 0 = 100
    int notPositional;        // 0x28: play without position even when one is given
    char pad_002c[0x14];
};

#endif
