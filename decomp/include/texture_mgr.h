// ZeroVec, Transform, Selection, Animator: declared here once (first used by src/texture_mgr.cpp).
#ifndef GURUMIN_TEXTURE_MGR_H
#define GURUMIN_TEXTURE_MGR_H

#include "node.h"

struct ZeroVec
{
    float x;
    float y;
    float z;
    float w;
    ZeroVec();
};
struct Transform
{
    int m_0000;
    ZeroVec position;
    ZeroVec rotation;
    ZeroVec scale;
    Matrix4 matrix;
    Transform();
};
struct Selection
{
    int m_0000;
    int m_0004;
    int m_0008;
    int pad_000c;
    int m_0010;
    int current;
    int M_10172a80(int a, int b);
    int M_10172aa0();
    void M_101733f0();
    int M_10173450(int id, int unused);
    void M_10172ac0(int a, int count);
    int M_10188eb0(int id, int a, int count);
    void M_10172cb0(float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1, float v1, float x2, float y2, float u2, float v2, int colour, int a, int b);
    int AddQuad(int id, float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1, float v1, float x2, float y2, float u2, float v2, int colour, int a, int b);
};
struct Animator
{
    void M_101687e0();
    int M_101692c0();
    int M_10184160();
};

#endif
