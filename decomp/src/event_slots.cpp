// A pool of 0x60 timed event slots (DAT_1020d660 = in use, DAT_104463f8 = elapsed, DAT_10468230 = duration,
// DAT_10447fd8 = event id, DAT_1021f640 = a flag byte). The scripts add events; a per-frame update elsewhere runs
// them. The search loops are unrolled by 8 by the compiler (the plain `for` below gives exactly that).

extern "C" int DAT_1020d660[0x60];
extern "C" int DAT_104463f8[0x60];
extern "C" int DAT_10468230[0x60];
extern "C" int DAT_10447fd8[0x60];
extern "C" char DAT_1021f640[0x60];

// Adds an event `id` lasting |value| frames with `flag` to the first free slot (nothing happens when all
// 0x60 slots are in use). Returns 0.
extern "C" int FUN_100119d0(int id, int value, char flag)
{
    int i;
    for (i = 0; i < 0x60; i++)
    {
        if (DAT_1020d660[i] == 0)
            break;
    }
    if (i != 0x60)
    {
        DAT_1020d660[i] = 1;
        DAT_104463f8[i] = 0;
        if (value < 0)
            value = -value;
        DAT_10468230[i] = value;
        DAT_10447fd8[i] = id;
        DAT_1021f640[i] = flag;
    }
    return 0;
}

// Adds an event `id` with a negative duration (-100000 - |value|): the update treats such slots as
// waiting for something else rather than counting frames. Returns 0.
extern "C" int FUN_10011aa0(int id, int value)
{
    int i;
    for (i = 0; i < 0x60; i++)
    {
        if (DAT_1020d660[i] == 0)
            break;
    }
    if (i != 0x60)
    {
        DAT_1020d660[i] = 1;
        DAT_104463f8[i] = 0;
        if (value < 0)
            value = -value;
        DAT_10468230[i] = -100000 - value;
        DAT_10447fd8[i] = id;
    }
    return 0;
}
