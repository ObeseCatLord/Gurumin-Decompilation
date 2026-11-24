// The character bank (EN build): the character model records and the model aliases of the 0x80 model slots.
// This is the one declaration of CharacterBank in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_CHARACTER_BANK_H
#define GURUMIN_EN_CHARACTER_BANK_H

#include "anim_model.h"

// Model slots of the bank.
const int CHARACTER_BANK_SLOTS = 0x80;

struct CharacterBank
{
    char m_0000[0x8860];                     // 0x00000: not used by EN code yet
    AnimModel *characters;                   // 0x08860: the character model records
    char m_8864[0xeac58 - 0x8864];           // 0x08864: not used by EN code yet
    int modelAlias[CHARACTER_BANK_SLOTS];    // 0xeac58: character a model slot shows
    int modelPartner[CHARACTER_BANK_SLOTS];  // 0xeae58: value paired with each slot

    int Assign(int slot, int character, int value);   // EN 0x743980 (JP 0x10166910)
};

#endif
