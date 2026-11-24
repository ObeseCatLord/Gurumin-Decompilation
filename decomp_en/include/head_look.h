// Head tracking of the player character (EN build; JP: head_look.cpp). This is the one declaration of these globals and
// functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_HEAD_LOOK_H
#define GURUMIN_EN_HEAD_LOOK_H

#include "char.h"

// Character slot of the player.
const int PLAYER_SLOT = 0;
// Modes of Player_SetHeadTracking.
const int HEAD_TRACK_RETURN = 0;      // let the head return to rest
const int HEAD_TRACK_START = 1;       // start tracking within the limits
const int HEAD_TRACK_REMOVE = -5;     // remove the tracking block
// Rotation limits of the tracking head in radians (x, y, z).
const float HEAD_LIMIT_X = 1.22f;
const float HEAD_LIMIT_Y = 0.27f;
const float HEAD_LIMIT_Z = 0.17f;
// Player_LookAround: target 1000 units to the side and 500 up, a quarter turn from the heading, switching every 10 calls.
const float LOOK_TARGET_UP = 500.0f;
const float LOOK_TARGET_SIDE = -1000.0f;
const float LOOK_QUARTER_TURN = 1.5708f;
const int LOOK_SWITCH_COUNT = 10;

// Head-look target. EN 0x015b9660 (JP DAT_10802ac8).
extern "C" Vec4 g_headLookTarget;
// Look-around counter: counts up on one side and down (negative) on the other. EN 0x01333334 (JP DAT_10c6358c).
extern "C" int g_lookAroundCounter;

// Moves the head-look target to the side of the player and restarts tracking. Returns 0. EN 0x64c9e0.
extern "C" int Player_LookAround(void);
// Starts (mode 1), releases (mode 0) or removes (mode -5) the head tracking. Returns 0. EN 0x609df0.
extern "C" int Player_SetHeadTracking(int mode);

#endif
