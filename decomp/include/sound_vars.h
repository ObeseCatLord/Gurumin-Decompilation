// Globals of music, sound effects and movie streams.
// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one
// declaration of each of them: include this header, never redeclare one. Comments say what is known.
#ifndef GURUMIN_SOUND_VARS_H
#define GURUMIN_SOUND_VARS_H

#include "bgm_track.h"
#include "sound_def.h"
#include "vec4.h"

extern "C" int DAT_10202358;                              // track playing
extern "C" int DAT_102027d8;
extern "C" char DAT_1020ade0[];
extern "C" int DAT_1020aee4;
extern "C" int DAT_1021a398[10];                          // sound id
extern "C" char DAT_102564a8[];                           // receives the rest of the .de2 header
extern "C" int DAT_1042d870[10];                          // 1 when the sound has a position
extern "C" int DAT_10447310[10];                          // frames left before the sound plays (0 = free slot)
extern "C" int DAT_104bf8ec;                              // fade kind (see above)
extern "C" char DAT_104bfd54;
extern "C" int DAT_104bfd58;
extern "C" int DAT_104c01b0;                              // non-zero = no listener update is sent; non-zero = sound effects off; non-zero = no sound process
extern "C" int DAT_104c01b8;                              // sound output setting (non-zero = mode 1)
extern "C" int DAT_104c02e0;                              // colour of fade kind 3
extern "C" char DAT_104c033c;                             // non-zero = always track 0
extern "C" BgmTrack DAT_108db358[0x41];
extern "C" char DAT_108db428[];                           // file extension of the sound files
extern "C" SoundDef DAT_108dc158[];
extern "C" Vec4 DAT_10c67f40;                             // listener position (scaled)
extern "C" Vec4 DAT_10c67fe0;                             // listener direction
extern "C" Vec4 DAT_10f08a00[10];                         // position of the sound
extern "C" char DAT_10f627d0[];                           // the game's data folder

#endif
