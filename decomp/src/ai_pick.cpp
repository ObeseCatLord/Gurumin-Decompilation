#include "world.h"

extern "C" int FUN_1013f880(void);

struct Matrix
{
    float m[16];
    Matrix();
    ~Matrix();
    void SetIdentity();
};

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
