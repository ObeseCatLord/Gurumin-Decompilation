// The loading indicator: while data loads, the loaders call this to keep a small spinner, a progress bar and the
// percentage on screen (in the top-left 64x64 corner, presented on its own).

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

struct TextureObject
{
    char pad_0000[0x38];
    // 0x1014bc60: draws the cell (u0, v0)-(u1, v1) into the rectangle, turned by `angle`.
    void DrawCell(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, int frame,
                    float alpha, float angle);
};

struct Game
{
    char pad_0000[8];
    int texMenu1;             // 0x08: the menu texture sheet
    int DrawDebugText(const char *text, int x, int y);   // 0x100091d0: debug-font text (game_panels.cpp)
};

// The engine object at 0x10836c70.
struct Engine
{
    int Present(const RECT *source);   // 0x1013edc0 (Render_Present)
    int PumpAndRecover();              // 0x1015c850 (render_shutdown.cpp)
};

extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" Game DAT_104b7200;
extern "C" Engine DAT_10836c70;
extern "C" int DAT_10f6552c;          // 1 once WM_QUIT was received
extern "C" DWORD DAT_104c032c;        // tick count of the last update
extern "C" int DAT_10f65478;          // 1 while the indicator draws (render target reset in progress)
extern "C" float DAT_104c031c;        // spinner angle
extern "C" float DAT_104c0320;        // loading progress in percent
extern "C" int FUN_101439e0(void);    // Render_ResetTarget
extern "C" void FUN_1014b390(void);   // render_reset.cpp

// 0x1000fb80 (Loading_Tick). At most every 34 ms: turns the spinner a little, draws it, the progress bar (44 pixels
// at 100 %) and the percentage, presents the corner and handles the window messages. Returns 1, or 0 once the game is
// quitting.
extern "C" int FUN_1000fb80(void)
{
    char text[20];
    if (DAT_10f6552c != 0)
        return 0;
    DWORD now = GetTickCount();
    if (abs((int)(DAT_104c032c - now)) > 33)
    {
        DAT_104c032c = now;
        DAT_10f65478 = 1;
        FUN_101439e0();
        DAT_104c031c += 0.02;
        DAT_10f65478 = 0;
        DAT_118d0ba8[DAT_104b7200.texMenu1].DrawCell(0, 0, 48, 48, 0, 0.5f, 0.1875f, 0.6875f, 0, 1.0f, DAT_104c031c);
        DAT_118d0ba8[DAT_104b7200.texMenu1].DrawCell(2, 51, (int)(DAT_104c0320 * 44.0f / 100.0f + 2.0f), 57,
                                                       0.75390625f, 0.25390625f, 0.7578125f, 0.2578125f, 0, 1.0f, 0);
        DAT_118d0ba8[DAT_104b7200.texMenu1].DrawCell(0, 48, 48, 64, 0.5f, 0.25f, 0.6875f, 0.3125f, 0, 1.0f, 0);
        sprintf(text, "%d%%", (int)DAT_104c0320);
        DAT_104b7200.DrawDebugText(text, 8, 45);
        RECT corner;
        corner.left = 0;
        corner.top = 0;
        corner.right = 64;
        corner.bottom = 64;
        DAT_10836c70.Present(&corner);
        DAT_10836c70.PumpAndRecover();
        FUN_1014b390();
    }
    return 1;
}
