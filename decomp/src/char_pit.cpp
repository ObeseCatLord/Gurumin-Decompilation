// An enemy dragged towards the player and into a pit (behaviour step run from 0x100dfcf0).

#include "vec4.h"

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();                           // 0x101ade80
};

struct CharModel
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0
    void SetPosition(float x, float y, float z);             // 0x10162610
    int IsClearOfMapLimited(const Vec4 *move);               // 0x101641b0
};

// A character seen from its model (world.h Character + 0x10): the per-character mode at 0x2498 lies beyond the
// 0x249c bytes of world.h's Character, which starts 0x10 bytes earlier.
struct CharacterBody
{
    CharModel model;
    char pad_0001[0xd4b];
    float alpha;                                     // 0xd4c (Character 0xd5c)
    char pad_0d50[0x15d8];
    Matrix4 matrix;                                  // 0x2328 (Character 0x2338)
    char pad_2368[0x114];
    float heading;                                   // 0x247c (Character 0x248c)
    float targetHeading;                             // 0x2480
    char pad_2484[0x14];
    int mode;                                        // 0x2498: 1 = normal, 2 = over the pit area
};

struct CharState
{
    int m_0000;
    int frame;                                       // 0x04: frames in this behaviour
    char pad_0008[0x38];
};

// A character slot record (world.h CharSlot) seen from 0x64 bytes earlier.
struct CharSlotFlags
{
    int busy;                                        // 0x00
    char pad_0004[0x105];
    char m_0109;                                     // 0x109 (CharSlot m_00a5)
    char pad_010a[0x7e];
};

extern "C" CharacterBody DAT_10db6ee8[0x80];
extern "C" CharState DAT_1020b430[0x80];
extern "C" CharSlotFlags DAT_10daab20[0x80];
extern "C" char DAT_1021f9c0[0x80];
extern "C" Vec4 DAT_10867e18[0x80];
extern "C" int DAT_104bfe28;                         // non-zero when the map has a pit
extern "C" float DAT_104bfe24;                       // the pit's floor height
extern "C" Vec4 DAT_104d29e8;                        // the pit area: minimum corner
extern "C" Vec4 DAT_104d29f8;                        // maximum corner
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove
#include <math.h>

// The game's atan2 wrapper (balloon_burst.cpp); the result is rounded to a float in a local.
inline float Atan2(float y, float x)
{
    float angle = (float)atan2(y, x);
    return angle;
}

// 0x100b9930 (Enemy_DragToPit). Turns character `index` towards the player and moves it 180 units per step along
// its facing (lower by 5 over the pit), counting the frames. After 150 frames (or once it is blocked) it fades out
// by 1/15 per frame and after 165 it is moved away (x -200000). While its height is 25 below the pit floor and its
// x/y lie inside the pit area its mode is 2, else 1. Returns 0.
extern "C" int FUN_100b9930(int index)
{
    DAT_10daab20[index].busy = 1;
    DAT_10daab20[index].m_0109 = 1;
    DAT_1021f9c0[index] = 0;
    Vec4 player;
    Vec4 position;
    Vec4 toPlayer;
    Vec4 move;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - position);
    DAT_1020b430[index].frame++;
    DAT_10db6ee8[index].model.PlayMotion(1, 0, 5, 1.0f);
    DAT_10db6ee8[index].targetHeading = Atan2(toPlayer.x, toPlayer.y);
    move.y = -180.0f;
    move.RotateZ(-DAT_10db6ee8[index].heading);
    if (DAT_10db6ee8[index].mode == 2)
        move.z = 5.0f;
    FUN_100241e0(index, move.x, move.y, move.z);
    move.z = 10.0f;
    if (DAT_1020b430[index].frame < 0x96 && DAT_10db6ee8[index].model.IsClearOfMapLimited(&move) == 0)
        DAT_1020b430[index].frame = 0x96;
    int frames = DAT_1020b430[index].frame;
    if (frames > 0x96)
    {
        DAT_10db6ee8[index].mode = 1;
        float alpha = DAT_10db6ee8[index].alpha - 0.066667f;
        DAT_10db6ee8[index].alpha = alpha;
        if (alpha < 0.0f)
            DAT_10db6ee8[index].alpha = 0;
    }
    if (frames >= 0xa5)
    {
        DAT_10db6ee8[index].model.SetPosition(-200000.0f, 0, 0);
        DAT_10867e18[index].x = -200000.0f;
    }
    if (DAT_104bfe28 != 0 && DAT_104bfe24 - 25.0f > DAT_10db6ee8[index].matrix.m[14])
    {
        int y = (int)DAT_10db6ee8[index].matrix.m[13];
        int x = (int)DAT_10db6ee8[index].matrix.m[12];
        if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
            y <= (int)DAT_104d29f8.y)
        {
            DAT_10db6ee8[index].mode = 2;
            return 0;
        }
    }
    DAT_10db6ee8[index].mode = 1;
    return 0;
}
