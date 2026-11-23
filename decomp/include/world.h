#pragma once

#include "vec3.h"
#include "char_state.h"
#include "char_slot.h"

#include "node.h"
#include "anim_model.h"

// The character records are the models themselves: DAT_10db6ee8 (anim_model.h), one AnimModel per slot. Older
// code saw them 0x10 bytes earlier as Character records at DAT_10db6ed8; a record's id was the previous slot's
// AnimModel::nextSlotId.

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

extern "C" EffectSlot DAT_10860df0[0x40];
extern "C" int DAT_10210d30[0x80];

