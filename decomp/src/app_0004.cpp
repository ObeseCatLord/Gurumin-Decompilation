#include "game.h"

int Game::M_10007ee0()
{
    for (int i = 0; i < 3; i++)
    {
        if (m_0110[i] != 0)
            return 1;
        if (m_008c[i] != 0 && m_0098[i] > 16)
            return 1;
    }
    return 0;
}

void Game::M_100082e0()
{
    for (int i = 0; i < 3; i++)
    {
        m_008c[i] = 0;
        m_0110[i] = 0;
        m_00a4[i] = 0;
        for (int j = 0; j < 0x82; j++)
            m_5db4[i][j] = 0;
    }
}

int Game::M_10008390(int id, int value)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    m_7654[slot] = value;
    return 1;
}

int Game::M_10008460(int id)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    m_00ec[slot] = 1;
    return 1;
}

int Game::M_10008770(int id, int value)
{
    int slot = FindSlot(id);
    if (slot != 3)
        m_0128[slot] = value;
    return 0;
}

int Game::M_100087b0(int id, int count)
{
    int slot = FindSlot(id);
    if (slot != 3)
        m_011c[slot] = count - 1;
    return 0;
}

struct Callee
{
    void Method();
};

typedef void (Callee::*CalleeMethod)();

extern "C" void __stdcall FUN_10004650(char *object, int stride, int count, CalleeMethod method)
{
    while (--count >= 0)
    {
        (((Callee *)object)->*method)();
        object += stride;
    }
}

struct Latch
{
    int m_0000;
    int m_0004;
    int pad_0008[3];
    int m_0014;
    int M_1000b9c0();
};

int Latch::M_1000b9c0()
{
    if (m_0014 == 10)
    {
        if (m_0000 == 0)
            m_0004 = 30;
        else if (m_0000 == 30)
            m_0004 = 0;
    }
    return 1;
}

