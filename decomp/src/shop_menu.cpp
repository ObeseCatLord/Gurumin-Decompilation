// The shop window: the plain entry list, the shop with and without the item preview, and the helmet upgrade list.

#include <stdio.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"
#include "node.h"
#include "debris.h"
#include "font_renderer.h"
#include "game.h"
#include "texture_object.h"
#include "char_slot.h"
#include "char_vars.h"
#include "game_vars.h"
#include "item_dictionary.h"
#include "item_info.h"
#include "script_flag.h"
#include "shop_menu.h"
#include "ui_vars.h"

extern "C" int FUN_1003c370(int item, float alpha, int x);   // Item_DrawCaption (item_caption.cpp)

// DAT_104bfd20: what the window shows.
#define SHOP_MODE_LIST 0           // plain list of the texts in DAT_10450808
#define SHOP_MODE_BUY_PREVIEW 1    // the items for sale, with the model of the one under the cursor
#define SHOP_MODE_BUY 2            // the items for sale, without a preview
#define SHOP_MODE_UPGRADE 3        // the helmets that can be upgraded

#define SHOP_FADE_FRAMES 15.0f     // DAT_104bfd2c counts the frames since the window opened
#define SHOP_ALPHA ((float)DAT_104bfd2c / SHOP_FADE_FRAMES)
#define SHOP_CURSOR_FADE 15        // the cursor shows once the window has faded in this far
#define SHOP_PREVIEW_FADE 14       // the preview shows from this frame on
#define SHOP_ROW_HEIGHT 0x16
#define SHOP_LIST_ROWS 5
#define SHOP_LIST_TEXT_SCALE 0.9f
#define SHOP_TEXT_SCALE 0.95f
#define SHOP_UPGRADE_ROWS 10
#define SHOP_BUY_ROWS 10
#define SHOP_BUY_ROWS_PREVIEW 8
#define SHOP_ROW_TOP 0x76

#define COLOUR_SELECTED 0xffffff
#define COLOUR_ENTRY 0xeedebe
#define COLOUR_TITLE 0xffefcf
#define COLOUR_UNAFFORDABLE_SELECTED 0x2f1f0f
#define COLOUR_UNAFFORDABLE 0x4f3f2f

#define TEXT_WHAT_TO_BUY "\x89\xbd\x82\xf0\x94\x83\x82\xa2\x82\xdc\x82\xb7\x82\xa9\x81H"
#define TEXT_WHICH_TO_UPGRADE "\x82\xc7\x82\xea\x82\xf0\x89\xfc\x91\xa2\x82\xb5\x82\xdc\x82\xb7\x82\xa9\x81H"
// Name, the level now and the next one, the price ("s105", "s140" and the like are the text engine's position tags).
#define FORMAT_UPGRADE "%ss105\x82k\x82\x96\x81" "D%ds140\x81\xcb\x82k\x82\x96\x81" "D%ds232%d"
#define FORMAT_UPGRADE_MAX "%ss105\x82k\x82\x96\x81" "D%ds232\x82l\x82`\x82w"
#define FORMAT_PRICE "%ss138%d"

// Upgrade prices by the level reached so far, and the price shown when nothing can be bought.
#define UPGRADE_PRICE_FIRST 10
#define UPGRADE_PRICE_LEVEL_1 0x32
#define UPGRADE_PRICE_LEVEL_2 0xfa
#define UPGRADE_PRICE_LEVEL_3 999
#define UPGRADE_PRICE_NONE 0x18696
#define UPGRADE_LEVEL_MAX 4
#define UPGRADE_FLAG_OFFSET 3205   // an item's level is in the script flag levelFlag + 3205

// The shop discounts of the worn helmet's face settings.
#define DISCOUNT_5_PERCENT 0x100000
#define DISCOUNT_10_PERCENT 0x200000
#define DISCOUNT_50_PERCENT 0x300000
#define PERCENT 100

// The item icon sheet: 8 x 8 cells, the UVs are inset against bleeding.
#define ICON_COLUMNS 8
#define ICON_CELL 0.125f
#define ICON_INSET 0.002f

// Preview model of the item under the cursor.
#define PREVIEW_DEFAULT_SCALE 100
#define PREVIEW_SCALE_STEP 0.01f
#define PREVIEW_HEIGHT_UNIT 320.0f
#define PREVIEW_SPIN_STEP 0.03f

// First entry shown in the window of the open slot (0 while it is closed).
static inline int FirstEntry()
{
    return DAT_104b7200.m_0110[DAT_104b7200.m_0088] == 0 ? 0 : DAT_104b7200.m_0134[DAT_104b7200.m_0088];
}

// Entry under the cursor in the window of the open slot (0 while it is closed).
static inline int CursorEntry()
{
    return DAT_104b7200.m_0110[DAT_104b7200.m_0088] == 0 ? 0 : DAT_104b7200.m_0140[DAT_104b7200.m_0088];
}

// 0x10056b30 (ShopMenu_Draw). A C++ function (the preview's temporary matrices need an EH frame). Draws the shop
// window by DAT_104bfd20: a list of DAT_104bfd24 texts (five rows, the box as wide as the widest text, which also
// sets the open window's rectangle); the items DAT_1042e2a8 lists for sale (10 rows, or 8 beside the 3D preview of
// the item under the cursor DAT_104bfd30), each with its icon, its name and price (helmet worn discounts, the price
// percentage and the multiplier DAT_10254590 applied, dark when the coins do not cover it) and its caption when
// under the cursor; or the helmets to upgrade (their name, level and the price of the next level, dark when
// DAT_10c6670c does not cover it). The cursor is drawn once the window has faded in.
void ShopMenu_Draw(void)
{
    int i;
    int y;
    int widest;
    if (!DAT_104bfd20)
    {
        widest = 0;
        for (i = 0; i < DAT_104bfd24; i++)
        {
            int width = DAT_1020e538.TextWidth(DAT_10450808[i], SHOP_LIST_TEXT_SCALE);
            if (width >= widest)
                widest = width;
        }
        int rows = DAT_104bfd24 > SHOP_LIST_ROWS ? SHOP_LIST_ROWS : DAT_104bfd24;
        int half = widest / 2;
        DAT_104b7200.DrawPanelAlt(300 - half, 0x78, widest + 0x28, rows * SHOP_ROW_HEIGHT + 0x2a, 0.0f, SHOP_ALPHA);
        y = 0x8e;
        for (i = 0; i < (DAT_104bfd24 > SHOP_LIST_ROWS ? SHOP_LIST_ROWS : DAT_104bfd24); i++)
        {
            int entry = FirstEntry() + i;
            if (CursorEntry() == entry)
                DAT_1020e538.DrawOutlined(0x140 - half, y, DAT_10450808[entry], COLOUR_SELECTED, 0.0f, SHOP_ALPHA,
                                          SHOP_LIST_TEXT_SCALE);
            else
                DAT_1020e538.DrawOutlined(0x140 - half, y, DAT_10450808[entry], COLOUR_ENTRY, 0.0f, SHOP_ALPHA,
                                          SHOP_LIST_TEXT_SCALE);
            y += SHOP_ROW_HEIGHT;
        }
        if (DAT_104bfd2c > SHOP_CURSOR_FADE)
            DAT_104b7200.DrawCursor(0x154 - half, DAT_104bfd30 * SHOP_ROW_HEIGHT + 0x86, 1.0f, DAT_104bf88c);
        rows = DAT_104bfd24 > SHOP_LIST_ROWS ? SHOP_LIST_ROWS : DAT_104bfd24;
        if (DAT_104b7200.m_0088 > -1)
        {
            DAT_104b7200.m_00b0[DAT_104b7200.m_0088] = 0x128 - half;
            DAT_104b7200.m_00bc[DAT_104b7200.m_0088] = 0x7a;
            DAT_104b7200.m_00c8[DAT_104b7200.m_0088] = widest + 0x28;
            DAT_104b7200.m_00d4[DAT_104b7200.m_0088] = rows * SHOP_ROW_HEIGHT + 0x2a;
        }
        return;
    }
    if (DAT_104bfd20 == SHOP_MODE_UPGRADE)
    {
        DAT_104b7200.DrawPanelAlt(0xd8, 0x2a, 0xd4, 0x40, 0.0f, SHOP_ALPHA);
        DAT_1020e538.DrawOutlined(0xec, 0x40, TEXT_WHICH_TO_UPGRADE, COLOUR_TITLE, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
        DAT_104b7200.DrawPanelAlt(0x90, 100, 0x168, 0x100, 0.0f, SHOP_ALPHA);
        DAT_104b7200.DrawPanelAlt(0x8c, 0x17a, 0x170, 0x4e, 0.2f, SHOP_ALPHA);
        y = SHOP_ROW_TOP;
        for (i = 0; i < (DAT_104bfd24 > SHOP_UPGRADE_ROWS ? SHOP_UPGRADE_ROWS : DAT_104bfd24); i++)
        {
            char name[0x100];
            char text[0x100];
            int entry = FirstEntry() + i;
            int item = DAT_1042e2a8[entry];
            strcpy(name, DAT_1088e650[item].caption);
            int level = ScriptFlag(DAT_1088e6b0[item].levelFlag + UPGRADE_FLAG_OFFSET);
            int price = UPGRADE_PRICE_NONE;
            if (level < 1)
                price = UPGRADE_PRICE_FIRST;
            if (level == 1)
                price = UPGRADE_PRICE_LEVEL_1;
            else if (level == 2)
                price = UPGRADE_PRICE_LEVEL_2;
            else if (level == 3)
                price = UPGRADE_PRICE_LEVEL_3;
            else if (level >= UPGRADE_LEVEL_MAX)
                price = UPGRADE_PRICE_NONE;
            if (level >= UPGRADE_LEVEL_MAX)
                sprintf(text, FORMAT_UPGRADE_MAX, name, level + 1);
            else
                sprintf(text, FORMAT_UPGRADE, name, level + 1, level + 2, price);
            int icon = DAT_1088e6b0[DAT_1042e2a8[entry]].icon;
            float u = icon % ICON_COLUMNS * ICON_CELL;
            float v = icon / ICON_COLUMNS * ICON_CELL;
            DAT_118d0ba8[DAT_104b7200.texIcon01].DrawTinted(0xac, y - 2, 0xc0, y + 0x12, u + ICON_INSET, v + ICON_INSET,
                                                              u + ICON_CELL - ICON_INSET, v + ICON_CELL - ICON_INSET, 0,
                                                              SHOP_ALPHA, 0, 0);
            if (price <= DAT_10c6670c)
            {
                if (CursorEntry() == entry)
                    DAT_1020e538.DrawOutlined(0xc2, y, text, COLOUR_SELECTED, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
                else
                    DAT_1020e538.DrawOutlined(0xc2, y, text, COLOUR_ENTRY, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
            }
            else
            {
                if (CursorEntry() == entry)
                    DAT_1020e538.DrawText(0xc2, y, text, COLOUR_UNAFFORDABLE_SELECTED, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
                else
                    DAT_1020e538.DrawText(0xc2, y, text, COLOUR_UNAFFORDABLE, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
            }
            DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(0x196, y - 2, 0x1c2, y + 0x14, 0, 0.875f, 0.25f, 1.0f, 0,
                                                              SHOP_ALPHA, 0, 0);
            if (CursorEntry() == entry && DAT_104bfd2c > SHOP_CURSOR_FADE)
                FUN_1003c370(DAT_1042e2a8[entry], 1.0f, 0x18);
            y += SHOP_ROW_HEIGHT;
        }
        if (DAT_104bfd2c < SHOP_CURSOR_FADE + 1)
            return;
        DAT_104b7200.DrawCursor(0x3c, (DAT_104bfd30 + 5) * SHOP_ROW_HEIGHT, 1.0f, DAT_104bf88c);
        return;
    }
    int left = 0;
    int top = 0;
    int maxRows = SHOP_BUY_ROWS;
    if (DAT_104bfd20 == SHOP_MODE_BUY)
    {
        left = 0x80;
        top = 0x10;
    }
    else if (DAT_104bfd20 == SHOP_MODE_BUY_PREVIEW)
    {
        left = 0x18;
        maxRows = SHOP_BUY_ROWS_PREVIEW;
        top = 0x10;
    }
    DAT_104b7200.DrawPanelAlt(0xe8, top + 0x2a, 0xb4, 0x40, 0.0f, SHOP_ALPHA);
    DAT_1020e538.DrawOutlined(0xfc, top + 0x40, TEXT_WHAT_TO_BUY, COLOUR_TITLE, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
    if (DAT_104bfd20 == SHOP_MODE_BUY_PREVIEW)
        DAT_104b7200.DrawPanelAlt(left + 0x40, top + 0x60, 0x110, 0xe0, 0.0f, SHOP_ALPHA);
    else
        DAT_104b7200.DrawPanelAlt(left + 0x40, top + 0x60, 0x110, 0x100, 0.0f, SHOP_ALPHA);
    if (DAT_104bfd20 == SHOP_MODE_BUY_PREVIEW)
        DAT_104b7200.DrawPanelAlt(0x160, top + 0x60, 0xe0, 0xe0, 0.9f, SHOP_ALPHA);
    DAT_104b7200.DrawPanelAlt(0x8c, 0x17a, 0x170, 0x4e, 0.2f, SHOP_ALPHA);
    if (DAT_104bfd20 == SHOP_MODE_BUY_PREVIEW && DAT_104bfd2c > SHOP_PREVIEW_FADE)
    {
        Matrix4 placement;
        Matrix4 raised;
        int item = DAT_1042e2a8[DAT_104bfd30];
        int scale = PREVIEW_DEFAULT_SCALE;
        int height = DAT_1088e6b0[item].previewHeight;
        if (DAT_1088e6b0[item].previewScale != 0)
            scale = DAT_1088e6b0[item].previewScale;
        placement = Scaling((float)scale * PREVIEW_SCALE_STEP, (float)scale * PREVIEW_SCALE_STEP,
                            (float)scale * PREVIEW_SCALE_STEP) *
                    RotationZ((float)DAT_104bf890 * PREVIEW_SPIN_STEP) * RotationY(-0.15f) * RotationX(-0.5f) *
                    TranslationMatrix(-57.0f, 0, 120.0f);
        DAT_104d2a08[DAT_1042e2a8[DAT_104bfd30]].StartProximityPass(placement.m, &DAT_10836430, &DAT_10836430, 0, 0);
        raised = DAT_10836430 * TranslationMatrix(0, (float)height / PREVIEW_HEIGHT_UNIT, 0);
        DAT_104d2a08[DAT_1042e2a8[DAT_104bfd30]].QueueDraw(&placement, &raised, 0, -1, 0, 0);
    }
    char name[0x100];
    char text[0x100];
    strncpy(text, text, 0x40);   // in the original: copies the still empty row text onto itself
    y = top + SHOP_ROW_TOP;
    for (i = 0; i < (DAT_104bfd24 > maxRows ? maxRows : DAT_104bfd24); i++)
    {
        int entry = FirstEntry() + i;
        int item = DAT_1042e2a8[entry];
        strcpy(name, DAT_1088e650[item].caption);
        int price = DAT_1088e6b0[item].price;
        if ((DAT_104c02c8 & DISCOUNT_50_PERCENT) == DISCOUNT_50_PERCENT)
            price = (int)((float)price * 0.5f);
        else if ((DAT_104c02c8 & DISCOUNT_10_PERCENT) != 0)
            price = (int)((float)price * 0.9f);
        else if ((DAT_104c02c8 & DISCOUNT_5_PERCENT) != 0)
            price = (int)((float)price * 0.95f);
        if (DAT_10c66cfc != 0)
            price = price * DAT_10c66cfc / PERCENT;
        sprintf(text, FORMAT_PRICE, name, price * DAT_10254590);
        int icon = DAT_1088e6b0[DAT_1042e2a8[entry]].icon;
        float u = icon % ICON_COLUMNS * ICON_CELL;
        float v = icon / ICON_COLUMNS * ICON_CELL;
        DAT_118d0ba8[DAT_104b7200.texIcon01].DrawTinted(left + 0x5c, y - 2, left + 0x70, y + 0x12, u + ICON_INSET,
                                                          v + ICON_INSET, u + ICON_CELL - ICON_INSET,
                                                          v + ICON_CELL - ICON_INSET, 0.0f, SHOP_ALPHA, 0, 0);
        if (price * DAT_10254590 <= DAT_10daaae8[0].coins)
        {
            if (CursorEntry() == entry)
                DAT_1020e538.DrawOutlined(left + 0x72, y, text, COLOUR_SELECTED, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
            else
                DAT_1020e538.DrawOutlined(left + 0x72, y, text, COLOUR_ENTRY, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
        }
        else
        {
            if (CursorEntry() == entry)
                DAT_1020e538.DrawText(left + 0x72, y, text, COLOUR_UNAFFORDABLE_SELECTED, 0.0f, SHOP_ALPHA,
                                      SHOP_TEXT_SCALE);
            else
                DAT_1020e538.DrawText(left + 0x72, y, text, COLOUR_UNAFFORDABLE, 0.0f, SHOP_ALPHA, SHOP_TEXT_SCALE);
        }
        DAT_118d0ba8[DAT_104b7200.texWaku02].DrawTinted(left + 0xe4, y - 2, left + 0x110, y + 0x14, 0.25f, 0.875f,
                                                          0.5f, 1.0f, 0.0f, SHOP_ALPHA, 0, 0);
        if (CursorEntry() == entry && DAT_104bfd2c > SHOP_CURSOR_FADE)
            FUN_1003c370(DAT_1042e2a8[entry], 1.0f, 0x18);
        y += SHOP_ROW_HEIGHT;
    }
    if (DAT_104bfd2c < SHOP_CURSOR_FADE + 1)
        return;
    DAT_104b7200.DrawCursor(left + 0x3c, (DAT_104bfd30 + 5) * SHOP_ROW_HEIGHT + top, 1.0f, DAT_104bf88c);
}
