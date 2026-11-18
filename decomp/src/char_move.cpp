// Character movement vectors. The character records are 0x249c bytes; this file views them from their model part
// (0x10db6ee8, 0x10 bytes into each Character of world.h), where all the fields used here fall inside one record.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    float LengthSquared3();  // 0x101ac6a0: x*x + y*y + z*z
};

struct CharRecord
{
    char pad_0000[0x238c];
    int m_238c;               // 0x238c: Character::m_239c (1 = ?; app_0011.cpp plays the idle motion when non-zero)
    char pad_2390[0x18];
    float moveX;              // 0x23a8: movement vector set by Character_SetMove
    float moveY;              // 0x23ac
    float moveZ;              // 0x23b0
    char pad_23b4[4];
    Vec4 m_23b8;              // 0x23b8: current speed vector (compared against 1.0 below)
    char pad_23c8[0xd0];
    int m_2498;               // 0x2498: set to 1 by the scene start for the player
};

extern "C" CharRecord DAT_10db6ee8[0x80];

// 0x100241e0 (Character_SetMove). Sets the movement vector of character `index` from (x, y, z): half of it in the
// state where m_2498 is 1 and m_238c is not 1, otherwise 0.7 of it, doubled to 1.4 while the character is nearly
// standing still (speed vector shorter than 1). Called by the enemy and player movement code. Returns 0.
extern "C" int FUN_100241e0(int index, float x, float y, float z)
{
    if (DAT_10db6ee8[index].m_238c != 1 && DAT_10db6ee8[index].m_2498 == 1)
    {
        DAT_10db6ee8[index].moveX = x * 0.5f;
        DAT_10db6ee8[index].moveY = y * 0.5f;
        DAT_10db6ee8[index].moveZ = z * 0.5f;
        return 0;
    }
    DAT_10db6ee8[index].moveX = x * 0.7f;
    DAT_10db6ee8[index].moveY = y * 0.7f;
    DAT_10db6ee8[index].moveZ = z * 0.7f;
    if (DAT_10db6ee8[index].m_23b8.LengthSquared3() < 1.0f)
    {
        DAT_10db6ee8[index].moveX = x * 1.4f;
        DAT_10db6ee8[index].moveY = y * 1.4f;
        DAT_10db6ee8[index].moveZ = z * 1.4f;
    }
    return 0;
}
