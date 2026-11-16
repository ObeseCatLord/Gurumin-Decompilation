// Functions from a translation unit compiled without optimisation (/Od): frame pointer, every local on the stack,
// int3 padding. Build with: build.bat ..\src_od\<file> /Od  (not part of build_all).
extern "C" char DAT_10f0ec60[][0x20];
extern "C" char DAT_10f0d548[][0x20];

extern "C" void FUN_1013e6f0(int index, const char *name, const char *path)
{
    {
        const char *source = name;
        char *dest = DAT_10f0ec60[index];
        char *start = dest;
        char c;
        do
        {
            c = *source;
            *dest = c;
            source++;
            dest++;
        } while (c != 0);
    }
    {
        const char *source = path;
        char *dest = DAT_10f0d548[index];
        char *start = dest;
        char c;
        do
        {
            c = *source;
            *dest = c;
            source++;
            dest++;
        } while (c != 0);
    }
}
