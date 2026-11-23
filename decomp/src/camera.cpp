// Camera and scene-direction helpers (camera mode, event camera, caption text, fade state).

#include <string.h>

#include "vec4.h"

struct Fader
{
    void M_101acc20(const Vec4 &value);
};

#include "node.h"

extern "C" int DAT_1101de1c;
extern "C" void FUN_10194360(const Vec4 *, const Vec4 *, int, int, int);
extern "C" Fader DAT_10f65688;
extern "C" Fader DAT_10f668e8;
extern "C" void FUN_1016ac90(int, int, int *);
extern "C" int DAT_10fabe88;
extern "C" int DAT_1101d8a0;
extern "C" int DAT_11018738;
extern "C" int DAT_1101d89c;
extern "C" int DAT_1101d8a8;
extern "C" int DAT_1101de24;
extern "C" int DAT_10ff1df0;
extern "C" float DAT_10203448;
extern "C" float DAT_1020344c;
extern "C" int DAT_1101d894;
extern "C" int DAT_1101ddec;
extern "C" char DAT_1101d9c0[4][0x100];
extern "C" int DAT_1101ddc0[4];
extern "C" char DAT_1101d8b8[0x100];
extern "C" int DAT_1101d9b8;
extern "C" int DAT_1101ddf4;
extern "C" int DAT_1101ddf0;
extern "C" int DAT_1101d890;
extern "C" int DAT_10ff1df8[0x47e];
extern "C" Node DAT_111be9c8[0x47e];

// Sets the camera control mode DAT_1101de1c to 1.
extern "C" int FUN_1016a8c0(void)
{
    DAT_1101de1c = 1;
    return 0;
}

// Sets the camera control mode to 2.
extern "C" int FUN_1016a8d0(void)
{
    DAT_1101de1c = 2;
    return 0;
}

// Sets the camera control mode to 0 (normal follow camera).
extern "C" int FUN_1016a8e0(void)
{
    DAT_1101de1c = 0;
    return 0;
}

// Places the camera at `eye` looking at `target` (FUN_10194360) and remembers both positions.
extern "C" int FUN_1016ac50(const Vec4 *eye, const Vec4 *target, int a, int b)
{
    FUN_10194360(eye, target, a, b, 0);
    DAT_10f65688.M_101acc20(*eye);
    DAT_10f668e8.M_101acc20(*target);
    return 0;
}

// Starts camera move `value`: resets the move state, looks the move up (FUN_1016ac90) and runs it for 10 frames.
extern "C" int FUN_1016af50(int value)
{
    int result;
    DAT_10fabe88 = 0;
    result = 0;
    DAT_1101d8a0 = 0;
    DAT_11018738 = -1;
    FUN_1016ac90(value, value, &result);
    DAT_1101d89c = result;
    DAT_1101d8a8 = 10;
    return 0;
}

// Sets the scene effect mode; mode 0 also turns the colour effect off and resets both effect scales to 1.
// Not byte-identical yet (see open.tsv).
extern "C" int FUN_1016b010(int mode)
{
    DAT_1101de24 = mode;
    if (mode == 0)
    {
        DAT_10ff1df0 = 0;
        DAT_10203448 = 1.0f;
        DAT_1020344c = 1.0f;
        DAT_1101d894 = 0;
        DAT_1101ddec = 0;
    }
    return 0;
}

// Sets caption line `index` (clamped to 0..3) to `text` and restarts its timer.
extern "C" int FUN_1016b1e0(int index, const char *text)
{
    int clamped = index > 0 ? index : 0;
    int slot = clamped > 3 ? 3 : clamped;
    strcpy(DAT_1101d9c0[slot], text);
    DAT_1101ddc0[slot] = 0;
    return 0;
}

// Sets the title caption text and restarts its timer.
extern "C" int FUN_1016b230(const char *text)
{
    strcpy(DAT_1101d8b8, text);
    DAT_1101d9b8 = 0;
    return 0;
}

// 1 while an event, effect or fade is running (the game should not accept normal input), else 0.
// Not byte-identical yet (see open.tsv).
extern "C" int FUN_1016c300(void)
{
    if (DAT_1101ddf4 != 0 || (DAT_1101de24 != 0 ? DAT_1101de24 != 5 : DAT_10ff1df0 != 0) || DAT_1101ddf0 != 1)
        return 1;
    return DAT_1101d890 != 0;
}

// Releases every used node of the 0x47e-entry effect node pool.
extern "C" int FUN_1016fab0(void)
{
    for (int i = 0; i < 0x47e; i++)
    {
        if (DAT_10ff1df8[i] != 0)
            DAT_111be9c8[i].Release();
    }
    return 0;
}

// Marks all effect node pool entries as unused.
extern "C" void FUN_1016faf0(void)
{
    memset(DAT_10ff1df8, 0, sizeof(DAT_10ff1df8));
}
