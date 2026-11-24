// The scratch area the map 0x15..0x18 mini games share (the same addresses mean different things in each game):
// dodge_game.h (map 0x16, obstacles) and football_state.h (map 0x18, the ball and its kick) both use these records.
#ifndef GURUMIN_MINIGAME_AREA_H
#define GURUMIN_MINIGAME_AREA_H

#include "matrix4.h"

// Obstacles that can be in play at once.
#define DODGE_OBJECT_COUNT 15

// One obstacle (0x40 bytes, DAT_1020c2f0).
struct DodgeObjectState
{
    union
    {
        int kind;        // 0x00: DODGE_KIND_*
        int footballKick;   // map 0x18: how the human's kick turned out (DAT_1020c2f0)
        int galaxyFade;     // neo_galaxy_update.cpp: frames of the fade-out once the Neo Galaxy fight is over
    };
    int reserved;        // 0x04: cleared at set-up, not used afterwards
    int driftX;          // 0x08: kinds 9..12: x step per frame before scaling (4..12)
    int driftY;          // 0x0c: kinds 9..12: y step per frame before scaling (16 - driftX)
    int pad[12];
};

// The obstacle's speed factor (0x10 bytes, DAT_10255510): its first float, drawn when it was spawned.
struct DodgeObjectSpeed
{
    union
    {
        float scale;
        float footballSpeedFactor;   // map 0x18: the ball's speed factor, 1 + frames / 10000, at most 2
    };
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

extern "C" DodgeObjectState DAT_1020c2f0[DODGE_OBJECT_COUNT];
extern "C" DodgeObjectSpeed DAT_10255510[DODGE_OBJECT_COUNT];
extern "C" DodgeObjectMatrices DAT_1086e0b8[DODGE_OBJECT_COUNT];   // menu_scene.h declares only its first matrix

#endif
