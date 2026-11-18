// Scans over the character slots 0x3a..0x7b (the enemy range). Slot kind 3 in DAT_1021f3c0 marks a live enemy.
// A position (m_2368.x) below -49999 means the character was moved out of the world (retired, see char_retire.cpp).

#include "world.h"

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    Vec4 Scaled(const float &scale);
    float Length2D();  // 0x101ac500: length of x/y
};

// Per-slot state table at 0x10daaaec, 0x188 bytes per slot (CharSlot in world.h starts 0x98 bytes later).
struct ActorSlot
{
    int m_0000;
    int m_0004;         // initial value of m_0000 (restored when the slot is reused)
    char pad_0008[0x94];
    int m_009c;
    char pad_00a0[0xe8];
};

extern "C" ActorSlot DAT_10daaaec[0x80];
extern "C" char DAT_1021f3c0[0x80];

// Clears the `m_0000` counter of every live, active enemy that is still in the world and has no pending
// action (m_009c == 0). Returns 1.
extern "C" int FUN_10011c60(void)
{
    for (int i = 0x3a; i < 0x7c; i++)
    {
        if (DAT_1021f3c0[i] == 3 && i >= 0x3a && DAT_10db6ed8[i].m_2368.x > -49999.0f && DAT_10daaaec[i].m_0000 > 0 &&
            DAT_10210d30[i] == 0 && DAT_10db6ed8[i].m_22fc != 0 && DAT_10daaaec[i].m_009c == 0)
            DAT_10daaaec[i].m_0000 = 0;
    }
    return 1;
}

// Returns the first enemy slot that has a model and is either out of the world or finished (counter <= 0 while
// its state is 2), or -1.
extern "C" int FUN_10011cd0(void)
{
    for (int i = 0x3a; i < 0x7c; i++)
    {
        if (DAT_1021f3c0[i] == 3 && i >= 0x3a)
        {
            if ((DAT_10db6ed8[i].m_2368.x < -50000.0f || (DAT_10daaaec[i].m_0000 <= 0 && DAT_10210d30[i] == 2)) &&
                DAT_10db6ed8[i].m_22fc != 0)
                return i;
        }
    }
    return -1;
}

// Root node of a character model (scene_node.cpp Node).
struct RootNode
{
    void M_101946b0(int value, int mask);
    void M_10194730(int value, int mask);
};

extern "C" char DAT_10315350[0x80];

// Respawns an enemy in the first enemy slot that is free or finished (FUN_10011cd0): resets the model's node
// states (0xfe and 0x2f with all bits), restores the slot's counter, clears the busy and spawn flags, sets
// m_0d5c to 1 and places the model with (a, b, c). Returns the slot, or -1 when none is available.
extern "C" int FUN_10011d40(float a, float b, float c)
{
    int i = FUN_10011cd0();
    if (i >= 0)
    {
        DAT_10db6ed8[i].m_22fc->M_101946b0(0xfe, -1);
        DAT_10db6ed8[i].m_22fc->M_10194730(0x2f, -1);
        DAT_10daaaec[i].m_0000 = DAT_10daaaec[i].m_0004;
        DAT_10210d30[i] = 0;
        DAT_10315350[i] = 0;
        DAT_10db6ed8[i].m_0d5c = 1.0f;
        DAT_10db6ed8[i].model.M_10162420(a, b, c);
        return i;
    }
    return -1;
}

// 0x1001ba40. 1 when the point (x, y) lies near one of the objects in slots 0x5c..0x7b whose kind byte is 0x5c: its
// 50x50 square around the point overlaps the object's 250x250 square (x/y only). The first argument is unused.
extern "C" int FUN_1001ba40(int unused, const Vec4 *point)
{
    Vec4 position;
    for (int i = 0x5c; i < 0x7c; i++)
    {
        if (DAT_1021f3c0[i] == 0x5c)
        {
            position.Copy(DAT_10db6ed8[i].matrix.GetTranslation());
            if (position.x - 125.0f < point->x + 25.0f && position.x + 125.0f > point->x - 25.0f &&
                position.y - 125.0f < point->y + 25.0f && position.y + 125.0f > point->y - 25.0f)
                return 1;
        }
    }
    return 0;
}

// 0x1001bec0. Slows `velocity` down to 5 % when it would take `position` closer (in x/y) to one of the objects of
// kind 0x14 in slots 0x5c..0x7b that is less than 400 units away. Returns 0.
extern "C" int FUN_1001bec0(const Vec4 *position, Vec4 *velocity)
{
    Vec4 delta;
    for (int i = 0x5c; i < 0x7c; i++)
    {
        if (DAT_1021f3c0[i] == 0x14)
        {
            delta.Copy(DAT_10db6ed8[i].matrix.GetTranslation() - *position);
            float distance = delta.Length2D();
            delta.Copy(delta - *velocity);
            float distanceAfter = delta.Length2D();
            if (distance < 400.0f && distance > distanceAfter)
            {
                velocity->Copy(velocity->Scaled(0.05f));
                return 0;
            }
        }
    }
    return 0;
}

extern "C" const char DAT_101d67f0[5];  // character slots checked by FUN_10034e80: 0, 13, 15, 58, 20

// 0x10034e80. 1 when any of the five characters in DAT_101d67f0, other than `index` itself, stands within `range`
// (x/y) of character `index`'s position, moved by `offset` when that is given. Returns 0 otherwise.
extern "C" int FUN_10034e80(int index, const Vec4 *offset, int range)
{
    Vec4 position;
    if (offset != 0)
        position.Copy(DAT_10db6ed8[index].matrix.GetTranslation() + *offset);
    else
        position.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
    Vec4 other;
    for (int i = 0; i < 5; i++)
    {
        if (DAT_101d67f0[i] != index)
        {
            other.Copy(DAT_10db6ed8[DAT_101d67f0[i]].matrix.GetTranslation());
            float distance = (other - position).Length2D();
            if (distance < (float)range)
                return 1;
        }
    }
    return 0;
}
