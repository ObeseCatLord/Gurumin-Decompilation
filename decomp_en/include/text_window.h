// The text window stack of the EN build (the in-game text entry and list windows): the globals every window method reads.
// This is the one declaration of them in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_TEXT_WINDOW_H
#define GURUMIN_EN_TEXT_WINDOW_H

#include "small_b11.h"

struct Mouse;
struct FontRenderer;

// Bytes of the scratch block every window's `scratch` view points at, and the side of a popup window.
const int TEXT_WINDOW_SCRATCH_SIZE = 0x200;
const int TEXT_POPUP_SIDE = 0x100;
// The full screen window's size.
const int TEXT_FULL_SCREEN_WIDTH = 640;
const int TEXT_FULL_SCREEN_HEIGHT = 480;
// Size of one text window.
const int TEXT_WINDOW_SIZE = 0x13d0;
// Margin the mouse hit test adds to a window's width and height.
const int TEXT_WINDOW_HIT_MARGIN = 0x20;
// Items (controls) one text window holds.
const int TEXT_WINDOW_MAX_ITEMS = 64;
// Characters of an item's label, including the terminator.
const int TEXT_ITEM_LABEL_SIZE = 0x18;
// Characters of the text of one choice, including the terminator.
const int TEXT_CHOICE_TEXT_SIZE = 0x80;
// TextWindow::itemKind values of the items AddPanel and AddLabel make (the other kinds are passed to AddItem).
const int TEXT_ITEM_KIND_PANEL = -2;
const int TEXT_ITEM_KIND_LABEL = -20;
// TextWindow::itemKind values of the other item makers (text_window_kinds.cpp).
const int TEXT_ITEM_KIND_PLAIN = -1;
const int TEXT_ITEM_KIND_CAPTION = -3;
const int TEXT_ITEM_KIND_BUTTON = -4;
const int TEXT_ITEM_KIND_TEXT = -5;
const int TEXT_ITEM_KIND_LIST_B = -8;
const int TEXT_ITEM_KIND_LIST_A = -9;
// Choices a list item has room for, and bytes of that room (TextWindowChoice is 0x84 bytes).
const int TEXT_LIST_CHOICE_CAPACITY = 256;
// Scale a text item's text is measured at when its width is left to the text.
const float TEXT_ITEM_MEASURE_SCALE = 0.8f;
// Height of a button or measured text item, and the margin a button adds to its text width.
const int TEXT_ITEM_LINE_HEIGHT = 16;
const int TEXT_BUTTON_MARGIN = 16;
// Scale the label text is measured at.
const float TEXT_LABEL_SCALE = 1.0f;

// One choice of a list item: the value it stands for and its text.
struct TextWindowChoice
{
    int value;                             // 0x00: the value stored in the item's result when this choice is picked
    char text[TEXT_CHOICE_TEXT_SIZE];      // 0x04: the choice's text
};

// What an item reads or shows: the variable it edits, or (for a label) the text it shows.
union TextItemSource
{
    int *value;                            // the variable the item edits
    const char *text;                      // the text of a label item
};

// One text window (the in-game text entry and list windows). Only the members recovered so far are named. The items
// are kept as parallel arrays indexed by item number (itemCount entries are in use).
struct TextWindow
{
    int left;                              // 0x000: left edge
    int top;                               // 0x004: top edge
    int width;                             // 0x008: width
    int height;                            // 0x00c: height
    int m_0010;                            // 0x010: first value the window makers store (purpose not recovered)
    int m_0014;                            // 0x014: second value the window makers store (purpose not recovered)
    int m_0018;                            // 0x018: cleared by the window makers and by SetEnabled
    int m_001c;                            // 0x01c: cleared by the window makers and by SetEnabled
    int m_0020;                            // 0x020: cleared by the window makers and by SetEnabled
    char m_0024[0x34 - 0x24];              // 0x024: not recovered
    int m_0034;                            // 0x034: state flag (the window ignores the mouse when this and the three below are clear)
    int m_0038;                            // 0x038: state flag
    int m_003c;                            // 0x03c: state flag (a window with this clear is skipped by the hit test)
    int m_0040;                            // 0x040: state flag
    int enabled;                           // 0x044: non-zero while the window takes part in mouse tests
    int m_0048;                            // 0x048: -1 when the window is made and whenever SetEnabled runs
    int m_004c;                            // 0x04c: -1 when the window is made and whenever SetEnabled runs
    int textInputPending;                  // 0x050: 1 while the window has started text input that the next switch must end
    int stacked;                           // 0x054: 1 = the window joins the window stack when enabled (see g_textWindowTop), 0 = it stands alone
    char m_0058[0x68 - 0x58];              // 0x058: not recovered
    char m_0068;                           // 0x068: cleared by the window makers
    char m_0069[0x6c - 0x69];              // 0x069: not recovered
    TextWindow *below;                     // 0x06c: the window under this one on the stack while it is stacked and enabled
    int m_0070;                            // 0x070: cleared by SetEnabled
    char m_0074;                           // 0x074: cleared by the window makers
    char m_0075[0x174 - 0x75];             // 0x075: not recovered
    TextItemSource itemSource[TEXT_WINDOW_MAX_ITEMS];   // 0x174: the variable or text each item reads
    int *choiceResult[TEXT_WINDOW_MAX_ITEMS];           // 0x274: the variable that receives the picked choice's value
    int itemX[TEXT_WINDOW_MAX_ITEMS];                   // 0x374: item position, x
    int itemY[TEXT_WINDOW_MAX_ITEMS];                   // 0x474: item position, y
    int choiceCount[TEXT_WINDOW_MAX_ITEMS];             // 0x574: choices in the item's list
    int selectedChoice[TEXT_WINDOW_MAX_ITEMS];          // 0x674: index of the choice currently picked
    int itemWidth[TEXT_WINDOW_MAX_ITEMS];               // 0x774: item width
    int itemHeight[TEXT_WINDOW_MAX_ITEMS];              // 0x874: item height
    int itemKind[TEXT_WINDOW_MAX_ITEMS];                // 0x974: item kind (TEXT_ITEM_KIND_*)
    char itemLabel[TEXT_WINDOW_MAX_ITEMS][TEXT_ITEM_LABEL_SIZE];   // 0xa74: the item's label text
    const char *itemText[TEXT_WINDOW_MAX_ITEMS];        // 0x1074: text of a TEXT_ITEM_KIND_TEXT item (was m_1074, retyped from int)
    int itemParam[TEXT_WINDOW_MAX_ITEMS];               // 0x1174: extra value of a panel item
    TextWindowChoice *choices[TEXT_WINDOW_MAX_ITEMS];   // 0x1274: the item's choices (room for the item's list)
    int m_1374;                            // 0x1374: cleared by SetEnabled
    char m_1378[0x1384 - 0x1378];          // 0x1378: not recovered
    int m_1384;                            // 0x1384: cleared by SetEnabled
    int m_1388;                            // 0x1388: cleared by SetEnabled
    int m_138c;                            // 0x138c: cleared by SetEnabled
    int itemCount;                         // 0x1390: items in use
    char m_1394[0x1398 - 0x1394];          // 0x1394: not recovered
    int m_1398;                            // 0x1398: cleared when the window is made and when SetEnabled turns it on
    char m_139c;                           // 0x139c: cleared by the window makers
    char m_139d[0x13a8 - 0x139d];          // 0x139d: not recovered
    Mouse *mouse;                          // 0x13a8: the mouse the window reads
    int m_13ac;                            // 0x13ac: third value the stacked and popup makers store (0 for the full screen maker)
    int m_13b0;                            // 0x13b0: fourth value the stacked and popup makers store (0 for the full screen maker)
    BufferView scratch;                    // 0x13b4: points at the shared scratch block g_textWindowScratch
    char m_13bc[TEXT_WINDOW_SIZE - 0x13bc];   // 0x13bc: not recovered

    // text_window_state.cpp (EN-only): the three window makers and the enable switch. Each maker returns 1.
    // EN 0x7b82c0: a 256 x 256 window at (x, y) that joins the window stack when enabled; the last two arguments are not read.
    int InitStacked(Mouse *windowMouse, int valueA, int valueB, int valueC, int valueD, int x, int y, int unusedA, int unusedB);
    // EN 0x7b88e0: the same window but standing alone (not stacked).
    int InitPopup(Mouse *windowMouse, int valueA, int valueB, int valueC, int valueD, int x, int y, int unusedA, int unusedB);
    // EN 0x7b16e0: a stand-alone window over the whole 640 x 480 screen at (0, 0).
    int InitFullScreen(Mouse *windowMouse, int valueA, int valueB);
    // EN 0x7b8690: turns the window on or off (stacking it when it is a stacked window), resets its selection state and
    // ends text input of the window that was on top before. Returns `enable`.
    int SetEnabled(int enable);
    // EN 0x7afac0 (EN-only, text_window_kinds.cpp): appends a caption item (kind TEXT_ITEM_KIND_CAPTION) at (x, y) of size itemW x itemH
    // whose label is a copy of `text`; a width of 0 measures the text at scale 0.8 (height one line). Clears `result` when given. Returns 0.
    int AddCaptionItem(int *result, int x, int y, int itemW, int itemH, const char *text);
    // EN 0x7afe10 (EN-only, text_window_items.cpp): appends an item of `kind` at (x, y) with the given list size, selection and size; `result` (when given) is cleared. Returns 0.
    int AddItem(int *result, int x, int y, int kind, int choices, int selected, int itemW, int itemH);
    // EN 0x7afd60 (EN-only): appends a panel item (kind TEXT_ITEM_KIND_PANEL) at (x, y) of the given size, carrying `param`. Returns 0.
    int AddPanel(int param, int x, int y, int itemW, int itemH);
    // EN 0x7aff80 (EN-only): appends a label item showing `text` at (x, y), sized with the window's font. Returns 0.
    int AddLabel(const char *text, int x, int y);
    // EN 0x7b0040 (EN-only): appends a choice (`text`, `value`) to item `item`; the first choice becomes the item's label and result. Returns the index of the new choice.
    int AddChoice(int item, const char *text, int value);
    // text_window_kinds.cpp (EN-only): the other item makers. Their names are provisional: only the kind number and
    // the layout they give the item are known. Each returns 0 unless said otherwise.
    int AddPlainItem(int *result, int x, int y, const char *text);   // EN 0x7b00f0: kind TEXT_ITEM_KIND_PLAIN sized from `text` at scale 1.0, label = text
    int AddButton(int *result, int *pickedInto, int x, int y, const char *text);   // EN 0x7afbb0: kind TEXT_ITEM_KIND_BUTTON, width = text width + 16, height 16, label = text
    int AddTextItem(int *result, int x, int y, int itemW, int itemH, const char *text);   // EN 0x7af9e0: kind TEXT_ITEM_KIND_TEXT keeping the pointer `text`; a width of 0 measures the text instead (height 16)
    int AddChoiceListA(int *result, int *pickedInto, int x, int y, int itemW, int itemH);   // EN 0x7afc90: empty list of kind TEXT_ITEM_KIND_LIST_A with room for 256 choices; returns its item index
    int AddChoiceListB(int *result, int *pickedInto, int x, int y, int itemW, int itemH);   // EN 0x7afeb0: the same with kind TEXT_ITEM_KIND_LIST_B
};

// 1 when the mouse of any of the first `count` windows of `windows` is inside that window (margin added to its size).
// EN 0x7b0440. No direct caller recovered.
extern "C" int TextWindows_AnyUnderMouse(TextWindow *windows, int count);

extern "C" TextWindow *g_textWindowTop;   // EN 0x0222b140: the window on top of the stack (0 = none); each window links to the one below it
extern "C" FontRenderer *g_textWindowFont;   // EN 0x0222b144: the font renderer stored by TextWindows_Reset, used by the window methods to measure text
extern "C" int g_textWindowDepth;         // EN 0x0222b148: number of windows stacked below the top one

extern "C" char g_textWindowScratch[TEXT_WINDOW_SCRATCH_SIZE];   // EN 0x02228140: scratch block shared by the windows' `scratch` views
extern "C" int g_textWindowChanged;   // EN 0x0222b150: set to 1 when SetEnabled changed which window is on top

// Empties the window stack, stores `font` and starts text input into `buffer` (TextInput_Begin). Returns 0. EN 0x7b8130.
extern "C" int TextWindows_Reset(char *buffer, FontRenderer *font);

#endif
