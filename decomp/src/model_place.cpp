// Placing an animated model (the CharModel of a Character, here as PathMover like in path_anim.cpp): moving it and
// resetting its motion state. Matrix4 has the engine's constructor and empty destructor because IdentityMatrix()
// returns a temporary (see anim_unload.cpp).

#include "vec4.h"

#include "matrix4.h"

Matrix4 IdentityMatrix();                           // 0x101b0ba0

#include "node.h"

#include "anim_model.h"

extern "C" Matrix4 DAT_118e69a8;

// 0x10162420 (AnimModel_Teleport). Like SetPosition, but the root node drops its saved states and the second
// matrix moves to (x, y, z) as well. Returns 0.
int AnimModel::Teleport(float x, float y, float z)
{
    Vec4 position;
    position.z = z;
    m_2370 = z;
    m_00b0 = z;
    position.x = x;
    position.y = y;
    m_023c = 0;
    nearCells = 0;
    m_228c.Fill(0);
    m_2288 = 0;
    m_229c = 0;
    m_0290 = 0;
    matrix = IdentityMatrix();
    m_2408.Fill(0);
    m_01d4 = 0;
    if (root != 0)
        root->ReleaseSavedStates();
    matrix.SetTranslation(position);
    m_2434.SetTranslation(position);
    DAT_118e69a8 = IdentityMatrix();
    m_2394 = 1;
    move.Set(0, 0, 0, 0);
    velocity.Set(0, 0, 0, 0);
    m_23d8.Set(0, 0, 0, 0);
    m_23c8.Set(0, 0, 0, 0);
    m_23f8.Set(0, 0, 0, 0);
    m_23e8.Set(0, 0, 0, 0);
    motionSpeed = 0;
    m_237c = 1.0f;
    m_2374 = 0;
    if (collider != 0)
        collider->hasCollision = 0;
    if (root != 0)
        root->hasCollision = 0;
    m_2384 = 0;
    m_2379 = 0;
    m_2378 = 0;
    m_237a = 15;
    return 0;
}

// 0x10162610 (AnimModel_Place). Moves the model to (x, y, z) with an identity orientation and stops it: the movement
// and speed vectors and the other motion state are cleared, the motion speed is set to 0 and the root and collision
// nodes lose their m_00a8 flag. DAT_118e69a8 is reset to the identity too. Returns 0.
int AnimModel::SetPosition(float x, float y, float z)
{
    Vec4 position;
    position.z = z;
    m_2370 = z;
    m_00b0 = z;
    position.x = x;
    position.y = y;
    m_228c.Fill(0);
    m_2288 = 0;
    m_2408.Fill(0);
    m_01d4 = 0;
    nearCells = 0;
    m_023c = 0;
    m_0290 = 0;
    m_229c = 0;
    matrix = IdentityMatrix();
    matrix.SetTranslation(position);
    DAT_118e69a8 = IdentityMatrix();
    m_2394 = 1;
    m_237c = 1.0f;
    motionSpeed = 0;
    move.Set(0, 0, 0, 0);
    velocity.Set(0, 0, 0, 0);
    m_23d8.Set(0, 0, 0, 0);
    m_23c8.Set(0, 0, 0, 0);
    m_23f8.Set(0, 0, 0, 0);
    m_23e8.Set(0, 0, 0, 0);
    m_2374 = 0;
    if (collider != 0)
        collider->hasCollision = 0;
    if (root != 0)
        root->hasCollision = 0;
    m_2384 = 0;
    m_2379 = 0;
    m_2378 = 0;
    m_237a = 15;
    return 0;
}
