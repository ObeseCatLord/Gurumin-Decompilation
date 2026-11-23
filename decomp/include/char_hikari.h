// The light that carries the player along a route (behaviour 0x136 of the per-kind update 0x100f9670, char_hikari.cpp).
// CharState words: hikariPoint / targetPoint are the route points last reached and headed for, hikariSpeed the
// speed counter, hikariReady 2 once set up, hikariStopped 1 while blocked, hikariFollower the character carried
// with it.
#ifndef GURUMIN_CHAR_HIKARI_H
#define GURUMIN_CHAR_HIKARI_H

#include "vec4.h"

#define HIKARI_READY 2                // CharState::hikariReady once the light is on its route
#define HIKARI_NODE "hikari"          // the light's own node
#define HIKARI_ROUTE_ROWS 16          // route points scanned for the start (kind HIKARI_START_KIND)
#define HIKARI_START_KIND 2           // the route point kind the light starts from
#define HIKARI_COORD_UNIT 10000       // coordinates are packed as x * 10000 + y
#define HIKARI_COORD_LIMIT 5000
#define HIKARI_IDLE_SPEED 10          // speed counter while it waits
#define HIKARI_TOP_SPEED 90
#define HIKARI_SLOW_SCALE 3.33333f    // below this distance per counter step the light slows down
#define HIKARI_SLOW_RANGE 300.0f
#define HIKARI_SLOW_FACTOR 0.25f
#define HIKARI_SLOW_BASE 90.0f
#define HIKARI_FAST_DIVISOR 11.25f    // speed counter / this = distance a frame
#define HIKARI_HALF_DIVISOR 5.625f
#define HIKARI_SHAKE_PERIOD 5         // in mode HIKARI_MODE_SHAKY one frame in five wobbles
#define HIKARI_SHAKE_RANGE 11
#define HIKARI_SHAKE_CENTRE 5
#define HIKARI_SHAKE_SCALE 0.4f
#define HIKARI_RIDER_HEIGHT 150.0f    // the player rides this far above the light
#define HIKARI_FOLLOWER_HEIGHT 2.0f
#define HIKARI_REACH_XY 5.0f          // a character this close (box) is carried along
#define HIKARI_REACH_Z 15.0f
#define HIKARI_FIRST_CARRIED 0x5c     // slots scanned for the character to carry
#define HIKARI_END_CARRIED 0x7c
#define HIKARI_END_HOLD 15

// CharState::behaviour values it sets.
#define HIKARI_MODE_ENDED 0
#define HIKARI_MODE_SHAKY 2
#define HIKARI_MODE_RESUMED 3         // the player no longer rides at this value and above
#define HIKARI_MODE_RESET 10          // jumps back to where it stood

// Maps that start the reset (DAT_10c62ed0).
#define HIKARI_MAP_A 0xcc
#define HIKARI_MAP_B 0xce

extern "C" int DAT_10c635ec;          // set to reset the light on map HIKARI_MAP_A (cleared by it)
extern "C" int DAT_10c65600;          // set to reset the light on map HIKARI_MAP_B
extern "C" int DAT_10c635bc;          // 1 once the reset on map HIKARI_MAP_B was done

// 0x100286d0 and 0x10029730: the route targets (route_walk.cpp, route_step.cpp).
extern "C" int FUN_100286d0(int slot, int *state, int point, int *nextPoint, Vec4 *target, int *group, int *value,
                            int *action);
extern "C" int FUN_10029730(int slot, int *lastPoint, int point, int *nextPoint, Vec4 *position, int *kind,
                            int *value, int *extra);
extern "C" int FUN_1013f880(void);     // the game's random number

extern "C" int FUN_1002cc70(int slot, int halfSpeed);   // Char_HikariRide

#endif
