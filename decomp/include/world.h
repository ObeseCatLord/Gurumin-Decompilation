#pragma once

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vec4;  // defined by each source file

// A character's world matrix (row-major; the translation m[12..14] is the character's position).
struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();  // 0x101ade80 (vec4.cpp: Matrix4::GetTranslation)
};

struct CharModel
{
    void M_1017b0e0(int motion, int a, int b, float speed);
    int M_10161a20(int texture);  // AnimModel::ApplyMaterialSwap
    int M_10162420(float x, float y, float z);  // places the model at (x, y, z)
    int M_1017b9b0(const char *path);
    int M_1017aeb0(const char *path);
    void M_101630b0(float value);
    void M_10162610(float x, float y, float z);
    void M_10161b60();
    int M_10161b90(int frame, float value);
    int M_10161be0(int a, int b);
    // 0x101628b0 (anim_model.cpp AnimModel::M_101628b0): queues a push (x, y, z) with `flag` unless the model is locked.
    int SetPendingPush(float x, float y, float z, char flag);
};

struct Character
{
    int id;
    char pad_0004[0xc];
    CharModel model;
    char pad_0011[0x6f];
    char *nearCells;          // 0x80: row of DAT_1021d338 (map cells near the character), 0 = no visibility data
    char pad_0084[0x1fc];
    int m_0280;               // 0x280: node id of the switch this character works (see switches.cpp)
    char pad_0284[0xad8];
    float m_0d5c;             // 0xd5c: 1.0 when spawned (model alpha or scale)
    char pad_0d60[0x159c];
    struct RootNode *m_22fc;  // 0x22fc: root node of the model (0x22ec inside CharModel)
    char pad_2300[0x38];
    union
    {
        CharMatrix matrix;    // 0x2338: world matrix
        struct
        {
            char pad_matrix[0x30];
            Vec3 m_2368;      // 0x2368: position (the matrix translation)
        };
    };
    char pad_2378[0x24];
    int m_239c;
    char pad_23a0[0xec];
    float heading;            // 0x248c: facing angle in radians
    float targetHeading;      // 0x2490: angle the character turns towards
    char pad_2494[8];
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
    char pad_001c[0xc];
    int m_0028;               // 0x28: non-zero = skip the ground probe of a shot (0x1001a590)
    char pad_002c[0x10];
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

