// Element, NodeBody: declared here once (first used by src/node_dtor.cpp).
#ifndef GURUMIN_NODE_DTOR_H
#define GURUMIN_NODE_DTOR_H

#include "matrix4.h"

struct Element
{
    float v[4];
    ~Element();
};
struct NodeBody
{
    char pad_0000[0x1cc];
    Matrix4 m_01cc;
    Matrix4 m_020c;
    Matrix4 m_024c;
    char pad_028c[0x48];
    Matrix4 m_02d4[0x10];
    ~NodeBody();
};

#endif
