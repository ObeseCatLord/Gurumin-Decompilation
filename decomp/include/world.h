#pragma once

#include "vec3.h"

#include "node.h"
#include "anim_model.h"

// The character records are the models themselves: DAT_10db6ee8 (anim_model.h), one AnimModel per slot. Older
// code saw them 0x10 bytes earlier as Character records at DAT_10db6ed8; a record's id was the previous slot's
// AnimModel::nextSlotId.

struct CharSlot
{
    int m_0000;
    int m_0004;
    int pad_0008;
    int m_000c;
    char pad_0010[0x2c];
    int m_003c;
    char pad_0040[0x20];
    int m_0060;
    char pad_0064[0x40];
    char m_00a4;              // 0xa4: number of switches the character is holding
    char m_00a5;
    char m_00a6;
    char m_00a7;
    char pad_00a8[0xe0];
};

struct CharState
{
    int m_0000;
    int m_0004[6];
    char pad_001c[0xc];
    int m_0028;               // 0x28: non-zero = skip the ground probe of a shot (0x1001a590)
    char pad_002c[0xc];
    int m_0038;               // 0x38
    int m_003c;
};

struct EffectSlot
{
    int id;
    int value;
    int pad_0008;
    int type;
    int m_0010;
    char pad_0014[8];
    char name[0xc0];          // 0x1c: model or motion name
    Vec4 position;            // 0xdc
    int m_00ec;
    int active;
    char pad_00f4[8];
};

extern "C" CharSlot DAT_10daab84[0x80];
extern "C" CharState DAT_1020b430[0x80];
extern "C" EffectSlot DAT_10860df0[0x40];
extern "C" int DAT_10210d30[0x80];

