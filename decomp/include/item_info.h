// ItemInfo: the item table, one 0xc0-byte record per item kind (DAT_1088e6b0, indexed by item number).
#ifndef GURUMIN_ITEM_INFO_H
#define GURUMIN_ITEM_INFO_H

struct ItemInfo
{
    int type;                     // 0x00: 1 = equipment with a level and abilities (a helmet), 4 = stacks to 99
    int levelFlag;                // 0x04: script flag holding the item's level (minus 3205)
    char pad_0008[4];
    int consumable;               // 0x0c: non-zero = using the item takes one from the stock
    char pad_0010[8];
    int icon;                     // 0x18: cell of the item icon sheet
    char pad_001c[8];
    int healsHealth;              // 0x24: non-zero = the item restores health (refused at full health)
    int restricted;               // 0x28: non-zero = the item cannot be used here
    int drillFullFlag;            // 0x2c: script flag that is set while the drill energy is full (0 = none)
    char pad_0030[0x18];
    int price;                    // 0x48: the price in the shop
    char pad_004c[4];
    int previewHeight;            // 0x50: height of the item in the preview, as 1/320 of a unit raised
    int previewScale;             // 0x54: size of the item in the preview in percent (0 = 100)
    char pad_0058[0x68];
};

extern "C" ItemInfo DAT_1088e6b0[];                       // the item table

#endif
