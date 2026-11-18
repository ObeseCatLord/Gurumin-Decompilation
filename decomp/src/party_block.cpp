// Whether a party character stands in the way of a move: used before a character steps so that it does not walk
// into the player or the companions.

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    float Length2D();                            // 0x101ac500: length of x/y
    Vec4 operator+(const Vec4 &other) const;     // 0x101ac9e0
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// A character record (world.h Character), only its world matrix.
struct Character
{
    char pad_0000[0x2338];
    CharMatrix matrix;            // 0x2338
    char pad_2378[0x124];
};

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Absolute value of a float (an inline helper: its argument is rounded to float first).
inline float AbsFloat(float value)
{
    return (float)fabs(value);
}

extern "C" Character DAT_10db6ed8[0x80];
extern "C" const char DAT_101d67f0[5];   // the party's character slots: 0, 13, 15, 58 and 20

// 0x10034cc0 (Party_BlocksMove). Returns 1 when one of the five party characters other than `self` would be within
// `range` of character `self` after it moves by `step` (on the ground plane) and either the move brings `self`
// closer to it while heading at it (within 0.25 rad) or it would be closer than 0.4 * `range`; else 0.
extern "C" int FUN_10034cc0(int self, const Vec4 &step, int range)
{
    Vec4 ahead = DAT_10db6ed8[self].matrix.GetTranslation() + step;
    Vec4 other;
    for (int i = 0; i < 5; i++)
    {
        if (DAT_101d67f0[i] == self)
            continue;
        other.Copy(DAT_10db6ed8[DAT_101d67f0[i]].matrix.GetTranslation());
        float distance = (other - ahead).Length2D();
        float gain = distance - (other - DAT_10db6ed8[self].matrix.GetTranslation()).Length2D();
        float heading = Atan2(step.x, step.y);
        float toOther = Atan2(other.x - ahead.x, other.y - ahead.y);
        if (distance < (float)range)
        {
            if (gain > 0.0f && AbsFloat(heading - toOther) < 0.25f)
                return 1;
            if (distance < (float)range * 0.4f)
                return 1;
        }
    }
    return 0;
}
