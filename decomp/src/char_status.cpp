// Per-frame status clamp of a character, called through the behaviour table of FUN_100d24a0. It is a copy of
// FUN_100b0250's opening and closing parts, but here the status arrays are read for character `index + 3` while the
// matrix, the offset vector and the state byte belong to character `index` (the original mixes the two indexes).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Copy(const Vec4 &source);               // 0x101acc20
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// Character record viewed from its world matrix (0x10db6ee8 + 0x2328), 0x249c bytes.
struct CharMatrixView
{
    CharMatrix matrix;
    char pad_0040[0x249c - 0x40];
};

// Per-character life record, 0x188 bytes (the same stride as CharSlot in world.h).
struct CharLife
{
    int life;                 // 0x00: hit points
    char pad_0004[0x139];
    char down;                // 0x13d: 1 once the life has run out
    char pad_013e[0x4a];
};

extern "C" short DAT_103153b4[];      // a per-character counter, at most 15
extern "C" int DAT_10219778[];        // a per-character level, at most 5
extern "C" unsigned char DAT_10315352[];   // a per-character flag, cleared every frame
extern "C" unsigned char DAT_102550e2[];   // a per-character value; the larger of it and DAT_102550e0 is kept
extern "C" unsigned char DAT_102550e0[];
extern "C" CharLife DAT_10daadfc[];
extern "C" char DAT_104686b2[];       // per-character mode (7 = the mode handled here)
extern "C" unsigned char DAT_1020d4c2[];   // per-character count compared with 4
extern "C" char DAT_1020d4c0[];       // per-character state written here (4 or 6)
extern "C" CharMatrixView DAT_10db9210[];
extern "C" Vec4 DAT_10836c74;         // reference position (in the engine object at 0x10836c70)
extern "C" Vec4 DAT_10f08080[];       // each character's offset from the reference position
extern "C" int DAT_104c00f8;          // non-zero = keep the offsets up to date

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
    if (DAT_10daadfc[status].life > 0)
    {
        DAT_10daadfc[status].down = 0;
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
        DAT_10daadfc[status].life = 0;
        DAT_10daadfc[status].down = 1;
    }
    return 0;
}
