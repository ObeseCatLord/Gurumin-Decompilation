// Game menu helpers. Up to three menu windows can be open at once; FindSlot(id) returns the window slot
// holding menu `id` (3 = not open). Per slot: m_0110 open flag, m_0140 cursor position, m_0188..m_01d0 layout.
// m_01dc[id] marks menus that remember their cursor, m_0434[id] holds the remembered position.

#include <string.h>
#include "game.h"

// Looks up a named script constant (case-insensitive prefix match over the m_95e14 table); -1 when unknown.
int Game::M_10002060(const char *name)
{
    for (int i = 0; i < m_a4614; i++)
    {
        if (_strnicmp(name, m_95e14[i].name, strlen(m_95e14[i].name)) == 0)
            return m_95e14[i].map;
    }
    return -1;
}

// Closes menu `id`: plays the close sound through the m_7660 callback and clears the slot. 0 when not open.
int Game::M_100083e0(int id)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    m_7660(2);
    m_0110[slot] = 0;
    return 1;
}

// Sets menu `id`'s cursor position (and remembers it when that menu keeps its cursor).
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

// Sets four layout values of menu `id` (window rectangle).
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

// Sets three more layout values of menu `id`.
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