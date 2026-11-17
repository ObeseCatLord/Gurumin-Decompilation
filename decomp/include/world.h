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
    int M_10162420(float x, float y, float z);  // places the model at (x, y, z)
    int M_1017b9b0(const char *path);
    int M_1017aeb0(const char *path);
    void M_101630b0(float value);
    void M_10162610(float x, float y, float z);
    void M_10161b60();
    int M_10161b90(int frame, float value);
    int M_10161be0(int a, int b);
};

struct Character
{
    int id;
    char pad_0004[0xc];
    CharModel model;
    char pad_0011[0x26f];
    int m_0280;               // 0x280: node id of the switch this character works (see switches.cpp)
    char pad_0284[0xad8];
    float m_0d5c;             // 0xd5c: 1.0 when spawned (model alpha or scale)
    char pad_0d60[0x159c];
    struct RootNode *m_22fc;  // 0x22fc: root node of the model (0x22ec inside CharModel)
    char pad_2300[0x68];
    Vec3 m_2368;
    char pad_2374[0x28];
    int m_239c;
    char pad_23a0[0xfc];
};

struct CharSlot
{
    int m_0000;
    int m_0004;
    int pad_0008;
    int m_000c;
    char pad_0010[0x2c];
    int m_003c;
    char pad_0040[0x20];
    int m_0060;
    char pad_0064[0x40];
    char m_00a4;              // 0xa4: number of switches the character is holding
    char m_00a5;
    char m_00a6;
    char m_00a7;
    char pad_00a8[0xe0];
};

struct CharState
{
    int m_0000;
    int m_0004[6];
    char pad_001c[0x20];
    int m_003c;
};

// Four floats with the Vec4 setter (0x101ac9c0); world.h users define their own Vec4, so it has its own name here.
struct Float4
{
    float x;
    float y;
    float z;
    float w;
    void Set(float x, float y, float z, float w);
};

struct EffectSlot
{
    int id;
    int value;
    int pad_0008;
    int type;
    int m_0010;
    char pad_0014[8];
    char name[0xc0];          // 0x1c: model or motion name
    Float4 position;          // 0xdc
    int m_00ec;
    int active;
    char pad_00f4[8];
};

extern "C" Character DAT_10db6ed8[0x80];
extern "C" CharSlot DAT_10daab84[0x80];
extern "C" CharState DAT_1020b430[0x80];
extern "C" EffectSlot DAT_10860df0[0x40];
extern "C" int DAT_10210d30[0x80];

