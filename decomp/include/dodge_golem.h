// The map 0x132 dodge game's stone golem pieces (dodge_golem.cpp): after the game is over (DodgeGame state 2) the
// model tree's "@BS_005fists:" and "@BS_005rocks:" nodes become character slots 0x3d..0x4c that fly in, snap onto the
// golem and hurl themselves at the player.
#ifndef GURUMIN_DODGE_GOLEM_H
#define GURUMIN_DODGE_GOLEM_H

#include "matrix4.h"
#include "node.h"

// First and last piece slots: 0x3d and 0x3e are the fists, 0x3f..0x47 loose rocks and 0x48..0x4c rocks that are
// linked to the pieces before them.
#define GOLEM_FIRST_FIST 0x3d
#define GOLEM_LAST_FIST 0x3e
#define GOLEM_FIRST_ROCK 0x3f
#define GOLEM_FIRST_LINKED_ROCK 0x48
#define GOLEM_LAST_LINKED_ROCK 0x4c

extern "C" Node *DAT_10e3dd2c;       // the golem's model tree (holds the "@BS_005" pieces and the player's bones)
extern "C" Matrix4 DAT_10e3dd68;     // the model tree's world matrix: positions are taken relative to its translation
extern "C" int DAT_10db03c8;         // effect option value handed to every effect the pieces start

extern "C" float FUN_10001f80(float y, float x);   // 0x10001f80 (math_float.cpp): atan2f

// 0x1005fc00 (DodgeGame_UpdateGolemPiece): one frame of piece `index`. Returns 0.
int DodgeGame_UpdateGolemPiece(int index);

#endif
