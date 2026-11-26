// The item caption of the inventory and shop screens (EN build; JP twin: decomp/src/item_caption.cpp).
#include <stdio.h>
#include <string.h>

#include "ability_text.h"
#include "char_attach.h"
#include "font_renderer.h"
#include "game.h"
#include "item_caption.h"
#include "item_info.h"
#include "render.h"

// Caption text buffer size.
const int CAPTION_TEXT_SIZE = 0x100;
// Outlined text: colour, depth and scale.
const int CAPTION_COLOUR = 0xffffff;
const float CAPTION_DEPTH = 0.0f;
const float CAPTION_SCALE = 0.8f;
// Name line and effect line: horizontal offset from the caption x, and vertical position.
const int CAPTION_NAME_OFFSET_X = 0x8c;
const int CAPTION_NAME_Y = 0x18c;
const int CAPTION_EFFECT_OFFSET_X = 0x88;
const int CAPTION_EFFECT_Y = 0x1a2;
// Script flags holding an equipment level start here (the item's flag number plus this).
const int LEVEL_FLAG_BASE = 0xc85;
// Narrow screens start at this width parameter; the divisors below depend on it and on the screen shape.
const int CAPTION_WIDE_PANEL = 10;
const float SCREEN_WIDER_THAN = 1.2f;
const float SCREEN_AT_LEAST = 1.0f;
const float WIDE_PANEL_WIDEST_DIVISOR = 2.6f;
const float WIDE_PANEL_WIDE_DIVISOR = 3.9f;
const float WIDE_PANEL_NARROW_DIVISOR = 2.5f;
const float NARROW_PANEL_WIDEST_DIVISOR = 2.3f;
const float NARROW_PANEL_WIDE_DIVISOR = 3.6f;
const float NARROW_PANEL_NARROW_DIVISOR = 2.2f;
// Back buffer pixels one caption character takes.
const float CAPTION_CHARACTER_PIXELS = 9.2f;
// Frames before the effect line starts scrolling.
const int CAPTION_SCROLL_DELAY = 5;
// "Lv" and the level number in the game's font, then the effect label and the abilities.
const char EFFECT_LINE_FORMAT[] = "\x82" "k\x82\x96\x81" "D%ds042Effect:%s";

// Item_DrawCaption: draws the name of item `item` and, for equipment, its level and abilities; the name is centred on
// `width`. The ability list scrolls when it is wider than the screen allows. Returns 0. Called by the status screen and
// the shop.
extern "C" int Item_DrawCaption(int item, float alpha, int x, int width)
{
    char name[CAPTION_TEXT_SIZE * 2];
    char text[CAPTION_TEXT_SIZE * 2];
    char line[CAPTION_TEXT_SIZE];
    name[0] = 0;
    strcpy(name, g_itemNames[item].name);
    if (name[0] == 0)
        strcpy(name, g_itemLongNames[item].name);
    sprintf(text, name);
    int left = x - width / 2;
    g_textRenderer.DrawOutlined(left + CAPTION_NAME_OFFSET_X, CAPTION_NAME_Y, text, CAPTION_COLOUR, CAPTION_DEPTH, alpha,
                                CAPTION_SCALE);
    if (g_itemTable[item].type == ITEM_TYPE_EQUIPMENT)
    {
        Ability_BuildNameList(Helmet_FaceSettings(item), text);
        g_textRenderer.DrawString(0, 0, 0, text, 0, 0);
        int length = (int)strlen(text);
        float divisor;
        if (width > CAPTION_WIDE_PANEL)
        {
            if (g_screenWidthDivisor > SCREEN_WIDER_THAN)
                divisor = (float)g_deviceBackBufferWidth / WIDE_PANEL_WIDEST_DIVISOR;
            else if (g_screenWidthDivisor > SCREEN_AT_LEAST)
                divisor = (float)g_deviceBackBufferWidth / WIDE_PANEL_WIDE_DIVISOR;
            else
                divisor = (float)g_deviceBackBufferWidth / WIDE_PANEL_NARROW_DIVISOR;
        }
        else
        {
            if (g_screenWidthDivisor > SCREEN_WIDER_THAN)
                divisor = (float)g_deviceBackBufferWidth / NARROW_PANEL_WIDEST_DIVISOR;
            else if (g_screenWidthDivisor > SCREEN_AT_LEAST)
                divisor = (float)g_deviceBackBufferWidth / NARROW_PANEL_WIDE_DIVISOR;
            else
                divisor = (float)g_deviceBackBufferWidth / NARROW_PANEL_NARROW_DIVISOR;
        }
        int visible = (int)(divisor / CAPTION_CHARACTER_PIXELS);
        if (length < visible)
        {
            sprintf(line, EFFECT_LINE_FORMAT, g_game.TestScriptFlag(g_itemTable[item].sortKey + LEVEL_FLAG_BASE) + 1,
                    text);
            g_textRenderer.DrawOutlined(left + CAPTION_EFFECT_OFFSET_X, CAPTION_EFFECT_Y, line, CAPTION_COLOUR,
                                        CAPTION_DEPTH, alpha, CAPTION_SCALE);
        }
        else
        {
            g_captionScrollLimit = length - visible;
            int start = g_captionScrollTimer - CAPTION_SCROLL_DELAY;
            if (start < 0)
                start = 0;
            if (start > g_captionScrollLimit)
                start = g_captionScrollLimit;
            text[start + visible] = 0;
            sprintf(line, EFFECT_LINE_FORMAT, g_game.TestScriptFlag(g_itemTable[item].sortKey + LEVEL_FLAG_BASE) + 1,
                    &text[start]);
            g_textRenderer.DrawOutlined(left + CAPTION_EFFECT_OFFSET_X, CAPTION_EFFECT_Y, line, CAPTION_COLOUR,
                                        CAPTION_DEPTH, alpha, CAPTION_SCALE);
        }
    }
    return 0;
}
