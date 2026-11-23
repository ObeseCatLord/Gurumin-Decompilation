// The summary panel of a save game (shown by the save/load menu and the main menu): coins, the second counter,
// the health, the date and time, the share of the map markers cleared and a grid of the map icons reached.

#include <stdio.h>
#include <windows.h>

#include "char_slot.h"
#include "font_renderer.h"
#include "game.h"
#include "game_vars.h"
#include "hud_digits.h"
#include "render_state.h"
#include "save_summary.h"
#include "script_flag.h"
#include "texture_object.h"
#include "ui_vars.h"

extern "C" int FUN_1001e1a0(const char *name, const char *fallback);  // loads a texture, `fallback` if missing
extern "C" int FUN_10174c70(int slot, int force);                      // Texture_Release

#define COIN_DIGITS_FIRST_DIVISOR 100000   // the coin count has six digits
#define COIN_DIGITS_X 0x6a
#define SECOND_COUNTER_FIRST_DIVISOR 1000  // the second counter has four digits
#define SECOND_COUNTER_X 0x12a
#define COUNTER_DIGITS_Y 0x1a1
#define DIGIT_ADVANCE 0x18
#define HEALTH_DIGITS_X 0xe4
#define HEALTH_DIGITS_Y 0x17e
#define HEALTH_SECOND_NUMBER_OFFSET 0x52
#define DIGIT_STYLE_PLAIN 0
#define DIGIT_GROW_HALF 0.5f

#define DATE_X_RIGHT 600
#define DATE_Y 0x176
#define TIME_X_RIGHT 0x24e
#define TIME_Y 0x19a
#define TEXT_SCALE 1.8f
#define TEXT_COLOUR_WHITE 0xffffff
#define DATE_TAG_DAY_ONE_DIGIT 75         // text size tag "s075" of the weekday when the day has one digit (+10: "s085")

#define MAP_ICON_FLAG_COUNT 27
#define MAP_CLEAR_POINTS_TOTAL 0x2f       // one point per marker reached, two for a master marker
#define MAP_ICON_FALLBACK "maruf00a.bmp"
#define ICON_COLUMNS 9
#define ICON_STEP_X 0x42
#define ICON_STEP_Y 0x58
#define ICON_ORIGIN 4
#define ICON_MASTER_THRESHOLD 1          // a flag value above this is a master marker
#define ICON_SHEET_INSET (1.0f / 256)
#define ICON_SHEET_EDGE (255.0f / 256)

#define PANEL_LABEL_Z 0.7f

// The divisor that picks the digit of a number out of its remainder: a tenth of `divisor`, at least 1.
static inline int DigitUnit(int divisor)
{
    int tenth = divisor / 10;
    return tenth > 1 ? tenth : 1;
}

// 0x10021e70 (SaveSummary_Draw). Draws the panel: the two label boxes of texWaku02 with the coin count (six digits,
// leading zeros left out) and the second counter (four digits), the two health labels with the health digits
// growing leftwards, the save date "yy/m/d (weekday)" and the time right-aligned in the text renderer, the percentage
// of the map clear points reached ("%d%% clear", measured over the 27 icon flags: 1 point for a flag that is set, 2
// for a value above 1), and for every set flag the map icon in a 9-column grid, framed (and inset) with texWaku04 or,
// when the flag's value is above 1, with texWaku05. Releases an icon texture again unless it is the map icon's. Returns 0.
extern "C" int FUN_10021e70(void)
{
    int divisor;
    int x;
    int width;
    char text[80];
    SYSTEMTIME now;
    int clearPoints;
    int flagIndex;
    int iconCount;
    int level;
    int icon;
    int left;
    int top;

    FUN_1013f070(RENDER_CULL_MODE, CULL_DEFAULT, 0);
    DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(0x22, 0x194, 0x62, 0x1bc, 0.25f, 0.875f, 0.5f, 1.0f, 0.0f, 1.0f,
                                                   0.0f, 0.0f);
    divisor = COIN_DIGITS_FIRST_DIVISOR;
    x = COIN_DIGITS_X;
    do
    {
        if (x > COIN_DIGITS_X || (DAT_10daaae8[0].coins % divisor) / DigitUnit(divisor) > 0 || divisor == 10)
        {
            FUN_10012d90(x, COUNTER_DIGITS_Y, (DAT_10daaae8[0].coins % divisor) / DigitUnit(divisor), 1.0f,
                         DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
            x += DIGIT_ADVANCE;
        }
        divisor = divisor / 10;
    } while (divisor > 1);

    DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(0xe2, 0x194, 0x122, 0x1bc, 0.0f, 0.875f, 0.25f, 1.0f, 0.0f, 1.0f,
                                                   0.0f, 0.0f);
    divisor = SECOND_COUNTER_FIRST_DIVISOR;
    x = SECOND_COUNTER_X;
    do
    {
        if (x > SECOND_COUNTER_X || (DAT_10c6670c % divisor) / DigitUnit(divisor) > 0 || divisor == 10)
        {
            FUN_10012d90(x, COUNTER_DIGITS_Y, (DAT_10c6670c % divisor) / DigitUnit(divisor), 1.0f,
                         DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
            x += DIGIT_ADVANCE;
        }
        divisor = divisor / 10;
    } while (divisor > 1);

    DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(0x24, 0x17a, 0x54, 0x192, 0.0f, 0.0f, 0.125f, 0.0625f,
                                                   PANEL_LABEL_Z, 1.0f, 0.0f, 0.0f);
    DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(0x96, 0x17a, 0xae, 0x192, 0.25f, 0.0f, 0.3125f, 0.0625f,
                                                   PANEL_LABEL_Z, 1.0f, 0.0f, 0.0f);
    divisor = 10;
    x = HEALTH_DIGITS_X;
    while (true)
    {
        if (DAT_10daaae8[0].stats[STAT_MAX_HEALTH] / DigitUnit(divisor) == 0)
            break;
        FUN_10012d90(x - HEALTH_SECOND_NUMBER_OFFSET, HEALTH_DIGITS_Y,
                     (DAT_10daaae8[0].stats[STAT_MAX_HEALTH] % divisor) / DigitUnit(divisor), 1.0f,
                     DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
        FUN_10012d90(x, HEALTH_DIGITS_Y, (DAT_10daaae8[0].stats[STAT_MAX_HEALTH] % divisor) / DigitUnit(divisor),
                     1.0f, DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
        x -= DIGIT_ADVANCE;
        divisor *= 10;
    }

    GetLocalTime(&now);
    sprintf(text, "%02ds030%d\x81^%ds%03d\x81i%s\x81j", now.wYear % 100, now.wMonth, now.wDay,
            (now.wDay / 10 != 0 ? 10 : 0) + DATE_TAG_DAY_ONE_DIGIT, DAT_101d6530[now.wDayOfWeek]);
    width = DAT_1020e538.TextWidth(text, TEXT_SCALE);
    DAT_1020e538.DrawText(DATE_X_RIGHT - width, DATE_Y, text, TEXT_COLOUR_WHITE, 0, 1.0f, TEXT_SCALE);
    sprintf(text, "%02d\x81" "F%02d", now.wHour, now.wMinute);
    width = DAT_1020e538.TextWidth(text, TEXT_SCALE);
    DAT_1020e538.DrawText(TIME_X_RIGHT - width, TIME_Y, text, TEXT_COLOUR_WHITE, 0, 1.0f, TEXT_SCALE);

    clearPoints = 0;
    for (flagIndex = 0; flagIndex < MAP_ICON_FLAG_COUNT; flagIndex++)
    {
        level = ScriptFlag(DAT_101d6578[flagIndex]);
        if (level != 0)
        {
            clearPoints++;
            if (level > 1)
                clearPoints++;
        }
    }
    sprintf(text, "%d\x81\x93\x83N\x83\x8a\x83" "A\x81[", (clearPoints * 100) / MAP_CLEAR_POINTS_TOTAL);
    DAT_1020e538.TextWidth(text, TEXT_SCALE);

    iconCount = 0;
    for (flagIndex = 0; flagIndex < MAP_ICON_FLAG_COUNT; flagIndex++)
    {
        level = ScriptFlag(DAT_101d6578[flagIndex]);
        if (level != 0)
        {
            icon = FUN_1001e1a0(DAT_101d65b0[flagIndex], MAP_ICON_FALLBACK);
            left = (iconCount % ICON_COLUMNS) * ICON_STEP_X + ICON_ORIGIN;
            top = (iconCount / ICON_COLUMNS) * ICON_STEP_Y + ICON_ORIGIN;
            DAT_118d0ba8[icon].DrawTinted(left + 0xd, top + 0x40, left + 0x4d, top + 0x80, ICON_SHEET_INSET,
                                          ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f, 0.0f);
            if (DAT_1020b228 != icon)
                FUN_10174c70(icon, 0);
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
            if (level > ICON_MASTER_THRESHOLD)
            {
                DAT_118d0ba8[DAT_104b7200.texWaku05].DrawTinted(left + 0xb, top + 0x3e, left + 0x4f, top + 0x82,
                    ICON_SHEET_INSET, ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f, 0.0f);
                DAT_118d0ba8[DAT_104b7200.texWaku05].DrawTinted(left + 0xd, top + 0x40, left + 0x4d, top + 0x80,
                    ICON_SHEET_INSET, ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f, 0.0f);
            }
            else
            {
                DAT_118d0ba8[DAT_104b7200.texWaku04].DrawTinted(left + 0xb, top + 0x3e, left + 0x4f, top + 0x82,
                    ICON_SHEET_INSET, ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f, 0.0f);
                DAT_118d0ba8[DAT_104b7200.texWaku04].DrawTinted(left + 0xd, top + 0x40, left + 0x4d, top + 0x80,
                    ICON_SHEET_INSET, ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f, 0.0f);
            }
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            iconCount++;
        }
    }
    return 0;
}
