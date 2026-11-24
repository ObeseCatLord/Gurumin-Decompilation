// The four drill claws that follow the player at drill level 4 (see drill_claws.h).

#include <math.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "effect_manager.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_setup.h"
#include "map_placement.h"
#include "action_select_vars.h"
#include "hop_spit_enemy.h"
#include "drill_claws.h"

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

// The map areas where the claws may start their strike: (area, phase) = (10, 5) or (0, 3).
#define CLAW_AREA_ALT 10
#define CLAW_AREA_ALT_PHASE 5
#define CLAW_AREA_MAIN 0
#define CLAW_AREA_MAIN_PHASE 3
#define CLAW_IN_AREA ((DAT_104c00e8 == CLAW_AREA_ALT && DAT_104c00f4 == CLAW_AREA_ALT_PHASE) || (DAT_104c00e8 == CLAW_AREA_MAIN && DAT_104c00f4 == CLAW_AREA_MAIN_PHASE))

__forceinline void HideClaws(void)
{
    DAT_102559f4[0]->SetFlags(CLAW_HIDDEN_FLAG);
    DAT_102559f4[1]->SetFlags(CLAW_HIDDEN_FLAG);
    DAT_102559f4[2]->SetFlags(CLAW_HIDDEN_FLAG);
    DAT_102559f4[3]->SetFlags(CLAW_HIDDEN_FLAG);
}

// 0x1008fea0 (DrillClaws_Update, a C++ function: it has an EH frame for its matrices). Runs every frame while the
// four "@tume01" claws exist. Level 3 with a gauge of 400 is the strike phase: the claws leave the player's claw node,
// pick the nearest enemy (or the one the player aims at), circle it at a radius of 200 and wander upwards around it,
// each launched a few ticks after the one before. Out of reach, or with the target dead, they fly back and dock on the
// player's claw node again, which is shown once all four are there. At a gauge of 300 (400 with the bank) in the right
// map area an aura effect is started. Returns 0.
int DrillClaws_Update(void)
{
    if (DAT_102559f4[0] == 0)
        return 0;
    if (DAT_102559f4[1] == 0)
        return 0;
    if (DAT_102559f4[2] == 0)
        return 0;
    if (DAT_102559f4[3] == 0)
        return 0;
    int level = DAT_10c64ad4;
    int gauge = DAT_10c64ad8;
    if (DAT_10c655f0 != CLAW_COSTUME_POPON)
    {
        if (DAT_10c64ad0 == 0 && DAT_10c64f10 == 0 && gauge >= CLAW_GAUGE_AURA && DAT_104c00e0 != 0 && CLAW_IN_AREA)
            StartClawEffect(CLAW_EFFECT_AURA, &DAT_10db6ee8[0].matrix);
    }
    else if (DAT_10c64ad0 == 0)
    {
        if (DAT_10c64f10 == 0 && gauge >= CLAW_GAUGE_AURA && DAT_104c00e0 != 0 && CLAW_IN_AREA)
            StartClawEffect(CLAW_EFFECT_AURA_POPON, &DAT_10db6ee8[0].matrix);
    }
    else if (DAT_10c64f10 == 0 && gauge >= CLAW_GAUGE_STRIKE && DAT_104c00e0 != 0 && CLAW_IN_AREA)
        StartClawEffect(CLAW_EFFECT_AURA_POPON, &DAT_10db6ee8[0].matrix);
    if (DAT_104bfe6c != 0 || DAT_10c655f0 == CLAW_COSTUME_POPON || DAT_10c62ed0 == CLAW_MAP_NO_CLAWS)
    {
        HideClaws();
        return 0;
    }
    int striking = 0;
    if (level != CLAW_LEVEL)
        HideClaws();
    else if (gauge == CLAW_GAUGE_STRIKE)
        striking = 1;
    if (DAT_10c64f10 == 0 && striking != 0 && DAT_104c00e0 != 0 &&
        CLAW_IN_AREA)
    {
        if (DAT_104c0144 == 0 && DAT_1020aeec[3] + DAT_1020aeec[2] + DAT_1020aeec[1] + DAT_1020aeec[0] == 0)
        {
            Vec4 rally;
            rally.x = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].heading - 1.57075f) * CLAW_FORWARD_DISTANCE;
            rally.y = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].heading - 1.57075f) * CLAW_FORWARD_DISTANCE;
            rally.z = CLAW_RALLY_HEIGHT;
            DAT_10f08d0c.Copy(DAT_10db6ee8[DAT_104bf8b4].matrix.GetTranslation() + rally);
            FUN_10065b10(0, CLAW_RECALL_SOUND, 0, 0);
            DAT_104c0144 = CLAW_TARGET_FREE;
            DAT_10256210 = 0;
            DAT_1032d678 = 0;
            DAT_1020aeec[0] = 0;
            DAT_1020aeec[1] = 0;
            DAT_1020aeec[2] = 0;
            DAT_1020aeec[3] = 0;
        }
    }
    if (striking != 0 && DAT_102024ac > 0)
    {
        if (DAT_104c0144 == CLAW_TARGET_FREE || DAT_104c0144 == CLAW_TARGET_SETTLED ||
            (DAT_104c0144 != CLAW_TARGET_NONE && DAT_104c0144 != DAT_102024ac && DAT_10256210 > CLAW_RETARGET_TICKS))
        {
            Vec4 anchor;
            DAT_104c0144 = DAT_102024ac;
            anchor.Copy(DAT_10db6ee8[DAT_102024ac].matrix.GetTranslation());
            anchor.z += CLAW_TARGET_HEIGHT;
            DAT_10f08d0c.Copy(anchor);
            DAT_10256210 = 0;
            DAT_1032d678 = 0;
            DAT_1020aeec[0] = 0;
            DAT_1020aeec[1] = 0;
            DAT_1020aeec[2] = 0;
            DAT_1020aeec[3] = 0;
        }
    }
    if (DAT_104c0144 > 0)
    {
        if (DAT_104c0144 < CLAW_TARGET_FREE)
        {
            if (DAT_104c0144 == CLAW_TARGET_SETTLED || striking == 0 ||
                !(DAT_10db6ee8[DAT_104c0144].matrix.GetTranslation().Distance(
                      DAT_10db6ee8[0].matrix.GetTranslation()) <= CLAW_LOST_DISTANCE))
            {
                DAT_104c0144 = CLAW_TARGET_NONE;
                DAT_10256210 = 0;
                DAT_1032d678 = 0;
                DAT_1020aeec[0] = CLAW_STATE_BURST;
                DAT_1020aeec[1] = CLAW_STATE_BURST;
                DAT_1020aeec[2] = CLAW_STATE_BURST;
                DAT_1020aeec[3] = CLAW_STATE_BURST;
            }
        }
        if (DAT_104c0144 > 0 && DAT_104c0144 < CLAW_TARGET_FREE)
        {
            if (!(DAT_10daaae8[DAT_104c0144].stats[STAT_HEALTH] > 0 && DAT_10daaae8[DAT_104c0144].m_0141 == 0))
            {
                DAT_104c0144 = CLAW_TARGET_FREE;
                DAT_10256210 = CLAW_FREE_TICKS;
                DAT_1032d678 = 0;
                DAT_1020aeec[0] = 0;
                DAT_1020aeec[1] = 0;
                DAT_1020aeec[2] = 0;
                DAT_1020aeec[3] = 0;
            }
        }
    }
    if (striking != 0 && DAT_104c0144 > 0 &&
        (DAT_104c0144 == CLAW_TARGET_FREE || DAT_10daaae8[DAT_104c0144].stats[STAT_HEALTH] > 0))
    {
        Node *tume = DAT_10db6ee8[0].root->Find(CLAW_NODE_NAME, 0);
        if (tume != 0)
            tume->SetFlags(CLAW_HIDDEN_FLAG);
        float orbit = CLAW_ORBIT_RADIUS;
        Vec4 target;
        Vec4 clawPos;
        Vec4 step;
        Vec4 other;
        if (DAT_104c0144 < CLAW_TARGET_FREE)
        {
            target.Copy(DAT_10db6ee8[DAT_104c0144].matrix.GetTranslation());
            target.z += CLAW_TARGET_HEIGHT;
        }
        else
            target.Copy(DAT_10f08d0c);
        if (DAT_104bfe1c == 0)
        {
            if (DAT_104c0144 < CLAW_TARGET_FREE && DAT_10256210 > CLAW_LAUNCH_MIN_TICKS &&
                FUN_1013f880() % CLAW_LAUNCH_ODDS == 0)
            {
                DAT_10256210 = 1;
                DAT_1020aeec[0] = CLAW_STATE_WAIT_1;
                DAT_1020aeec[1] = CLAW_STATE_WAIT_2;
                DAT_1020aeec[2] = CLAW_STATE_WAIT_3;
                DAT_1020aeec[3] = CLAW_STATE_WAIT_4;
            }
            if (DAT_10256210 < CLAW_TICKS_MAX)
                DAT_10256210++;
        }
        if (DAT_1032d678 < CLAW_TICKS_MAX)
            DAT_1032d678++;
        for (int i = 0; i < CLAW_COUNT; i++)
        {
            DAT_102559f4[i]->ClearFlags(CLAW_HIDDEN_FLAG);
            clawPos.Copy(DAT_102559f4[i]->matrix.GetTranslation());
            if (DAT_104c0144 == CLAW_TARGET_FREE || DAT_104c0144 == CLAW_TARGET_SETTLED)
            {
                int nearest = 0;
                int nearestDistance = CLAW_SEEK_RANGE;
                for (int slot = CLAW_SEEK_FIRST_SLOT; slot < CLAW_SEEK_END_SLOT; slot++)
                {
                    if (DAT_104bfe30 == 0 &&
                        (DAT_1021f3c0[slot] == CLAW_KIND_ENEMY_A || DAT_1021f3c0[slot] == CLAW_KIND_ENEMY_B) &&
                        DAT_10daaae8[slot].stats[STAT_BUSY] == 0 &&
                        (DAT_102239d0[slot] != 0 || DAT_10daaae8[slot].stats[STAT_HEALTH] > 0) &&
                        DAT_10daaae8[slot].m_0141 == 0)
                    {
                        float distance = DAT_1020e338[slot]->GetTranslation().Distance(clawPos);
                        if (distance < (float)nearestDistance)
                        {
                            nearestDistance = (int)distance;
                            nearest = slot;
                        }
                    }
                }
                if (nearest > 0)
                {
                    Vec4 enemyPos = DAT_1020e338[nearest]->GetTranslation();
                    if (DAT_104bfe30 == 0 && DAT_104c00ac == 0 &&
                        (DAT_1021f3c0[nearest] == CLAW_KIND_ENEMY_A || DAT_1021f3c0[nearest] == CLAW_KIND_ENEMY_B) &&
                        (DAT_102239d0[nearest] != 0 || DAT_10daaae8[nearest].stats[STAT_HEALTH] > 0))
                    {
                        if ((clawPos - enemyPos).Length2D() <=
                            (float)(DAT_102192c0[nearest] + CLAW_SEEK_REACH) +
                                DAT_10db6ee8[DAT_104bf8b4].colliderReach)
                        {
                            DAT_104c0144 = nearest;
                            DAT_10256210 = 0;
                            DAT_1032d678 = 0;
                            DAT_1020aeec[0] = 0;
                            DAT_1020aeec[1] = 0;
                            DAT_1020aeec[2] = 0;
                            DAT_1020aeec[3] = 0;
                            target.Copy(DAT_10db6ee8[nearest].matrix.GetTranslation());
                            target.z += CLAW_TARGET_HEIGHT;
                            DAT_10f08d0c.Copy(target);
                        }
                    }
                }
            }
            other.Copy(DAT_102559f4[i]->matrix.GetTranslation());
            other.Copy((target - DAT_10f08d0c) + other);
            DAT_102559f4[i]->matrix.SetTranslation(other);
            clawPos.Copy(DAT_102559f4[i]->matrix.GetTranslation());
            if (DAT_1020aeec[i] == CLAW_STATE_ORBIT && target.Distance(clawPos) > orbit + CLAW_ORBIT_MARGIN)
            {
                float speed = (float)(target.Distance(clawPos) * CLAW_APPROACH_SCALE);
                speed = MAX(CLAW_SPEED_MIN, MIN(speed, CLAW_SPEED_MAX_FAR));
                other.Set(CLAW_QUADRANT_OFFSET_X, 0, CLAW_QUADRANT_OFFSET_Z, 0);
                other.RotateZ((float)i * CLAW_QUADRANT_TURN * 0.5);
                step.Copy((target + other) - clawPos);
                step.Normalized3();
                step.Copy(step * speed);
                clawPos.Copy(clawPos + step);
            }
            else if (DAT_1020aeec[i] == CLAW_STATE_ORBIT && target.Distance(clawPos) < orbit - CLAW_ORBIT_MARGIN)
            {
                step.Copy(clawPos - target);
                step.Normalized3();
                step.Copy(step * orbit);
                step.RotateZ(CLAW_SWEEP_TURN);
                clawPos.Copy(target + step);
            }
            else if (DAT_104c0144 == CLAW_TARGET_FREE && DAT_10256210 > CLAW_FREE_TICKS)
                DAT_104c0144 = CLAW_TARGET_SETTLED;
            else
            {
                int state = DAT_1020aeec[i];
                if (state == CLAW_STATE_BURST)
                {
                    step.Copy(clawPos - target);
                    step.Normalized3();
                    step.Copy(step * CLAW_BURST_STEP);
                    clawPos.Copy(clawPos + step);
                    if (++DAT_1020d64c[i] > CLAW_BURST_FRAMES)
                    {
                        DAT_1020d64c[i] = 0;
                        DAT_1020aeec[i] = CLAW_STATE_ORBIT;
                    }
                    if (DAT_1020d64c[i] == 1)
                        StartClawEffect(CLAW_EFFECT_BURST, &DAT_102559f4[i]->matrix);
                }
                else if (state == CLAW_STATE_WAIT_4)
                {
                    if (DAT_10256210 > CLAW_WAIT_TICKS_4)
                    {
                        DAT_1020aeec[i] = CLAW_STATE_BURST;
                        DAT_1020d64c[i] = 0;
                    }
                }
                else if (state == CLAW_STATE_WAIT_3)
                {
                    if (DAT_10256210 > CLAW_WAIT_TICKS_3)
                    {
                        DAT_1020aeec[i] = CLAW_STATE_BURST;
                        DAT_1020d64c[i] = 0;
                    }
                }
                else if (state == CLAW_STATE_WAIT_2)
                {
                    if (DAT_10256210 > CLAW_WAIT_TICKS_2)
                    {
                        DAT_1020aeec[i] = CLAW_STATE_BURST;
                        DAT_1020d64c[i] = 0;
                    }
                }
                else if (state == CLAW_STATE_WAIT_1)
                {
                    if (DAT_10256210 > CLAW_WAIT_TICKS_1)
                    {
                        DAT_1020aeec[i] = CLAW_STATE_BURST;
                        DAT_1020d64c[i] = 0;
                    }
                }
                else if (state == CLAW_STATE_HOLD)
                {
                    if (++DAT_1020d64c[i] > 30)
                        DAT_1020aeec[i] = CLAW_STATE_SWEEP;
                    step.Copy(clawPos - target);
                    step.Normalized3();
                    step.Copy(step * orbit);
                    step.RotateZ(CLAW_SWEEP_TURN);
                    clawPos.Copy(target + step);
                    step.RotateX(CLAW_SWEEP_TURN);
                    other.Copy(target + step);
                    if (other.z > clawPos.z)
                        clawPos.Copy(other);
                    else
                    {
                        step.RotateX(-CLAW_SWEEP_TURN);
                        other.Copy(target + step);
                        if (other.z > clawPos.z)
                            clawPos.Copy(other);
                        else
                        {
                            step.RotateY(-CLAW_SWEEP_TURN);
                            other.Copy(target + step);
                            if (other.z > clawPos.z)
                                clawPos.Copy(other);
                            else
                            {
                                step.RotateY(CLAW_SWEEP_TURN);
                                other.Copy(target + step);
                                if (other.z > clawPos.z)
                                    clawPos.Copy(other);
                            }
                        }
                    }
                }
                else if (state == CLAW_STATE_RISE)
                {
                    if (++DAT_1020d64c[i] > 30)
                        DAT_1020aeec[i] = CLAW_STATE_SWEEP;
                    step.Copy(clawPos - target);
                    step.Normalized3();
                    step.Copy(step * orbit);
                    step.RotateZ(CLAW_SWEEP_TURN);
                    clawPos.Copy(target + step);
                    if (clawPos.z < target.z + CLAW_CLIMB_HEIGHT)
                    {
                        step.RotateX(CLAW_SWEEP_TURN);
                        other.Copy(target + step);
                        if (other.z > clawPos.z)
                            clawPos.Copy(other);
                        else
                        {
                            step.RotateX(-CLAW_SWEEP_TURN);
                            other.Copy(target + step);
                            if (other.z > clawPos.z)
                                clawPos.Copy(other);
                            else
                            {
                                step.RotateY(-CLAW_SWEEP_TURN);
                                other.Copy(target + step);
                                if (other.z > clawPos.z)
                                    clawPos.Copy(other);
                                else
                                {
                                    step.RotateY(CLAW_SWEEP_TURN);
                                    other.Copy(target + step);
                                    if (other.z > clawPos.z)
                                        clawPos.Copy(other);
                                }
                            }
                        }
                        DAT_1020aeec[i] = CLAW_STATE_HOLD;
                        DAT_1020d64c[i] = 0;
                    }
                    else
                    {
                        step.RotateX(CLAW_SWEEP_TURN);
                        clawPos.Copy(target + step);
                    }
                }
                else
                {
                    DAT_1020aeec[i] = CLAW_STATE_SWEEP;
                    if (FUN_1013f880() % CLAW_SWEEP_ODDS == 0)
                    {
                        DAT_1020d64c[i] = 0;
                        DAT_1020aeec[i] = CLAW_STATE_RISE;
                    }
                    step.Copy(clawPos - target);
                    step.Normalized3();
                    step.Copy(step * orbit);
                    step.RotateZ(CLAW_SWEEP_TURN);
                    clawPos.Copy(target + step);
                    if (clawPos.z < target.z + CLAW_CLIMB_HEIGHT)
                    {
                        step.RotateX(CLAW_SWEEP_TURN);
                        other.Copy(target + step);
                        if (other.z > clawPos.z)
                            clawPos.Copy(other);
                        else
                        {
                            step.RotateX(-CLAW_SWEEP_TURN);
                            other.Copy(target + step);
                            if (other.z > clawPos.z)
                                clawPos.Copy(other);
                            else
                            {
                                step.RotateY(-CLAW_SWEEP_TURN);
                                other.Copy(target + step);
                                if (other.z > clawPos.z)
                                    clawPos.Copy(other);
                                else
                                {
                                    step.RotateY(CLAW_SWEEP_TURN);
                                    other.Copy(target + step);
                                    if (other.z > clawPos.z)
                                        clawPos.Copy(other);
                                }
                            }
                        }
                        DAT_1020aeec[i] = CLAW_STATE_HOLD;
                        DAT_1020d64c[i] = 0;
                    }
                }
            }
            step.Copy(target - clawPos);
            step.Normalized3();
            if (DAT_1032d678 > CLAW_FACING_TICKS)
                DAT_102559f4[i]->matrix = DirectionMatrix(step);
            DAT_102559f4[i]->matrix.SetTranslation(clawPos);
        }
        DAT_10f08d0c.Copy(target);
    }
    else
    {
        Node *tume = DAT_10db6ee8[0].root->Find(CLAW_NODE_NAME, 0);
        if (tume != 0)
        {
            Vec4 spareA;
            Vec4 clawPos;
            Vec4 step;
            Vec4 spareB;
            Matrix4 dock;
            if (DAT_10256210 < CLAW_TICKS_MAX)
                DAT_10256210++;
            int docked = 0;
            for (int i = 0; i < CLAW_COUNT; i++)
            {
                dock = tume->worldMatrix;
                if (i == 0)
                    dock = TranslationMatrix(0, CLAW_DOCK_OFFSET_Y, CLAW_DOCK_OFFSET_Z) * dock;
                else if (i == 1)
                    dock = (TranslationMatrix(0, CLAW_DOCK_OFFSET_Y, CLAW_DOCK_OFFSET_Z) *
                            RotationZ(CLAW_DOCK_TURN_QUARTER)) * dock;
                else if (i == 2)
                    dock = (TranslationMatrix(0, CLAW_DOCK_OFFSET_Y, CLAW_DOCK_OFFSET_Z) *
                            RotationZ(-CLAW_DOCK_TURN_QUARTER)) * dock;
                else if (i == 3)
                    dock = (TranslationMatrix(0, CLAW_DOCK_OFFSET_Y, CLAW_DOCK_OFFSET_Z) *
                            RotationZ(CLAW_DOCK_TURN_HALF)) * dock;
                clawPos.Copy(DAT_102559f4[i]->matrix.GetTranslation());
                if (DAT_1020aeec[i] != 0)
                {
                    if (dock.GetTranslation().Distance(clawPos) < CLAW_DOCK_DISTANCE)
                    {
                        DAT_1020aeec[i] = 0;
                        FUN_10065b10(0, CLAW_DOCK_SOUND, 0, 0);
                    }
                    else
                    {
                        DAT_102559f4[i]->ClearFlags(CLAW_HIDDEN_FLAG);
                        float speed = dock.GetTranslation().Distance(clawPos) *
                                      ((double)DAT_10256210 * CLAW_RETURN_RATE + CLAW_RETURN_BASE);
                        speed = MIN(MAX(CLAW_SPEED_MIN, speed), (float)DAT_10256210);
                        step.Copy(dock.GetTranslation() - clawPos);
                        step.Normalized3();
                        step.Copy(step * speed);
                        clawPos.Copy(clawPos + step);
                        step.Copy(dock.GetTranslation() - clawPos);
                        step.Normalized3();
                        if (dock.GetTranslation().Distance(clawPos) < CLAW_FACING_DISTANCE)
                            DAT_102559f4[i]->matrix = dock;
                        else
                            DAT_102559f4[i]->matrix = DirectionMatrix(step);
                        DAT_102559f4[i]->matrix.SetTranslation(clawPos);
                        float limit = MAX(CLAW_DOCK_DISTANCE, speed);
                        if (dock.GetTranslation().Distance(clawPos) < limit)
                        {
                            DAT_1020aeec[i] = 0;
                            FUN_10065b10(0, CLAW_DOCK_SOUND, 0, 0);
                        }
                    }
                }
                if (DAT_1020aeec[i] == 0)
                {
                    docked++;
                    DAT_102559f4[i]->matrix = dock;
                    if (gauge == CLAW_GAUGE_STRIKE)
                        DAT_102559f4[i]->ClearFlags(CLAW_HIDDEN_FLAG);
                    else
                        DAT_102559f4[i]->SetFlags(CLAW_HIDDEN_FLAG);
                }
            }
            if (docked == CLAW_COUNT)
                tume->ClearFlags(CLAW_HIDDEN_FLAG);
        }
        else
            HideClaws();
    }
    return 0;
}
