// Drawing the short-lived effect sprites (the two EffectRecordB tables of char_effects.cpp): each record is one
// textured square, fading out over its lifetime, queued in the effect sprite batch.

#include "vec4.h"

struct Matrix4
{
    float m[16];
};

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

struct TextureObject
{
    char data[0x38];
};

// The effect sprite batch (texture_mgr.cpp Selection).
struct SpriteBatch
{
    int SetBlend(int mode, int unused);          // 0x10173450
    // 0x10188f10: queues a square from (left, top) to (right, bottom) with texture coordinates (u0, v0)-(u1, v1),
    // depth `z`, the alpha and two colours.
    int AddSprite(TextureObject *texture, float left, float top, float right, float bottom, float u0, float v0,
                  float u1, float v1, float z, float alpha, float r, float g, float b, float r2, float g2, float b2,
                  float m_0078);
};

// A sprite record (char_effects.cpp EffectRecordB, 0xc4 bytes).
struct EffectSprite
{
    int type;                    // effect definition of the sprite, -1 = free
    Vec4 position;               // 0x04: world position (table A) or screen position (table B)
    float colour[3];             // 0x14
    float alpha;                 // 0x20
    float colour2[3];            // 0x24
    char pad_0030[0x44];
    float size;                  // 0x74: half the width of the square
    float m_0078;
    float u0;                    // 0x7c
    float u1;                    // 0x80
    char pad_0084[0x18];
    float v0;                    // 0x9c
    float v1;                    // 0xa0
    char pad_00a4[0x18];
    short lifetime;              // 0xbc: frames the record lives
    short age;                   // 0xbe: frames it has lived
    int pad_00c0;
};

// Effect definition (char_effects.cpp EffectType, 0x3d8 bytes), only the fields used here.
struct EffectType
{
    int id;
    int kind;                    // 0x04: 1 and 0x12 are sprites
    char pad_0008[0x64];
    int depthTest;               // 0x6c: non-zero = drawn with render state 9 on
    char pad_0070[0x18];
    int fullAlphaB;              // 0x88: table B: non-zero = the sprite starts at full alpha (else half)
    char pad_008c[0xc];
    int fullAlphaA;              // 0x98: the same for table A
    char pad_009c[0x60];
    int texture;                 // 0xfc: texture slot
    int blend;                   // 0x100: 0-6
    char pad_0104[0x15c];
    char flat;                   // 0x260: table A: drawn at depth 0
    char pad_0261[0x177];
};

struct EffectManager
{
    int DrawSpriteA(int index, const Matrix4 &view, int unused);
    int DrawSpriteB(int index, const Matrix4 &view, int unused);
};

extern "C" EffectSprite DAT_11134cc8[];          // table A: sprites placed in the world
extern "C" EffectSprite DAT_118a8888[];          // table B: sprites placed on the screen
extern "C" EffectType DAT_1101dfdc[0x47e];
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" SpriteBatch DAT_1101c804;
extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// 0x101890f0 (EffectManager::DrawSpriteA). Queues sprite `index` of table A: its world position is projected with
// `view` to the 640x480 screen and the square drawn around it with the sprite's blend mode, its alpha fading with
// the remaining lifetime. Returns 0 (nothing is drawn unless the definition is a sprite).
int EffectManager::DrawSpriteA(int index, const Matrix4 &view, int unused)
{
    int type = DAT_11134cc8[index].type;
    FUN_1013f070(0, 0, 0);
    if (DAT_1101dfdc[type].depthTest != 0)
        FUN_1013f070(9, 1, 0);
    else
        FUN_1013f070(9, 0, 0);
    if (DAT_1101dfdc[type].kind == 1 || DAT_1101dfdc[type].kind == 0x12)
    {
        Vec4 screen;
        screen.Copy(Project(DAT_11134cc8[index].position, view));
        float size = DAT_11134cc8[index].size;
        screen.x = screen.x * 640.0f / 2.0f + 320.0f;
        screen.y = 240.0f - screen.y * 480.0f / 2.0f;
        int blend = DAT_1101dfdc[type].blend;
        if (blend == 0)
            DAT_1101c804.SetBlend(0, 0);
        else if (blend == 1)
            DAT_1101c804.SetBlend(1, 0);
        else if (blend == 2)
            DAT_1101c804.SetBlend(2, 0);
        else if (blend == 3)
            DAT_1101c804.SetBlend(10, 0);
        else if (blend == 4)
            DAT_1101c804.SetBlend(11, 0);
        else if (blend == 5)
            DAT_1101c804.SetBlend(12, 0);
        else if (blend == 6)
            DAT_1101c804.SetBlend(13, 0);
        if (DAT_1101dfdc[type].flat)
            screen.z = 0;
        if (DAT_1101dfdc[type].fullAlphaA != 0)
            DAT_1101c804.AddSprite(&DAT_118d0ba8[DAT_1101dfdc[type].texture], screen.x - size, screen.y - size,
                                   screen.x + size, screen.y + size, DAT_11134cc8[index].u0,
                                   DAT_11134cc8[index].v0, DAT_11134cc8[index].u1, DAT_11134cc8[index].v1, screen.z,
                                   (float)(DAT_11134cc8[index].lifetime - DAT_11134cc8[index].age) *
                                       DAT_11134cc8[index].alpha /
                                       (float)DAT_11134cc8[index].lifetime,
                                   DAT_11134cc8[index].colour[0], DAT_11134cc8[index].colour[1],
                                   DAT_11134cc8[index].colour[2], DAT_11134cc8[index].colour2[0],
                                   DAT_11134cc8[index].colour2[1], DAT_11134cc8[index].colour2[2],
                                   DAT_11134cc8[index].m_0078);
        else
            DAT_1101c804.AddSprite(&DAT_118d0ba8[DAT_1101dfdc[type].texture], screen.x - size, screen.y - size,
                                   screen.x + size, screen.y + size, DAT_11134cc8[index].u0,
                                   DAT_11134cc8[index].v0, DAT_11134cc8[index].u1, DAT_11134cc8[index].v1, screen.z,
                                   (float)(DAT_11134cc8[index].lifetime - DAT_11134cc8[index].age) *
                                       DAT_11134cc8[index].alpha * 0.5f /
                                       (float)DAT_11134cc8[index].lifetime,
                                   DAT_11134cc8[index].colour[0], DAT_11134cc8[index].colour[1],
                                   DAT_11134cc8[index].colour[2], DAT_11134cc8[index].colour2[0],
                                   DAT_11134cc8[index].colour2[1], DAT_11134cc8[index].colour2[2],
                                   DAT_11134cc8[index].m_0078);
    }
    return 0;
}

// 0x10189340 (EffectManager::DrawSpriteB). The same for sprite `index` of table B, whose position is already in
// screen coordinates. Returns 0.
int EffectManager::DrawSpriteB(int index, const Matrix4 &view, int unused)
{
    int type = DAT_118a8888[index].type;
    FUN_1013f070(0, 0, 0);
    if (DAT_1101dfdc[type].depthTest != 0)
        FUN_1013f070(9, 1, 0);
    else
        FUN_1013f070(9, 0, 0);
    if (DAT_1101dfdc[type].kind == 1 || DAT_1101dfdc[type].kind == 0x12)
    {
        Vec4 screen;
        screen.Copy(DAT_118a8888[index].position);
        float size = DAT_118a8888[index].size;
        int blend = DAT_1101dfdc[type].blend;
        if (blend == 0)
            DAT_1101c804.SetBlend(0, 0);
        else if (blend == 1)
            DAT_1101c804.SetBlend(1, 0);
        else if (blend == 2)
            DAT_1101c804.SetBlend(2, 0);
        else if (blend == 3)
            DAT_1101c804.SetBlend(10, 0);
        else if (blend == 4)
            DAT_1101c804.SetBlend(11, 0);
        else if (blend == 5)
            DAT_1101c804.SetBlend(12, 0);
        else if (blend == 6)
            DAT_1101c804.SetBlend(13, 0);
        if (DAT_1101dfdc[type].fullAlphaB != 0)
            DAT_1101c804.AddSprite(&DAT_118d0ba8[DAT_1101dfdc[type].texture], screen.x - size, screen.y - size,
                                   screen.x + size, screen.y + size, DAT_118a8888[index].u0,
                                   DAT_118a8888[index].v0, DAT_118a8888[index].u1, DAT_118a8888[index].v1, screen.z,
                                   (float)(DAT_118a8888[index].lifetime - DAT_118a8888[index].age) *
                                       DAT_118a8888[index].alpha /
                                       (float)DAT_118a8888[index].lifetime,
                                   DAT_118a8888[index].colour[0], DAT_118a8888[index].colour[1],
                                   DAT_118a8888[index].colour[2], DAT_118a8888[index].colour2[0],
                                   DAT_118a8888[index].colour2[1], DAT_118a8888[index].colour2[2],
                                   DAT_118a8888[index].m_0078);
        else
            DAT_1101c804.AddSprite(&DAT_118d0ba8[DAT_1101dfdc[type].texture], screen.x - size, screen.y - size,
                                   screen.x + size, screen.y + size, DAT_118a8888[index].u0,
                                   DAT_118a8888[index].v0, DAT_118a8888[index].u1, DAT_118a8888[index].v1, screen.z,
                                   (float)(DAT_118a8888[index].lifetime - DAT_118a8888[index].age) *
                                       DAT_118a8888[index].alpha * 0.5f /
                                       (float)DAT_118a8888[index].lifetime,
                                   DAT_118a8888[index].colour[0], DAT_118a8888[index].colour[1],
                                   DAT_118a8888[index].colour[2], DAT_118a8888[index].colour2[0],
                                   DAT_118a8888[index].colour2[1], DAT_118a8888[index].colour2[2],
                                   DAT_118a8888[index].m_0078);
    }
    return 0;
}
