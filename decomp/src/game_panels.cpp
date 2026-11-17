#include "game.h"

struct TextureObject
{
    char pad_0000[0x38];
    void M_1014ce40(int left, int top, int right, int bottom, int frame, float alpha, int a, int b);
    void M_1014d4e0(int left, int top, int right, int bottom, int frame, float alpha, int a, int b);
    void M_1014d850(int left, int top, int right, int bottom, int frame, float alpha, int a, int b);
    void M_1014e940(int left, int top, int right, int bottom, int frame, float alpha, int a, int b);
};

extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" void FUN_1013f070(int, int, int);

int Game::M_100087f0(int x, int y, int w, int h, int frame, float alpha)
{
    FUN_1013f070(2, 0, 0);
    DAT_118d0ba8[texMenu14].M_1014d4e0(x, y, x + w, y + h, frame, alpha, 1, 0);
    DAT_118d0ba8[texMenu15].M_1014ce40(x, y, x + w, y + h, frame, alpha * 0.5f, 1, 0);
    return 0;
}

int Game::M_10008880(int x, int y, int w, int h, int frame, float alpha)
{
    FUN_1013f070(2, 0, 0);
    DAT_118d0ba8[texMenu4].M_1014e940(x, y, x + w, y + h, frame, alpha, 1, 0);
    DAT_118d0ba8[texMenu15].M_1014d850(x, y, x + w, y + h, frame, alpha * 0.5f, 1, 0);
    return 0;
}
