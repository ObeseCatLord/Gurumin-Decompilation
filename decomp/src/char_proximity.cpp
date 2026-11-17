// Proximity checks between the player (character 0) and other characters.

#include "world.h"

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    float LengthSquared3();                      // 0x101ac6a0
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// The part of a Character record (world.h) used here: its world matrix.
struct CharacterPlacement
{
    char pad_0000[0x2338];
    Matrix4 matrix;          // 0x2338: world matrix (translation = position)
    char pad_2378[0x124];
};

extern "C" int FUN_10043cc0(int index);

// For an idle character (state 0): plays its motion 2 and, when the player is within 10 * m_0004[1] units,
// triggers FUN_10043cc0 (the character notices the player). Returns 0.
extern "C" int FUN_10045b80(int index)
{
    CharacterPlacement *placed = (CharacterPlacement *)DAT_10db6ed8;
    float distance2 = (placed[0].matrix.GetTranslation() - placed[index].matrix.GetTranslation()).LengthSquared3();
    if (DAT_1020b430[index].m_0000 == 0)
    {
        DAT_10db6ed8[index].model.M_1017b0e0(2, 0, 5, 1.0f);
        if (distance2 < (float)(DAT_1020b430[index].m_0004[1] * DAT_1020b430[index].m_0004[1] * 100))
            FUN_10043cc0(index);
    }
    return 0;
}
