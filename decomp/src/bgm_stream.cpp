extern "C" int DAT_102027d8;
extern "C" char DAT_104bfd54;
extern "C" int DAT_104bfd58;
extern "C" int FUN_10001b70(int);
extern "C" int FUN_10001ba0(int stream);
extern "C" int FUN_10001bf0(int stream, int fadeTime);
extern "C" int FUN_10001a90(int stream, const char *path, int a, int b, int c, int d);
extern "C" char *FUN_1013f910(const char *name);

extern "C" void FUN_10010480(const char *name, int a, int b, int c, int d, int fade)
{
    FUN_10001b70((10 - DAT_102027d8) * 500);
    if (DAT_104bfd54 != 0)
    {
        if (fade != 0)
            FUN_10001bf0(DAT_104bfd58, 2000);
        else
            FUN_10001ba0(DAT_104bfd58);
    }
    DAT_104bfd58 = (DAT_104bfd58 - 1) & 1;
    DAT_104bfd54 = 1;
    FUN_10001a90(DAT_104bfd58, FUN_1013f910(name), c, a, b, d);
}
