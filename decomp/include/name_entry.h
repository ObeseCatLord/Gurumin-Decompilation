// The name entry mode (game mode 10, run by the frame step): declarations of its update and of the globals and
// routines only it uses.
#ifndef GURUMIN_NAME_ENTRY_H
#define GURUMIN_NAME_ENTRY_H

// Texture slots of the five face pictures cf02_00, cf02_07, cf02_27, cf02_03 and cf02_04 the screen loads.
extern "C" short DAT_1042d9d8[5];

// 0x10038520: the scene routine of the name entry (draws the windows and the face pictures).
extern "C" int FUN_10038520(void);
// 0x1000cbe0 (Caption_Fade, app_0008.cpp): fades the caption.
extern "C" int FUN_1000cbe0(void);
// 0x100121e0 (Char_ReloadModel, app_0010.cpp): reloads the model of character `index`.
extern "C" int FUN_100121e0(int index);
// 0x1015fd30: resets the text input.
extern "C" int FUN_1015fd30(void);
// 0x1013e6f0: sets the word `text` that messages replace by `name`.
extern "C" int FUN_1013e6f0(int id, const char *text, const char *name);

// 0x10116550 (NameEntry_Update; C++ linkage: the original has an EH frame). Game mode 10: one frame of the name
// entry. Returns 0.
int NameEntry_Update();

#endif
