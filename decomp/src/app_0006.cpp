#include <string.h>
#include "game.h"

int Game::M_10002060(const char *name)
{
    for (int i = 0; i < m_a4614; i++)
    {
        if (_strnicmp(name, m_95e14[i].name, strlen(m_95e14[i].name)) == 0)
            return m_95e14[i].value;
    }
    return -1;
}

int Game::M_100083e0(int id)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    m_7660(2);
    m_0110[slot] = 0;
    return 1;
}

int Game::M_10008650(int id, int value)
{
    int slot = FindSlot(id);
    if (slot != 3)
    {
        m_0140[slot] = value;
        if (m_01dc[id] != 0)
            m_0434[id] = (char)m_0140[slot];
    }
    return 0;
}

int Game::M_100086b0(int id, int a, int b, int c, int d)
{
    int slot = FindSlot(id);
    if (slot != 3)
    {
        m_01ac[slot] = a;
        m_01a0[slot] = b;
        m_0194[slot] = c;
        m_0188[slot] = d;
    }
    return 0;
}

int Game::M_10008710(int id, int a, int b, int c)
{
    int slot = FindSlot(id);
    if (slot != 3)
    {
        m_01b8[slot] = c;
        m_01c4[slot] = a;
        m_01d0[slot] = b;
    }
    return 0;
}