// The field menu drawn every frame while the overlay mode is open (DAT_104bfe1c): the system menu (page 1: save,
// manuals, back to the title, quit; shortened by the map and the mini game) or the list of items (page 0, fifteen rows
// scrolled by DAT_104c0048), then the label of what the pointer would act on and the key prompt over the player.
// Coordinates are 640x480 screen pixels.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "doppler_loops.h"
#include "font_renderer.h"
#include "game.h"
#include "game_vars.h"
#include "item_dictionary.h"
#include "item_info.h"
#include "map_vars.h"
#include "mole_game.h"
#include "render_state.h"
#include "render_vars.h"
#include "script_system.h"
#include "texture_object.h"
#include "ui_vars.h"
#include "field_item_menu.h"
#include "field_item_menu_draw.h"

extern "C" int FUN_1003c370(int item, float alpha, int x);   // Item_DrawCaption (item_caption.cpp)

#define MAP_END_MENU_PAGE 1        // DAT_104c02bc: the system menu
#define MAP_ITEM_MENU_PAGE 0       // DAT_104c02bc: the list of items

#define FADE_STEPS 8.0f            // DAT_104bfe18 counts the frames the menu has been open
#define BALLOON_SCALE 0.7f
#define TEXT_SCALE 0.9f
#define TEXT_OUTLINE 0x406618
#define COLOUR_LABEL 0xffffff

// The command bars of the system menu: cells of the bar sheet on texWaku01, 22 pixels apart.
#define BAR_LEFT 500
#define BAR_RIGHT 0x208
#define BAR_TOP 0x52
#define BAR_HEIGHT 0x14
#define BAR_PITCH 0x16
#define BAR_CELL 0.125f
#define BAR_CELL_V0 0.125f
#define BAR_CELL_V1 0.25f

// The row highlight (a cell of texWaku02, drawn additively) and the cursor.
#define HIGHLIGHT_LEFT 0x1ea
#define HIGHLIGHT_RIGHT 0x278
#define HIGHLIGHT_TOP 0x50
#define HIGHLIGHT_BOTTOM 0x74
#define HIGHLIGHT_ALPHA 0.07f
#define CURSOR_X 0x1de
#define CURSOR_TOP 0x4c
#define PULSE_STEPS 16

// Entries of DAT_104485e8 that are commands, not items.
#define ENTRY_ESCAPE 0xa0
#define ENTRY_SAVE 0xa1
#define ENTRY_MANUAL 0xa2
#define ENTRY_MOVES 0xa3
#define ENTRY_TITLE 0xa4
#define ENTRY_QUIT 0xa5
#define ENTRY_MINIGAME 0xa9
#define COMMAND_ENTRY_COUNT 15

// The panel with the description of the command under the cursor.
#define DESCRIPTION_X 0x78
#define DESCRIPTION_Y 0x17a
#define DESCRIPTION_WIDTH 0x170
#define DESCRIPTION_HEIGHT 0x4e
#define DESCRIPTION_Z 0.2f
#define DESCRIPTION_TEXT_X 0x8c
#define DESCRIPTION_TEXT_Y 0x18c

// The item list: one row per owned item, 22 pixels apart, with the icon from the 8 x 8 icon sheet (texIcon01).
#define ITEM_COUNT 0x80
#define ITEM_ROWS 15
#define ITEM_ROW_TOP 0x52
#define ITEM_ROW_HEIGHT 0x16
#define ITEM_LIST_SCROLL_MIN 0xe
#define ITEM_ICON_COLUMNS 8
#define ITEM_ICON_CELL 0.125f
#define ITEM_ICON_INSET 0.002f
#define ITEM_COUNT_X 0x262
#define ITEM_COUNT_SCALE 0.5f
#define ARROW_UP_Y 0x46
#define ARROW_DOWN_Y 0x19a
#define ARROW_BLINK_PERIOD 10
#define ARROW_BLINK_ON 6
#define COLOUR_ARROW 0xffefcf
#define ITEM_TYPE_EQUIPMENT 1      // ItemInfo.type values
#define ITEM_TYPE_STACKS 4
#define COLOUR_ITEM 0xeedebe
#define COLOUR_ITEM_KEPT 0xccffcc
#define COLOUR_ITEM_STACKS 0xffcccc
#define COLOUR_ITEM_USABLE 0xffffcc
#define COLOUR_ITEM_OTHER 0xccccff

// The label of what the pointer is over (a frame at the lower right, fading in with DAT_104c0099).
#define LABEL_FADE_FRAMES 15.0f
#define LABEL_QUEUE_TALK 10000     // DAT_104bf888 from here on: a character to talk to
#define LABEL_QUEUE_NAMED 100      // from here on: an object with a caption (DAT_104bf888 - 100)
#define LABEL_FRAME_X 0x200
#define LABEL_FRAME_Y 0x136
#define LABEL_FRAME_RIGHT 0x284
#define LABEL_FRAME_BOTTOM 0x176
#define LABEL_TEXT_Y 0x14a

// The key prompt shown over the player for the pointer's target (DAT_104bf8f8, DAT_1021d138, DAT_1021f740).
#define PROMPT_FRAMES 10
#define PROMPT_ATTACK_A 3
#define PROMPT_ATTACK_B 2
#define PROMPT_TALK 6
#define PROMPT_NAMED 0xf
#define PROMPT_EXAMINE 5
#define PROMPT_OTHER_SHEET 1000
#define SOUND_PROMPT 0x356

#define TEXT_SAVE "\x83Z\x81[\x83u\x82\xb7\x82\xe9"
#define TEXT_ESCAPE "\x82\xb1\x82\xb1\x82\xa9\x82\xe7\x92" "E\x8fo"
#define TEXT_QUIT "\x83Q\x81[\x83\x80\x8fI\x97\xb9"
#define TEXT_TITLE "\x83^\x83" "C\x83g\x83\x8b\x82\xd6\x96\xdf\x82\xe9"
#define TEXT_MOVES "\x95K\x8e" "E\x8bZ\x83\x8a\x83X\x83g"
#define TEXT_MANUAL "\x91\x80\x8d\xec\x83}\x83j\x83\x85\x83" "A\x83\x8b"
#define TEXT_MINIGAME "\x83~\x83j\x83Q\x81[\x83\x80\x92\x86\x92" "f"
#define TEXT_DESC_ESCAPE "\x83\x8f\x81[\x83\x8b\x83h\x83}\x83" "b\x83v\x82\xd6\x94\xb2\x82\xaf\x8fo\x82\xb5\x82\xdc\x82\xb7\x81" "B"
#define TEXT_DESC_SAVE "\x8c\xbb\x8d\xdd\x82\xcc\x8f\xf3\x8b\xb5\x82\xf0\x83Z\x81[\x83u\x82\xb5\x82\xdc\x82\xb7\x81" "B"
#define TEXT_DESC_MANUAL "\x8a\xee\x96{\x91\x80\x8d\xec\x83}\x83j\x83\x85\x83" "A\x83\x8b\x82\xf0\x8aJ\x82\xab\x82\xdc\x82\xb7\x81" "B"
#define TEXT_DESC_MOVES "\x95K\x8e" "E\x8bZ\x82\xcc\x83\x8a\x83X\x83g\x82\xf0\x8aJ\x82\xab\x82\xdc\x82\xb7\x81" "B"
#define TEXT_DESC_TITLE "\x83^\x83" "C\x83g\x83\x8b\x89\xe6\x96\xca\x82\xc9\x96\xdf\x82\xe8\x82\xdc\x82\xb7\x81" "B"
#define TEXT_DESC_QUIT "\x83Q\x81[\x83\x80\x82\xf0\x8fI\x97\xb9\x82\xb5\x82\xc4\x82v\x82\x89\x82\x8e\x82\x84\x82\x8f\x82\x97\x82\x93\x82\xc9\x96\xdf\x82\xe8\x82\xdc\x82\xb7\x81" "B"
#define TEXT_DESC_MINIGAME "\x83~\x83j\x83Q\x81[\x83\x80\x82\xf0\x8fI\x97\xb9\x82\xb5\x82\xdc\x82\xb7\x81" "B"
#define FORMAT_STACK "\x82w%d"
#define TEXT_EXAMINE "\x82\xb5\x82\xe7\x82\xd7\x82\xe9"
#define TEXT_TALK "\x82\xa9\x82\xa2\x82\xed"
#define TEXT_ARROW_UP "\x81\xa3"
#define TEXT_ARROW_UP_OUTLINE "\x81\xa2"
#define TEXT_ARROW_DOWN "\x81\xa5"
#define TEXT_ARROW_DOWN_OUTLINE "\x81\xa4"

// Bar `row` of the command list (cell `cell` of the bar sheet) and its text.
#define COMMAND_BAR(row, cell)                                                                                     \
    DAT_118d0ba8[DAT_104b7200.texWaku01].DrawTinted(BAR_LEFT, BAR_TOP + (row) * BAR_PITCH, BAR_RIGHT,              \
                                                    BAR_TOP + (row) * BAR_PITCH + BAR_HEIGHT, (cell) * BAR_CELL,   \
                                                    BAR_CELL_V0, ((cell) + 1) * BAR_CELL, BAR_CELL_V1, 0, fade, 0, 0)
#define COMMAND_TEXT(row, text)                                                                                    \
    DAT_1020e538.DrawOutlinedIn(BAR_RIGHT, BAR_TOP + 2 + (row) * BAR_PITCH, text, COLOUR_LABEL, 0, fade, TEXT_SCALE,  \
                                TEXT_OUTLINE)

// Highlights the row under the cursor (a pulsing additive glow) and draws the cursor, for a list of `rows` commands.
#define COMMAND_CURSOR(rows)                                                                                       \
    DAT_104c0044 = (rows);                                                                                         \
    if (DAT_104bfe90 >= (rows))                                                                                 \
        DAT_104bfe90 = (rows) - 1;                                                                                 \
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);                                                                 \
    float pulse[PULSE_STEPS] = {3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f, 17.0f,                               \
                                15.0f, 13.0f, 11.0f, 9.0f, 7.0f, 5.0f, 3.0f, 1.0f};                               \
    DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(                                                               \
        HIGHLIGHT_LEFT, DAT_104bfe90 * BAR_PITCH + HIGHLIGHT_TOP, HIGHLIGHT_RIGHT,                                 \
        DAT_104bfe90 * BAR_PITCH + HIGHLIGHT_BOTTOM, 0.5f, 0.875f, 0.8125f, 1.0f, 0,                              \
        fade * pulse[DAT_104bf890 & (PULSE_STEPS - 1)] * HIGHLIGHT_ALPHA, 0, 0);                                   \
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);                                                               \
    if (DAT_104c0135 == 0)                                                                                         \
        DAT_104b7200.DrawCursor(CURSOR_X, DAT_104bfe90 * BAR_PITCH + CURSOR_TOP, 1.0f, DAT_104bf88c)

// The description of the command under the cursor.
#define COMMAND_DESCRIPTION(text)                                                                                  \
    DAT_104b7200.DrawPanelAlt(DESCRIPTION_X, DESCRIPTION_Y, DESCRIPTION_WIDTH, DESCRIPTION_HEIGHT, DESCRIPTION_Z,  \
                              fade);                                                                               \
    DAT_1020e538.DrawOutlined(DESCRIPTION_TEXT_X, DESCRIPTION_TEXT_Y, text, COLOUR_LABEL, 0, fade, TEXT_SCALE)

// Adds item `first + n` to the list of owned items that have a name.
#define LIST_OWNED_ITEM(n)                                                                                         \
    do                                                                                                             \
    {                                                                                                              \
        if (DAT_103150c0[first + (n)] != 0 && DAT_1088e650[first + (n)].caption[0] != 0)                           \
            list[count++] = &DAT_103150c0[first + (n)] - DAT_103150c0;                                                                           \
    } while (0)

// The tail every command list ends with: the entry table is set, then the description of the entry under the cursor.
#define DESCRIBE(text) DAT_1020e538.DrawOutlined(DESCRIPTION_TEXT_X, DESCRIPTION_TEXT_Y, text, COLOUR_LABEL, 0, fade, TEXT_SCALE)
#define COMMAND_TAIL                                                                                               \
    DAT_104bfe94 = 1;                                                                                          \
    if (0 < DAT_104bfe18)                                                                                          \
    {                                                                                                              \
        fade = (float)DAT_104bfe18 / FADE_STEPS;                                                                   \
        DAT_104b7200.DrawPanelAlt(DESCRIPTION_X, DESCRIPTION_Y, DESCRIPTION_WIDTH, DESCRIPTION_HEIGHT,            \
                                  DESCRIPTION_Z, fade);                                                            \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_ESCAPE)                                                            \
            DESCRIBE(TEXT_DESC_ESCAPE);                                                                            \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_SAVE)                                                              \
            DESCRIBE(TEXT_DESC_SAVE);                                                                              \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_MANUAL)                                                            \
            DESCRIBE(TEXT_DESC_MANUAL);                                                                            \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_MOVES)                                                             \
            DESCRIBE(TEXT_DESC_MOVES);                                                                             \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_TITLE)                                                             \
            DESCRIBE(TEXT_DESC_TITLE);                                                                             \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_QUIT)                                                              \
            DESCRIBE(TEXT_DESC_QUIT);                                                                              \
        if (DAT_104485e8[DAT_104bfe90] == ENTRY_MINIGAME)                                                          \
            DESCRIBE(TEXT_DESC_MINIGAME);                                                                          \
    }

// The frame of the pointer label (frame and fill from the menu sheets, fading in with DAT_104c0099).
#define LABEL_FRAME_CORNER                                                                                         \
    FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_WRAP, 0);                                                 \
    DAT_118d0ba8[DAT_104b7200.texMenu1].DrawFrameAlt(LABEL_FRAME_X, LABEL_FRAME_Y, LABEL_FRAME_RIGHT,              \
                                                     LABEL_FRAME_BOTTOM, 0, (float)DAT_104c0099 / LABEL_FADE_FRAMES, \
                                                     1, 0);                                                        \
    DAT_118d0ba8[DAT_104b7200.texMenu15].DrawFillAlt(LABEL_FRAME_X, LABEL_FRAME_Y, LABEL_FRAME_RIGHT,              \
                                                     LABEL_FRAME_BOTTOM, 0,                                        \
                                                     ((float)DAT_104c0099 / LABEL_FADE_FRAMES) * 0.5f, 1, 0)

// The blinking mark in the label's corner (two cells of the sheet, swapped every 16 frames).
#define LABEL_BLINK                                                                                                \
    blink = DAT_104bf890 >> 4 & 1;                                                                                 \
    DAT_118d0ba8[DAT_104b7200.texWaku01].DrawTinted(0x20c, 0x146, 0x224, 0x15e, (float)((blink + 0xb) * 0x10) / 256.0f, \
                                                    0.3125f, (float)((blink + 0xc) * 0x10) / 256.0f, 0.375f, 0,    \
                                                    (float)DAT_104c0099 / LABEL_FADE_FRAMES, 0, 0)

// 0x1003c570 (FieldItemMenu_Draw). Clears the command table DAT_104485e8, then while the overlay is open draws page 1
// (the command list: the mini game's, the one without "save", the one for the maps that cannot save, or the full one
// with "leave this place"; each with its highlight, cursor and the description of the command under it) or page 0
// (the owned items sorted by level flag, fifteen rows from the scroll position DAT_104c0048 with icon, name coloured by
// kind and stack count, scroll arrows, the caption of the item under the cursor). While DAT_104bfd74 is 4 it shows the
// label of the pointer's target (the verb or the object's caption by the length of the text), and last picks the key
// prompt over the player for the target. Returns 0.
int FieldItemMenu_Draw()
{
    int i;
    float fade;
    for (i = 0; i < COMMAND_ENTRY_COUNT; i++)
        DAT_104485e8[i] = 0;
    if (DAT_104bfe1c != 0)
    {
        if (DAT_104c02bc == MAP_END_MENU_PAGE)
        {
            if (DAT_10c6505c != 0)
            {
                fade = (float)DAT_104bfe18 / FADE_STEPS;
                if (0 < DAT_104bfe18)
                {
                    DAT_104b7200.DrawBalloon(0x1e4, 0x2e, 1, 10, 0, 2, fade * BALLOON_SCALE);
                    COMMAND_BAR(0, 0);
                    COMMAND_BAR(1, 2);
                    COMMAND_BAR(2, 3);
                    COMMAND_BAR(3, 4);
                    COMMAND_BAR(4, 5);
                    COMMAND_TEXT(0, TEXT_MINIGAME);
                    COMMAND_TEXT(1, TEXT_MANUAL);
                    COMMAND_TEXT(2, TEXT_MOVES);
                    COMMAND_TEXT(3, TEXT_TITLE);
                    COMMAND_TEXT(4, TEXT_QUIT);
                    COMMAND_CURSOR(5);
                    DAT_104485e8[0] = ENTRY_MINIGAME;
                    DAT_104485e8[1] = ENTRY_MANUAL;
                    DAT_104485e8[2] = ENTRY_MOVES;
                    DAT_104485e8[3] = ENTRY_TITLE;
                    DAT_104485e8[4] = ENTRY_QUIT;
                    COMMAND_TAIL
                }
            }
            else if (DAT_10c66c9c != 0)
            {
                fade = (float)DAT_104bfe18 / FADE_STEPS;
                if (0 < DAT_104bfe18)
                {
                    DAT_104b7200.DrawBalloon(0x1e4, 0x2e, 1, 10, 0, 2, fade * BALLOON_SCALE);
                    COMMAND_BAR(0, 2);
                    COMMAND_BAR(1, 3);
                    COMMAND_BAR(2, 4);
                    COMMAND_BAR(3, 5);
                    COMMAND_TEXT(0, TEXT_MANUAL);
                    COMMAND_TEXT(1, TEXT_MOVES);
                    COMMAND_TEXT(2, TEXT_TITLE);
                    COMMAND_TEXT(3, TEXT_QUIT);
                    COMMAND_CURSOR(4);
                    DAT_104485e8[0] = ENTRY_MANUAL;
                    DAT_104485e8[1] = ENTRY_MOVES;
                    DAT_104485e8[2] = ENTRY_TITLE;
                    DAT_104485e8[3] = ENTRY_QUIT;
                    COMMAND_TAIL
                }
            }
            else if (DAT_10c652cc == 0 || DAT_10c66d14 != 0 || DAT_10c62ed0 == 2 || DAT_10c62ed0 == 3 ||
                     DAT_10c62ed0 == 4 || DAT_10c62ed0 == 5 || DAT_10c62ed0 == 6 || DAT_10c62ed0 == 0xc ||
                     DAT_10c62ed0 == 0x191)
            {
                fade = (float)DAT_104bfe18 / FADE_STEPS;
                if (0 < DAT_104bfe18)
                {
                    DAT_104b7200.DrawBalloon(0x1e4, 0x2e, 1, 10, 0, 2, fade * BALLOON_SCALE);
                    COMMAND_BAR(0, 1);
                    COMMAND_BAR(1, 2);
                    COMMAND_BAR(2, 3);
                    COMMAND_BAR(3, 4);
                    COMMAND_BAR(4, 5);
                    COMMAND_TEXT(0, TEXT_SAVE);
                    COMMAND_TEXT(1, TEXT_MANUAL);
                    COMMAND_TEXT(2, TEXT_MOVES);
                    COMMAND_TEXT(3, TEXT_TITLE);
                    COMMAND_TEXT(4, TEXT_QUIT);
                    COMMAND_CURSOR(5);
                    DAT_104485e8[0] = ENTRY_SAVE;
                    DAT_104485e8[1] = ENTRY_MANUAL;
                    DAT_104485e8[2] = ENTRY_MOVES;
                    DAT_104485e8[3] = ENTRY_TITLE;
                    DAT_104485e8[4] = ENTRY_QUIT;
                    COMMAND_TAIL
                }
            }
            else
            {
                fade = (float)DAT_104bfe18 / FADE_STEPS;
                if (0 < DAT_104bfe18)
                {
                    DAT_104b7200.DrawBalloon(0x1e4, 0x2e, 1, 10, 0, 2, fade * BALLOON_SCALE);
                    COMMAND_BAR(0, 0);
                    COMMAND_BAR(1, 1);
                    COMMAND_BAR(2, 2);
                    COMMAND_BAR(3, 3);
                    COMMAND_BAR(4, 4);
                    COMMAND_BAR(5, 5);
                    COMMAND_TEXT(0, TEXT_ESCAPE);
                    COMMAND_TEXT(1, TEXT_SAVE);
                    COMMAND_TEXT(2, TEXT_MANUAL);
                    COMMAND_TEXT(3, TEXT_MOVES);
                    COMMAND_TEXT(4, TEXT_TITLE);
                    COMMAND_TEXT(5, TEXT_QUIT);
                    COMMAND_CURSOR(6);
                    DAT_104485e8[0] = ENTRY_ESCAPE;
                    DAT_104485e8[1] = ENTRY_SAVE;
                    DAT_104485e8[2] = ENTRY_MANUAL;
                    DAT_104485e8[3] = ENTRY_MOVES;
                    DAT_104485e8[4] = ENTRY_TITLE;
                    DAT_104485e8[5] = ENTRY_QUIT;
                    COMMAND_TAIL
                }
            }
        }
        if (DAT_104bfe1c != 0 && DAT_104c02bc == MAP_ITEM_MENU_PAGE)
        {
            fade = (float)DAT_104bfe18 / FADE_STEPS;
            if (0 < DAT_104bfe18)
            {
                int list[ITEM_COUNT];
                int count = 0;
                DAT_104b7200.DrawBalloon(0x1e4, 0x2e, 0, 10, 0, 2, fade * BALLOON_SCALE);
                for (int first = 0; first < ITEM_COUNT; first += 8)
                {
                    LIST_OWNED_ITEM(0);
                    LIST_OWNED_ITEM(1);
                    LIST_OWNED_ITEM(2);
                    LIST_OWNED_ITEM(3);
                    LIST_OWNED_ITEM(4);
                    LIST_OWNED_ITEM(5);
                    LIST_OWNED_ITEM(6);
                    LIST_OWNED_ITEM(7);
                }
                qsort(list, count, sizeof(int), FUN_10017a60);
                if (count <= ITEM_LIST_SCROLL_MIN)
                    DAT_104c0048 = 0;
                if (DAT_104bfe90 >= 0 && count != 0)
                {
                    int item;
                    int seen = 0;
                    DAT_104b7200.DrawPanelAlt(DESCRIPTION_X, DESCRIPTION_Y, DESCRIPTION_WIDTH, DESCRIPTION_HEIGHT,
                                              DESCRIPTION_Z, fade);
                    for (i = 0; i < count; i++)
                    {
                        item = list[i];
                        if (DAT_103150c0[item] != 0 && DAT_1088e650[item].caption[0] != 0)
                        {
                            if (seen == DAT_104c0048 + DAT_104bfe90)
                                break;
                            seen++;
                        }
                    }
                    FUN_1003c370(item, fade, 0);
                }
                if (DAT_104c0048 != 0)
                {
                    DAT_1020e538.DrawText(ITEM_COUNT_X, ARROW_UP_Y,
                                          DAT_104bf890 % ARROW_BLINK_PERIOD < ARROW_BLINK_ON ? TEXT_ARROW_UP
                                                                                              : TEXT_ARROW_UP_OUTLINE,
                                          COLOUR_ARROW, 0, fade, 1.0f);
                }
                if (count - DAT_104c0048 > ITEM_ROWS)
                {
                    DAT_1020e538.DrawText(ITEM_COUNT_X, ARROW_DOWN_Y,
                                          DAT_104bf890 % ARROW_BLINK_PERIOD < ARROW_BLINK_ON ? TEXT_ARROW_DOWN
                                                                                              : TEXT_ARROW_DOWN_OUTLINE,
                                          COLOUR_ARROW, 0, fade, 1.0f);
                }
                int shown = 0;
                int skip = DAT_104c0048;
                int y = ITEM_ROW_TOP;
                for (i = 0; i < count; i++)
                {
                    int item = list[i];
                    if (skip != 0 && DAT_103150c0[item] != 0 && DAT_1088e650[item].caption[0] != 0)
                    {
                        skip--;
                    }
                    else if (shown < ITEM_ROWS)
                    {
                        if (DAT_103150c0[item] != 0)
                        {
                            if (DAT_1088e650[item].caption[0] != 0)
                            {
                                DAT_104485e8[shown] = item;
                                int icon = DAT_1088e6b0[item].icon;
                                float u = (float)(icon % ITEM_ICON_COLUMNS) * ITEM_ICON_CELL;
                                float v = (float)(icon / ITEM_ICON_COLUMNS) * ITEM_ICON_CELL;
                                DAT_118d0ba8[DAT_104b7200.texIcon01].DrawTinted(
                                    BAR_LEFT, y, BAR_RIGHT, y + BAR_HEIGHT, u + ITEM_ICON_INSET, v + ITEM_ICON_INSET,
                                    u + ITEM_ICON_CELL - ITEM_ICON_INSET, v + ITEM_ICON_CELL - ITEM_ICON_INSET, 0, fade,
                                    0, 0);
                                if (DAT_10daabb4 == item)
                                    DAT_118d0ba8[DAT_104b7200.texWaku01].DrawTinted(0x1f6, y, 0x20a, y + BAR_HEIGHT,
                                                                                    0.75f, 0.125f, 0.875f, 0.25f, 0,
                                                                                    fade, 0, 0);
                                if (DAT_10daabb8 == item)
                                    DAT_118d0ba8[DAT_104b7200.texWaku01].DrawTinted(0x1f6, y, 0x20a, y + BAR_HEIGHT,
                                                                                    0.75f, 0.125f, 0.875f, 0.25f, 0,
                                                                                    fade, 0, 0);
                                int colour;
                                int type = DAT_1088e6b0[item].type;
                                if (type == 0)
                                {
                                    if (DAT_108f667c[item].keepOnUse == 0)
                                        colour = COLOUR_ITEM;
                                    else
                                        colour = COLOUR_ITEM_KEPT;
                                }
                                else if (type == ITEM_TYPE_STACKS)
                                    colour = COLOUR_ITEM_STACKS;
                                else if (type == 2 || type == 3)
                                    colour = COLOUR_ITEM_USABLE;
                                else
                                    colour = COLOUR_ITEM_OTHER;
                                DAT_1020e538.DrawOutlinedIn(BAR_RIGHT, y + 2, DAT_1088e650[item].caption, colour, 0, fade,
                                                            TEXT_SCALE, TEXT_OUTLINE);
                                if (DAT_104bfe90 == shown)
                                {
                                    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                                    float pulse[PULSE_STEPS] = {3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f, 17.0f,
                                                                15.0f, 13.0f, 11.0f, 9.0f, 7.0f, 5.0f, 3.0f, 1.0f};
                                    DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(
                                        HIGHLIGHT_LEFT, y - 2, HIGHLIGHT_RIGHT, y + 0x22, 0.5f, 0.875f, 0.8125f, 1.0f, 0,
                                        fade * pulse[DAT_104bf890 & (PULSE_STEPS - 1)] * HIGHLIGHT_ALPHA, 0, 0);
                                    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                                }
                                if (DAT_103150c0[item] > 1)
                                {
                                    char text[0x100];
                                    sprintf(text, FORMAT_STACK, (int)DAT_103150c0[item]);
                                    DAT_1020e538.DrawText(ITEM_COUNT_X, y + 8, text, COLOUR_LABEL, 0, fade,
                                                          ITEM_COUNT_SCALE);
                                }
                                shown++;
                                y += ITEM_ROW_HEIGHT;
                            }
                        }
                    }
                }
                if (shown == 0)
                {
                    DAT_104bfe94 = 0;
                }
                else
                {
                    if (shown <= DAT_104bfe90)
                        DAT_104bfe90 = shown - 1;
                    if (DAT_104c0135 == 0)
                        DAT_104b7200.DrawCursor(CURSOR_X, DAT_104bfe90 * BAR_PITCH + CURSOR_TOP, 1.0f, DAT_104bf88c);
                    DAT_104bfe94 = 1;
                }
                if (DAT_104bfe1c != 0)
                {
                    DAT_1030c478[DAT_104bfd74] = (char)DAT_104bfe90;
                    DAT_1049f1f8[DAT_104bfd74] = (char)DAT_104c0048;
                }
                DAT_104c0044 = 0;
                for (i = 0; i < count; i++)
                {
                    if (DAT_103150c0[list[i]] != 0 && DAT_1088e650[list[i]].caption[0] != 0)
                        DAT_104c0044++;
                }
            }
        }
    }
    if (DAT_104bfd74 == 4)
    {
        unsigned int blink;
        if (DAT_104bf888 >= LABEL_QUEUE_TALK)
        {
            LABEL_FRAME_CORNER;
            LABEL_BLINK;
            DAT_1020e538.DrawOutlined(0x230, LABEL_TEXT_Y, TEXT_TALK, COLOUR_LABEL, 0,
                                      (float)DAT_104c0099 / LABEL_FADE_FRAMES, 1.0f);
        }
        else if (DAT_104bf888 >= LABEL_QUEUE_NAMED)
        {
            const char *caption = DAT_10860e0c[DAT_104bf888 - LABEL_QUEUE_NAMED].text;
            int length;
            int x;
            LABEL_FRAME_CORNER;
            LABEL_BLINK;
            length = strlen(caption);
            if (length < 5)
                x = 0x238;
            else if (length < 7)
                x = 0x230;
            else if (length < 9)
                x = 0x228;
            else
                x = 0;
            if (x != 0)
                DAT_1020e538.DrawOutlined(x, LABEL_TEXT_Y, caption, COLOUR_LABEL, 0,
                                          (float)DAT_104c0099 / LABEL_FADE_FRAMES, 1.0f);
        }
        else if (DAT_104bf888 != 0)
        {
            DAT_104b7200.DrawPanelAlt(LABEL_FRAME_X, LABEL_FRAME_Y, 0x84, 0x40, 0, (float)DAT_104c0099 / LABEL_FADE_FRAMES);
            LABEL_BLINK;
            DAT_1020e538.DrawOutlined(0x228, LABEL_TEXT_Y, TEXT_EXAMINE, COLOUR_LABEL, 0,
                                      (float)DAT_104c0099 / LABEL_FADE_FRAMES, 1.0f);
        }
    }
    if (DAT_1032d69c == 0 || DAT_104c00d0 != 0 || DAT_104bfdf0 != 0 || (DAT_104bfd74 != 4 && DAT_104bfe7c != 0))
    {
        DAT_104bfe8c = 0;
    }
    else
    {
        int sheet = 0;
        if (DAT_104bfd74 != 4)
            sheet = PROMPT_OTHER_SHEET;
        if (DAT_104c01f8 == 1)
        {
            if (DAT_104bfe1c != 0)
                return 0;
            if (DAT_104bf8f8[0] == 0 || DAT_1021d138[0] != PROMPT_ATTACK_A)
            {
                DAT_1021d138[0] = PROMPT_ATTACK_A;
                DAT_1021f740[0] = PROMPT_FRAMES;
                DAT_104bf8f8[0] = 1;
                return 0;
            }
        }
        else
        {
            if (DAT_104c01f8 != 2)
            {
                if (DAT_104bf888 >= LABEL_QUEUE_TALK)
                {
                    if (DAT_104bfe1c == 0)
                    {
                        if (DAT_104bf8f8[0] == 0 || DAT_1021d138[0] != sheet + PROMPT_TALK)
                        {
                            DAT_104bf8f8[0] = 1;
                            DAT_1021d138[0] = sheet + PROMPT_TALK;
                        }
                        DAT_1021f740[0] = PROMPT_FRAMES;
                    }
                    if (DAT_104bfe8c != 4)
                        FUN_1003ad20(SOUND_PROMPT, 0, 1.0f);
                    DAT_104bfe8c = 4;
                    return 0;
                }
                if (DAT_104bf888 >= LABEL_QUEUE_NAMED)
                {
                    if (DAT_104bfe1c == 0)
                    {
                        if (DAT_104bf8f8[0] == 0 || DAT_1021d138[0] != sheet + PROMPT_NAMED)
                        {
                            DAT_104bf8f8[0] = 1;
                            DAT_1021d138[0] = sheet + PROMPT_NAMED;
                        }
                        DAT_1021f740[0] = PROMPT_FRAMES;
                    }
                    if (DAT_104bfe8c != 5)
                        FUN_1003ad20(SOUND_PROMPT, 0, 1.0f);
                    DAT_104bfe8c = 5;
                    return 0;
                }
                if (DAT_104bf888 != 0)
                {
                    if (DAT_104bfe1c == 0)
                        FUN_1000d620(0, sheet + PROMPT_EXAMINE, PROMPT_FRAMES);
                    if (DAT_104bfe8c != 6)
                        FUN_1003ad20(SOUND_PROMPT, 0, 1.0f);
                    DAT_104bfe8c = 6;
                    return 0;
                }
                DAT_104bfe8c = 0;
                return 0;
            }
            if (DAT_104bfe1c != 0)
                return 0;
            if (DAT_104bf8f8[0] == 0 || DAT_1021d138[0] != PROMPT_ATTACK_B)
            {
                DAT_1021d138[0] = PROMPT_ATTACK_B;
                DAT_1021f740[0] = PROMPT_FRAMES;
                DAT_104bf8f8[0] = 1;
                return 0;
            }
        }
        DAT_1021f740[0] = PROMPT_FRAMES;
    }
    return 0;
}
