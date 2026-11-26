// Weapon models in a character's hands (EN build; JP: decomp/src/weapon_attach.cpp).
#ifndef GURUMIN_EN_WEAPON_ATTACH_H
#define GURUMIN_EN_WEAPON_ATTACH_H

struct Node;

// Weapon numbers Char_AttachWeapon knows (right-hand model, some with a left-hand partner).
enum WeaponNumber
{
    WEAPON_FIRST = 0x32,          // first weapon with a model (EEQ_19)
    WEAPON_PAIR_A = 0x36,         // EEQ_21 with its left-hand part
    WEAPON_PAIR_B = 0x37,         // EEQ_21b with its left-hand part
    WEAPON_LAST = 0x3d            // last weapon with a model (EEQ_24b in both hands)
};
// CharState::values indices that hold the "weapon unknown" marks of the right and left hand.
const int CHARSTATE_WEAPON_MARK_RIGHT = 1;
const int CHARSTATE_WEAPON_MARK_LEFT = 2;
// Value of both marks after a weapon without a model.
const int WEAPON_MARK_NONE = -2;
// Value stored in g_shagaTargetB of the slot being refreshed while the weapon motion restarts.
const int WEAPON_MOTION_TIMER = 0xcc;
// Render mode argument of SetRenderModeExcept after a weapon change.
const int WEAPON_RENDER_MODE = 0x1f;

// Non-zero while the character models must refresh their render mode after a weapon change. EN 0x021e3618 (JP DAT_10f654b0).
extern "C" int g_refreshRenderMode;

// Gives character `index` weapon `weapon` (0x32..0x3d; 0 keeps the current ones): removes the old hand models asked for
// by `right` and `left`, loads the weapon's model file(s) under the hand nodes, then restarts the "anime2" motion of
// shaga slot `slot`. Returns 0. EN 0x6aa070 (JP 0x1001a990).
extern "C" int Char_AttachWeapon(int index, int weapon, int right, int left, int slot);

#endif
