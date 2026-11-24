// Packed character parameters (JP twin: char_score.cpp).

#include "char.h"

// Divisor that leaves the formation number (the digits from 100000 up) of the packed value.
const int PACKED_FORMATION_DIVISOR = 100000;
// The packed value's middle four digits: value / 10 % 10000.
const int PACKED_MODE_SHIFT = 10;
const int PACKED_MODE_MODULUS = 10000;
// CharState::values index of the formation number.
const int CHARSTATE_FORMATION = 1;
// g_charBaseKind value standing for kind 0 (0 means "not saved yet").
const int CHAR_BASE_KIND_ZERO = -1;

// Splits the packed value: digits from 100000 up go to the formation number (CharState::values[1]), the middle four
// digits to CharState::behaviour. When those middle digits are not zero the character is switched to kind 0x62,
// remembering its original kind, colour and two stat values the first time so they can be restored later. Called when
// a character slot is set up from the map. Returns 0.
extern "C" int Char_UnpackParameter(int index)
{
    int value = g_charRecords[index].packedParameter;
    g_charState[index].values[CHARSTATE_FORMATION] = value / PACKED_FORMATION_DIVISOR;
    int rest = value / PACKED_MODE_SHIFT % PACKED_MODE_MODULUS;
    g_charState[index].behaviour = rest;
    if (rest != 0)
    {
        g_charKindSwitched[index] = 1;
        if (g_charBaseKind[index] == 0)
        {
            g_charBaseKind[index] = g_charRecords[index].kind;
            if (g_charBaseKind[index] == 0)
                g_charBaseKind[index] = CHAR_BASE_KIND_ZERO;
            g_charSavedColourByte[index] = g_charRecords[index].colourByte;
            g_charSavedEffectColour[index] = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            g_charSavedStat4[index] = g_charRecords[index].stats[STAT_VALUE_4];
        }
        g_charRecords[index].kind = CHAR_KIND_PACKED;
    }
    return 0;
}
