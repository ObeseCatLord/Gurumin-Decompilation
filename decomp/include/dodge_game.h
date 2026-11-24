// The map 0x16 mini game (dodge_game.cpp): up to 15 obstacles (walls, falling poles and sweeping diagonals, each a
// line segment between two matrices) cross the arena; touching one knocks the player down and ends the game, one that
// leaves the arena scores a point.
#ifndef GURUMIN_DODGE_GAME_H
#define GURUMIN_DODGE_GAME_H

#include "matrix4.h"

// Obstacles that can be in play at once.
#define DODGE_OBJECT_COUNT 15

// DodgeObjectState::kind: 0 = free slot, else the obstacle's shape and motion; kinds above 4 are the fast variants.
// 1/5 wall sweeping +x, 2/6 wall sweeping -x, 3/4 bar falling along -y, 7/8 pole falling along -y, 9/11 and 10/12
// diagonals that run round the arena's edge from the top left / right corner.
#define DODGE_KIND_SWEEP_RIGHT 1
#define DODGE_KIND_SWEEP_LEFT 2
#define DODGE_KIND_BAR 3
#define DODGE_KIND_FAST_BAR 4
#define DODGE_KIND_FAST_SWEEP_RIGHT 5
#define DODGE_KIND_FAST_SWEEP_LEFT 6
#define DODGE_KIND_POLE 7
#define DODGE_KIND_FAST_POLE 8
#define DODGE_KIND_DIAGONAL_RIGHT 9
#define DODGE_KIND_DIAGONAL_LEFT 10
#define DODGE_KIND_FAST_DIAGONAL_RIGHT 11
#define DODGE_KIND_FAST_DIAGONAL_LEFT 12

// DAT_1020c2b0 (DodgeGame state): 0 = set up on the first frame, 1 = running, 2 = over.
#define DODGE_STATE_SETUP 0
#define DODGE_STATE_RUNNING 1
#define DODGE_STATE_OVER 2

// One obstacle (0x40 bytes, DAT_1020c2f0).
struct DodgeObjectState
{
    int kind;            // 0x00: DODGE_KIND_*
    int reserved;        // 0x04: cleared at set-up, not used afterwards
    int driftX;          // 0x08: kinds 9..12: x step per frame before scaling (4..12)
    int driftY;          // 0x0c: kinds 9..12: y step per frame before scaling (16 - driftX)
    int pad[12];
};

// The obstacle's speed factor (0x10 bytes, DAT_10255510): its first float, drawn when it was spawned.
struct DodgeObjectSpeed
{
    float scale;
    float pad[3];
};

// The obstacle's segment and its effects (0x180 bytes, DAT_1086e0b8): the translations of endA and endB are the segment's
// end points; landingA/landingB are the same one step further down, passed to the landing splash effect.
struct DodgeObjectMatrices
{
    Matrix4 endA;
    Matrix4 endB;
    Matrix4 landingA;
    Matrix4 landingB;
    Matrix4 pad[2];
};

// The arena (DAT_10803a78, floats read from the map record DAT_10210fc0[0] at set-up): obstacles enter at spawnY and
// leave at endY, x runs from minX to maxX, z is the height the segments lie at.
struct DodgeArena
{
    float minX;
    float spawnY;
    float minZ;
    float pad_000c;
    float maxX;
    float endY;
    float maxZ;
    float pad_001c;
};

extern "C" int DAT_1020c2b0;                        // DodgeGame state (DODGE_STATE_*)
extern "C" int DAT_1020c2b4;                        // frames since the game started (raises the obstacle speed over time)
extern "C" int DAT_1020c2b8;                        // frames since the last obstacle was spawned
extern "C" int DAT_1020c6b0;                        // 2 once the player was hit
extern "C" DodgeObjectState DAT_1020c2f0[DODGE_OBJECT_COUNT];
extern "C" DodgeObjectSpeed DAT_10255510[DODGE_OBJECT_COUNT];
extern "C" DodgeObjectMatrices DAT_1086e0b8[DODGE_OBJECT_COUNT];   // menu_scene.h declares only its first matrix
extern "C" DodgeArena DAT_10803a78;

extern "C" int FUN_100119d0(int index, int damage, int a);   // EventSlots_Add: shows the damage on character `index`
extern "C" int FUN_1000e750(void);                           // AntiTamper_StoreValues

int Map16_UpdateDodgeGame();    // 0x10053020

#endif
