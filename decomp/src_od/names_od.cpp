// Functions from a translation unit compiled without optimisation but with intrinsics (/Od /Oi): frame pointer,
// every local on the stack, int3 padding, strcpy expanded inline.
// Build with: tools/variants.py ..\src_od\<file> 1 /Od /Oi  (not part of build_all).
#include <string.h>

extern "C" char DAT_10f0ec60[][0x20];
extern "C" char DAT_10f0d548[][0x20];

extern "C" int FUN_1013e6f0(int index, const char *name, const char *path)
{
    strcpy(DAT_10f0ec60[index], name);
    strcpy(DAT_10f0d548[index], path);
    return 0;
}
