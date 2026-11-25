// Per-frame updates of the special maps (EN build; JP: decomp/include/map_vars.h and the minigame headers).
#ifndef GURUMIN_EN_MAP_SPECIAL_H
#define GURUMIN_EN_MAP_SPECIAL_H

// Maps that run a minigame update every frame while the map timer is positive.
const int MAP_TARGET_RACE = 0x15;
const int MAP_DODGE_GAME = 0x16;
const int MAP_MOLE_GAME = 0x17;
const int MAP_FOOTBALL_GAME = 0x18;
// Event raised when the map timer reaches 0.
const int EVENT_MAP_TIME_UP = 0x1bc;
// Effect started above the ball on the football map after the time is up, and its height above the ball.
const int EFFECT_FOOTBALL_MARKER = 0x76d;
const float FOOTBALL_MARKER_HEIGHT = 30.0f;
// Character slot of the football map's ball.
const int FOOTBALL_BALL_SLOT = 59;

extern "C" int g_mapTimer;   // EN 0x01334e04 (JP DAT_10c6505c): frames left on the special map's timer (0 = just ran out, negative = over)

// The minigame updates, one per special map.
extern "C" void Map15_UpdateTargetRace(void);     // EN 0x5f52a0 (JP 0x1007dab0)
extern "C" void Map16_UpdateDodgeGame(void);      // EN 0x6383c0 (JP 0x10053020)
extern "C" void Map17_UpdateMoleGame(void);       // EN 0x6365f0 (JP 0x100524d0)
extern "C" void Map18_UpdateFootballGame(void);   // EN 0x64dbb0 (JP 0x1007f1e0)

// While the map timer runs, calls the map's own update; raises the time-up event when it reaches 0; after that, on the
// football map, starts a marker effect above the ball every frame. Returns 0. EN 0x635480 (JP 0x100bd960).
int Map_UpdateSpecial();

#endif
