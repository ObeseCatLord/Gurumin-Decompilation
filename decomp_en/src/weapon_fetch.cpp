// Enemies that lost a weapon (EN build). Ported from the matched JP source decomp/src/weapon_fetch.cpp: the behaviour of a
// character whose hand weapon was knocked away. It walks to the dropped weapon, which lies at a node of a slot's
// attached-node row, and picks it up again.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_shots.h"
#include "collide_query.h"
#include "char_move.h"
#include "engine.h"
#include "node.h"
#include "weapon_attach.h"
#include "weapon_fetch.h"

// Enemy_FetchDroppedWeapon: character `index` has lost the weapon in one hand: a mark word holds weapon number * 1000 +
// the slot whose node row carries the dropped weapon (-1 = none, -2 = unknown weapon). The left hand is handled first,
// else the right. It turns towards the weapon and steps 12 units a frame towards it with a lift of 24, going round
// obstacles by turning 95 degrees to one side (the side is picked at random and kept while it stays blocked); after 120
// blocked frames, when the ground ahead is not walkable or when the weapon's node is missing, it gives up and picks its
// kind again. Within 100 units of the weapon it attaches it again, adds the weapon number to its script code and clears
// the marks. Plays motion 2 (to the weapon) or 1 (walking) at the end. Returns 0. Called by the enemy behaviour for
// characters with a lost weapon mark.
extern "C" int Enemy_FetchDroppedWeapon(int index)
{
    Vec4 target;
    Vec4 self;
    int motion = -1;
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    int noGround = 0;
    if (Slot_HasNoGround(index, &self) != 0)
        noGround = 1;
    float savedHeading = g_characterModels[index].heading;
    if (g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] != WEAPON_MARK_NONE && g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] > -1)
    {
        Node *weapon = g_charNodes[index][FETCH_NODE_LEFT];
        if (weapon != 0)
        {
            motion = 2;
            target.Copy(weapon->worldMatrix.GetTranslation());
            float dy = target.y - self.y;
            float dx = target.x - self.x;
            float turn = (float)atan2((double)dx, (double)dy) + PI_APPROX;
            g_characterModels[index].targetHeading = turn;
            g_characterModels[index].heading = turn;
            if (noGround != 0)
            {
                Char_SelectKind(index, 0, 1);
            }
            else
            {
                if (Char_ProbeAhead(index, 0, (target - self).Length2D(), 1, 1, 0) != 0 ||
                    g_charNodes[g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] % WEAPON_SLOT_RANGE] == 0)
                {
                    Char_SelectKind(index, 0, 1);
                }
                else
                {
                    Vec4 step;
                    step.y = FETCH_STEP;
                    step.RotateZ(-g_characterModels[index].targetHeading);
                    step.z += FETCH_LIFT;
                    AnimModel *model = &g_characterModels[index];
                    if (model->IsClearOfMapLimited(&step) != 0)
                    {
                        step.z = 0;
                        Character_SetMove(index, step.x, step.y, step.z);
                        g_shotSlots[index].position.Copy(step);
                        g_shotSlots[index].position.z = 0;
                    }
                    else
                    {
                        Char_SelectKind(index, 0, 1);
                    }
                    motion = 1;
                    if (model->IsClearAny(&step) != 0)
                    {
                        int blocked = g_charState[index].values[CHARSTATE_FETCH_BLOCKED];
                        if (blocked > 0)
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked - 1;
                        else if (blocked < 0)
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked + 1;
                    }
                    else
                    {
                        int blocked = g_charState[index].values[CHARSTATE_FETCH_BLOCKED];
                        if (blocked > 0)
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked + 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else if (blocked < 0)
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked - 1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        else if (Random() % 2 != 0)
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = -1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        Character_SetMove(index, step.x, step.y, step.z);
                        g_shotSlots[index].position.Copy(step);
                        g_shotSlots[index].position.z = 0;
                        if (g_charState[index].values[CHARSTATE_FETCH_BLOCKED] * g_charState[index].values[CHARSTATE_FETCH_BLOCKED] > FETCH_GIVE_UP_SQUARED)
                        {
                            Char_SelectKind(index, 0, 1);
                        }
                    }
                }
            }
            if ((target - self).LengthSquared3() < FETCH_PICKUP_SQUARED)
            {
                int code = g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT];
                Char_AttachWeapon(index, (code / 1000) % 100, 0, 1, code % 1000);
                g_charSetupCode[index] += (g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] / 1000 % 100) * WEAPON_CODE_UNIT;
                g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] = -1;
                if (g_characterModels[index].root->Find("@migitesou", 0) != 0)
                    g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] = -1;
                if (g_characterModels[index].root->Find("@hidaritesou", 0) != 0)
                    g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] = -1;
            }
        }
        else
        {
            Char_SelectKind(index, 0, 1);
        }
    }
    else if (g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] != WEAPON_MARK_NONE)
    {
        Node *weapon = g_charNodes[index][FETCH_NODE_RIGHT];
        if (weapon != 0)
        {
            motion = 2;
            target.Copy(weapon->worldMatrix.GetTranslation());
            float dy = target.y - self.y;
            float dx = target.x - self.x;
            float turn = (float)atan2((double)dx, (double)dy) + PI_APPROX;
            g_characterModels[index].targetHeading = turn;
            g_characterModels[index].heading = turn;
            if (noGround != 0)
            {
                Char_DropEquipment(index, 0, 1);
                Char_SelectKind(index, 0, 1);
            }
            else
            {
                if (Char_ProbeAhead(index, 0, (target - self).Length2D(), 1, 1, 0) != 0 ||
                    g_charNodes[g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] % WEAPON_SLOT_RANGE] == 0)
                {
                    Char_DropEquipment(index, 0, 1);
                    Char_SelectKind(index, 0, 1);
                }
                else
                {
                    Vec4 step;
                    step.y = FETCH_STEP;
                    step.RotateZ(-g_characterModels[index].targetHeading);
                    step.z = FETCH_LIFT;
                    AnimModel *model = &g_characterModels[index];
                    if (model->IsClearOfMapLimited(&step) != 0)
                    {
                        step.z = 0;
                        Character_SetMove(index, step.x, step.y, step.z);
                        g_shotSlots[index].position.Copy(step);
                        g_shotSlots[index].position.z = 0;
                    }
                    else
                    {
                        Char_DropEquipment(index, 0, 1);
                        Char_SelectKind(index, 0, 1);
                    }
                    motion = 1;
                    if (model->IsClearAny(&step) != 0)
                    {
                        int blocked = g_charState[index].values[CHARSTATE_FETCH_BLOCKED];
                        if (blocked > 0)
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked - 1;
                        else if (blocked < 0)
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked + 1;
                    }
                    else
                    {
                        int blocked = g_charState[index].values[CHARSTATE_FETCH_BLOCKED];
                        if (blocked > 0)
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked + 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else if (blocked < 0)
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = blocked - 1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        else if (Random() % 2 != 0)
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = 1;
                            step.RotateZ(FETCH_SIDE_ANGLE);
                        }
                        else
                        {
                            g_charState[index].values[CHARSTATE_FETCH_BLOCKED] = -1;
                            step.RotateZ(-FETCH_SIDE_ANGLE);
                        }
                        Character_SetMove(index, step.x, step.y, step.z);
                        g_shotSlots[index].position.Copy(step);
                        g_shotSlots[index].position.z = 0;
                        if (g_charState[index].values[CHARSTATE_FETCH_BLOCKED] * g_charState[index].values[CHARSTATE_FETCH_BLOCKED] > FETCH_GIVE_UP_SQUARED)
                        {
                            Char_DropEquipment(index, 0, 1);
                            Char_SelectKind(index, 0, 1);
                        }
                    }
                }
            }
            if ((target - self).LengthSquared3() < FETCH_PICKUP_SQUARED)
            {
                int code = g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT];
                Char_AttachWeapon(index, (code / 1000) % 100, 1, 0, code % 1000);
                if (g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] != -1)
                    g_charSetupCode[index] += (g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] / 1000 % 100) * WEAPON_CODE_UNIT;
                Char_SelectKind(index, g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] == WEAPON_MARK_NONE, 0);
                g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] = -1;
                if (g_characterModels[index].root->Find("@migitesou", 0) != 0)
                    g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] = -1;
                if (g_characterModels[index].root->Find("@hidaritesou", 0) != 0)
                    g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] = -1;
            }
        }
        else
        {
            Char_DropEquipment(index, 0, 1);
            Char_SelectKind(index, 0, 1);
        }
    }
    else
    {
        Char_SelectKind(index, 0, 1);
    }
    g_characterModels[index].heading = savedHeading;
    if (motion > -1)
        g_characterModels[index].PlayMotion(motion, 0, 5, 1.0f);
    return 0;
}
