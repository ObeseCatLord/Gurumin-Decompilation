// The BS_001 boss (a fighter with two arms that comes with a partner slot; bs001_update.cpp): its per-frame
// behaviour and the globals only it uses. State words: Bs001State, Bs001Aux, Bs001Fade (char_state.h).
#ifndef GURUMIN_BS001_H
#define GURUMIN_BS001_H

#include "enemy_voice.h"
#include "actor_scan.h"
#include "volume_fade.h"

#include "vec4.h"
#include "matrix4.h"

// CharSlot::kind of the boss.
#define BS001_KIND 100

// Slots of the four piece models (bs_001en, bs_001pa, bs_001be, bs_001bp; each one also loads bs_001m).
#define BS001_PART_FIRST 0x58
#define BS001_PART_COUNT 4

// Bs001State::state: what the boss does.
enum Bs001Action
{
    BS001_REST = 0,            // stands and watches; picks a move after a second or so
    BS001_STAND_TURN = 1,      // stands, turns to the player and occasionally pounds the ground
    BS001_WALK = 2,            // walks at the player
    BS001_RUN = 3,             // runs at the player
    BS001_LUNGE = 5,           // lunges, then swings both arms
    BS001_SHOUT = 6,           // roars (the three shock rings), spins up a ring of effects
    BS001_STAGGER = 7,         // staggers after a hit
    BS001_DOWN = 8,            // falls down
    BS001_RISE = 9,            // gets up again at its home position
    BS001_REVIVE = 10,         // revives and returns to the dispatcher
    BS001_KNOCKED = 0xb,       // is knocked back (cut-scene fall)
    BS001_RETREAT = 0xc,       // steps back
    BS001_PUNCH = 0xd,         // punches
    BS001_CLAW = 0xe,          // claws
    BS001_SMASH = 0xf,         // overhead smash
    BS001_CHASE = 0x10,        // chases the player
    BS001_HURT_STAGGER = 0x11, // stagger of the hurt phase
    BS001_SMASH_FAST = 0x12,   // smash after a chase
    BS001_CHOOSE = 100         // picks the next move
};

// Where the boss's bones are kept: DAT_1042d918[BS001_BONE_*] (shared with BS_003, see bs003.h).
enum Bs001Bone
{
    BS001_BONE_LEFT_TOE = 0,
    BS001_BONE_RIGHT_TOE = 1,
    BS001_BONE_LEFT_FOOT = 2,
    BS001_BONE_RIGHT_FOOT = 3,
    BS001_BONE_HEAD = 4,
    BS001_BONE_FOOTSTEPS = 5,
    BS001_BONE_SPINE = 6
};

// DAT_1021bdf8[slot][BS001_NODE_*]: the boss's arm bones.
enum Bs001Node
{
    BS001_NODE_RIGHT_UPPER_ARM = 0,
    BS001_NODE_LEFT_UPPER_ARM = 1,
    BS001_NODE_LEFT_HAND = 2,
    BS001_NODE_LEFT_FINGER = 3,
    BS001_NODE_RIGHT_HAND = 4
};

extern "C" int DAT_10c636c8;   // non-zero = the camera trigger of the defeat scene is armed

// 0x100b9bb0 (Enemy_PlayVoice, enemy_voice.cpp): plays voice `sound` of character `index` unless its cooldown runs.

// 0x1010c650 (Bs001_Update; C++ linkage: the original has an EH frame for its matrices). One frame of the boss in
// slot `index`: its fade in and out, the hit reactions, the defeat scene, the choice of the next move and the moves
// themselves. Returns 0 (1 while the defeat scene runs).
int Bs001_Update(int index);

#endif
