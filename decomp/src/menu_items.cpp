// Adding entries to an open menu window (Game, see app_0006.cpp for the window slots). Each entry has a label (up to
// 0x35 characters), a kind (m_5db4), a target (m_579c: the variable a flag or range entry sets, or a value) and a
// range (m_69e4..m_63cc). Every add also clears the variables of the window's flag entries (kind 0), so a menu
// starts with none of its flags set.

#include <string.h>
#include "game.h"

// 0x1000b060. Adds a value entry (kind 1) with label `label` and value `value` to menu window `id`. Returns 0x82 (the
// clearing loop's count), or 0 when the window is not open or has no free entry.
int Game::AddValueEntry(int id, const char *label, int value)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (m_069c[slot][i][0] == 0)
            break;
    }
    if (i == 0x80)
        return 0;
    strcpy(m_069c[slot][i], label);
    m_579c[slot].entries[i] = (int *)value;
    m_5db4[slot][i] = 1;
    m_63cc[slot][i] = -1;
    m_69e4[slot][i] = 0;
    m_00e0[slot] = i;
    for (i = 0; i < 0x82; i++)
    {
        if (m_579c[slot].entries[i] != 0 && m_5db4[slot][i] == 0)
            *m_579c[slot].entries[i] = 0;
    }
    return i;
}

// 0x1000b2c0. Adds a flag entry (kind 0) to menu window `id`: `flag` (cleared now) is the variable it sets. Returns
// 0x82, or 0 when the window is not open or has no free entry.
int Game::AddFlagEntry(int id, const char *label, int *flag)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (m_069c[slot][i][0] == 0)
            break;
    }
    if (i == 0x80)
        return 0;
    if (flag != 0)
        *flag = 0;
    strcpy(m_069c[slot][i], label);
    m_579c[slot].entries[i] = flag;
    m_5db4[slot][i] = 0;
    m_63cc[slot][i] = -1;
    m_69e4[slot][i] = 0;
    m_00e0[slot] = i;
    for (i = 0; i < 0x82; i++)
    {
        if (m_579c[slot].entries[i] != 0 && m_5db4[slot][i] == 0)
            *m_579c[slot].entries[i] = 0;
    }
    return i;
}

// 0x1000b4e0. AddFlagEntry with entry kind 5.
int Game::AddKind5Entry(int id, const char *label, int *flag)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (m_069c[slot][i][0] == 0)
            break;
    }
    if (i == 0x80)
        return 0;
    if (flag != 0)
        *flag = 0;
    strcpy(m_069c[slot][i], label);
    m_579c[slot].entries[i] = flag;
    m_5db4[slot][i] = 5;
    m_63cc[slot][i] = -1;
    m_69e4[slot][i] = 0;
    m_00e0[slot] = i;
    for (i = 0; i < 0x82; i++)
    {
        if (m_579c[slot].entries[i] != 0 && m_5db4[slot][i] == 0)
            *m_579c[slot].entries[i] = 0;
    }
    return i;
}

// 0x1000b750. Adds a range entry (kind 2) setting `value` (cleared now) between `low` and `high`. Returns 0x82, or 0
// when the window is not open or has no free entry.
int Game::AddRangeEntry(int id, const char *label, int low, int high, int *value)
{
    int slot = FindSlot(id);
    if (slot == 3)
        return 0;
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (m_069c[slot][i][0] == 0)
            break;
    }
    if (i == 0x80)
        return 0;
    if (value != 0)
        *value = 0;
    strcpy(m_069c[slot][i], label);
    m_63cc[slot][i] = high;
    m_579c[slot].entries[i] = value;
    m_5db4[slot][i] = 2;
    m_69e4[slot][i] = low;
    m_00e0[slot] = i;
    for (i = 0; i < 0x82; i++)
    {
        if (m_579c[slot].entries[i] != 0 && m_5db4[slot][i] == 0)
            *m_579c[slot].entries[i] = 0;
    }
    return i;
}
