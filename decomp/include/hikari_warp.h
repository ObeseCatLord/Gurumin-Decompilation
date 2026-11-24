// The warp light (behaviours 0x1f, 0x22 and 0x30 of the per-kind update 0x100f9670, hikari_warp.cpp): a light that
// follows the route points of its map record like the light of char_hikari.h. The player standing in it is taken
// along (hidden, his position held on the light) and put down again when the route ends; a character found at the
// light's start is carried with it and dust effects trail behind. Variant 2 is the one that takes the player.
#ifndef GURUMIN_HIKARI_WARP_H
#define GURUMIN_HIKARI_WARP_H

#include "char_hikari.h"

// 0x1002ae70 (Char_HikariWarp; C++ linkage: the original has an EH frame)
int Char_HikariWarp(int slot, int variant);

#define WARP_VARIANT_SLOW 0           // speed counter / WARP_SLOW_DIVISOR
#define WARP_VARIANT_HALF 1           // speed counter / HIKARI_HALF_DIVISOR
#define WARP_VARIANT_PLAYER 2         // fixed speed; takes the player along
#define WARP_SLOW_DIVISOR 10.0f
#define WARP_FIXED_SPEED 65.0f
#define WARP_STEADY_DIVISOR 2.5f      // behaviour WARP_MODE_STEADY
#define WARP_MODE_BOARDING 1          // CharState::behaviour values: the player is inside (global flag DAT_104c00c8)
#define WARP_MODE_STEADY 5            // steady ride, stops for obstacles
#define WARP_MODE_RESTART 99          // set the light up again
#define WARP_NEAR_Z 30.0f             // the player this close (height) and WARP_NEAR_XY (ground) to the light is inside it
#define WARP_NEAR_XY 75.0f
#define WARP_INSIDE_FRAMES 20.0f      // DAT_10255160[slot][1]: how long the player counts as inside after he was near
#define WARP_DROP_FRAMES 3.0f         // DAT_10255160[slot][0]: frames the player is held on the light after it arrived
#define WARP_LIFT_RISING 2
#define WARP_WOBBLE_BIG_RANGE 21      // behaviour HIKARI_MODE_SHAKY, variant 2: half of the frames wobble this much
#define WARP_WOBBLE_BIG_CENTRE 10
#define WARP_WOBBLE_BIG_SCALE 0.8f
#define WARP_WOBBLE_CALM_PERIOD 5
#define WARP_WOBBLE_CALM_RANGE 11
#define WARP_WOBBLE_CALM_CENTRE 5
#define WARP_WOBBLE_CALM_SCALE 0.4f
#define WARP_TRAIL_COUNT 2            // dust puffs started along the light's last step
#define WARP_TURN_AROUND 3.14159f     // the dust blows back against the direction of travel
#define WARP_ANY_NODE_VALUE 0x8e      // Node::M_10194730 value while the light moves
#define WARP_END_NODE_VALUE 0x8f      // and after it
#define WARP_NODE_MASK_A 0x400
#define WARP_NODE_MASK_B 0x200
#define WARP_MAP_SWAP 0x12d           // on this map an obstacle turns the light round (swaps its two route points)
#define WARP_HALF_STEP 0.5f

// Effects: the trail of the player's costume (by drill level), the light's dust and the carried character's.
#define WARP_EFFECT_TRAIL_0 0x650
#define WARP_EFFECT_TRAIL_1 0x8ef8
#define WARP_EFFECT_TRAIL_2 0x8ef9
#define WARP_EFFECT_TRAIL_3 0x8efa
#define WARP_EFFECT_TRAIL_ALL 0x8ca0
#define WARP_EFFECT_DUST 0x84d0
#define WARP_EFFECT_DUST_SPECIAL 0x84d1    // costume 6
#define WARP_EFFECT_LIGHT 0x730
#define WARP_EFFECT_CARRIED 0x731
#define WARP_EFFECT_CARRIED_SPECIAL 0x733  // the carried character is of slot kind WARP_SPECIAL_KIND
#define WARP_SPECIAL_KIND 5
#define WARP_SPECIAL_COSTUME 6

#endif
