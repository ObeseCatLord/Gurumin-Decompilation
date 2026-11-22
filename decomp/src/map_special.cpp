// Map-specific per-frame updates of maps 0x15..0x18 (DAT_10c62ed0 = current map). C++ rather than extern "C":
// the function keeps an exception frame for its Matrix4 local.

#include "vec4.h"

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                  // 0x101ae770
    ~Matrix4();                                 // 0x1013eda0 (empty)
    void SetIdentity();                         // 0x101ae7a0
    Vec4 GetTranslation();                      // 0x101ade80
    void SetTranslation(Vec4 position);         // 0x101adef0
};

struct EffectManager
{
    // 0x10172350: starts effect `effect` at `position`; `attach` is an optional matrix to place it with.
    int Start(int effect, Vec4 position, int a, int b, int c, int d, int e, Matrix4 *attach, int g, int *handle);
};

// Map object that raises script events (switches.cpp MapObject).
struct MapObject
{
    int RaiseEvent(int id);                     // 0x100020d0
};

extern "C" int DAT_10c6505c;           // map script timer: > 0 running, 0 = raise the end event, < 0 = ended
extern "C" int DAT_10c62ed0;           // current map
extern "C" Matrix4 DAT_10e40204;       // world matrix of the object the map 0x18 effect is placed on
extern "C" EffectManager DAT_10daaa80;
extern "C" MapObject DAT_10874848;
extern "C" void FUN_1007dab0(void);
extern "C" void FUN_10053020(void);
extern "C" void FUN_100524d0(void);
extern "C" void FUN_1007f1e0(void);

// 0x100bd960 (Map_UpdateSpecial). While the map timer runs, calls the map's own update (maps 0x15..0x18). When it
// reaches 0 event 0x1bc is raised; afterwards the timer keeps counting down, and on map 0x18 effect 0x76d is started
// 30 units above the object at DAT_10e40204 every frame. Returns 0.
int Map_UpdateSpecial()
{
    if (DAT_10c6505c > 0)
    {
        switch (DAT_10c62ed0)
        {
        case 0x15:
            FUN_1007dab0();
            break;
        case 0x16:
            FUN_10053020();
            break;
        case 0x17:
            FUN_100524d0();
            break;
        case 0x18:
            FUN_1007f1e0();
            break;
        }
    }
    if (DAT_10c6505c == 0)
    {
        DAT_10c6505c = -1;
        DAT_10874848.RaiseEvent(0x1bc);
    }
    if (DAT_10c6505c < 0)
    {
        DAT_10c6505c--;
        if (DAT_10c62ed0 == 0x18)
        {
            Vec4 position;
            Matrix4 placement;
            placement.SetIdentity();
            placement.SetTranslation(DAT_10e40204.GetTranslation());
            placement.m[14] += 30.0f;
            DAT_10daaa80.Start(0x76d, position, -1, -1, 0, 0, 0, &placement, 0, 0);
        }
    }
    return 0;
}
