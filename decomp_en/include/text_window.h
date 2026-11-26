// The text window stack of the EN build (the in-game text entry and list windows): the globals every window method reads.
// This is the one declaration of them in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_TEXT_WINDOW_H
#define GURUMIN_EN_TEXT_WINDOW_H

struct Mouse;
struct FontRenderer;

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
    int m_0010[(0x34 - 0x10) / 4];         // 0x010: not recovered
    int m_0034;                            // 0x034: state flag (the window ignores the mouse when this and the three below are clear)
    int m_0038;                            // 0x038: state flag
    int m_003c;                            // 0x03c: state flag (a window with this clear is skipped by the hit test)
    int m_0040;                            // 0x040: state flag
    int enabled;                           // 0x044: non-zero while the window takes part in mouse tests
    char m_0048[0x174 - 0x48];             // 0x048: not recovered
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
    char m_1374[0x1390 - 0x1374];          // 0x1374: not recovered
    int itemCount;                         // 0x1390: items in use
    char m_1394[0x13a8 - 0x1394];          // 0x1394: not recovered
    Mouse *mouse;                          // 0x13a8: the mouse the window reads
    char m_13ac[TEXT_WINDOW_SIZE - 0x13ac];   // 0x13ac: not recovered

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

// Empties the window stack, stores `font` and starts text input into `buffer` (TextInput_Begin). Returns 0. EN 0x7b8130.
extern "C" int TextWindows_Reset(char *buffer, FontRenderer *font);

#endif
