#include <stdlib.h>

struct Object
{
    Object();
    ~Object();
};

struct CharModel
{
    void M_1017b0e0(int motion, int a, int b, float speed);
};

struct Character
{
    int id;
    char pad_0004[0xc];
    CharModel model;
    char pad_0011[0x248b];
};

extern "C" void FUN_101b21c0(void);
extern "C" void FUN_1003ad20(int, int, float);
extern "C" int DAT_104c01b0;
extern "C" int DAT_102027d8;
extern "C" int FUN_10001b70(int);
extern "C" int DAT_104c0198;
extern "C" int DAT_104c00d0;
extern "C" Character DAT_10db6ed8[0x80];

Object::Object()
{
}

Object::~Object()
{
}

extern "C" int FUN_1013edb0(void)
{
    return 0;
}

extern "C" int FUN_1013eed0(void)
{
    return 1;
}

extern "C" int FUN_1013e6d0(void)
{
    FUN_101b21c0();
    return 1;
}

extern "C" int FUN_1003b670(int value)
{
    FUN_1003ad20(0x355, 0, 1.0f);
    if (DAT_104c01b0 == 0)
        FUN_10001b70((100 - (DAT_102027d8 * 5 + 50) / 2) * 100);
    DAT_104c0198 = value;
    DAT_104c00d0 = 0x816;
    DAT_10db6ed8[0].model.M_1017b0e0(0x59, 1, 5, 1.0f);
    rand();
    return 0;
}
