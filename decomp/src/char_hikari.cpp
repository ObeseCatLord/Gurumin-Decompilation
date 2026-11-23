// The light that carries the player along a map route (behaviour 0x136 of the per-kind update 0x100f9670): the
// light follows the route points of its map record, and while it moves the player (character 0) rides above it and
// a character standing at its start is carried along. Obstacles stop it until the way is clear.

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "map_records.h"
#include "game.h"

#include "char_hikari.h"

// 0x1002cc70 (Char_HikariRide). One frame of the light `slot`. On its first call it finds its start (the first
// route point of kind HIKARI_START_KIND when its record's script flag is set, else point 0), puts itself there and
// looks up its first target. A map-specific trigger sets behaviour HIKARI_MODE_RESET. While it is waiting
// (wait set or behaviour 0) it stays still, or when it was blocked and the way is clear it resumes. Otherwise it
// steps along its route, its speed counter rising to HIKARI_TOP_SPEED (and falling towards route corners), the
// player rides above it while the behaviour is below HIKARI_MODE_RESUMED and the character found near its start
// is carried with it. When the route ends it stops. `halfSpeed` makes it twice as slow. Returns 0.
extern "C" int FUN_1002cc70(int slot, int halfSpeed)
{
    Vec4 target;
    Vec4 move;
    Vec4 ahead;
    int value;
    int action;
    float zero;
    int lift;
    int next;
    int group;
    target.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
    next = 0;
    lift = 0;
    DAT_10db6ee8[slot].mode = 0;
    DAT_10db6ee8[slot].m_0294 = 1;
    if (DAT_1020b430[slot].hikariReady != HIKARI_READY)
    {
        int record = DAT_10db6ee8[slot].bankIndex;
        DAT_1020b430[slot].hikariReady = HIKARI_READY;
        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find(HIKARI_NODE, 0);
        if (DAT_10210fc0[record].points[0].code == 0 || !DAT_10874848.TestScriptFlag(DAT_10210fc0[record].points[0].code))
        {
            DAT_1020b430[slot].hikariPoint = 0;
            DAT_1020b430[slot].targetPoint = 0;
        }
        else
        {
            int row = 0;
            while (row < HIKARI_ROUTE_ROWS && DAT_10210fc0[record].points[row].z != HIKARI_START_KIND)
                row++;
            if (row == HIKARI_ROUTE_ROWS)
            {
                DAT_1020b430[slot].hikariPoint = 0;
                DAT_1020b430[slot].targetPoint = 0;
            }
            else
            {
                DAT_1020b430[slot].hikariPoint = row;
                DAT_1020b430[slot].targetPoint = row;
            }
        }
        group = DAT_10210fc0[0].points[record * MAP_RECORD_ROWS + DAT_1020b430[slot].targetPoint].x / HIKARI_COORD_UNIT;
        if (group > HIKARI_COORD_LIMIT)
            group -= HIKARI_COORD_UNIT;
        DAT_10db6ee8[slot].matrix.m[12] = (float)group;
        group = DAT_10210fc0[0].points[record * MAP_RECORD_ROWS + DAT_1020b430[slot].targetPoint].x % HIKARI_COORD_UNIT;
        if (group > HIKARI_COORD_LIMIT)
            group -= HIKARI_COORD_UNIT;
        DAT_10db6ee8[slot].matrix.m[13] = (float)group;
        group = DAT_10210fc0[0].points[record * MAP_RECORD_ROWS + DAT_1020b430[slot].targetPoint].y / HIKARI_COORD_UNIT;
        if (group > HIKARI_COORD_LIMIT)
            group -= HIKARI_COORD_UNIT;
        DAT_10db6ee8[slot].matrix.m[14] = (float)group;
        target.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
        FUN_100286d0(slot, &DAT_1020b430[slot].hikariPoint, DAT_1020b430[slot].targetPoint, &next, &target, &group,
                     &value, &action);
        DAT_1020b430[slot].targetPoint = next;
        target.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
        zero = 0.0f;
        DAT_10802bf8[slot].step.Fill(zero);
    }
    if ((DAT_10c635ec != 0 && DAT_10c62ed0 == HIKARI_MAP_A) ||
        (DAT_10c65600 != 0 && DAT_10c62ed0 == HIKARI_MAP_B && DAT_10c635bc == 0))
    {
        DAT_1020b430[slot].behaviour = HIKARI_MODE_RESET;
        DAT_10c635ec = 0;
        DAT_10c635bc = 1;
    }
    if (DAT_1020b430[slot].wait == 0 && DAT_1020b430[slot].behaviour != 0)
    {
        if (DAT_1020b430[slot].hikariSound == 0)
        {
            DAT_1020b430[slot].hikariSound = 1;
            DAT_1021bdf8[slot][0]->RestartColourPlugs("anime", DAT_104bf8a4);
        }
        if (FUN_10029730(slot, &DAT_1020b430[slot].hikariPoint, DAT_1020b430[slot].targetPoint, &next, &target, &group,
                         &value, &action) == 1 &&
            DAT_1020b430[slot].behaviour != 0)
        {
            DAT_1020b430[slot].behaviour = HIKARI_MODE_ENDED;
            DAT_1020b430[slot].targetPoint = next;
            DAT_1020b430[slot].hikariScanned = 0;
            DAT_1020b430[slot].hikariHold = HIKARI_END_HOLD;
            return 0;
        }
        if (DAT_1020b430[slot].hikariScanned == 0)
        {
            int carried;
            DAT_1020b430[slot].hikariScanned = 1;
            for (carried = HIKARI_FIRST_CARRIED; carried < HIKARI_END_CARRIED; carried++)
            {
                if (slot != carried &&
                    DAT_10db6ee8[slot].matrix.m[12] + HIKARI_REACH_XY > DAT_10db6ee8[carried].matrix.m[12] - HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[12] - HIKARI_REACH_XY < DAT_10db6ee8[carried].matrix.m[12] + HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[13] + HIKARI_REACH_XY > DAT_10db6ee8[carried].matrix.m[13] - HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[13] - HIKARI_REACH_XY < DAT_10db6ee8[carried].matrix.m[13] + HIKARI_REACH_XY &&
                    DAT_10db6ee8[slot].matrix.m[14] + HIKARI_REACH_Z > DAT_10db6ee8[carried].matrix.m[14] - HIKARI_REACH_Z &&
                    DAT_10db6ee8[slot].matrix.m[14] - HIKARI_REACH_Z < DAT_10db6ee8[carried].matrix.m[14] + HIKARI_REACH_Z)
                {
                    DAT_1020b430[slot].hikariFollower = carried;
                    DAT_104bfdd8 = carried;
                    break;
                }
            }
            if (carried == HIKARI_END_CARRIED)
                DAT_1020b430[slot].hikariFollower = 0;
        }
        DAT_1020b430[slot].targetPoint = next;
        DAT_10db6ee8[slot].motionFrame = DAT_104bf8a4;
        move.Copy(target - DAT_10db6ee8[slot].matrix.GetTranslation());
        if (DAT_1020b430[slot].hikariSpeed < HIKARI_TOP_SPEED)
            DAT_1020b430[slot].hikariSpeed++;
        if (group == 2 || group == 4)
        {
            if (move.Length() < (float)DAT_1020b430[slot].hikariSpeed * HIKARI_SLOW_SCALE)
                DAT_1020b430[slot].hikariSpeed =
                    (int)(HIKARI_SLOW_BASE - (HIKARI_SLOW_RANGE - move.Length()) * HIKARI_SLOW_FACTOR);
        }
        if (move.x != 0.0f || move.y != 0.0f || move.z != 0.0f)
            move.Normalized3();
        if (halfSpeed != 0)
            move.Copy(move * ((float)DAT_1020b430[slot].hikariSpeed / HIKARI_HALF_DIVISOR));
        else
            move.Copy(move * ((float)DAT_1020b430[slot].hikariSpeed / HIKARI_FAST_DIVISOR));
        if (DAT_1020b430[slot].behaviour == HIKARI_MODE_SHAKY && FUN_1013f880() % HIKARI_SHAKE_PERIOD == 0)
        {
            move.x = (float)(FUN_1013f880() % HIKARI_SHAKE_RANGE - HIKARI_SHAKE_CENTRE) * HIKARI_SHAKE_SCALE + move.x;
            move.y = (float)(FUN_1013f880() % HIKARI_SHAKE_RANGE - HIKARI_SHAKE_CENTRE) * HIKARI_SHAKE_SCALE + move.y;
        }
        int riding;
        if (DAT_1020b430[slot].behaviour < HIKARI_MODE_RESUMED)
        {
            ahead.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            DAT_10db6ee8[0].SetPosition(DAT_10db6ee8[slot].matrix.m[12], DAT_10db6ee8[slot].matrix.m[13],
                                        move.z + DAT_10db6ee8[slot].matrix.m[14] + HIKARI_RIDER_HEIGHT);
            riding = 1;
            if (move.z > 0.0f)
                lift = 2;
        }
        else
            riding = 0;
        if (DAT_1020b430[slot].hikariFollower != 0)
        {
            DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].SetPosition(
                move.x + DAT_10db6ee8[slot].matrix.m[12], move.y + DAT_10db6ee8[slot].matrix.m[13],
                move.z + DAT_10db6ee8[slot].matrix.m[14] + HIKARI_FOLLOWER_HEIGHT);
            if (DAT_104bfdd0 == 0)
                DAT_104bfdd8 = DAT_1020b430[slot].hikariFollower;
        }
        if (DAT_1020b430[slot].behaviour == HIKARI_MODE_RESET)
        {
            DAT_10db6ee8[slot].matrix.m[12] = target.x;
            DAT_10db6ee8[slot].matrix.m[13] = target.y;
            DAT_10db6ee8[slot].matrix.m[14] = target.z;
        }
        else if (DAT_10db6ee8[slot].IsClearAny(&move) == 0 && lift == 0)
        {
            DAT_1020b430[slot].state = DAT_1020b430[slot].state != 1;
            DAT_1020b430[slot].hikariStopped = 1;
        }
        else
        {
            DAT_10db6ee8[slot].matrix.m[12] += move.x;
            DAT_10db6ee8[slot].matrix.m[13] += move.y;
            DAT_10db6ee8[slot].matrix.m[14] += move.z;
        }
        DAT_10db6ee8[slot].velocity.Fill(0.0f);
        if (riding)
            DAT_10db6ee8[0].SetPosition(ahead.x, ahead.y, ahead.z + move.z);
        DAT_10802bf8[slot].step.Copy(move);
        return 0;
    }
    if (DAT_1020b430[slot].hikariStopped != 0)
    {
        if (DAT_10db6ee8[slot].IsClearAny(&DAT_10802bf8[slot].step) != 0 || DAT_10802bf8[slot].step.z > 0.0f)
        {
            DAT_1020b430[slot].hikariStopped = 0;
            DAT_1020b430[slot].behaviour = HIKARI_MODE_RESUMED;
            return 0;
        }
    }
    else
    {
        DAT_1020b430[slot].hikariSpeed = HIKARI_IDLE_SPEED;
        if (DAT_1020b430[slot].hikariSound != 0)
        {
            DAT_1020b430[slot].hikariSound = 0;
            DAT_1021bdf8[slot][0]->RestartColourPlugs("stop", DAT_104bf8a4);
        }
    }
    return 0;
}
