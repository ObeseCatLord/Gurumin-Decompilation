// Item icons and the pixel-rectangle sprite helper (EN build). Ported from the matched JP source
// decomp/src/item_icon.cpp.
#include "game.h"
#include "item_info.h"
#include "texture_mgr.h"

// Icons per row and column of the icon sheet; each icon is a 1/8 cell of the sheet.
const int ICONS_PER_ROW = 8;
const float ICON_CELL = 0.125f;
// UV inset that keeps the icon from bleeding into its neighbours.
const float ICON_UV_INSET = 0.002f;
// Icon size on screen and its offset below the given y.
const int ICON_SIZE = 16;
const int ICON_TOP_OFFSET = 2;

// Draws the icon of the item in inventory slot `slot` as a 16x16 sprite at (x, y), insetting the UVs by 0.002 to avoid
// bleeding from the neighbouring icons. Called by the inventory and shop lists.
extern "C" int Item_DrawIcon(int x, int y, int slot, float alpha)
{
    int icon = g_itemTable[g_inventory[slot]].icon;
    float u = icon % ICONS_PER_ROW * ICON_CELL;
    float v = icon / ICONS_PER_ROW * ICON_CELL;
    g_textures[g_game.texIcon01].DrawTinted(x, y + ICON_TOP_OFFSET, x + ICON_SIZE, y + ICON_TOP_OFFSET + ICON_SIZE,
                                            u + ICON_UV_INSET, v + ICON_UV_INSET, u + ICON_CELL - ICON_UV_INSET,
                                            v + ICON_CELL - ICON_UV_INSET, 0, alpha, 0, 0, ASPECT_STRETCH);
    return 0;
}

// Draws texture slot `slot` as a screen rectangle (left, top)-(right, bottom) using a source rectangle given in texture
// pixels (u0, v0)-(u1, v1), converted here to 0..1 texture coordinates. Returns 0 for a negative slot.
extern "C" int Texture_DrawSpritePixels(int slot, int left, int top, int right, int bottom, float u0, float v0, float u1,
                                        float v1, float z, float alpha, float angle)
{
    if (slot < 0)
        return 0;
    float height = (float)g_textures[slot].height;
    float width = (float)g_textures[slot].width;
    g_textures[slot].DrawTinted(left, top, right, bottom, u0 / width, v0 / height, u1 / width, v1 / height, z, alpha,
                                angle, 0, ASPECT_STRETCH);
    return 1;
}
