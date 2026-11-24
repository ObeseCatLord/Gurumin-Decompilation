// The partner skills (partner_skill.cpp): the special moves of the characters in slots 6, 7, 11 and 16, run once a
// frame from the enemy update 0x100e0320 and from the chase 0x100bcce0.
#ifndef GURUMIN_PARTNER_SKILL_H
#define GURUMIN_PARTNER_SKILL_H

#include "vec4.h"
#include "matrix4.h"

// Character slots whose skill Partner_SpecialSkill runs.
#define PARTNER_HEALER 6
#define PARTNER_SHIELD 7
#define PARTNER_BLASTER 11
#define PARTNER_HOMING 16

extern "C" Matrix4 DAT_10869278;    // the matrix the healer's effect follows (put on the healer, then on the player)
extern "C" float DAT_102551d4;      // heading of the shield partner's grab, towards the enemy it picked
extern "C" int DAT_10c64ed0;        // non-zero = ends the map 0xd2 condition of the skills (unknown)

// 0x10078060 (Partner_SpecialSkill; C++ linkage: the original has an EH frame). One frame of the skill of the
// character in slot `index` (anything else returns -1) standing at `position`. `found` is set to 1 while the skill
// keeps it busy and `steer` receives the step it takes then. Returns the motion to play (-1 = none).
int Partner_SpecialSkill(int index, Vec4 *position, int *found, Vec4 *steer);

#endif
