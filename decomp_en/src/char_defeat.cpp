// Defeat marking variant (EN build, EN-only: no JP twin found).
#include "char.h"

// Behaviour number the character switches to when it is marked defeated by Char_MarkDefeatedAndSwitch.
const int DEFEATED_SWITCH_BEHAVIOUR = 1;

// Char_MarkDefeatedAndSwitch: like Char_MarkDefeated but also sets the behaviour of the slot to
// DEFEATED_SWITCH_BEHAVIOUR. Returns 1 (and changes nothing) when the character is busy, else 0. Called through the
// script command table (case 0x9b of the character dispatch at 0x6a0950).
extern "C" int Char_MarkDefeatedAndSwitch(int index)
{
    if (g_charPhase[index] != 0)
        return 1;
    g_charRecords[index].defeated = 1;
    g_charState[index].behaviour = DEFEATED_SWITCH_BEHAVIOUR;
    return 0;
}
