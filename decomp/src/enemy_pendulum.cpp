// A swinging enemy (0x10044560, called from the behaviour dispatch 0x100dfcf0): it drops from 500 units above its
// place to 100 above it, then swings on a 400 unit line from there like a pendulum until it sees the player.

#include "vec4.h"

#include "matrix4.h"

Matrix4 RotationY(float angle);                  // 0x101b1bb0

#include "node.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = start, 1 = dropping, 2 = swinging, 3 = landed
    int wait;                 // 0x04: frames in the current state
    int talk;                 // 0x08: hundreds = waits to be told, units = swing speed limit
    int angle;                // 0x0c: swing angle in tenths of a degree
    int backwards;            // 0x10: 1 while swinging back
    int swing;                // 0x14: angle change per frame
    int fall;                 // 0x18: drop speed (more negative each frame)
    char pad_001c[0x24];
};

// The last position and step of each character (route_walk.cpp StepRecord); here the position holds the pivot.
struct LastPosition
{
    Vec4 position;
    Vec4 step;
    char pad_0020[0x20];
};

// A character slot record (world.h CharSlot), only the flag used here.
struct CharSlot
{
    char pad_0000[0xa5];
    char m_00a5;              // 0xa5: cleared while it swings
    char pad_00a6[0xe2];
};

struct CollisionWorld
{
    int Collide(int name, int mask, Vec4 *from, Vec4 *to, Vec4 *hit);  // Node::Collide 0x101a8e60
};

extern "C" NpcState DAT_1020b430[0x80];
extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" CharSlot DAT_10daab84[0x80];
extern "C" int DAT_10210b30[0x80];            // non-zero = told to react
extern "C" char DAT_10255a88[0x80];
extern "C" CollisionWorld *DAT_10255f08;
extern "C" unsigned short DAT_104c0380;       // frame counter
extern "C" int DAT_1032d69c;                  // 1 once the game built-ins are registered
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove
extern "C" int FUN_10043cc0(int index);       // Char_ResetState

// 0x10044560 (Enemy_Pendulum). Fades in (over 30 frames) while dropping from 500 units above its place to 100
// above it, then swings on a 400 unit line about that pivot, turned with its heading: the speed grows by 2 a frame
// up to the talk id's units, the model's root tilts with the line. When the player comes within 800 units and is
// in sight (checked every ten frames) it lets go (state 3) and its state is reset once its motion has settled.
// Being told to react skips straight to that unless the talk id has hundreds. Returns 0.
extern "C" int FUN_10044560(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    int visible = 0;
    int frames;
    if (DAT_1020b430[index].talk / 100 != 0)
        DAT_10255a88[index] = 0;
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        if (DAT_1020b430[index].talk / 100 == 0)
        {
            DAT_1020b430[index].state = 3;
            DAT_1020b430[index].wait = 0;
        }
    }
    if (DAT_104c0380 % 10 == index % 10)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z += 50.0f;
        to.Copy(player);
        to.z += 50.0f;
        if (DAT_10255f08 == 0 || DAT_10255f08->Collide(0, 4, &from, &to, &hit) == 0)
            visible = 1;
    }
    if (DAT_1020b430[index].state == 0)
    {
        DAT_10db6ee8[index].PlayMotion(3, 0, 5, 1.0f);
        DAT_10daab84[index].m_00a5 = 0;
        DAT_10802bf8[index].position.Copy(self);
        DAT_10802bf8[index].position.z += 100.0f;
        DAT_10db6ee8[index].SetPosition(self.x, self.y, self.z + 500.0f);
        DAT_1020b430[index].state = 1;
        DAT_10db6ee8[index].alpha = 0;
        DAT_10db6ee8[index].mode = 0;
        return 0;
    }
    if (DAT_1020b430[index].state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(3, 0, 5, 1.0f);
        frames = ++DAT_1020b430[index].wait;
        DAT_1020b430[index].fall -= 3;
        DAT_10db6ee8[index].mode = 0;
        if (frames < 30)
            DAT_10db6ee8[index].alpha = (float)frames / 30.0f;
        else if (frames == 30)
            DAT_10db6ee8[index].alpha = 1.0f;
        if ((DAT_10802bf8[index].position - self).LengthSquared3() < 400.0f)
        {
            DAT_10db6ee8[index].mode = 0;
            DAT_1020b430[index].state = 2;
            DAT_1020b430[index].swing = DAT_1020b430[index].talk % 100;
            DAT_1020b430[index].fall = -(DAT_1020b430[index].fall / 2);
            return 0;
        }
        FUN_100241e0(index, 0, 0, (float)DAT_1020b430[index].fall);
        return 0;
    }
    if (DAT_1020b430[index].state == 2)
    {
        DAT_10db6ee8[index].PlayMotion(3, 0, 5, 1.0f);
        frames = ++DAT_1020b430[index].wait;
        if (frames < 30)
            DAT_10db6ee8[index].alpha = (float)frames / 30.0f;
        else if (frames == 30)
            DAT_10db6ee8[index].alpha = 1.0f;
        DAT_1020b430[index].angle += DAT_1020b430[index].swing;
        if (DAT_1020b430[index].backwards == 0)
        {
            DAT_1020b430[index].swing -= 2;
            if (DAT_1020b430[index].swing < -(DAT_1020b430[index].talk % 100))
                DAT_1020b430[index].swing = -(DAT_1020b430[index].talk % 100);
        }
        else
        {
            DAT_1020b430[index].swing += 2;
            if (DAT_1020b430[index].swing > DAT_1020b430[index].talk % 100)
                DAT_1020b430[index].swing = DAT_1020b430[index].talk % 100;
        }
        if (DAT_1020b430[index].angle <= 0 && DAT_1020b430[index].swing < 0)
            DAT_1020b430[index].backwards = 1;
        else if (DAT_1020b430[index].angle > 0 && DAT_1020b430[index].swing >= 0)
            DAT_1020b430[index].backwards = 0;
        DAT_10db6ee8[index].root->matrix = RotationY((float)DAT_1020b430[index].angle * -3.1415f / 1800.0f);
        Vec4 axis;
        Vec4 line;
        axis.y = -1.0f;
        axis.RotateZ(-DAT_10db6ee8[index].heading);
        line.z = -400.0f;
        line.RotateAxis(axis, (float)DAT_1020b430[index].angle * 3.1415f / 1800.0f);
        axis.Copy(DAT_10802bf8[index].position + line);
        DAT_10db6ee8[index].SetPosition(axis.x, axis.y, axis.z + 400.0f);
        if (distance < 800.0f && visible != 0 && DAT_1032d69c != 0)
        {
            DAT_1020b430[index].state = 3;
            DAT_1020b430[index].wait = 0;
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_10db6ee8[index].mode = 1;
            DAT_10db6ee8[index].m_238c = 0;
            return 0;
        }
    }
    else if (DAT_1020b430[index].state == 3)
    {
        DAT_10db6ee8[index].root->matrix.SetIdentity();
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].mode = 1;
        if (DAT_10db6ee8[index].m_238c != 0)
            FUN_10043cc0(index);
    }
    return 0;
}
