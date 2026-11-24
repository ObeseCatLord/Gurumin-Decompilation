// ItemName, ItemText: declared here once (first used by src/item_caption.cpp).
#ifndef GURUMIN_ITEM_CAPTION_H
#define GURUMIN_ITEM_CAPTION_H

struct ItemName

{

    char name[0x40];

    char pad_0040[0x80];

};
struct ItemText

{

    char name[0x40];

    int keepOnUse;                // 0x40: non-zero = using the item does not take one from the stock

    char pad_0044[0x1c];

    int heldState;                // 0x60: DAT_104bfdf0 while the item is held

    int useScript;                // 0x64: number of the script text run when the item is used (DAT_10c5e1c4)

    char pad_0068[0xc];

    int m_0074;                   // 0x74: weapon kind of the item (2 or 3 = swords)

    char pad_0078[0x88];

    char modelPath[0x140];        // 0x100: the item's model file

};
typedef char ItemTextSizeCheck[sizeof(ItemText) == 0x240 ? 1 : -1];

#endif
