// Shots fired by characters and the ground probe (EN build; JP: char_shots.cpp). This is the one declaration of these
// globals and functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_CHAR_SHOTS_H
#define GURUMIN_EN_CHAR_SHOTS_H

#include "char.h"

// A shot helper slot follows its owner slot: the first and one past the last helper offset.
const int SHOT_HELPER_FIRST = 2;
const int SHOT_HELPER_END = 8;
// Entries of a slot's row of g_shotParams.
enum ShotParam
{
    SHOT_ANGLE = 0,      // angle of the slot (pitch)
    SHOT_HEADING = 1,    // direction the shot flies
    SHOT_SPEED = 2,      // speed
    SHOT_PARAM_COUNT = 4 // floats per row
};
// CharState::values entries of a shot: frames to live, spread flag, and a counter.
const int SHOT_VALUE_LIFE = 0;
const int SHOT_VALUE_SPREAD = 1;
const int SHOT_VALUE_COUNTER = 2;
// CharState::values entry of a character that skips the ground probe when non-zero.
const int CHARSTATE_SKIP_GROUND_PROBE = 9;
// Frames a shot lives, its speed, and the spread arithmetic (random number mod 250 minus 125, divided by 5000).
const int SHOT_LIFE_FRAMES = 300;
const float SHOT_SPEED_START = 5.0f;
const int SHOT_SPREAD_RANGE = 250;
const int SHOT_SPREAD_CENTRE = 125;
const float SHOT_SPREAD_DIVISOR = 5000.0f;
// Ground probe: collision mask, and the ray start above and end below the probe point.
const unsigned int GROUND_PROBE_MASK = 0x4000000;
const float GROUND_PROBE_UP = 50.0f;
const float GROUND_PROBE_DOWN = 350.0f;

// Per-slot record of a shot (0x40 bytes).
struct ShotSlot
{
    Vec4 position;            // 0x00: start position of the shot
    Vec4 step;                // 0x10: per-frame step of the shot (cleared with the position when the character resets)
    char m_0020[0x20];        // 0x20: not used by EN code yet
};

extern "C" ShotSlot g_shotSlots[CHAR_SLOT_COUNT];                 // EN 0x00da7b00 (JP DAT_10802bf8): record of each shot slot
extern "C" float g_shotParams[CHAR_SLOT_COUNT][SHOT_PARAM_COUNT];   // EN 0x00bc90a8 (JP DAT_10255160): angle, heading and speed of each slot

// Starts a shot of character `owner` from `position` in the first free helper slot. Returns 0. EN 0x6cb910.
extern "C" int Char_FireShot(int owner, const Vec4 *position, int spread);
// Ground probe for slot `slot` at `distance` along its angle. Returns 1 on a hit. EN 0x6a4d20.
extern "C" int Char_ProbeGround(int slot, int unused, float distance);

#endif
