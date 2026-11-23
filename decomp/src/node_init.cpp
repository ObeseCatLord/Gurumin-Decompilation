// Initialising a scene node (Node, 0x18a0 bytes; scene_node.cpp has the fields that are understood). The fields are
// set in the original's order; most of their meanings are still open. Matrix4 has the engine's constructor and empty
// destructor because IdentityMatrix() returns a temporary.

#include <string.h>

#include "vec4.h"

#include "matrix4.h"

Matrix4 IdentityMatrix();                           // 0x101b0ba0

#include "render_vars.h"

#include "node.h"

// 0x10191500 (Node::Init). Resets every field of a new node: called "root", no mesh, children or skin, identity world
// and local matrices, cull margin 3.84, unit scale (tint and m_1868..m_1874), and the remaining defaults.
void Node::Init()
{
    m_1860 = 0;
    isStatic = 0;
    staticCount = 0;
    staticNodes = 0;
    m_0088 = 0;
    kind = 0xff;
    m_0298.Fill(0);
    m_1899 = 30;
    m_0074 = 0;
    drawCallback = 0;
    m_187a = 0;
    m_187b = 45;
    pathKind = 0;
    m_187c = 0;
    m_187d = 0;
    m_187e = 0xff;
    m_0098 = DAT_11b07118;
    m_1858 = 0;
    m_1884 = 0;
    m_1888 = 45.0f;
    m_188c = 0;
    m_1890 = 0;
    m_028c = 0;
    m_189a = 0xff;
    m_189b = 0xff;
    element = 0;
    m_1880 = 1.0f;
    tint.Fill(1.0f);
    m_1878 = 0xe0;
    m_0080 = 0;
    m_0084 = 0;
    m_007c = 0;
    mesh = 0;
    bounds.Set(0, 0, 0, 0);
    frameSource = 0;
    frame = 0;
    strcpy(name, "root");
    animated = 0;
    frameCount = 0;
    frameSize = 0;
    extra = 0;
    saved = 0;
    skin = 0;
    visibilityCells = 0;
    for (int i = 0; i < 16; i++)
    {
        plugType[i] = 0;
        plugs[i] = 0;
        plugTypeCopy[i] = 0;
        plugData[i] = 0;
    }
    hasCollision = 0;
    m_00b8 = 0;
    flags = 0;
    cullMargin = 3.84f;
    group = 0;
    m_0070 = 0;
    m_1894 = 0;
    frameMatrixCount = 0;
    parent = 0;
    skinned = 0;
    materialCount = 0;
    materials = 0;
    childCount = 0;
    frameMatrices = 0;
    m_0060 = 0;
    packs = 0;
    indexLists = 0;
    packCount = 0;
    worldMatrix = IdentityMatrix();
    worldMatrix.m[12] = 10000.0f;
    matrix = IdentityMatrix();
    m_070c[0] = 0;
    memset(childNames, 0, sizeof(childNames));
    m_1868.x = 1.0f;
    m_1868.y = 1.0f;
    m_1868.z = 1.0f;
    m_1868.w = 1.0f;
}
