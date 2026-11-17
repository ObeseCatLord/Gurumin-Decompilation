#include <string.h>
#include "game.h"

struct TextureObject
{
    char pad_0000[0x38];
    void M_1014c430(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, int frame, int color, float alpha, int b);
    void M_1014bc60(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, int frame, float alpha, int b);
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

int Game::M_10008fa0(int x, int y, int color, int frame)
{
    m_7630 = x;
    m_7634 = y;
    FUN_1013f070(0, 1, 0);
    DAT_118d0ba8[texMenu1].M_1014c430(m_7638 - 4, m_763c + 2, m_7638 + 0x1c, m_763c + 0x22, 1.0f / 256, 97.0f / 256,
                                      31.0f / 256, 127.0f / 256, 0, color, (frame - 5) / 20.0f, 0);
    FUN_1013f070(0, 0, 0);
    return 1;
}

int Game::M_10008910(int x, int y, int lit, int rows, int a, int b, int color)
{
    FUN_1013f070(2, 0, 0);
    int right = x + 0xa0;
    if (lit != 0)
        DAT_118d0ba8[texWaku01].M_1014c430(x, y, right, y + 0x20, 0.0f, 0.875f, 0.625f, 1.0f, 0, color, 0, 0);
    else
        DAT_118d0ba8[texWaku01].M_1014c430(x, y, right, y + 0x20, 0.0f, 0.5f, 0.625f, 0.625f, 0, color, 0, 0);
    for (int i = 0; i < rows; i++)
        DAT_118d0ba8[texWaku01].M_1014c430(x, y + 0x20 + i * 0x20, right, y + 0x40 + i * 0x20, 0.0f, 0.625f, 0.625f, 0.75f, 0, color, 0, 0);
    y += rows * 0x20;
    DAT_118d0ba8[texWaku01].M_1014c430(x, y + 0x20, right, y + 0x3f, 0.0f, 0.75390625f, 0.625f, 0.875f, 0, color, 0, 0);
    return 0;
}

int Game::M_100091d0(const char *text, int x, int y)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        int glyph = text[i] - 0x20;
        if (glyph > 0)
        {
            int row = (glyph >> 5) * 16;
            int column = (glyph & 0x1f) * 8;
            float v = row * (1.0f / 256);
            float u = column * (1.0f / 256);
            DAT_118d0ba8[texMenu1].M_1014bc60(x, y, x + 8, y + 0x10, u, v, u + 0.03125f, v + 0.0625f, 0, 1.0f, 0);
        }
        x += 8;
    }
    return 1;
}
