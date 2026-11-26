// Per-character update dispatchers of the EN build (JP: char_dispatch.cpp, char_turn.cpp). This is the one declaration
// of these functions and globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_CHAR_DISPATCH_H
#define GURUMIN_EN_CHAR_DISPATCH_H

#include "char.h"

// Behaviour kinds (CharRecord::kind) with their own update routine.
enum CharKindUpdate
{
    CHAR_KIND_BS001 = 100,       // first boss
    CHAR_KIND_DEFEAT = 101,      // marks the character defeated
    CHAR_KIND_BS002 = 105,       // second boss
    CHAR_KIND_BS006 = 107        // sixth boss
};

// Behaviour numbers of the enemy updaters (g_charBehaviour).
enum EnemyBehaviour
{
    BEHAVIOUR_STALK_STRIKE_A = 1,
    BEHAVIOUR_STALK_STRIKE_B = 2,
    BEHAVIOUR_STALK_CLAW_A = 3,
    BEHAVIOUR_STALK_CLAW_B = 4,
    BEHAVIOUR_STALK_CLAW_C = 5,
    BEHAVIOUR_STALK_CLAW_D = 6,
    BEHAVIOUR_DASH_SLASH = 7,
    BEHAVIOUR_HOVER_CALL_A = 8,
    BEHAVIOUR_HOVER_CALL_B = 9,
    BEHAVIOUR_PERCH_SWOOP = 10,
    BEHAVIOUR_CHASE_LUNGE = 11,
    BEHAVIOUR_AIMED_SHOTS = 12,
    BEHAVIOUR_STAR_VOLLEY = 13,
    BEHAVIOUR_SPIN_FIRE = 14,
    BEHAVIOUR_CALL_BARRAGE = 15,
    BEHAVIOUR_CALL_BURSTS_A = 16,
    BEHAVIOUR_CALL_BURSTS_B = 17,
    BEHAVIOUR_DIGIT_COUNTER = 99
};

// Heading arithmetic of Char_LimitTurn: half a turn and a full turn as the game writes them.
const float TURN_HALF = 3.1415f;
const float TURN_FULL = 6.283f;

// Non-zero while character updates are suspended. EN 0x0091fe1c (JP DAT_104bfe78).
extern "C" int g_updatesSuspended;
// Behaviour number of each character slot. EN 0x0091e770 (JP DAT_1020b230).
extern "C" int g_charBehaviour[CHAR_SLOT_COUNT];

// Update routines called by the dispatchers (each takes the character slot).
extern "C" int Bs001_Update(int index);               // EN 0x67a380
extern "C" int Bs002_Update(int index);               // EN 0x680280
extern "C" int Bs006_Update(int index);               // EN 0x6883a0
extern "C" int CharKind47_Update(int index);          // EN 0x699990
extern "C" int CharKind48_Update(int index);          // EN 0x699dc0
extern "C" int CharKind49_Update(int index);          // EN 0x699f20
extern "C" int CharKind4a_Update(int index);          // EN 0x69a2e0
extern "C" int CharKind4b_Update(int index);          // EN 0x69a550
extern "C" int CharKind4c_Update(int index);          // EN 0x69aa90
extern "C" int CharKind4d_Update(int index);          // EN 0x69ba20
extern "C" int CharKind4e_Update(int index);          // EN 0x69c9f0
extern "C" int CharKind4f_Update(int index);          // EN 0x69d8a0
extern "C" int CharKind50_Update(int index);          // EN 0x69e4a0
extern "C" int CharKind51_Update(int index);          // EN 0x69f740
extern "C" int Enemy_StalkAndStrike(int index, int variant);   // EN 0x6aa540
extern "C" int Enemy_StalkAndClaw(int index, int variant);     // EN 0x6a7c50
extern "C" int Enemy_StalkAndDashSlash(int index);             // EN 0x6a5ab0
extern "C" int Enemy_HoverAndCallStrike(int index, int variant);   // EN 0x6a6c00
extern "C" int Enemy_PerchAndSwoop(int index);                 // EN 0x6965a0
extern "C" int Enemy_ChaseLungeAndSpit(int index, int variant);    // EN 0x655020
extern "C" int Enemy_AimedShots(int index);                    // EN 0x694f90
extern "C" int Enemy_StarVolley(int index);                    // EN 0x695e20
int Enemy_SpinFire(int index);                                 // EN 0x6991e0 (C++ linkage: its temporaries are unwound)
int Enemy_CallBarrage(int index);                              // EN 0x6975e0 (C++ linkage: its temporaries are unwound)
int Enemy_CallBursts(int index, int variant);                  // EN 0x695710 (C++ linkage: its temporaries are unwound)
extern "C" int DigitCounter_Update(int index);                 // EN 0x63bc80

// Runs the kind-specific update of boss kinds 100, 101, 105 and 107. Returns 0. EN 0x699840.
extern "C" int Char_DispatchBossKind(int index);
// Runs the update of kinds 0x47..0x51. EN 0x6998e0.
extern "C" int Char_DispatchSpecialKind(int index);
// Runs the behaviour updater chosen by the behaviour number of the character. Returns 0. EN 0x6a3d30.
extern "C" int Char_DispatchBehaviour(int index);
// Limits how far character `index` turns this step. Returns 0. EN 0x646230.
extern "C" int Char_LimitTurn(int index, float maxTurn);

#endif
