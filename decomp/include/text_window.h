// The three on-screen text windows (DAT_10255960, 0x30 bytes each) and the picture rectangles they show
// (DAT_1049efd8). Drawn by src/text_windows_draw.cpp (FUN_1007d0a0); reset when a map is unloaded.
#ifndef GURUMIN_TEXT_WINDOW_H
#define GURUMIN_TEXT_WINDOW_H

#define TEXT_WINDOW_COUNT 3
#define TEXT_WINDOW_SLOT_COUNT 12
#define PICTURE_RECORD_COUNT 10

// TextWindow::kind
#define TEXT_WINDOW_NONE 0
#define TEXT_WINDOW_FRAMED 1    // a framed window with a text block, or a picture
#define TEXT_WINDOW_CARD 3      // a full-screen story card whose lines appear as `progress` grows

// TextWindow::style
#define TEXT_WINDOW_STYLE_PLAIN 0          // the panel is drawn by Game::DrawPanelAlt
#define TEXT_WINDOW_STYLE_PICTURE_RECT 2   // a picture, rectangle given in pixels of a square texture
#define TEXT_WINDOW_STYLE_PICTURE_FULL 3   // a picture, from its origin to the whole texture

struct TextWindow
{
    union
    {
        int slots[TEXT_WINDOW_SLOT_COUNT];
        struct
        {
            int kind;           // 0x00: TEXT_WINDOW_NONE / FRAMED / CARD
            int x;              // 0x04: left of the contents
            int y;              // 0x08: top of the contents
            int right;          // 0x0c: right of the contents
            int bottom;         // 0x10: bottom of the contents
            int progress;       // 0x14: window opacity in percent / story card frames
            int m_0018;
            int m_001c;
            int m_0020;
            int picture;        // 0x24: PictureRecord index (-1 = none)
            int style;          // 0x28: TEXT_WINDOW_STYLE_*
            int tile;           // 0x2c: texture size of a style 2 picture; the picture tile of a story card
        };
    };
};

// Where a picture lies in its texture slot (0x18 bytes).
struct PictureRecord
{
    int slot;       // 0x00: texture slot
    int m_0004;
    int left;       // 0x08
    int top;        // 0x0c
    int right;      // 0x10
    int bottom;     // 0x14
};

extern "C" TextWindow DAT_10255960[TEXT_WINDOW_COUNT];
extern "C" PictureRecord DAT_1049efd8[PICTURE_RECORD_COUNT];

extern "C" int DAT_10c66c98;                  // motion the player starts while a story card is up
extern "C" int DAT_10c64f8c;                  // non-zero = skip the pictures
extern "C" short DAT_10f08af0;                // size of the screen copy texture: width
extern "C" short DAT_10f08af2;                // height
extern "C" int FUN_10052480(void);            // the overlay mode's per-frame step

#endif
