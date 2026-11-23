// Message window, sound effects, inventory removal, animation playback.

#include <string.h>

#include "node.h"

#include "char_vars.h"
#include "game_vars.h"

extern "C" char *FUN_1013f910(const char *name);
extern "C" void FUN_10001980(const char *, int, int, int, int, int, int);
extern "C" void FUN_100016c0(const char *, int, int, int, int, int, int);
extern "C" int DAT_10db9380;
extern "C" void FUN_1000ec70(int, float, float, float, float);
extern "C" signed char DAT_103150c0[];
extern "C" void FUN_1000e810(void);

// Opens the message window with `text` (and an optional second line) for `frames` frames.
extern "C" int FUN_1000ed70(const char *text, const char *subtext, int frames)
{
    DAT_104bfd4c = frames;
    DAT_104c01fc = 0;
    DAT_104bf8c8 = 1;
    DAT_104bf8cc = 0;
    DAT_104bf8d0 = 0;
    DAT_10467ea0[0] = 0;
    strcpy(DAT_10219bc0, text);
    if (subtext != 0)
        strcpy(DAT_10467ea0, subtext);
    return 1;
}

// Plays sound file `name` from the music folder: looping (FUN_10001980) or once (FUN_100016c0), at `volume`.
extern "C" void FUN_10010420(const char *name, int, int, int loop, int volume)
{
    if (loop != 0)
        FUN_10001980(FUN_1013f910(name), 0, 0, 0, 0, loop, volume);
    else
        FUN_100016c0(FUN_1013f910(name), 0, 0, 0, 0, 0, volume);
}

// Resets the battle/HUD state for a new scene and, when the combo counter reached 0x45 or more, triggers the
// combo-finish effect (FUN_1000ec70 with full white).
extern "C" int FUN_100114a0(void)
{
    DAT_10468128 = 0;
    DAT_104c023e = 0;
    DAT_104c0194 = 0;
    DAT_104c0168 = 0;
    DAT_104c016c = 0;
    if (DAT_104c00e0 != 0)
    {
        DAT_10db9380 = 1;
        DAT_10db9254 = 0;
        DAT_10db9264 = 1.0f;
        DAT_10db7c34 = 1.0f;
        DAT_104c00e0 = 0;
        DAT_104c01a4 = 0;
    }
    DAT_104c0190 = 0;
    if (DAT_104c0108 >= 0x45)
        FUN_1000ec70(5, 1.0f, 1.0f, 1.0f, 1.0f);
    DAT_104c0108 = 0;
    return 0;
}

// Inventory_Remove: takes *amount of item *index. When there are not enough, takes all and returns 0 with
// *amount = -taken; otherwise returns 1 and clears *amount. Refreshes the anti-tamper copies.
extern "C" int FUN_100120a0(const int *index, int *amount)
{
    signed char stock = DAT_103150c0[*index];
    if (stock <= 0)
        return 0;
    if (*amount >= stock)
    {
        *amount = -stock;
        DAT_103150c0[*index] = 0;
        FUN_1000e810();
        return 0;
    }
    DAT_103150c0[*index] = stock - *amount;
    *amount = 0;
    FUN_1000e810();
    return 1;
}

// Plays `node`'s animation forwards or backwards (reverse) and remembers it as the current UI animation.
extern "C" int FUN_100158b0(Node *node, int reverse)
{
    DAT_104c01dc = node;
    if (reverse != 0)
    {
        DAT_104c01e4 = node->frameMatrixCount - 1;
        DAT_104c01e0 = -1;
    }
    else
    {
        DAT_104c01e4 = 0;
        DAT_104c01e0 = 1;
    }
    node->SetFlags(0x20);
    DAT_10db6f5e = 1;
    return 0;
}
