// Destructor of the scene node body (0x18a0 bytes per node).

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

// 0x10191480. Empty body: the compiler destroys the matrix members and the array of sixteen matrices.
NodeBody::~NodeBody()
{
}

