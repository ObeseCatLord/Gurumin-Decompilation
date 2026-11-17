#pragma once

struct GameNamedEntry
{
    int value;
    char pad_0004[0x10];
    char name[0xd4];
};

// Empty engine base class; its destructor is the shared empty function 0x1013eda0.
struct Object
{
    ~Object();
};

// One scene node body (0x18a0 bytes, destructor 0x10191480, see src/node_dtor.cpp). Game owns 32 of them.
struct GameNodeBody
{
    char data[0x18a0];
    ~GameNodeBody();
};

// The game object: global instance DAT_104b7200. Holds menu textures, HUD state, the script variable tables and
// 32 node bodies for menu models.
struct Game : Object
{
    int m_0000;
    int m_0004;
    int texMenu1;
    int texMenu4;
    int m_0010;
    int texMenu2;
    int texMenu5;
    int texBalloon5;
    int texBalloon6;
    int texFrame;
    int m_0028;
    int texMap01;
    int texMap02;
    int texUdChip;
    int texWaku01;
    int texWaku02;
    int texWaku03;
    int texWaku04;
    int texIcon01;
    int texWaku05;
    char pad_0050[0x2c];
    int texBalloon7;
    int texMenu15;
    int texMenu14;
    int m_0088;
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
    char pad_068c[4];
    int m_0690;
    char pad_0694[0x5720];
    int m_5db4[3][0x82];
    char pad_63cc[0xc30];
    int m_6ffc;
    char pad_7000[0x630];
    int m_7630;
    int m_7634;
    int m_7638;
    int m_763c;
    int m_7640;
    int m_7644;
    int m_7648;
    int m_764c;
    int m_7650;
    int m_7654[3];
    void (*m_7660)(int);
    int m_7664;
    char pad_7668[0x1200];
    GameNodeBody m_8868[0x20];  // 0x8868: menu model nodes, destroyed in ~Game
    char pad_39c68[0x5c1ac];
    GameNamedEntry m_95e14[0x100];
    int m_a4614;
    char pad_a4618[0x34a07c];
    int m_3ee694[4000];

    ~Game();
    int FindSlot(int id);
    void M_10007f20();
    int LoadMenuTextures(int value);
    int M_100022b0(int index);
    int M_10002300(int index, int value);
    int M_10007ee0();
    void M_100082e0();
    int M_10008390(int id, int value);
    int M_10008460(int id);
    int M_10008770(int id, int value);
    int M_100087b0(int id, int count);
    int M_100087f0(int x, int y, int w, int h, int frame, float alpha);
    int M_10008880(int x, int y, int w, int h, int frame, float alpha);
    int M_10008910(int x, int y, int lit, int rows, int a, int b, int color);
    int M_10008fa0(int x, int y, int color, int frame);
    int M_100091d0(const char *text, int x, int y);
    int M_10002060(const char *name);
    int M_100083e0(int id);
    int M_10008650(int id, int value);
    int M_100086b0(int id, int a, int b, int c, int d);
    int M_10008710(int id, int a, int b, int c);
    int M_100084b0(int id, int x, int y, int width, int height);
    int M_10008330(int id, int value);
    int M_1000b2c0(int id, const char *label, int value);
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
