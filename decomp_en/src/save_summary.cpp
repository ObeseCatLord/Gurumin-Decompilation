// The summary panel of a save game (EN build): coins, the second counter, the health, the date and time, the share of
// the map markers cleared and a grid of the map icons reached. Ported from the matched JP source
// decomp/src/save_summary.cpp.
#include <stdio.h>
#include <windows.h>

#include "anti_tamper.h"
#include "char.h"
#include "dialogs.h"
#include "font_renderer.h"
#include "game.h"
#include "hud_digits.h"
#include "map_movie.h"
#include "map_records.h"
#include "render_command.h"
#include "save_summary.h"
#include "texture_mgr.h"

// The coin count has six digits, the second counter four.
const int COIN_DIGITS_FIRST_DIVISOR = 100000;
const int COIN_DIGITS_X = 0x6a;
const int SECOND_COUNTER_FIRST_DIVISOR = 1000;
const int SECOND_COUNTER_X = 0x12a;
const int COUNTER_DIGITS_Y = 0x1a1;
// Horizontal step from one digit to the next.
const int DIGIT_ADVANCE = 0x18;
const int HEALTH_DIGITS_X = 0xe4;
const int HEALTH_DIGITS_Y = 0x17e;
// Distance of the first health number to the left of the second.
const int HEALTH_SECOND_NUMBER_OFFSET = 0x52;
const int DIGIT_STYLE_PLAIN = 0;
const float DIGIT_GROW_HALF = 0.5f;

// Right edges and tops of the date and time lines, their text scale and colour.
const int DATE_X_RIGHT = 600;
const int DATE_Y = 0x176;
const int TIME_X_RIGHT = 0x24e;
const int TIME_Y = 0x19a;
const float TEXT_SCALE = 1.8f;
const int TEXT_COLOUR_WHITE = 0xffffff;
// Text size tag "s085" of the weekday when the day has one digit (+10: "s095" with two digits).
const int DATE_TAG_DAY_ONE_DIGIT = 85;
const int DATE_TAG_TWO_DIGIT_EXTRA = 10;
// Years are shown with two digits.
const int YEAR_DIGITS_DIVISOR = 100;

// Percentage of the clear points: 47 in all (one per marker reached, two for a master marker).
const int MAP_CLEAR_POINTS_TOTAL = 0x2f;
const int PERCENT = 100;
const char MAP_ICON_FALLBACK[] = "maruf00a.bmp";
const int ICON_COLUMNS = 9;
const int ICON_STEP_X = 0x42;
const int ICON_STEP_Y = 0x58;
const int ICON_ORIGIN = 4;
// A flag value above this is a master marker.
const int ICON_MASTER_THRESHOLD = 1;
const float ICON_SHEET_INSET = 1.0f / 256;
const float ICON_SHEET_EDGE = 255.0f / 256;
const float PANEL_LABEL_Z = 0.7f;

// The divisor that picks the digit of a number out of its remainder: a tenth of `divisor`, at least 1.
static inline int DigitUnit(int divisor)
{
    int tenth = divisor / 10;
    return tenth > 1 ? tenth : 1;
}

// The decimal digit of `value` that `divisor` (a power of ten) selects.
static inline int DigitOf(int value, int divisor)
{
    return (value % divisor) / DigitUnit(divisor);
}

// Draws the panel: the two label boxes of the second HUD sheet with the coin count (six digits, leading zeros left
// out) and the second counter (four digits), the two health labels with the health digits growing leftwards, the save
// date "yy/m/d (weekday)" and the time right-aligned in the text renderer, the percentage of the map clear points
// reached ("%d%% clear", measured over the 27 icon flags: 1 point for a flag that is set, 2 for a value above 1), and
// for every set flag the map icon in a 9-column grid, framed (and inset) with the fourth HUD sheet or, when the flag's
// value is above 1, with the fifth. Releases an icon texture again unless it is the map icon's. Returns 0.
extern "C" int SaveSummary_Draw(void)
{
    char text[80];
    SYSTEMTIME now;

    Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
    g_textures[g_dialogGame.texWaku02].DrawTinted(0x22, 0x194, 0x62, 0x1bc, 0.25f, 0.875f, 0.5f, 1.0f, 0.0f, 1.0f,
                                                 0.0f, 0.0f, ASPECT_STRETCH);
    int divisor = COIN_DIGITS_FIRST_DIVISOR;
    int x = COIN_DIGITS_X;
    int digitsDrawn = 0;
    do
    {
        if (digitsDrawn > 0 || DigitOf(g_charRecords[0].money, divisor) > 0 || divisor == 10)
        {
            int digit = DigitOf(g_charRecords[0].money, divisor);
            Number_DrawRightAligned(x, COUNTER_DIGITS_Y, digit, 1.0f, DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
            digitsDrawn++;
            x += DIGIT_ADVANCE;
        }
        divisor = divisor / 10;
    } while (divisor > 1);

    g_textures[g_dialogGame.texWaku02].DrawTinted(0xe2, 0x194, 0x122, 0x1bc, 0.0f, 0.875f, 0.25f, 1.0f, 0.0f, 1.0f,
                                                 0.0f, 0.0f, ASPECT_STRETCH);
    divisor = SECOND_COUNTER_FIRST_DIVISOR;
    x = SECOND_COUNTER_X;
    digitsDrawn = 0;
    do
    {
        if (digitsDrawn > 0 || DigitOf(g_guardedValue5, divisor) > 0 || divisor == 10)
        {
            int digit = DigitOf(g_guardedValue5, divisor);
            Number_DrawRightAligned(x, COUNTER_DIGITS_Y, digit, 1.0f, DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
            digitsDrawn++;
            x += DIGIT_ADVANCE;
        }
        divisor = divisor / 10;
    } while (divisor > 1);

    g_textures[g_dialogGame.texWaku02].DrawTinted(0x24, 0x17a, 0x54, 0x192, 0.0f, 0.0f, 0.125f, 0.0625f, PANEL_LABEL_Z,
                                                 1.0f, 0.0f, 0.0f, ASPECT_STRETCH);
    g_textures[g_dialogGame.texWaku02].DrawTinted(0x96, 0x17a, 0xae, 0x192, 0.25f, 0.0f, 0.3125f, 0.0625f,
                                                 PANEL_LABEL_Z, 1.0f, 0.0f, 0.0f, ASPECT_STRETCH);
    divisor = 10;
    x = HEALTH_DIGITS_X;
    while (true)
    {
        if (g_charRecords[0].stats[STAT_MAX_HEALTH] / DigitUnit(divisor) == 0)
            break;
        Number_DrawRightAligned(x - HEALTH_SECOND_NUMBER_OFFSET, HEALTH_DIGITS_Y,
                                DigitOf(g_charRecords[0].stats[STAT_MAX_HEALTH], divisor), 1.0f,
                                DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
        Number_DrawRightAligned(x, HEALTH_DIGITS_Y,
                                DigitOf(g_charRecords[0].stats[STAT_MAX_HEALTH], divisor), 1.0f,
                                DIGIT_STYLE_PLAIN, DIGIT_GROW_HALF, 0.0f);
        x -= DIGIT_ADVANCE;
        divisor *= 10;
    }

    GetLocalTime(&now);
    sprintf(text, "%02ds030%d\x81^%ds%03d\x81i%s\x81j", now.wYear % YEAR_DIGITS_DIVISOR, now.wMonth, now.wDay,
            (now.wDay / 10 != 0 ? DATE_TAG_TWO_DIGIT_EXTRA : 0) + DATE_TAG_DAY_ONE_DIGIT,
            g_weekdayNames[now.wDayOfWeek]);
    int width = g_textRenderer.TextWidth(text, TEXT_SCALE);
    g_textRenderer.DrawText(DATE_X_RIGHT - width, DATE_Y, text, TEXT_COLOUR_WHITE, 0, 1.0f, TEXT_SCALE);
    sprintf(text, "%02d\x81" "F%02d", now.wHour, now.wMinute);
    width = g_textRenderer.TextWidth(text, TEXT_SCALE);
    g_textRenderer.DrawText(TIME_X_RIGHT - width, TIME_Y, text, TEXT_COLOUR_WHITE, 0, 1.0f, TEXT_SCALE);

    int clearPoints = 0;
    for (int flagIndex = 0; flagIndex < MAP_ICON_FLAG_COUNT; flagIndex++)
    {
        int level = g_game.TestScriptFlag(g_mapIconFlags[flagIndex]);
        if (level != 0)
        {
            clearPoints++;
            if (level > ICON_MASTER_THRESHOLD)
                clearPoints++;
        }
    }
    sprintf(text, "%d% Clear", (clearPoints * PERCENT) / MAP_CLEAR_POINTS_TOTAL);
    g_textRenderer.TextWidth(text, TEXT_SCALE);

    int iconCount = 0;
    for (int flagIndex = 0; flagIndex < MAP_ICON_FLAG_COUNT; flagIndex++)
    {
        int level = g_game.TestScriptFlag(g_mapIconFlags[flagIndex]);
        if (level != 0)
        {
            int icon = Texture_Load(g_mapIconNames[flagIndex], MAP_ICON_FALLBACK);
            int left = (iconCount % ICON_COLUMNS) * ICON_STEP_X + ICON_ORIGIN;
            int top = (iconCount / ICON_COLUMNS) * ICON_STEP_Y + ICON_ORIGIN;
            g_textures[icon].DrawTinted(left + 0xd, top + 0x40, left + 0x4d, top + 0x80, ICON_SHEET_INSET,
                                        ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f, 0.0f,
                                        ASPECT_STRETCH);
            if (g_mapIconTextureSlot != icon)
                Texture_Release(icon, 0);
            Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
            int frameSheet = level > ICON_MASTER_THRESHOLD ? g_dialogGame.texWaku05 : g_dialogGame.texWaku04;
            g_textures[frameSheet].DrawTinted(left + 0xb, top + 0x3e, left + 0x4f, top + 0x82, ICON_SHEET_INSET,
                                              ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f,
                                              0.0f, ASPECT_STRETCH);
            g_textures[frameSheet].DrawTinted(left + 0xd, top + 0x40, left + 0x4d, top + 0x80, ICON_SHEET_INSET,
                                              ICON_SHEET_INSET, ICON_SHEET_EDGE, ICON_SHEET_EDGE, 0.0f, 1.0f, 0.0f,
                                              0.0f, ASPECT_STRETCH);
            Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            iconCount++;
        }
    }
    return 0;
}
