// Drawing the short-lived effect sprites (the two EffectRecordB tables of char_effects.cpp): each record is one
// textured square, fading out over its lifetime, queued in the effect sprite batch.

#include "vec4.h"

#include "matrix4.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

#include "texture_object.h"

#include "sprite_batch.h"

#include "effect_manager.h"

extern "C" SpriteBatch DAT_1101c804;
extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// 0x101890f0 (EffectManager::DrawSpriteA). Queues sprite `index` of table A: its world position is projected with
// `view` to the 640x480 screen and the square drawn around it with the sprite's blend mode, its alpha fading with
// the remaining lifetime. Returns 0 (nothing is drawn unless the definition is a sprite).
int EffectManager::DrawSpriteA(int index, const Matrix4 &view, const Matrix4 *camera)
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
int EffectManager::DrawSpriteB(int index, const Matrix4 &view, const Matrix4 *camera)
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
