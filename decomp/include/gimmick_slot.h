// GimmickSlot: the record of one lever or chest of the map (0xfc bytes, DAT_10860cec). The same table is seen by
// world.h as EffectSlot (DAT_10860df0), which starts 8 bytes into each record.
#ifndef GURUMIN_GIMMICK_SLOT_H
#define GURUMIN_GIMMICK_SLOT_H

#include "app_0004.h"
#include "item_caption.h"

struct Node;
struct AnimModel;   // anim_model.h

struct GimmickSlot : Latch
{
    int m_0018;
    char pad_001c[8];
    char modelName[0x40];         // 0x24
    char motionOn[0x20];          // 0x64: colour plug motion played when the gimmick is switched on
    char motionOff[0x20];         // 0x84: colour plug motion played when it is switched off
    char pad_00a4[0x40];
    char pad_00e4[0x10];          // 0xe4: position
    Node *node;                   // 0xf4: the gimmick's model (0 = none)
    union
    {
        int active;               // 0xf8
        AnimModel *owner;         // 0xf8: the character model of a lever or chest (map triggers)
    };
};

extern "C" GimmickSlot DAT_10860cec[0x41];
extern "C" ItemName DAT_1088e650[];                       // item names, 0xc0 bytes apart (DAT_1088e670 seen 0x20 later)

#endif
