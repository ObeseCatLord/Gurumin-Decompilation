// A character pushing a wheeled cart back and forth (0x10047440, behaviour 0x1e of Char_RunBehaviour 0x100dfcf0):
// the cart's "EEQ_25" model part turns with the distance rolled.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

Matrix4 RotationX(float angle);                  // 0x101b1ad0

#include "node.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

// A character slot record (world.h CharSlot), only the kind is used here.
struct CharSlot
{
    int kind;                 // 0x00: behaviour kind
    char pad_0004[0x184];
};

extern "C" CharSlot DAT_10daab84[0x80];
extern "C" Node *DAT_1021bdf8[0x80][9];       // [0]: the cart's wheel node
extern "C" Matrix4 DAT_10868838[0x80][6];     // saved node matrices of each character
extern "C" float DAT_10255160[0x80][4];       // [0]: the wheel's angle
extern "C" int DAT_10210b30[0x80];            // non-zero = told to react
extern "C" int DAT_104bf8f8[0x7c];            // key prompt shown (app_0008.cpp)
extern "C" int DAT_1021d138[0x7c];            // key prompt id
extern "C" int DAT_1021f740[0x7c];            // key prompt frames left
extern "C" unsigned short DAT_104c0380;       // frame counter
extern "C" int FUN_10043e20(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange,
                            float nearRange, int checkWalls);           // Char_CanSee
extern "C" int FUN_1001b670(int index, float range, int count, ...);   // Enemies_WakeNearby
extern "C" int FUN_100241e0(int index, float x, float y, float z);     // Character_SetMove
extern "C" int FUN_10043cc0(int index);                                 // Char_ResetState
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10047440 (Npc_PushCart). Finds the cart's wheel node ("EEQ_25" or "EEQ_25b"; with neither its state is
// reset) and saves its matrix. Then it pushes the cart forwards (motion 2) with a growing push while the way
// ahead is clear, easing off when blocked; at speed over 5 it pulls back until the speed drops below 0, kicking up
// dust (effect 0x2df) every other frame. The wheel turns with the backwards speed. When it sees the player (within
// the talk id's units) it wakes the enemies within 1500 and stops checking; when told to react it shows prompt 15,
// turns to the player and its state is reset (kinds 0x44 and 0x59 keep the wheel angle). Returns 0.
int FUN_10047440(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    int *react = &DAT_10210b30[index];
    if (*react == 0 && (DAT_1020b430[index].state != 1 || DAT_1020b430[index].sawPlayer == 0))
    {
        if ((DAT_1020b430[index].sawPlayer |= FUN_10043e20(&self, &player, DAT_10db6ee8[index].heading,
                                                         (float)DAT_1020b430[index].talk, 1.099525f, 250.0f, 1)) != 0)
            FUN_1001b670(index, 1500.0f, 0);
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (DAT_1020b430[index].state == 0)
        {
            if ((DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find("@EEQ_25:Layer1", 0)) == 0 &&
                (DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find("@EEQ_25b:Layer1", 0)) == 0)
            {
                FUN_10043cc0(index);
                return 0;
            }
            DAT_10868838[index][0] = DAT_1021bdf8[index][0]->matrix;
            DAT_1020b430[index].state = 1;
        }
        else if (DAT_1020b430[index].state == 1)
        {
            Vec4 move;
            Vec4 ahead;
            ahead.y = -20.0f;
            move.y = (float)DAT_1020b430[index].push / 4.0f - 20.0f;
            float speed = move.y;
            if (move.y > 5.0f)
                DAT_1020b430[index].state = 2;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            ahead.RotateZ(-DAT_10db6ee8[index].heading);
            if (DAT_10db6ee8[index].IsClearOfMapLimited(&ahead) == 0)
                DAT_1020b430[index].push++;
            else
            {
                DAT_1020b430[index].push -= 3;
                if (DAT_1020b430[index].push < 0)
                    DAT_1020b430[index].push = 0;
            }
            FUN_100241e0(index, move.x, move.y, 0);
            if (speed < 0.0f)
            {
                float *angle = &DAT_10255160[index][0];
                speed = speed * -0.01f + *angle;
                *angle = speed;
                if (speed > 6.283f)
                    *angle = speed - 6.283f;
                DAT_1021bdf8[index][0]->matrix = RotationX(-*angle);
                DAT_1021bdf8[index][0]->matrix.SetTranslation(DAT_10868838[index][0].GetTranslation());
            }
        }
        else if (DAT_1020b430[index].state == 2)
        {
            Vec4 move;
            Vec4 ahead;
            ahead.y = 20.0f;
            move.y = (float)DAT_1020b430[index].push / 4.0f - 20.0f;
            if (move.y < 0.0f)
                DAT_1020b430[index].state = 1;
            ahead.z = -5.0f;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            ahead.RotateZ(-DAT_10db6ee8[index].heading);
            if (DAT_10db6ee8[index].IsClearOfMapLimited(&ahead) == 0)
                DAT_1020b430[index].push -= 2;
            else
                DAT_1020b430[index].push++;
            if (DAT_104c0380 % 2 != 0)
            {
                Vec4 offset;
                Matrix4 matrix;
                matrix.SetIdentity();
                matrix.SetTranslation(self);
                FUN_1001d8b0(index, 0x2df, offset, -1, -1, 0, 0, 0, &matrix, 0);
            }
            FUN_100241e0(index, move.x, move.y, 0);
        }
    }
    else
    {
        float *angle = &DAT_10255160[index][0];
        float saved = *angle;
        if (DAT_104bf8f8[index] == 0 || DAT_1021d138[index] != 15)
        {
            DAT_104bf8f8[index] = 1;
            DAT_1021d138[index] = 15;
        }
        DAT_1021f740[index] = 60;
        DAT_10db6ee8[index].targetHeading = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
        FUN_10043cc0(index);
        int kind = DAT_10daab84[index].kind;
        if (kind == 0x44 || kind == 0x59)
            *angle = saved;
        *react = 0;
    }
    return 0;
}
