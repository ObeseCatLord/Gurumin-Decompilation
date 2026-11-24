// Per-character update dispatchers and turn limiting (EN build). JP twins: char_dispatch.cpp, char_turn.cpp.
#include "char_dispatch.h"

// Special kinds with one update routine each (0x47..0x51).
const int KIND_SPECIAL_47 = 0x47;
const int KIND_SPECIAL_48 = 0x48;
const int KIND_SPECIAL_49 = 0x49;
const int KIND_SPECIAL_4A = 0x4a;
const int KIND_SPECIAL_4B = 0x4b;
const int KIND_SPECIAL_4C = 0x4c;
const int KIND_SPECIAL_4D = 0x4d;
const int KIND_SPECIAL_4E = 0x4e;
const int KIND_SPECIAL_4F = 0x4f;
const int KIND_SPECIAL_50 = 0x50;
const int KIND_SPECIAL_51 = 0x51;

// Runs the kind-specific update of character `index` (kinds 100, 101, 105 and 107) while it is in the world and
// updates are not suspended. Always returns 0. Called every frame for boss characters.
extern "C" int Char_DispatchBossKind(int index)
{
    if (g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] > CHAR_OFFWORLD_X && g_updatesSuspended == 0)
    {
        switch (g_charRecords[index].kind)
        {
        case CHAR_KIND_BS001:
            Bs001_Update(index);
            return 0;
        case CHAR_KIND_DEFEAT:
            Char_MarkDefeated(index);
            return 0;
        case CHAR_KIND_BS002:
            Bs002_Update(index);
            return 0;
        case CHAR_KIND_BS006:
            Bs006_Update(index);
            return 0;
        }
    }
    return 0;
}

// Runs the update routine of character `index` for kinds 0x47-0x51 (one routine per kind). Returns its result, or 0
// for other kinds. Called every frame for those characters.
extern "C" int Char_DispatchSpecialKind(int index)
{
    int kind = g_charRecords[index].kind;
    if (kind == KIND_SPECIAL_47)
        return CharKind47_Update(index);
    if (kind == KIND_SPECIAL_48)
        return CharKind48_Update(index);
    if (kind == KIND_SPECIAL_49)
        return CharKind49_Update(index);
    if (kind == KIND_SPECIAL_4A)
        return CharKind4a_Update(index);
    if (kind == KIND_SPECIAL_4B)
        return CharKind4b_Update(index);
    if (kind == KIND_SPECIAL_4C)
        return CharKind4c_Update(index);
    if (kind == KIND_SPECIAL_4D)
        return CharKind4d_Update(index);
    if (kind == KIND_SPECIAL_4E)
        return CharKind4e_Update(index);
    if (kind == KIND_SPECIAL_4F)
        return CharKind4f_Update(index);
    if (kind == KIND_SPECIAL_50)
        return CharKind50_Update(index);
    if (kind == KIND_SPECIAL_51)
        return CharKind51_Update(index);
    return 0;
}

// Runs the behaviour updater of character `index` chosen by its behaviour number (1..17 and 99; any other number uses
// Enemy_ChaseLungeAndSpit variant 0). Returns 0. Called every frame for enemies.
extern "C" int Char_DispatchBehaviour(int index)
{
    switch (g_charBehaviour[index])
    {
    case BEHAVIOUR_STALK_STRIKE_A:
        Enemy_StalkAndStrike(index, 0);
        return 0;
    case BEHAVIOUR_STALK_STRIKE_B:
        Enemy_StalkAndStrike(index, 1);
        return 0;
    case BEHAVIOUR_STALK_CLAW_A:
    case BEHAVIOUR_STALK_CLAW_C:
        Enemy_StalkAndClaw(index, 2);
        return 0;
    case BEHAVIOUR_STALK_CLAW_B:
    case BEHAVIOUR_STALK_CLAW_D:
        Enemy_StalkAndClaw(index, 1);
        return 0;
    case BEHAVIOUR_DASH_SLASH:
        Enemy_StalkAndDashSlash(index);
        return 0;
    case BEHAVIOUR_HOVER_CALL_A:
        Enemy_HoverAndCallStrike(index, 0);
        return 0;
    case BEHAVIOUR_HOVER_CALL_B:
        Enemy_HoverAndCallStrike(index, 1);
        return 0;
    case BEHAVIOUR_PERCH_SWOOP:
        Enemy_PerchAndSwoop(index);
        return 0;
    case BEHAVIOUR_CHASE_LUNGE:
        Enemy_ChaseLungeAndSpit(index, 1);
        return 0;
    case BEHAVIOUR_AIMED_SHOTS:
        Enemy_AimedShots(index);
        return 0;
    case BEHAVIOUR_STAR_VOLLEY:
        Enemy_StarVolley(index);
        return 0;
    case BEHAVIOUR_SPIN_FIRE:
        Enemy_SpinFire(index);
        return 0;
    case BEHAVIOUR_CALL_BARRAGE:
        Enemy_CallBarrage(index);
        return 0;
    case BEHAVIOUR_CALL_BURSTS_A:
        Enemy_CallBursts(index, 0);
        return 0;
    case BEHAVIOUR_CALL_BURSTS_B:
        Enemy_CallBursts(index, 1);
        return 0;
    case BEHAVIOUR_DIGIT_COUNTER:
        DigitCounter_Update(index);
        return 0;
    default:
        Enemy_ChaseLungeAndSpit(index, 0);
        return 0;
    }
}

// Limits how far character `index` turns this step: the difference between its target heading and its heading, wrapped
// into -pi..pi, is clamped to -maxTurn..maxTurn and the target heading is set to heading + that difference. Returns 0.
extern "C" int Char_LimitTurn(int index, float maxTurn)
{
    float turn = g_characterModels[index].targetHeading - g_characterModels[index].heading;
    while (turn > TURN_HALF)
        turn -= TURN_FULL;
    while (turn < -TURN_HALF)
        turn += TURN_FULL;
    if (turn > maxTurn)
        turn = maxTurn;
    else if (turn < -maxTurn)
        turn = -maxTurn;
    g_characterModels[index].targetHeading = turn + g_characterModels[index].heading;
    return 0;
}
