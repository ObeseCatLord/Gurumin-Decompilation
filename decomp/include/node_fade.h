// FadeParent, FadeNode, FadeOwner: declared here once (first used by src/node_fade.cpp).
#ifndef GURUMIN_NODE_FADE_H
#define GURUMIN_NODE_FADE_H

#include "matrix4.h"

struct FadeParent
{
    char pad_0000[0x204];
    float glow;               // 0x204: strength from the distance to the viewer
};
struct FadeNode
{
    char pad_0000[0x1cc];
    Matrix4 matrix;           // 0x1cc
    char pad_020c[0xac];
    float alpha;              // 0x2b8: 1 = fully visible
    char pad_02bc[0x490];
    FadeParent *parent;       // 0x74c
};
struct FadeOwner
{
    char pad_0000[0x1899];
    unsigned char brightness; // 0x1899
};

#endif
