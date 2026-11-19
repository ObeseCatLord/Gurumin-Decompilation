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

// 0x1013e790 (Names_Expand). Copies `text` to `out`, replacing every occurrence of the four registered names
// (FUN_1013e6f0) by their paths; at each step the name found earliest in the rest of the text is replaced. Returns 1.
extern "C" int FUN_1013e790(char *out, const char *text)
{
    const char *p = text;
    out[0] = 0;
    while (1)
    {
        int best;
        int found = best = 0;
        int matches[4];
        for (int i = 0; i < 4; i++)
        {
            matches[i] = 0;
            if (DAT_10f0ec60[i][0] != 0)
                matches[i] = (int)strstr(p, DAT_10f0ec60[i]);
            if (matches[i] != 0 && (found == 0 || matches[i] < matches[best]))
            {
                found = 1;
                best = i;
            }
        }
        if (found != 0)
        {
            int length = strlen(out);
            int count = matches[best] - (int)p;
            strncpy(out + length, p, count);
            out[length + count] = 0;
            strcat(out, DAT_10f0d548[best]);
            p = (const char *)matches[best] + strlen(DAT_10f0ec60[best]);
        }
        else
        {
            strcat(out, p);
            break;
        }
    }
    return 1;
}
