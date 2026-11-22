// A sinking block (character behaviour run from 0x100f9670): it waits high above the map, drops to the ground once
// DAT_10db0544 falls below half of DAT_10db0548, rises back by 5 units a frame and then shoves the player out of
// the way.

#include "vec4.h"

struct Vec3
{
    float x;
    float y;
    float z;
};

#include "matrix4.h"

struct AnimModel
{
    int SetPosition(float x, float y, float z);  // 0x10162610 (model_place.cpp PathMover::SetPosition)
    int IsClearAny(const Vec4 *move);            // 0x101640b0: 0 when the move collides
};

struct RootNode
{
    void M_10194730(int value, int mask);        // sets a node state bit in the whole tree
};

// A character record (world.h Character), only the fields used here.
struct Character
{
    char pad_0000[0x10];
    AnimModel model;              // 0x10
    char pad_0011[0x22eb];
    RootNode *root;               // 0x22fc
    char pad_2300[0x38];
    Matrix4 matrix;               // 0x2338: world matrix; matrix._41/_42/_43 (0x2368) is the position
    char pad_2378[0x40];
    Vec4 move;                    // 0x23b8: the move of this frame
    char pad_23c8[0xd4];
};

// Field 0x24a8 of each character record (npc_approach.cpp CharMoveLock).
struct CharMoveLock
{
    int locked;                   // 2 = the player is standing on something
    char pad_0004[0x2498];
};

// Per-character AI state (world.h CharState): 0 = not placed, 1 = waiting, 2 = dropped.
struct BlockState
{
    int state;
    char pad_0004[0x3c];
};

// A 0x40-byte record per character slot (route_walk.cpp StepRecord): the position of the last frame.
struct LastPosition
{
    Vec4 position;
    char pad_0010[0x30];
};

extern "C" Character DAT_10db6ed8[0x80];
extern "C" CharMoveLock DAT_10db9380[0x80];
extern "C" BlockState DAT_1020b430[0x80];
extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" int DAT_10db0544;
extern "C" int DAT_10db0548;
extern "C" int DAT_1049f0f0;                     // number of entries in the two lists below
extern "C" Vec3 *DAT_1032d5b8[];                 // positions of the characters moved by routes
extern "C" char DAT_1021bbc0[];

// 0x1002a490 (Block_Update). One frame of block `index`. On the first call it is listed with the moved characters,
// placed 5000 units up and node state 0x41 is set (masks 0x200 and 0x400). While waiting it drops to z -2000 once
// DAT_10db0544 is below half of DAT_10db0548. Below z -120 it rises 5 units a frame; above it pushes the player
// away from its centre: while the player stands on something and the reversed player move is clear, the player
// is lifted onto the block (block z + 12) and moved 3 units outwards, otherwise, when the block is blocked, it is
// moved 10 units towards the block's centre. The position is remembered for the next frame. Returns 0.
extern "C" int FUN_1002a490(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        // the camera list holds Vec3 pointers; the matrix's translation row (_41, _42, _43) is the position
        DAT_1032d5b8[DAT_1049f0f0] = (Vec3 *)&DAT_10db6ed8[index].matrix._41;
        DAT_1021bbc0[DAT_1049f0f0] = 0;
        DAT_1049f0f0++;
        DAT_1020b430[index].state = 1;
        DAT_10db6ed8[index].model.SetPosition(DAT_10db6ed8[index].matrix._41, DAT_10db6ed8[index].matrix._42,
                                              5000.0f);
        DAT_10db6ed8[index].root->M_10194730(0x41, 0x200);
        DAT_10db6ed8[index].root->M_10194730(0x41, 0x400);
    }
    else if (DAT_1020b430[index].state == 1)
    {
        if (DAT_10db0544 < DAT_10db0548 / 2)
        {
            DAT_1020b430[index].state = 2;
            DAT_10db6ed8[index].model.SetPosition(DAT_10db6ed8[index].matrix._41, DAT_10db6ed8[index].matrix._42,
                                                  -2000.0f);
        }
    }
    else if (DAT_10db6ed8[index].matrix._43 < -120.0f)
        DAT_10db6ed8[index].matrix._43 += 5.0f;
    else
    {
        Vec4 player;
        Vec4 block;
        Vec4 away;
        player.Copy(DAT_10db6ed8[0].matrix.GetTranslation());
        block.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
        away.Copy(block - player);
        if (away.Length2D() > 1.0f)
            away.Normalized3();
        else
            away.x = 1.0f;
        if (DAT_10db9380[0].locked == 2 &&
            DAT_10db6ed8[index].model.IsClearAny(&(DAT_10db6ed8[0].move * -1.0f)) == 0 &&
            DAT_10db9380[0].locked == 2 && DAT_10db6ed8[0].matrix._43 > -140.0f)
        {
            // standing on the block: lifted onto it and moved outwards
            DAT_10db6ed8[0].matrix._43 = block.z + 12.0f;
            DAT_10db6ed8[0].matrix._41 = away.x * 3.0f + DAT_10db6ed8[0].matrix._41;
            DAT_10db6ed8[0].matrix._42 = away.y * 3.0f + DAT_10db6ed8[0].matrix._42;
        }
        else if (DAT_10db6ed8[index].model.IsClearAny(0) == 0)
        {
            DAT_10db6ed8[0].matrix._41 = DAT_10db6ed8[0].matrix._41 - away.x * 10.0f;
            DAT_10db6ed8[0].matrix._42 = DAT_10db6ed8[0].matrix._42 - away.y * 10.0f;
        }
    }
    DAT_10802bf8[index].position.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
    return 0;
}
