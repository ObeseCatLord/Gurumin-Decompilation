// The field item menu (the list of items opened from the lower right corner of the screen, FieldItemMenu_Update in
// src/field_item_menu.cpp): the globals and routines only it uses.
#ifndef GURUMIN_FIELD_ITEM_MENU_H
#define GURUMIN_FIELD_ITEM_MENU_H

extern "C" int DAT_1020c2bc;                 // set while the player must not walk (a queued script or the open menu)
extern "C" int DAT_1020c2d8;                 // cleared every frame
extern "C" int DAT_1020c304;                 // non-zero = the field input is locked (counts DAT_1020c354 up to 10)
extern "C" int DAT_1020c318;                 // x of the player on the screen (the mouse walks towards the pointer from it)
extern "C" int DAT_1020c31c;                 // y of the player on the screen
extern "C" int DAT_1020c33c;                 // script number to queue (+ 20000) while the player stands still
extern "C" int DAT_1020c344;                 // set to 1 when an item is taken in hand
extern "C" int DAT_1020c354;                 // frames the field input stays locked after a menu window or the book
extern "C" int DAT_1030c510;                 // set when the item entry 0xa4 is picked
extern "C" int DAT_104c02bc;                 // the open item page (0 or 1)
extern "C" char DAT_1065cb39;                // a key of the current input frame (DemoRecording::buttons_now[0xd]); opens the menu
extern "C" int DAT_10c63518;                 // set to cancel the queued walk (cleared once seen)

// 0x1000edf0 (Message_ShowAlt, app_0009.cpp): shows a message in the alternative style.
extern "C" int FUN_1000edf0(const char *text, const char *subtext, int frames);
// 0x1000d620 (KeyPrompt_Show, app_0008.cpp): shows the key prompt over a character.
extern "C" int FUN_1000d620(int index, int key, int frames);
// 0x1000da20 (History_Push, app_0008.cpp): puts a script text on the message history.
extern "C" int FUN_1000da20(int text);
// 0x100120a0 (Inventory_Remove, app_0013.cpp): takes *amount of item *index from the stock.
extern "C" int FUN_100120a0(const int *index, int *amount);
// 0x1001f270 (Map_SelectMovie, map_movie_select.cpp): picks the background movie of the map.
int Map_SelectMovie();
// 0x10174c70 (Texture_Release, texture_mgr.cpp): releases a texture slot.
extern "C" int FUN_10174c70(int slot, int force);

// 0x100ba2a0 (FieldItemMenu_Update, field_item_menu.cpp): one frame of the field: mouse and pad input of the item
// menu, the mouse walk, using an item. Returns 0.
int FieldItemMenu_Update();

#endif
