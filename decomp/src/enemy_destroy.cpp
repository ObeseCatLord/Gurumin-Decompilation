// Destroying a character (an enemy or a breakable object). The character records are viewed from their model part
// at 0x10db6ee8 (like char_move.cpp).

#include "vec4.h"

#include "matrix4.h"

struct CharRecord
{
    void Place(float x, float y, float z);       // 0x10162610: moves the model
    char pad_0000[0x270];
    int flag;                 // 0x270: script flag set when it is destroyed (0 = none)
    int model;                // 0x274: index of the model left behind (path table DAT_10894698)
    char pad_0278[0x10];
    int m_0288;               // 0x288: set to 6 when destroyed
    int m_028c;               // 0x28c: cleared when destroyed
    char pad_0290[0x2098];
    Matrix4 matrix;        // 0x2328: world matrix
    char pad_2368[0x118];
    float targetHeading;      // 0x2480: angle the character turns towards
    char pad_2484[0x18];
};

struct EffectManager
{
    // 0x10172350 (char_effects.cpp): starts effect `effect` at `position`.
    int M_10172350(int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g, int *handle);
};

// The engine object at 0x10836c70 (engine_1013e.cpp).
struct Engine
{
    int ResetFrameTimer();                       // 0x1013eee0: FrameTimer_ResetNext
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" EffectManager DAT_10daaa80;
extern "C" Engine DAT_10836c70;
extern "C" char DAT_10894698[][0x124];   // model paths
extern "C" char DAT_1021f3c0[0x80];      // slot kind
extern "C" int DAT_10c62edc[0x1000];     // script flags
extern "C" int DAT_10c62ed0;             // current map
extern "C" int FUN_1003ad20(int sound, const Vec4 &position, float volume);   // Sound_PlayEffect
extern "C" int FUN_100119d0(int id, int value, char flag);                    // EventSlots_Add
// EffectModel_PlaceColoured (effect_model.cpp): places the model `path` at (x, y, z) turned by (ox, oy, oz) degrees.
extern "C" int FUN_1003a880(const char *path, float x, float y, float z, int start, float ox, float oy, float oz,
                            const Vec4 &colour);

// 0x100541f0 (Enemy_Destroy). Destroys character `index`: plays sound 0x336 and starts effect 0x28a 50 units above
// it, and on maps below 100 adds event slot (index, 10). Then, unless m_028c was set again meanwhile, its slot kind
// becomes 0xaf, sound 0x337 plays, its model (DAT_10894698[model]) is placed where it stood facing its heading, its
// script flag is set to 0x29a and the character is moved out of the world. Returns 0.
extern "C" int FUN_100541f0(int index)
{
    DAT_10db6ee8[index].m_0288 = 6;
    DAT_10db6ee8[index].m_028c = 0;
    FUN_1003ad20(0x336, DAT_10db6ee8[index].matrix.GetTranslation(), 1.0f);
    Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
    top.z += 50.0f;
    DAT_10daaa80.M_10172350(0x28a, top, -1, 0, 0, 0, 0, 0, 0, 0);
    if (DAT_10c62ed0 < 100)
        FUN_100119d0(index, 10, 1);
    if (DAT_10db6ee8[index].m_028c == 0)
    {
        Vec4 position = DAT_10db6ee8[index].matrix.GetTranslation();
        DAT_1021f3c0[index] = (char)0xaf;
        FUN_1003ad20(0x337, DAT_10db6ee8[index].matrix.GetTranslation(), 1.0f);
        int model = DAT_10db6ee8[index].model;
        Vec4 colour;
        colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
        FUN_1003a880(DAT_10894698[model], position.x, position.y, position.z, 0, 0, 0,
                     DAT_10db6ee8[index].targetHeading * 180.0f / 3.1415f, colour);
        DAT_10836c70.ResetFrameTimer();
        int flag = DAT_10db6ee8[index].flag;
        if (flag <= 3999 && flag != 0)
            DAT_10c62edc[flag] = 0x29a;
        DAT_10db6ee8[index].Place(-250000.0f, 0, 0);
    }
    return 0;
}
