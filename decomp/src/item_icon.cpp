#include "game.h"

struct ItemInfo
{
    int type;
    char pad_0004[0x14];
    int icon;
    char pad_001c[0xa4];
};

struct TextureObject
{
    char pad_0000[0x38];
    void M_1014c430(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, int frame, int color, float alpha, int b);
};

extern "C" Game DAT_104b7200;
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" ItemInfo DAT_1088e6b0[];
extern "C" int DAT_1042e2a8[];

extern "C" int FUN_1000c200(int x, int y, int slot, int color)
{
    int icon = DAT_1088e6b0[DAT_1042e2a8[slot]].icon;
    float u = icon % 8 * 0.125f;
    float v = icon / 8 * 0.125f;
    DAT_118d0ba8[DAT_104b7200.texIcon01].M_1014c430(x, y + 2, x + 0x10, y + 0x12, u + 0.002f, v + 0.002f, u + 0.125f - 0.002f,
                                                     v + 0.125f - 0.002f, 0, color, 0, 0);
    return 0;
}
