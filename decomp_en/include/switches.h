// Switch and door gimmicks of the EN build (JP: decomp/include/switches.h, char_vars.h): per-slot tables and the
// functions that turn a switch on or off. This is the one declaration of these tables in decomp_en.
#ifndef GURUMIN_EN_SWITCHES_H
#define GURUMIN_EN_SWITCHES_H

#include "char.h"

// g_charKind values of the switch kinds.
const int SWITCH_KIND_LEVER_DOOR = 0x13;
const int SWITCH_KIND_PLATE_DOOR = 0x14;
// Frames shown by each switch kind in the on and off position.
const short SWITCH_FRAME_LEVER_ON = 1;
const short SWITCH_FRAME_LEVER_OFF = 0x3d;
const short SWITCH_FRAME_PLATE_ON = 0x1f;
const short SWITCH_FRAME_PLATE_OFF = 0x5b;
// Sound effect ids of the switches (turned on / off).
const int SOUND_LEVER_ON = 0x38a;
const int SOUND_LEVER_OFF = 0x38c;
const int SOUND_PLATE_ON = 0x389;
const int SOUND_PLATE_OFF = 0x38b;
// Volume the switch sounds play at.
const float SWITCH_SOUND_VOLUME = 1.0f;

extern "C" short g_switchFrame[CHAR_SLOT_COUNT];      // EN 0x00920888 (JP DAT_1020b128): the frame a switch shows
extern "C" short g_switchFlag[CHAR_SLOT_COUNT];       // EN 0x00920d20 (JP DAT_1032d4b8): script flag a switch sets (0 = none)
extern "C" short g_switchDoorId[CHAR_SLOT_COUNT];     // EN 0x00920a18 (JP DAT_10219ec0): door id the switch's events use
extern "C" int g_soundsMuted;                         // EN 0x0091f534 (JP DAT_104bfe6c): 1 while sounds are muted (cut-scenes)

// Plays sound effect `sound` at `position` with `volume`. EN 0x63fc10 (JP 0x1003ad20).
extern "C" int Sound_PlayEffect(int sound, const Vec4 *position, float volume);

// Turns the switch in slot `slot` on / off. EN 0x5cb890, 0x5cb750.
extern "C" int Switch_TurnOn(int slot);
extern "C" int Switch_TurnOff(int slot);

#endif
