// Per-frame status clamp of a character, called through the behaviour table of FUN_100d24a0. It is a copy of
// FUN_100b0250's opening and closing parts, but here the status arrays are read for character `index + 3` while the
// matrix, the offset vector and the state byte belong to character `index` (the original mixes the two indexes).

#include "vec4.h"

#include "matrix4.h"
#include "char_slot.h"

// Character record viewed from its world matrix (0x10db6ee8 + 0x2328), 0x249c bytes.
struct CharMatrixView
{
    Matrix4 matrix;
    char pad_0040[0x249c - 0x40];
};


#include "char_tables.h"
#include "char_vars.h"

extern "C" char DAT_1020d4c0[];       // per-character state written here (4 or 6)
extern "C" CharMatrixView DAT_10db9210[];

// 0x1001a840 (Char_ClampStatus). Clamps the counter (15) and level (5), clears the flag and keeps the larger value. A character whose
// life has run out is marked down. Otherwise, in mode 7 the state becomes 6 when the count is above 4 and 4 when
// not, after refreshing the character's offset from the reference position. Returns 0.
extern "C" int FUN_1001a840(int index)
{
    int status = index + 3;   // the character whose status arrays are used (also written index + 3 below)
    if (DAT_103153b4[status] > 15)
        DAT_103153b4[status] = 15;
    if (DAT_10219778[status] > 5)
        DAT_10219778[status] = 5;
    if (DAT_10315352[index + 3] > 0)
        DAT_10315352[index + 3] = 0;
    unsigned char value = DAT_102550e2[index + 3];
    if (value != 0)
        DAT_102550e0[index] = DAT_102550e0[index] >= value ? DAT_102550e0[index] : value;
    if (DAT_10daaae8[status + 2].stats[STAT_HEALTH] > 0)
    {
        DAT_10daaae8[status + 2].m_0141 = 0;
        if (DAT_104686b2[index + 3] == 7)
        {
            if (DAT_1020d4c2[index + 3] > 4)
            {
                if (DAT_104c00f8 != 0)
                    DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db9210[index].matrix.GetTranslation());
                DAT_1020d4c0[index] = 6;
                return 0;
            }
            if (DAT_104c00f8 != 0)
                DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db9210[index].matrix.GetTranslation());
            DAT_1020d4c0[index] = 4;
            return 0;
        }
    }
    else
    {
        DAT_10daaae8[status + 2].stats[STAT_HEALTH] = 0;
        DAT_10daaae8[status + 2].m_0141 = 1;
    }
    return 0;
}
