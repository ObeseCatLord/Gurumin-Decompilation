// Fading message boxes: 0x7c entries, each with a state, a timer and a fade value (0..14, in steps of 2).
// States 1/2/3 fade in, fade out and hold for 60 frames; states 11/12/13 do the same but hold until a key or mouse
// button is pressed (after at least 9 frames).

#include "tracker.h"
extern "C" int DAT_104bfb08[0x7c];   // state of each message box
extern "C" int DAT_10467c20[0x7c];   // frames the box has been held
extern "C" int DAT_104481d8[0x7c];   // fade value, 0 (gone) .. 14 (fully shown)
extern "C" unsigned int DAT_104bfd5c;  // frames spent waiting for a key
#include "script_vars.h"
#include "ui_vars.h"

// 0x1000cdf0 (Messages_Update). Advances every message box by one frame (see above). A pending DAT_104c0339
// request with bit 0 or 1 of DAT_104c0230 set is answered (DAT_104c0338 = 1) on a key or button press. Returns 0.
extern "C" int FUN_1000cdf0(void)
{
    char left;
    char middle;
    char right;
    DAT_1020d9e0.M_10001110(0, &left, &middle, &right, 0, 0);
    if (DAT_104c0339 != 0 && (DAT_104c0230 & 3) != 0 &&
        (DAT_1065cb30 != 0 || left == 2 || middle == 2 || right == 2))
    {
        DAT_104c0339 = 0;
        DAT_104c0338 = 1;
    }
    for (int i = 0; i < 0x7c; i++)
    {
        if (DAT_104bfb08[i] > 9)
        {
            DAT_104bfd5c++;
            if (DAT_104bfb08[i] == 13)
                DAT_10467c20[i]++;
            if (DAT_104bfb08[i] == 13 &&
                (DAT_1065cb30 != 0 || left == 2 || middle == 2 || right == 2) && DAT_104bfd5c > 8)
            {
                DAT_104bfb08[i] = 12;
                DAT_10467c20[i] = 0;
                DAT_104bfd5c = 0;
                DAT_104bfd10 = 0;
            }
            if (DAT_104bfb08[i] == 11)
                DAT_104481d8[i] += 2;
            if (DAT_104bfb08[i] == 12)
                DAT_104481d8[i] -= 2;
            if (DAT_104481d8[i] == 0)
                DAT_104bfb08[i] = 0;
            if (DAT_104481d8[i] >= 15)
                DAT_104bfb08[i] = 13;
        }
        else
        {
            if (DAT_104bfb08[i] == 3)
                DAT_10467c20[i]++;
            if (DAT_10467c20[i] > 60)
            {
                DAT_104bfb08[i] = 2;
                DAT_10467c20[i] = 0;
            }
            if (DAT_104bfb08[i] == 1)
                DAT_104481d8[i] += 2;
            if (DAT_104bfb08[i] == 2)
                DAT_104481d8[i] -= 2;
            if (DAT_104481d8[i] == 0)
                DAT_104bfb08[i] = 0;
            if (DAT_104481d8[i] >= 15)
                DAT_104bfb08[i] = 3;
        }
    }
    return 0;
}
