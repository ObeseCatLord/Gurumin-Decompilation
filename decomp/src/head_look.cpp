// Head tracking of the player character: the head bone's parent node gets a saved-state block whose rotation limits
// bound how far the head may turn towards its target (Node::M_101948f0 enters the tracking state 0x400000).

struct Vec3
{
    float x;
    float y;
    float z;
};

// Saved-state block of a node (scene_node.cpp NodeSaved, 0xdc bytes).
struct NodeSaved
{
    char pad_0000[0x20];
    int state;                // 0x20: 2 = let the tracked rotation return to rest
    char pad_0024[0x54];
    Vec3 limitMin;            // 0x78: lowest rotation per axis (radians)
    int pad_0084;
    Vec3 limitMax;            // 0x88: highest rotation per axis
};

struct Node
{
    char pad_0000[0x74c];
    Node *parent;             // 0x74c
    char pad_0750[0x1104];
    NodeSaved *saved;         // 0x1854
    int M_101947b0();                 // allocates `saved` on first use
    int M_101948f0(Node *target);     // enters the tracking state (0 = default target)
    int M_10194980();                 // frees `saved`
};

extern "C" Node *DAT_10db91dc;  // the player's head node (character 0, field 0x2304)

// mode 1: starts head tracking, limited to +-1.22 rad around x, +-0.27 around y and +-0.17 around z;
// mode 0: lets the head return to rest; mode -5: removes the tracking block. Returns 0.
extern "C" int FUN_100165e0(int mode)
{
    if (DAT_10db91dc != 0)
    {
        if (mode == 1)
        {
            DAT_10db91dc->parent->M_101947b0();
            DAT_10db91dc->parent->saved->limitMin.x = -1.22f;
            DAT_10db91dc->parent->saved->limitMin.y = -0.27f;
            DAT_10db91dc->parent->saved->limitMin.z = -0.17f;
            DAT_10db91dc->parent->saved->limitMax.x = 1.22f;
            DAT_10db91dc->parent->saved->limitMax.y = 0.27f;
            DAT_10db91dc->parent->saved->limitMax.z = 0.17f;
            DAT_10db91dc->parent->M_101948f0(0);
            return 0;
        }
        if (mode == 0)
        {
            if (DAT_10db91dc->parent->saved != 0)
            {
                DAT_10db91dc->parent->saved->state = 2;
                return 0;
            }
        }
        else if (mode == -5 && DAT_10db91dc->parent->saved != 0)
            DAT_10db91dc->parent->M_10194980();
    }
    return 0;
}

// The look-around target point and the player's position and heading.
#include "vec4.h"

struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();      // 0x101ade80
};

extern "C" Vec4 DAT_10802ac8;          // where the player's head looks
extern "C" int DAT_10c6358c;           // look-around step: 0..9 = one side, -1..-9 = the other side
extern "C" CharMatrix DAT_10db9210;    // the player's world matrix (character 0)
extern "C" float DAT_10db9364;         // the player's heading (character 0)

// 0x10019210 (Player_LookAround). Moves the head-look target to a point 1000 units to one side of the player and
// 500 units up, switching sides every ten calls, and (re)starts head tracking. Returns 0.
extern "C" int FUN_10019210(void)
{
    DAT_10802ac8.z = 500.0f;
    DAT_10802ac8.y = -1000.0f;
    if (DAT_10c6358c >= 0)
    {
        DAT_10802ac8.RotateZ(1.5708f - DAT_10db9364);
        DAT_10802ac8.Copy(DAT_10802ac8 + DAT_10db9210.GetTranslation());
        DAT_10c6358c++;
        if (DAT_10c6358c >= 10)
        {
            DAT_10c6358c = -1;
            FUN_100165e0(1);
            return 0;
        }
    }
    else
    {
        DAT_10802ac8.RotateZ(-DAT_10db9364 - 1.5708f);
        DAT_10802ac8.Copy(DAT_10802ac8 + DAT_10db9210.GetTranslation());
        DAT_10c6358c--;
        if (DAT_10c6358c <= -10)
            DAT_10c6358c = 1;
    }
    FUN_100165e0(1);
    return 0;
}
