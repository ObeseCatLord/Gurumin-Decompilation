// An enemy dragged towards the player and into a pit (behaviour step run from 0x100dfcf0).

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_state.h"
#include "char_slot.h"

#include "char_tables.h"
#include "char_vars.h"
#include "map_vars.h"
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
    DAT_10daaae8[index].stats[STAT_BUSY] = 1;
    DAT_10daaae8[index].m_0141 = 1;
    DAT_1021f9c0[index] = 0;
    Vec4 player;
    Vec4 position;
    Vec4 toPlayer;
    Vec4 move;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - position);
    DAT_1020b430[index].wait++;
    DAT_10db6ee8[index].PlayMotion(1, 0, 5, 1.0f);
    DAT_10db6ee8[index].targetHeading = Atan2(toPlayer.x, toPlayer.y);
    move.y = -180.0f;
    move.RotateZ(-DAT_10db6ee8[index].heading);
    if (DAT_10db6ee8[index].mode == 2)
        move.z = 5.0f;
    FUN_100241e0(index, move.x, move.y, move.z);
    move.z = 10.0f;
    if (DAT_1020b430[index].wait < 0x96 && DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0)
        DAT_1020b430[index].wait = 0x96;
    int frames = DAT_1020b430[index].wait;
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
        DAT_10db6ee8[index].SetPosition(-200000.0f, 0, 0);
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
