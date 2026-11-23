// The screen fade drawn over each frame. DAT_104bf8ec picks the kind in its hundreds (0 black, 1 white, 2 red,
// 3 the colour in DAT_104c02e0) and an iris wipe when it is not a whole hundred; DAT_104bf8f0 is the fade amount
// (0-100, set by the fade functions in volume_fade.cpp).

#include <math.h>

#include "texture_object.h"

extern "C" int DAT_104bf8ec;                     // fade kind (see above)
extern "C" float DAT_104bf8f0;                   // fade amount, 0-100
extern "C" int DAT_104c02e0;                     // colour of fade kind 3
extern "C" int DAT_104b7224;                     // texture slot of the iris wipe
extern "C" void FUN_10143e00(int left, int top, int right, int bottom, int colour, int mode, float alpha);  // fills a rectangle
extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// 0x1000dba0 (Fade_Draw). Draws the screen fade: a full-screen rectangle in the kind's colour with opacity
// amount / 100, or for an iris wipe (and amounts up to 95) the iris texture, which wobbles around the centre on a
// circle that shrinks as the amount grows and is drawn larger the higher the amount; above 95 the wipe is black.
extern "C" void FUN_1000dba0(void)
{
    int rest = DAT_104bf8ec;
    int kind = 0;
    if (rest >= 300)
    {
        kind = 3;
        rest -= 300;
    }
    else if (rest >= 200)
    {
        kind = 2;
        rest -= 200;
    }
    else if (rest >= 100)
    {
        kind = 1;
        rest -= 100;
    }
    if (rest == 0)
    {
        if (kind == 3)
            FUN_10143e00(0, 0, 640, 480, DAT_104c02e0, 0, DAT_104bf8f0 / 100.0f);
        else if (kind == 2)
            FUN_10143e00(0, 0, 640, 480, 0xff0000, 0, DAT_104bf8f0 / 100.0f);
        else if (kind != 0)
            FUN_10143e00(0, 0, 640, 480, 0xffffff, 0, DAT_104bf8f0 / 100.0f);
        else
            FUN_10143e00(0, 0, 640, 480, 0, 0, DAT_104bf8f0 / 100.0f);
    }
    else if (DAT_104bf8f0 > 95.0f)
        FUN_10143e00(0, 0, 640, 480, 0, 0, DAT_104bf8f0 / 100.0f);
    else
    {
        int x = (int)(sin(DAT_104bf8f0 / 15.0f) * (100.0f - DAT_104bf8f0 * 0.5f));
        int y = (int)(cos(DAT_104bf8f0 / 15.0f) * (100.0f - DAT_104bf8f0 * 0.5f));
        int grow = (int)(DAT_104bf8f0 * DAT_104bf8f0 * 0.7f);
        FUN_1013f070(2, 1, 0);
        FUN_1013f070(0, 2, 0);
        float edge = (float)grow / 480.0f + 1.0f;
        float start = (float)-grow / 480.0f;
        DAT_118d0ba8[DAT_104b7224].DrawTinted(x - 220, y - 300, x + 860, y + 780, start, start, edge, edge, 0, 1.0f, 0,
                                              0);
        FUN_1013f070(0, 0, 0);
        FUN_1013f070(2, 0, 0);
    }
}
