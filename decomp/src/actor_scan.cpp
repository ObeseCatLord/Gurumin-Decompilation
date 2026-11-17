// Scans over the character slots 0x3a..0x7b (the enemy range). Slot kind 3 in DAT_1021f3c0 marks a live enemy.
// A position (m_2368.x) below -49999 means the character was moved out of the world (retired, see char_retire.cpp).

#include "world.h"

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
