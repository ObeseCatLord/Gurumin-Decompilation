// The BS_002 boss (bs002_update.cpp, kind 105): a beast that circles the player on a ring, climbs and drops, spits rings of
// shots and lunges. Its slot and the slot before it (the second half of its body) are updated together. Its state words
// are Bs002State (char_state.h); the words that lie before the record are Bs002Extra here.
#ifndef GURUMIN_BS002_H
#define GURUMIN_BS002_H

#include "vec4.h"
#include "node.h"
#include "char_state.h"
#include "enemy_voice.h"
#include "volume_fade.h"

// CharSlot::kind of the boss.
#define BS002_KIND 105

// The record before the boss's CharState record: DAT_1020b3f0 + 0x40 * slot is the CharState of slot - 1 (the second
// half of the body), whose words the boss uses for itself.
struct Bs002Extra
{
    int shotRing;                 // 0x00: rings of effects started so far (0..8)
    int attacking;                // 0x04: 1 while an attack state runs (the motion is not interrupted by a hit)
    int variant;                  // 0x08: random 1..6 picked by the lunge and the roar, the pick of the claw swing
    int furyLevel;                // 0x0c: counts the lunges of the ring walk (state 4) that ended in a swing
    int phase;                    // 0x10: 0 = healthy, 1 = hurt (below half health), 2 = enraged, 3 = after the roar
    int lungeReady;               // 0x14: 1 once a lunge was done (state 8 may then end in the pick)
    int damageTaken;              // 0x18: damage since the last stagger
    int tinted;                   // 0x1c: 1 while the screen tint of the roars is on
    int pad_0020[4];
    int lastHealth;               // 0x30: health at the end of the last frame
    int roarStep;                 // 0x34: 0 when the roar starts, 10 while it fades
    int endingSetUp;              // 0x38: the defeat scene: 1 once it was set up
    int pad_003c;
};

extern "C" Bs002Extra DAT_1020b3f0[CHAR_SLOT_COUNT];   // 0x1020b3f0 + 0x40 * slot
extern "C" Node *DAT_1021bdd4[CHAR_SLOT_COUNT][9];     // the five nodes the boss keeps before DAT_1021bdf8's entry
extern "C" int DAT_10c64660;                           // non-zero = the camera trigger of the defeat scene is armed

// 0x10034810 (group_tables.cpp): moves `current` 55 % of the way towards `target` while halving `steps`; the result
// goes to *out.
extern "C" int FUN_10034810(float target, float current, float *out, float steps, int first);

// 0x100543a0 (Balloon_Blast, balloon_burst.cpp): lets the balloons react to a blast at `blast`.
extern "C" int FUN_100543a0(Vec4 blast);

// 0x10104640 (Bs002_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One frame
// of the boss in slot `index`: the defeat scene once the game is over (DAT_10210d30), otherwise the fight: its states,
// the reaction to hits, the movement and the effects. Returns 0 (1 while the defeat scene runs).
int Bs002_Update(int index);

#endif
