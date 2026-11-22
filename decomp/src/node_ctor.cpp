// Scene node constructor.

#include "matrix4.h"

#include "vec4.h"

struct SceneNode
{
    char pad_0000[0x1cc];
    Matrix4 local;
    Matrix4 world;
    Matrix4 m_024c;
    char pad_028c[0xc];
    Vec4 m_0298;
    int m_02a8;
    Vec4 m_02ac;
    Vec4 m_02bc;
    char pad_02cc[8];
    Matrix4 m_02d4[0x10];
    Vec4 m_06d4;
    Vec4 m_06e4[2];
    char pad_0704[0x1164];
    Vec4 m_1868;
    SceneNode();
    void M_10191500();
};

// Member constructors run first (matrices and vectors), then Init (0x10191500) fills in the defaults.
SceneNode::SceneNode()
{
    M_10191500();
}
