// Enemies that lost a weapon: behaviour of a character whose hand weapon was knocked away (see weapon_attach.cpp). It
// walks to the dropped weapon, which lies at a node of a slot's attached-node row, and picks it up again.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "char_state.h"

#include "char_tables.h"

#include "last_position.h"

#include "char_vars.h"

#include "strafe_enemy.h"

#include "weapon_fetch.h"

extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_1001a450(int slot, const Vec4 *point);                   // Slot_HasNoGround
extern "C" int FUN_100242c0(int index, Vec4 *ground, float depth, int fromHead, int testOnly, const Vec4 *offset);
extern "C" int FUN_100241e0(int index, float x, float y, float z);          // Character_SetMove
extern "C" int FUN_1001b0a0(int index, int alternative, int quiet);         // Char_SelectKind
#include "char_equipment.h"
extern "C" int FUN_1001a990(int index, int weapon, int right, int left, int slot);  // Char_AttachWeapon

// 0x100a8ac0 (Enemy_FetchDroppedWeapon). Character `index` has lost the weapon in one hand: a mark word holds
// weapon number * 1000 + the slot whose node row carries the dropped weapon (-1 = none, -2 = unknown weapon). The
// left hand is handled first (node [1] of the row), else the right (node [2]). It turns towards the weapon and steps
// 12 units a frame towards it with a lift of 24, going round obstacles by turning 95 degrees to one side (the side is
// picked at random and kept while it stays blocked); after 120 blocked frames, when the ground ahead is not
// walkable or when the weapon's node is missing, it gives up and picks its kind again. Within 100 units of the weapon
// it attaches it again, adds the weapon number to its script code and clears the marks. Plays motion 2 (to the
// weapon) or 1 (walking) at the end. Returns 0.
extern "C" int FUN_100a8ac0(int index)
{
    Vec4 target;
    Vec4 self;
    int motion = -1;
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    int noGround = 0;
    if (FUN_1001a450(index, &self) != 0)
        noGround = 1;
    float savedHeading = DAT_10db6ee8[index].heading;
    if (DAT_1020b430[index].weaponMarkB != WEAPON_MARK_UNKNOWN && DAT_1020b430[index].weaponMarkB > -1)
    {
        Node *weapon = DAT_1021bdf8[index][1];
        if (weapon != 0)
        {
            motion = 2;
            target.Copy(weapon->worldMatrix.GetTranslation());
            float dy = target.y - self.y;
            float dx = target.x - self.x;
            float turn = (float)atan2((double)dx, (double)dy) + PI_APPROX;
            DAT_10db6ee8[index].targetHeading = turn;
            DAT_10db6ee8[index].heading = turn;
            if (noGround != 0)
            {
                FUN_1001b0a0(index, 0, 1);
            }
            else
            {
                if (FUN_100242c0(index, 0, (target - self).Length2D(), 1, 1, 0) != 0 ||
                    DAT_1021bdf8[DAT_1020b430[index].weaponMarkB % WEAPON_SLOT_RANGE] == 0)
                {
                    FUN_1001b0a0(index, 0, 1);
                }
                else
                {
                    Vec4 step;
                    step.y = FETCH_STEP;
                    step.RotateZ(-DAT_10db6ee8[index].targetHeading);
                    step.z += FETCH_LIFT;
                    AnimModel *model = &DAT_10db6ee8[index];
                    if (model->IsClearOfMapLimited(&step) != 0)
                    {
                        step.z = 0;
                        FUN_100241e0(index, step.x, step.y, step.z);
                        DAT_10802bf8[index].position.Copy(step);
                        DAT_10802bf8[index].position.z = 0;
                    }
                    else
                    {
                        FUN_1001b0a0(index, 0, 1);
                    }
                    motion = 1;
                    if (model->IsClearAny(&step) != 0)
                    {
                        int blocked = DAT_1020b430[index].wait;
                        if (blocked > 0)
                            DAT_1020b430[index].wait = blocked - 1;
                        else if (blocked < 0)
                            DAT_1020b430[index].wait = blocked + 1;
                    }
                    else
                    {
                        int blocked = DAT_1020b430[index].wait;
                        if (blocked > 0)
                        {
                            DAT_1020b430[index].wait = blocked + 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else if (blocked < 0)
                        {
                            DAT_1020b430[index].wait = blocked - 1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        else if (FUN_1013f880() % 2 != 0)
                        {
                            DAT_1020b430[index].wait = 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else
                        {
                            DAT_1020b430[index].wait = -1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        FUN_100241e0(index, step.x, step.y, step.z);
                        DAT_10802bf8[index].position.Copy(step);
                        DAT_10802bf8[index].position.z = 0;
                        if (DAT_1020b430[index].wait * DAT_1020b430[index].wait > FETCH_GIVE_UP_SQUARED)
                        {
                            FUN_1001b0a0(index, 0, 1);
                        }
                    }
                }
            }
            if ((target - self).LengthSquared3() < FETCH_PICKUP_SQUARED)
            {
                int code = DAT_1020b430[index].weaponMarkB;
                FUN_1001a990(index, (code / 1000) % 100, 0, 1, code % 1000);
                DAT_102548c0[index] += (DAT_1020b430[index].weaponMarkB / 1000 % 100) * WEAPON_CODE_UNIT;
                DAT_1020b430[index].weaponMarkB = -1;
                if (DAT_10db6ee8[index].root->Find("@migitesou", 0) != 0)
                    DAT_1020b430[index].weaponMarkA = -1;
                if (DAT_10db6ee8[index].root->Find("@hidaritesou", 0) != 0)
                    DAT_1020b430[index].weaponMarkB = -1;
            }
        }
        else
        {
            FUN_1001b0a0(index, 0, 1);
        }
    }
    else if (DAT_1020b430[index].weaponMarkA != WEAPON_MARK_UNKNOWN)
    {
        Node *weapon = DAT_1021bdf8[index][2];
        if (weapon != 0)
        {
            motion = 2;
            target.Copy(weapon->worldMatrix.GetTranslation());
            float dy = target.y - self.y;
            float dx = target.x - self.x;
            float turn = (float)atan2((double)dx, (double)dy) + PI_APPROX;
            DAT_10db6ee8[index].targetHeading = turn;
            DAT_10db6ee8[index].heading = turn;
            if (noGround != 0)
            {
                FUN_100615f0(index, 0, 1);
                FUN_1001b0a0(index, 0, 1);
            }
            else
            {
                if (FUN_100242c0(index, 0, (target - self).Length2D(), 1, 1, 0) != 0 ||
                    DAT_1021bdf8[DAT_1020b430[index].weaponMarkA % WEAPON_SLOT_RANGE] == 0)
                {
                    FUN_100615f0(index, 0, 1);
                    FUN_1001b0a0(index, 0, 1);
                }
                else
                {
                    Vec4 step;
                    step.y = FETCH_STEP;
                    step.RotateZ(-DAT_10db6ee8[index].targetHeading);
                    step.z = FETCH_LIFT;
                    AnimModel *model = &DAT_10db6ee8[index];
                    if (model->IsClearOfMapLimited(&step) != 0)
                    {
                        step.z = 0;
                        FUN_100241e0(index, step.x, step.y, step.z);
                        DAT_10802bf8[index].position.Copy(step);
                        DAT_10802bf8[index].position.z = 0;
                    }
                    else
                    {
                        FUN_100615f0(index, 0, 1);
                        FUN_1001b0a0(index, 0, 1);
                    }
                    motion = 1;
                    if (model->IsClearAny(&step) != 0)
                    {
                        int blocked = DAT_1020b430[index].wait;
                        if (blocked > 0)
                            DAT_1020b430[index].wait = blocked - 1;
                        else if (blocked < 0)
                            DAT_1020b430[index].wait = blocked + 1;
                    }
                    else
                    {
                        int blocked = DAT_1020b430[index].wait;
                        if (blocked > 0)
                        {
                            DAT_1020b430[index].wait = blocked + 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else if (blocked < 0)
                        {
                            DAT_1020b430[index].wait = blocked - 1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        else if (FUN_1013f880() % 2 != 0)
                        {
                            DAT_1020b430[index].wait = 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else
                        {
                            DAT_1020b430[index].wait = -1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        FUN_100241e0(index, step.x, step.y, step.z);
                        DAT_10802bf8[index].position.Copy(step);
                        DAT_10802bf8[index].position.z = 0;
                        if (DAT_1020b430[index].wait * DAT_1020b430[index].wait > FETCH_GIVE_UP_SQUARED)
                        {
                            FUN_100615f0(index, 0, 1);
                            FUN_1001b0a0(index, 0, 1);
                        }
                    }
                }
            }
            if ((target - self).LengthSquared3() < FETCH_PICKUP_SQUARED)
            {
                int code = DAT_1020b430[index].weaponMarkA;
                FUN_1001a990(index, (code / 1000) % 100, 1, 0, code % 1000);
                if (DAT_1020b430[index].weaponMarkB != -1)
                    DAT_102548c0[index] += (DAT_1020b430[index].weaponMarkA / 1000 % 100) * WEAPON_CODE_UNIT;
                FUN_1001b0a0(index, DAT_1020b430[index].weaponMarkB == WEAPON_MARK_UNKNOWN, 0);
                DAT_1020b430[index].weaponMarkA = -1;
                if (DAT_10db6ee8[index].root->Find("@migitesou", 0) != 0)
                    DAT_1020b430[index].weaponMarkA = -1;
                if (DAT_10db6ee8[index].root->Find("@hidaritesou", 0) != 0)
                    DAT_1020b430[index].weaponMarkB = -1;
            }
        }
        else
        {
            FUN_100615f0(index, 0, 1);
            FUN_1001b0a0(index, 0, 1);
        }
    }
    else
    {
        FUN_1001b0a0(index, 0, 1);
    }
    DAT_10db6ee8[index].heading = savedHeading;
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, 0, 5, 1.0f);
    return 0;
}
