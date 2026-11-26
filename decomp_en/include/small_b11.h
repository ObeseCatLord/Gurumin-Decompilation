// Small EN-only functions and globals found in batch 11 (section 4: no JP twin). This is the one declaration of these
// globals and functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_SMALL_B11_H
#define GURUMIN_EN_SMALL_B11_H

#include "anim_model.h"

// The character model array a model-binding call was last given. EN 0x01664684.
extern "C" AnimModel *g_boundModels;
// Second binding value, reset to 0 together with g_boundModels. EN 0x01664174.
extern "C" int g_boundModelsExtra;
// Cursor position the 0x222 menu window last reported. EN 0x00b40da8.
extern "C" int g_menuCursorReported;
// Maximum characters of the text input being started. EN 0x02228054.
extern "C" int g_textInputLimit;
// Value picked by the last menu choice. EN 0x00a14f8c.
extern "C" int g_menuChoice;
// Mode of the menu choice (set to MENU_CHOICE_PICKED by MenuChoice_Set). EN 0x00a14f88.
extern "C" int g_menuChoiceMode;
// g_menuChoiceMode after MenuChoice_Request asked for a choice menu.
const int MENU_CHOICE_REQUESTED = 1;
// g_menuChoiceMode after a choice was made.
const int MENU_CHOICE_PICKED = 2;
// Third value of the pending choice request. EN 0x00a14f90.
extern "C" int g_menuChoiceExtra;
// Caption pointer the pending request was given (0 = none; the text itself is copied to g_menuChoiceCaption). EN 0x00a14f94.
extern "C" const char *g_menuChoiceCaptionGiven;
// Copy of the pending request's caption. EN 0x00a14f98.
extern "C" char g_menuChoiceCaption[];

// Clears the root node of every model file entry. EN 0x7347f0. Called when the model files are reset.
extern "C" void ModelFileRoots_Clear(void);
// Clears the in-use flag of every effect node. EN 0x741930.
extern "C" void EffectNodes_ClearLoaded(void);
// Clears the cached model tree of every cache slot. EN 0x7556b0.
extern "C" void NodeCache_ClearAll(void);
// Remembers the model array and resets the extra value. Returns 0. EN 0x7347d0.
extern "C" int Models_Bind(AnimModel *models);
// Window callback of the 0x222 menu: remembers the cursor. Returns 0. EN 0x62e3b0.
extern "C" int MenuCursor_Report(int cursor, char *caption);
// Sets the text input length limit. Returns 1. EN 0x7ad520.
extern "C" int TextInput_SetLimit(int limit);
// Records a menu choice. Returns 0. EN 0x63e980.
extern "C" int MenuChoice_Set(int value);
// Records a request for a choice menu (value, caption text, third value) for the main loop to open. Returns 0. EN 0x63e9a0
// (see menu_choice_request.cpp).
extern "C" int MenuChoice_Request(int value, const char *caption, int extra);

// Script values above this are negative numbers stored as value - SCRIPT_VALUE_WRAP.
const int SCRIPT_VALUE_NEGATIVE_START = 5000;
const int SCRIPT_VALUE_WRAP = 10000;
// Turns a stored script value into its signed number. EN 0x5ebf40.
extern "C" int ScriptValue_ToSigned(int stored);

// The controller the input system reads (a record of the DirectInput device); only the index is named.
struct InputPad
{
    char m_0000[0x14];      // 0x00: not recovered
    int deviceIndex;        // 0x14: index of the device
};
// The current controller (0 = none). EN 0x02228138.
extern "C" InputPad *g_inputPad;
// Value returned by Input_GetPadIndex when there is no controller.
const int INPUT_NO_PAD = -1;
// Index of the current controller, INPUT_NO_PAD when there is none. EN 0x7add90.
extern "C" int Input_GetPadIndex(void);
// Polls the controllers (large routine, not recovered yet): `pad` is a controller index (-1 = all). EN 0x7ae5c0.
extern "C" int Input_Update(int pad, int second, int first, int flag);
// Polls every controller with the two given values. EN 0x7addb0.
extern "C" int Input_UpdateAll(int first, int second);

// Scratch block the app allocated at start-up (0 = none). EN 0x02226920.
extern "C" void *g_scratchBlock;
// Frees the scratch block. Returns 0. EN 0x7a99f0.
extern "C" int ScratchBlock_Free(void);

// The image reader of the Packs function table (EN 0x7be2b0): `command` 1 closes the reader's file; the other commands
// open and decode an image into `buffer` (`rowBytes` is PACKS_READER_ROW_BYTES).
typedef int (*PacksReaderFunction)(int command, void *buffer, int rowBytes, void *widthOut, void *heightOut, int flag);
// EN 0x0222691c: entry of the Packs function table that points at the image reader.
extern "C" PacksReaderFunction g_packsReader;
// The image file the preview reader opened (0 = none). EN 0x02226934.
extern "C" struct _iobuf *g_packsImageFile;
// Row size in bytes the readers' scratch buffer is laid out for.
const int PACKS_READER_ROW_BYTES = 0x780;
// Reader command that closes the file.
const int PACKS_READER_CLOSE = 1;
// Frees the scratch block, tells the image reader to close and closes the image file. Returns 0. EN 0x7a9bb0
// (packs_reader_close.cpp).
extern "C" int Packs_CloseImageReader(void);

// A pointer to a caller-owned memory block and its size in bytes (an output buffer the program fills).
struct BufferView
{
    void *data;     // 0x00: the block
    int size;       // 0x04: size of the block in bytes
    int Set(void *newData, int newSize);   // EN 0x734cc0: points the view at a block. Returns 1
};

// Returns its argument unchanged (an unused hook of the sound code). EN 0x77bc90.
extern "C" int Hook_ReturnArgument(int value);

#endif
