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

// Number of switch hold slots, and the holders below this value are character slots (0x80 + n is object n).
const int SWITCH_HOLD_SLOTS = 0x80;
const int SWITCH_HOLDER_OBJECT_BASE = 0x80;
// Held bonus added to an object's switch state while it is held.
const int SWITCH_HELD_BONUS = 1000;

extern "C" void *g_switchHeldObject[SWITCH_HOLD_SLOTS];    // EN 0x009180e0 (JP DAT_1030bbf8): object held in each hold slot (0 = unused)
extern "C" int g_switchHolder[SWITCH_HOLD_SLOTS];          // EN 0x009183d0 (JP DAT_10255b08): holder of each hold slot
extern "C" short g_objectSwitchState[SWITCH_HOLD_SLOTS];   // EN 0x00b919e0 (JP DAT_10254838): per-object switch state; SWITCH_HELD_BONUS is added while held

extern "C" int g_switchEffectStarted[SWITCH_HOLD_SLOTS];   // EN 0x009188e0 (JP DAT_1042e0a8): 1 once the hold slot's effect was started
extern "C" int g_switchEffectKind[SWITCH_HOLD_SLOTS];      // EN 0x0091ba70 (JP DAT_10219fc0): effect kind of each hold slot (0, 1 or 2)
extern "C" int g_switchSlotSetUp[SWITCH_HOLD_SLOTS];       // EN 0x0091b870 (JP DAT_10219c40): 1 while the hold slot is set up (written by Switch_Hold)
extern "C" unsigned int g_gameFrames;                      // EN 0x00a1ddfc (JP DAT_104bf890): frames since the game started

// Starts the effects of the switch objects held near the camera. EN 0x5ac530 (JP Switch_UpdateEffects).
extern "C" int Switch_UpdateEffects(void);

// Lets `holder` (a character slot, or SWITCH_HOLDER_OBJECT_BASE + an object index) hold `object` (0 = the holder's own
// model) with effect kind `kind` (0, 1 or 2). The object must not be held yet and a hold slot must be free. A character
// gets hold bit 1 (kinds 0 and 1) or 2 (kind 2) and, for slot kind 0x60, its switch node turned on; an object gets its
// held bonus. Returns 1, or 0 when the object is already held or no slot is free. EN 0x5a1fa0 (JP 0x10057c40).
extern "C" int Switch_Hold(int holder, void *object, int kind);

// Lets go of what character slot `holder` holds. EN 0x5c9fd0 (JP 0x10057b90).
extern "C" int Switch_Release(int holder);

// Plays sound effect `sound` at `position` with `volume`. EN 0x63fc10 (JP 0x1003ad20).
extern "C" int Sound_PlayEffect(int sound, const Vec4 *position, float volume);

// Turns the switch in slot `slot` on / off. EN 0x5cb890, 0x5cb750.
extern "C" int Switch_TurnOn(int slot);
extern "C" int Switch_TurnOff(int slot);

#endif
