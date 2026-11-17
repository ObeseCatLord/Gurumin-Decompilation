// Item icons.

#include "game.h"

// Item table at 0x1088e6b0, 0xc0 bytes per item; `icon` indexes the 8x8 icon sheet texIcon01.
struct ItemInfo
{
    int type;
    char pad_0004[0x14];
    int icon;
    char pad_001c[0xa4];
};

struct TextureObject
{
    char pad_0000[0x8];
    int width;    // 0x08: texture width in pixels
    int height;   // 0x0c: texture height in pixels
    char pad_0010[0x28];
    void M_1014c430(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, int frame, int color, float alpha, int b);
};

extern "C" Game DAT_104b7200;
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" ItemInfo DAT_1088e6b0[];
extern "C" int DAT_1042e2a8[];

// Draws the icon of the item in inventory slot `slot` as a 16x16 sprite at (x, y), insetting the UVs by 0.002
// to avoid bleeding from the neighbouring icons.
extern "C" int FUN_1000c200(int x, int y, int slot, int color)
{
    int icon = DAT_1088e6b0[DAT_1042e2a8[slot]].icon;
    float u = icon % 8 * 0.125f;
    float v = icon / 8 * 0.125f;
    DAT_118d0ba8[DAT_104b7200.texIcon01].M_1014c430(x, y + 2, x + 0x10, y + 0x12, u + 0.002f, v + 0.002f, u + 0.125f - 0.002f,
                                                     v + 0.125f - 0.002f, 0, color, 0, 0);
    return 0;
}

// Draws texture slot `slot` as a screen rectangle (left, top)-(right, bottom) using a source rectangle given in
// texture pixels (u0, v0)-(u1, v1), converted here to 0..1 texture coordinates. Returns 0 for a negative slot.
extern "C" int FUN_101616e0(int slot, int left, int top, int right, int bottom, float u0, float v0, float u1, float v1,
                            int frame, int color, float alpha)
{
    if (slot < 0)
        return 0;
    float height = (float)DAT_118d0ba8[slot].height;
    float width = (float)DAT_118d0ba8[slot].width;
    DAT_118d0ba8[slot].M_1014c430(left, top, right, bottom, u0 / width, v0 / height, u1 / width, v1 / height, frame, color,
                                  alpha, 0);
    return 1;
}
