extern "C" int DAT_00908134[8];
extern "C" int DAT_00920028;
extern "C" int DAT_009e2410, DAT_009e2528[32], DAT_009e25a8[32], DAT_009e2628[32];
extern "C" int FUN_005a2da0(int v)
{
    DAT_00908134[DAT_00920028] = v;
    DAT_00920028 = (DAT_00920028 + 1) & 7;
    return 0;
}
extern "C" int FUN_005a24d0(int a, int b)
{
    int i = DAT_009e2410;
    DAT_009e2528[i] = a;
    DAT_009e25a8[i] = b;
    DAT_009e2628[i] = 0;
    i++;
    if (i > 31) i = 0;
    DAT_009e2410 = i;
    return 0;
}
extern "C" int FUN_005ac910(void)
{
    for (int i = 0; i < 32; i++) {
        if (DAT_009e2528[i] != 0 && ++DAT_009e2628[i] > 30) {
            DAT_009e2628[i] = 0;
            DAT_009e2528[i] = 0;
        }
    }
    return 0;
}
struct Counter { int value; int max; int Step(); };
int Counter::Step()
{
    if (value < max) value++;
    if (value > max) value--;
    return 0;
}
