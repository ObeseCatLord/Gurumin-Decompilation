// The BS_006 boss (bs006_update.cpp, kind 107): a dragon whose body is its own slot plus fifteen piece slots after it
// (slots index + 2 .. index + 16: legs, chest plate, tail and so on, each with a node of the dragon's model). Its state
// words are Bs006State (char_state.h); the defeat timer is the word 11 of the record after it (DAT_1020b49c).
#ifndef GURUMIN_BS006_H
#define GURUMIN_BS006_H

#include "vec4.h"
#include "node.h"
#include "char_state.h"
#include "kind_counter.h"
#include "enemy_voice.h"
#include "volume_fade.h"
#include "char_tables.h"

// CharSlot::kind of the boss.
#define BS006_KIND 107

// The slots after the boss: its fifteen pieces (slots index + 2 .. index + 16).
#define BS006_PIECE_COUNT 15
#define BS006_FIRST_PIECE_SLOT 2

// Bs006Node(index, BS006_NODE_*): the nodes the boss keeps. The table has nine entries per slot, the boss fills eighteen:
// the entries after the ninth run into the entries of the slot after it (DAT_1021bdf8[index + 1]).
#define BS006_NODE_JAW 0
#define BS006_NODE_SPHERE 1
#define BS006_NODE_RIGHT_FINGER 2
#define BS006_NODE_LEFT_FINGER 3
#define BS006_NODE_RIGHT_TOE 4
#define BS006_NODE_LEFT_TOE 5
#define BS006_NODE_RIGHT_KNEEPAD_NULL 6
#define BS006_NODE_LEFT_KNEEPAD_NULL 7
#define BS006_NODE_CHEST_NULL 8
#define BS006_NODE_TAIL_1 9
#define BS006_NODE_TAIL_2 10
#define BS006_NODE_TAIL_3 11
#define BS006_NODE_TAIL_4 12
#define BS006_NODE_TAIL_5 13
#define BS006_NODE_LEFT_FINGER_B 14
#define BS006_NODE_RIGHT_FINGER_B 15
#define BS006_NODE_LEFT_TOE_B 16
#define BS006_NODE_RIGHT_TOE_B 17

__forceinline Node *&Bs006Node(int index, int which)
{
    return DAT_1021bdf8[index + which / 9][which % 9];
}

// DAT_1042d918[BS006_BONE_*] (Node pointers, found by name in state 0).
#define BS006_BONE_NECK 0
#define BS006_BONE_SPINE 1
#define BS006_BONE_TAIL_ROOT 2
#define BS006_BONE_EYE 15

// The two character slots whose kind (DAT_1021f3c0) goes negative while they are hurt (the slam shakes less then).
extern "C" int DAT_1020c3b4;
extern "C" int DAT_1020c3b8;

// 0x100d24a0 (Bs006_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One frame
// of the boss in slot `index`: the defeat scene once the game is over (DAT_10210d30), otherwise the fight: its states,
// the reaction to hits, the movement and the effects. Returns 0 (1 while the defeat scene runs).
int Bs006_Update(int index);

#endif
