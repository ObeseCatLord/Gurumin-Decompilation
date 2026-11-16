#pragma once

struct GameNamedEntry
{
    int value;
    char pad_0004[0x10];
    char name[0xd4];
};

struct Game
{
    char pad_0000[0x8c];
    int m_008c[3];
    int m_0098[3];
    int m_00a4[3];
    char pad_00b0[0x3c];
    int m_00ec[3];
    char pad_00f8[0x18];
    int m_0110[3];
    int m_011c[3];
    int m_0128[3];
    char pad_0134[0xc];
    int m_0140[3];
    char pad_014c[0x3c];
    int m_0188[3];
    int m_0194[3];
    int m_01a0[3];
    int m_01ac[3];
    int m_01b8[3];
    int m_01c4[3];
    int m_01d0[3];
    char m_01dc[0x258];
    char m_0434[0x258];
    char pad_068c[0x5728];
    int m_5db4[3][0x82];
    char pad_63cc[0x1288];
    int m_7654[3];
    void (*m_7660)(int);
    char pad_7664[0x8e7b0];
    GameNamedEntry m_95e14[0x100];
    int m_a4614;
    char pad_a4618[0x34a07c];
    int m_3ee694[4000];

    int FindSlot(int id);
    int M_100022b0(int index);
    int M_10002300(int index, int value);
    int M_10007ee0();
    void M_100082e0();
    int M_10008390(int id, int value);
    int M_10008460(int id);
    int M_10008770(int id, int value);
    int M_100087b0(int id, int count);
    int M_10002060(const char *name);
    int M_100083e0(int id);
    int M_10008650(int id, int value);
    int M_100086b0(int id, int a, int b, int c, int d);
    int M_10008710(int id, int a, int b, int c);
};

inline int Game::FindSlot(int id)
{
    int slot = 0;
    do
    {
        if (m_0110[slot] != 0 && m_00a4[slot] == id)
            break;
        slot++;
    } while (slot < 3);
    return slot;
}
