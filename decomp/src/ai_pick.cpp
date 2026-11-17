// Enemy AI: picking the next behaviour.
// Each enemy kind has its own picker. A picker rolls 0..99 with the game's random function (FUN_1013f880) and sets
// the enemy's state (CharState::m_0000 in DAT_1020b430) to behaviour 0, 1 or 2 with fixed odds, clearing the
// behaviour's two timers (m_0004[0], m_0004[1]). The enemy updaters (enemy_ai.cpp) call a picker when the current
// behaviour has run its course. Changing the thresholds below changes how often each move is used.

#include "world.h"

extern "C" int FUN_1013f880(void);

struct Matrix
{
    float m[16];
    Matrix();
    ~Matrix();
    void SetIdentity();
};

// Attached mesh slots (64 at 0x10edbe08, 0xb08 bytes each): extra models such as held items.
struct MeshSlot
{
    int pad_0000;
    void *mesh;
    char pad_0008[0xb00];
    void M_10037b40(const void *a, const void *b);
};

extern "C" MeshSlot DAT_10edbe08[0x40];
extern "C" char DAT_108392d8[];
extern "C" char DAT_10836330[];

// Picker A: behaviour 0 40 %, behaviour 1 25 %, behaviour 2 35 %.
extern "C" int FUN_10026af0(int index)
{
    int roll = FUN_1013f880() % 100;
    if (roll < 40)
    {
        DAT_1020b430[index].m_0000 = 0;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 65)
    {
        DAT_1020b430[index].m_0000 = 1;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        DAT_1020b430[index].m_0000 = 2;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
    }
    return 0;
}

// Picker B: behaviour 0 60 %, behaviour 1 20 %, behaviour 2 20 %.
extern "C" int FUN_10026b70(int index)
{
    int roll = FUN_1013f880() % 100;
    if (roll < 60)
    {
        DAT_1020b430[index].m_0000 = 0;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 80)
    {
        DAT_1020b430[index].m_0000 = 1;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        DAT_1020b430[index].m_0000 = 2;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
    }
    return 0;
}

// Picker C (two behaviours only): behaviour 0 65 %, behaviour 1 35 %.
extern "C" int FUN_10026bf0(int index)
{
    int roll = FUN_1013f880() % 100;
    if (roll < 65)
    {
        DAT_1020b430[index].m_0000 = 0;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        DAT_1020b430[index].m_0000 = 1;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
    }
    return 0;
}

// Picker D: behaviour 0 60 %, behaviour 1 10 %, behaviour 2 30 %.
extern "C" int FUN_10026c80(int index)
{
    int roll = FUN_1013f880() % 100;
    if (roll < 60)
    {
        DAT_1020b430[index].m_0000 = 0;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 70)
    {
        DAT_1020b430[index].m_0000 = 1;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        DAT_1020b430[index].m_0000 = 2;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
    }
    return 0;
}

// Picker E: behaviour 0 20 %, behaviour 1 60 %, behaviour 2 20 %.
extern "C" int FUN_10026d00(int index)
{
    int roll = FUN_1013f880() % 100;
    if (roll < 20)
    {
        DAT_1020b430[index].m_0000 = 0;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 80)
    {
        DAT_1020b430[index].m_0000 = 1;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
        return 0;
    }
    if (roll < 100)
    {
        DAT_1020b430[index].m_0000 = 2;
        DAT_1020b430[index].m_0004[0] = 0;
        DAT_1020b430[index].m_0004[1] = 0;
    }
    return 0;
}

// 0x1003a940: updates every used mesh slot with the current world matrices (DAT_108392d8, DAT_10836330).
// The identity matrix built at the end is unused (left over from older code).
int UpdateMeshSlots()
{
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10edbe08[i].mesh != 0)
            DAT_10edbe08[i].M_10037b40(DAT_108392d8, DAT_10836330);
    }
    Matrix identity;
    identity.SetIdentity();
    return 0;
}
