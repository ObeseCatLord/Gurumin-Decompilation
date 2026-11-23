// State of the player's action selector (FUN_10011520): the motion table and the inputs the selector reads.
// One declaration of each; include this header, never redeclare them.
#ifndef GURUMIN_ACTION_SELECT_VARS_H
#define GURUMIN_ACTION_SELECT_VARS_H

#include "matrix4.h"

#define ACTION_TABLE_STEPS 8  // motion columns per action state

extern "C" int DAT_102024d0[][ACTION_TABLE_STEPS];  // motion for [action state][step]; -1 = none
extern "C" int DAT_10202510;                        // motion used when a locked chain falls back to state 2
extern "C" Matrix4 *DAT_1020e338[];                 // world matrices of the targets the player can lunge at
extern "C" unsigned char DAT_104687b4;              // gauge that must exceed 11 for the target action
extern "C" int DAT_104c02b4;                        // non-zero = a locked chain may fall back to state 2
extern "C" int DAT_10c64aac;                        // action input C
extern "C" int DAT_10c64ab4;                        // action input A
extern "C" int DAT_10c64ab8;                        // action input B
extern "C" int DAT_10c64abc;                        // target action input
extern "C" int DAT_10c64ac0;                        // finishing action input
extern "C" int DAT_10c64ac4;                        // finishing action variant
extern "C" int DAT_10c64f10;                        // non-zero = the selector's special actions are disabled
extern "C" unsigned char DAT_10db6f74;              // action level of the player
extern "C" char DAT_10db712a;                       // player form (above 3 = armed)
extern "C" int DAT_10db9274;                        // non-zero = the player is restricted

#endif
