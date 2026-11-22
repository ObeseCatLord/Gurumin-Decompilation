// Placing an animated model (the CharModel of a Character, here as PathMover like in path_anim.cpp): moving it and
// resetting its motion state. Matrix4 has the engine's constructor and empty destructor because IdentityMatrix()
// returns a temporary (see anim_unload.cpp).

#include "vec4.h"

#include "matrix4.h"

Matrix4 IdentityMatrix();                           // 0x101b0ba0

// A node the model hangs in (its root or the collision it moves against); only the flag cleared here.
struct PlacedNode
{
    char pad_0000[0xa8];
    int m_00a8;
    int ReleaseSavedStates();                       // 0x10194510 (Node::ReleaseSavedStates)
};

struct PathMover
{
    char pad_0000[0x70];
    int m_0070;
    char pad_0074[0x3c];
    float m_00b0;             // 0xb0: height the model was placed at
    char pad_00b4[0x120];
    int m_01d4;
    char pad_01d8[0x64];
    int m_023c;
    char pad_0240[0x50];
    int m_0290;
    char pad_0294[0x1ff4];
    char m_2288;
    char pad_2289[3];
    Vec4 m_228c;
    int m_229c;
    char pad_22a0[0x4c];
    PlacedNode *root;         // 0x22ec: the model's node tree
    char pad_22f0[0x18];
    PlacedNode *collider;     // 0x2308: the map collision this model moves against
    char pad_230c[0x1c];
    Matrix4 world;            // 0x2328: world matrix
    char pad_2368[4];
    float motionSpeed;        // 0x236c
    float m_2370;             // 0x2370: height the model was placed at
    int m_2374;
    char m_2378;
    char m_2379;
    char m_237a;
    char pad_237b;
    float m_237c;
    char pad_2380[4];
    int m_2384;
    char pad_2388[0xc];
    int m_2394;
    char pad_2398[0x10];
    Vec4 move;                // 0x23a8: movement vector (char_move.cpp)
    Vec4 m_23b8;              // 0x23b8: speed vector
    Vec4 m_23c8;
    Vec4 m_23d8;
    Vec4 m_23e8;
    Vec4 m_23f8;
    Vec4 m_2408;
    char pad_2418[0x1c];
    Matrix4 m_2434;           // 0x2434: a second matrix (the ground position, char_ground.cpp)
    int Teleport(float x, float y, float z);
    int SetPosition(float x, float y, float z);
};

extern "C" Matrix4 DAT_118e69a8;

// 0x10162420 (AnimModel_Teleport). Like SetPosition, but the root node drops its saved states and the second
// matrix moves to (x, y, z) as well. Returns 0.
int PathMover::Teleport(float x, float y, float z)
{
    Vec4 position;
    position.z = z;
    m_2370 = z;
    m_00b0 = z;
    position.x = x;
    position.y = y;
    m_023c = 0;
    m_0070 = 0;
    m_228c.Fill(0);
    m_2288 = 0;
    m_229c = 0;
    m_0290 = 0;
    world = IdentityMatrix();
    m_2408.Fill(0);
    m_01d4 = 0;
    if (root != 0)
        root->ReleaseSavedStates();
    world.SetTranslation(position);
    m_2434.SetTranslation(position);
    DAT_118e69a8 = IdentityMatrix();
    m_2394 = 1;
    move.Set(0, 0, 0, 0);
    m_23b8.Set(0, 0, 0, 0);
    m_23d8.Set(0, 0, 0, 0);
    m_23c8.Set(0, 0, 0, 0);
    m_23f8.Set(0, 0, 0, 0);
    m_23e8.Set(0, 0, 0, 0);
    motionSpeed = 0;
    m_237c = 1.0f;
    m_2374 = 0;
    if (collider != 0)
        collider->m_00a8 = 0;
    if (root != 0)
        root->m_00a8 = 0;
    m_2384 = 0;
    m_2379 = 0;
    m_2378 = 0;
    m_237a = 15;
    return 0;
}

// 0x10162610 (AnimModel_Place). Moves the model to (x, y, z) with an identity orientation and stops it: the movement
// and speed vectors and the other motion state are cleared, the motion speed is set to 0 and the root and collision
// nodes lose their m_00a8 flag. DAT_118e69a8 is reset to the identity too. Returns 0.
int PathMover::SetPosition(float x, float y, float z)
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
    m_0070 = 0;
    m_023c = 0;
    m_0290 = 0;
    m_229c = 0;
    world = IdentityMatrix();
    world.SetTranslation(position);
    DAT_118e69a8 = IdentityMatrix();
    m_2394 = 1;
    m_237c = 1.0f;
    motionSpeed = 0;
    move.Set(0, 0, 0, 0);
    m_23b8.Set(0, 0, 0, 0);
    m_23d8.Set(0, 0, 0, 0);
    m_23c8.Set(0, 0, 0, 0);
    m_23f8.Set(0, 0, 0, 0);
    m_23e8.Set(0, 0, 0, 0);
    m_2374 = 0;
    if (collider != 0)
        collider->m_00a8 = 0;
    if (root != 0)
        root->m_00a8 = 0;
    m_2384 = 0;
    m_2379 = 0;
    m_2378 = 0;
    m_237a = 15;
    return 0;
}
