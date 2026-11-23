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

    char name[0x240];

};

#endif
