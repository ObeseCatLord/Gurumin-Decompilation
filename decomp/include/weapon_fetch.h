// Enemy_FetchDroppedWeapon (0x100a8ac0, weapon_fetch.cpp): the weapon-mark words of CharState (weaponMarkA = right
// hand, weaponMarkB = left hand) hold weapon number * 1000 + the slot whose node row carries the dropped weapon.
#ifndef GURUMIN_WEAPON_FETCH_H
#define GURUMIN_WEAPON_FETCH_H

#define WEAPON_MARK_UNKNOWN -2          // the weapon had no model
#define WEAPON_SLOT_RANGE 1000          // mark % this = the slot carrying the dropped weapon
#define WEAPON_CODE_UNIT 1000000        // the weapon number joins the character's script code in this place
#define FETCH_STEP -12.0f               // step towards the weapon (before turning by the heading)
#define FETCH_LIFT 24.0f                // vertical part of the step
#define FETCH_SIDE_ANGLE 1.65806f       // radians (95 degrees) turned away from a blocked way
#define FETCH_GIVE_UP_SQUARED 14400     // gives up after 120 blocked frames
#define FETCH_PICKUP_SQUARED 10000.0f   // picks the weapon up within 100 units

extern "C" int FUN_100a8ac0(int index);  // Enemy_FetchDroppedWeapon

#endif
