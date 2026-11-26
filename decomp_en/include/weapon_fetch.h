// Enemy_FetchDroppedWeapon (EN 0x6a98c0, weapon_fetch.cpp; JP 0x100a8ac0): the weapon-mark words of CharState (the right
// hand's is CHARSTATE_WEAPON_MARK_RIGHT, the left's CHARSTATE_WEAPON_MARK_LEFT) hold weapon number * 1000 + the slot
// whose node row carries the dropped weapon.
#ifndef GURUMIN_EN_WEAPON_FETCH_H
#define GURUMIN_EN_WEAPON_FETCH_H

const int WEAPON_SLOT_RANGE = 1000;          // mark % this = the slot carrying the dropped weapon
const int WEAPON_CODE_UNIT = 1000000;        // the weapon number joins the character's script code in this place
#define FETCH_STEP -12.0f                    // step towards the weapon (before turning by the heading)
#define FETCH_LIFT 24.0f                     // vertical part of the step
#define FETCH_SIDE_ANGLE 1.65806f            // radians (95 degrees) turned away from a blocked way
const int FETCH_GIVE_UP_SQUARED = 14400;     // gives up after 120 blocked frames
#define FETCH_PICKUP_SQUARED 10000.0f        // picks the weapon up within 100 units
#define PI_APPROX 3.14159f                   // the game's rounding of pi added to the heading
// Index in CharState::values of the signed count of blocked frames (positive = going round to the right, negative = left).
const int CHARSTATE_FETCH_BLOCKED = 0;
// Index in a character's node row of the left and right hand weapon nodes.
const int FETCH_NODE_LEFT = 3;
const int FETCH_NODE_RIGHT = 2;

extern "C" int Enemy_FetchDroppedWeapon(int index);

#endif
