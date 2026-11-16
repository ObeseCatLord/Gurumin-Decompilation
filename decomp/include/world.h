#pragma once

struct Vec3
{
    float x;
    float y;
    float z;
};

struct CharModel
{
    void M_1017b0e0(int motion, int a, int b, float speed);
    int M_1017aeb0(const char *path);
    void M_101630b0(float value);
    void M_10162610(float x, float y, float z);
};

struct Character
{
    int id;
    char pad_0004[0xc];
    CharModel model;
    char pad_0011[0x2357];
    Vec3 m_2368;
    char pad_2374[0x28];
    int m_239c;
    char pad_23a0[0xfc];
};

struct CharSlot
{
    int m_0000;
    char pad_0004[0x5c];
    int m_0060;
    char pad_0064[0x41];
    char m_00a5;
    char pad_00a6[0xe2];
};

struct CharState
{
    int m_0000;
    int m_0004[6];
    char pad_001c[0x20];
    int m_003c;
};

struct EffectSlot
{
    int id;
    int value;
    int pad_0008;
    int type;
    char pad_0010[0xdc];
    int m_00ec;
    int active;
    char pad_00f4[8];
};

extern "C" Character DAT_10db6ed8[0x80];
extern "C" CharSlot DAT_10daab84[0x80];
extern "C" CharState DAT_1020b430[0x80];
extern "C" EffectSlot DAT_10860df0[0x40];
extern "C" int DAT_10210d30[0x80];

