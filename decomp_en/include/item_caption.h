// The item caption of the inventory and shop screens (EN build; JP: decomp/src/item_caption.cpp).
#ifndef GURUMIN_EN_ITEM_CAPTION_H
#define GURUMIN_EN_ITEM_CAPTION_H

// Bytes of an item name in the first name table (a 0xc0 byte record per item) and in the second one (0x240 per item).
const int ITEM_NAME_SIZE = 0x40;
const int ITEM_NAME_LONG_SIZE = 0x110;
const int ITEM_NAME_RECORD_SIZE = 0xc0;
const int ITEM_NAME_LONG_RECORD_SIZE = 0x240;

// First item name table: the name sits at the start of each record.
struct ItemNameRecord
{
    char name[ITEM_NAME_SIZE];                                  // 0x00: item name (empty = use the second table)
    char rest[ITEM_NAME_RECORD_SIZE - ITEM_NAME_SIZE];          // 0x40: the first fields of the previous record's neighbour, not a name
};

// Second item name table, used when the first name is empty.
struct ItemNameLongRecord
{
    char name[ITEM_NAME_LONG_SIZE];                             // 0x00: item name
    char rest[ITEM_NAME_LONG_RECORD_SIZE - ITEM_NAME_LONG_SIZE];// 0x110: the model path of the next item's table entry
};

extern "C" ItemNameRecord g_itemNames[];            // EN 0x00f5e418 (JP DAT_1088e670): item names, by item number
extern "C" ItemNameLongRecord g_itemLongNames[];    // EN 0x00fc6424 (JP DAT_108f667c): second item names, by item number

extern "C" unsigned char g_captionScrollTimer;      // EN 0x00b493f3 (JP DAT_104c02cc): frames the caption has been shown (scroll position)
extern "C" unsigned char g_captionScrollLimit;      // EN 0x00b79940 (JP DAT_104c02cd): how far the ability list can scroll

// Draws the caption of item `item` at horizontal position `x` (centred on `width`) with `alpha`: the item name and, for
// equipment, the "Effect:" line with its abilities (scrolling when too long for the screen). Returns 0. EN 0x5cdf70
// (JP 0x1003c370). Called by the status screen and the shop.
extern "C" int Item_DrawCaption(int item, float alpha, int x, int width);

#endif
