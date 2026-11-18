#include <math.h>

// Camera easing in the scene frame (FUN_100f5a80): the camera target (DAT_118e6cb8) and position (DAT_118e69e8) move
// towards their goals every frame, and the results are copied to the other camera vectors.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Copy(const Vec4 &source);               // 0x101acc20
    Vec4 operator+(const Vec4 &other) const;     // 0x101ac9e0
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
    Vec4 Scaled(const float &factor);            // 0x101acb00
    Vec4();                                      // 0x101ac990: zero
    float Length2D();                            // 0x101ac500: length of x/y
    void RotateZ(float angle);                   // 0x101ac300
};

struct NodeMatrix
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// A scene node (scene_node.cpp Node), only its world matrix.
struct Node
{
    char pad_0000[0x24c];
    NodeMatrix world;         // 0x24c
};

extern "C" Vec4 DAT_118e6cb8;     // camera target
extern "C" Vec4 DAT_118e69e8;     // camera position
extern "C" Vec4 DAT_10edbd48;     // goal of the target (the followed node's position when there is one)
extern "C" Vec4 DAT_10866d98;     // goal of the position
extern "C" Vec4 DAT_10f08aa0;     // the target of the previous frame
extern "C" Vec4 DAT_104d29d8;     // the position of the previous frame
extern "C" Vec4 DAT_118e6c78;     // copies of the target
extern "C" Vec4 DAT_10f668e8;
extern "C" Vec4 DAT_11af3e28;     // copies of the position
extern "C" Vec4 DAT_10f65688;
extern "C" int DAT_104c0358;      // 1 once the camera vectors were set up
extern "C" Node *DAT_104c039c;    // the node the camera follows (0 = none)
extern "C" void FUN_1016a8d0(void);

// 0x10018fb0 (Camera_Ease). On the first call sets the goals from the current camera (the target goal from the
// followed node when there is one) and every camera vector from them. Then each frame the target goal follows the
// node, the target moves 15 % of the way to its goal plus 85 % of the way to its previous value (likewise the
// position), and the copies are refreshed. Returns 0.
extern "C" int FUN_10018fb0(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_10866d98.Copy(DAT_118e69e8);
        if (DAT_104c039c != 0)
            DAT_10edbd48.Copy(DAT_104c039c->world.GetTranslation());
        else
            DAT_10edbd48.Copy(DAT_118e6cb8);
        DAT_118e6cb8.Copy(DAT_10edbd48);
        DAT_118e69e8.Copy(DAT_10866d98);
        DAT_118e6c78.Copy(DAT_118e6cb8);
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_104d29d8.Copy(DAT_118e69e8);
        DAT_10f08aa0.Copy(DAT_118e6cb8);
        DAT_104c0358 = 1;
    }
    if (DAT_104c039c != 0)
        DAT_10edbd48.Copy(DAT_104c039c->world.GetTranslation());
    DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8).Scaled(0.15f) +
                      (DAT_10f08aa0 - DAT_118e6cb8).Scaled(0.85f));
    DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8).Scaled(0.15f) +
                      (DAT_104d29d8 - DAT_118e69e8).Scaled(0.85f));
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}

extern "C" Vec4 DAT_108393d8;     // the point the camera orbits (w: its heading)
extern "C" Vec4 DAT_10da93f0;     // the orbit centre (the player's position)
extern "C" Node *DAT_1021c624;    // a node the target goal drifts towards (0 = none)

// 0x1001a0d0 (Camera_Orbit). Like Camera_Ease with an orbit: the position goal keeps the camera's heading around
// the orbit centre at the distance of DAT_108393d8 from it, turned by that point's heading; the target goal is the
// centre (drifting 5 % towards DAT_1021c624 when set). The target and position then move 20 % towards their goals
// plus 80 % towards their previous values, and the copies are refreshed. Returns 0.
extern "C" int FUN_1001a0d0(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
    }
    Vec4 offset;
    offset.Copy(DAT_118e69e8 - DAT_118e6cb8);
    float heading = (float)atan2(offset.x, offset.y) + 3.1415f;
    offset.x = 0;
    offset.y = -(DAT_108393d8 - DAT_10da93f0).Length2D();
    offset.RotateZ(DAT_108393d8.w - heading);
    offset.Copy(offset + DAT_10da93f0);
    DAT_10866d98.Copy(offset);
    DAT_10edbd48.Copy(DAT_10da93f0);
    if (DAT_1021c624 != 0)
        DAT_10edbd48.Copy(DAT_10edbd48.Scaled(0.95f) + DAT_1021c624->world.GetTranslation().Scaled(0.05f));
    DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8).Scaled(0.2f) +
                      (DAT_10f08aa0 - DAT_118e6cb8).Scaled(0.8f));
    DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8).Scaled(0.2f) +
                      (DAT_104d29d8 - DAT_118e69e8).Scaled(0.8f));
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}
