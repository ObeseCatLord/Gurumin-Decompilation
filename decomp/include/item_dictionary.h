// The item dictionary mode (game mode 5, run by the frame step): declarations of its update and of the
// globals and routines only it uses.
#ifndef GURUMIN_ITEM_DICTIONARY_H
#define GURUMIN_ITEM_DICTIONARY_H

// One 0xc0-byte record of the item table at DAT_1088e650 (the same table as DAT_1088e6b0 and DAT_1088e670, seen
// from 0x60 and 0x20 bytes earlier).
struct ItemRecord
{
    char caption[0x20];          // 0x00: first character 0 = the item has no dictionary entry
    char pad_0020[0x70];
    int hidden;                  // 0x90: non-zero = the item is left out of the dictionary
    char pad_0094[0x2c];
};

extern "C" ItemRecord DAT_1088e650[0x60];
extern "C" float DAT_10dc4a70;                            // 0.5 while the dictionary is open
extern "C" float DAT_10dc4a74;                            // 0.5 while the dictionary is open
extern "C" unsigned int DAT_104c0328;                     // frames the dictionary has run: scrolls its background
extern "C" int DAT_1030c50c;                              // set to 1 when the dictionary closes
extern "C" char DAT_104c0324;                             // cleared when the dictionary closes
extern "C" int DAT_1020283c;                              // texture slot of the dictionary background
extern "C" int DAT_1030c6c0;                              // flag the dictionary's list entries point at

// 0x10115cd0 (ItemDictionary_Update, item_dictionary.cpp): one frame of the dictionary. Returns 0.
int ItemDictionary_Update();

// 0x10038070 (DrawItemPreview, item_preview.cpp): the scene routine while the item list shows a preview.
int DrawItemPreview(void);
// 0x1000c200 (Item_DrawIcon, item_icon.cpp): draws the icon of the item in list slot `slot` at (x, y).
extern "C" int FUN_1000c200(int x, int y, int slot, float alpha);
// 0x10038010: draws the menu windows and the fades (the scene routine while the menus run without a preview).
extern "C" int FUN_10038010(void);
// 0x10017a60: qsort comparison of two item numbers by their DAT_1088e6b0 levelFlag.
extern "C" int __cdecl FUN_10017a60(const void *first, const void *second);
// 0x1000cb30 (Message_SetTitle, app_0007.cpp): sets the title of the message box.
extern "C" int FUN_1000cb30(const char *text);
// 0x1000cb90 (Caption_Set): copies the text to the caption and starts its fade.
extern "C" int FUN_1000cb90(const char *text);
// 0x101603c0 (SystemSav_ReadRecord): reads record `record` of the save file `name` into *value.
extern "C" int FUN_101603c0(const char *name, int record, int *value);
// 0x1003a9d0 (Overlay_UpdateFade, app_0010.cpp): advances the fade and closes the open overlay mode.
extern "C" void FUN_1003a9d0(void);
// 0x1015f4e0 (TextInput_Update): per-frame text input update.
extern "C" int FUN_1015f4e0(void);
// 0x10112140 (Menu_Main): the main menu, shop, sound configuration and name entry.
extern "C" int FUN_10112140(void);
// 0x1000cdf0 (Messages_Update, message_fades.cpp): fades the message boxes.
extern "C" int FUN_1000cdf0(void);

#endif
