// The field menu drawn over the field (FieldItemMenu_Draw, src/field_item_menu_draw.cpp): the list of items and the
// system menu opened from the lower right corner, and the label of what the pointer would act on.
#ifndef GURUMIN_FIELD_ITEM_MENU_DRAW_H
#define GURUMIN_FIELD_ITEM_MENU_DRAW_H

// One record of the target captions (0xfc bytes apart): the name of the object the pointer is over.
struct TargetCaption
{
    char text[0xfc];
};

extern "C" int DAT_10c66c9c;                  // non-zero = the system menu has no "save" entry
extern "C" TargetCaption DAT_10860e0c[];      // captions of the objects (DAT_104bf888 - 100 is the record)

// 0x1003c570 (FieldItemMenu_Draw, field_item_menu_draw.cpp): draws the field menu and the pointer label and keeps the
// key prompt of what the pointer is over. Returns 0.
int FieldItemMenu_Draw();

#endif
